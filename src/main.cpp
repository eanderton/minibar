/*
Copyright (c) 2013, Eric Anderton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
*/


#include <stdarg.h>

#include <string>
#include <exception>
#include <vector>
#include <memory>
#include <fstream>

#include "utils.h"
#include "minibar.h"

namespace minibar{

const char* STATUS_200 = "Status: 200 OK\r\nContent-type: application/json\r\n\r\n";
const char* STATUS_400 = "Status: 400 Bad Request\r\nContent-type: application/json\r\n\r\n";
const char* STATUS_401 = "Status: 401 Unauthorized\r\nContent-type: application/json\r\n";
const char* STATUS_405 = "Status: 405 Method Not Allowed\r\n";
const char* STATUS_500 = "Status: 500 Internal Server Error\r\nContent-type: application/json\r\n\r\n";

#define BUFFER_SIZE 1024

std::string FCGX_ReadString(FCGX_Stream* stream){
    std::unique_ptr<char> buf(new char[BUFFER_SIZE]);
    std::string result;
    size_t amount;
    
    do {
        amount = FCGX_GetStr(buf.get(),BUFFER_SIZE,stream);
        result.append(buf.get(),amount);
    } while(amount == BUFFER_SIZE);

    return result;
}

void FCGX_WriteString(const std::string str,FCGX_Stream* stream){
    FCGX_PutStr(str.data(),str.length(),stream);
}

void LogPrint(const char* format,...){
    va_list vptr;
    va_start(vptr,format);
    FCGX_VFPrintF(errStream,format,vptr);
    FCGX_FFlush(errStream);
}

void WriteJson(Json::Value value,FCGX_Stream* stream){
    Json::StyledWriter writer;
    std::string str = writer.write(value);
    
    FCGX_WriteString(str,stream);
    FCGX_FFlush(stream);
}

#define DebugPrint(...) LogPrint(__VA_ARGS__)


// loads config file from the filename specified by the web server
Json::Value LoadConfig(){
    char* filename = FCGX_GetParam("SCRIPT_FILENAME",envp);

    Json::Value root;
    Json::Reader reader;

    std::ifstream inf(filename);
    std::string data((std::istreambuf_iterator<char>(inf)), 
    std::istreambuf_iterator<char>());

    if(!reader.parse(data,root,false)){
        throw MinibarException(reader.getFormattedErrorMessages());
    }

    return root;
}

// adds the request content envelope (json) to the root doc object
Json::Value GetRequestContent(){
    Json::Reader reader;
    Json::Value request;

    std::string data = FCGX_ReadString(inStream);

    // set an empty object if there's no data
    if(data.length() == 0){
        data = "[]";
    }

    if(!reader.parse(data,request,false)){
        throw MinibarException(reader.getFormattedErrorMessages());
    }

    return request;
}

char hex_lookup[256] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
     0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

void ParseHex(const char ch,int* accumulator){
    int value = hex_lookup[ch];
    if(value == -1){
        throw MinibarException("Invalid character in query string hex expression");
    }
    *accumulator = (*accumulator<<4) | value;
}

enum{ KEY, VALUE };

Json::Value GetQueryString(){
    Json::Value result(Json::ValueType::objectValue);
    char* query = FCGX_GetParam("QUERY_STRING",envp);
    int state = KEY;
    char* p = query;
    std::string key,value;
    int i;

    // bail out if there's no query string
    if(p == NULL || *p == '\0') return result;

    key.reserve(20);
    value.reserve(20);

    while(*p != '\0'){
        switch(*p){
        case '=':
            state = VALUE;
            key = value;
            value.clear();
            break;
        case '&':
            DebugPrint("%s=%s",key.c_str(),value.c_str());
            result[key] = value;
            key.clear();
            value.clear();
            state = KEY;
            break;
        case '%':
            i = 0;
            ParseHex(*(++p),&i); //TODO: handle EOF here
            ParseHex(*(++p),&i);
            value += (char)i;
            break;
        case '+':
            value += ' ';
            break;
        default:
            value += *p;
        }
        p++;
    }
    if(state == VALUE){
        // finish off the k/v pair
        result[key] = value;
    }
    else{
        result[value] = Json::Value();
    }
    return result;
}

std::string getRestTarget(){
    std::string restTarget;
    restTarget += FCGX_GetParam("REQUEST_METHOD",envp);
    restTarget += FCGX_GetParam("PATH_INFO",envp);
    return restTarget;
}

void ProcessRequest(){
    SqlDb db;
    SqlStatement stmt;
    Config config;

    try{
        DebugPrint("Handling Request");

        // parse configuration file and establish root JSON object
        //TODO: cache this instead
        config.clear();
        config.loadConfig(FCGX_GetParam("SCRIPT_FILENAME",envp));       
 
        // compose the query path and get the query_node indicated by the path
        Json::Value pathValues; 
        Json::Value queryNode = config.getRestNode(getRestTarget(),pathValues);         

        // configure parameter evaluation context
        Json::Value paramContext;
        paramContext["conf"] = config.getRoot();
        paramContext["path"] = pathValues;
        paramContext["request"] = GetRequestContent();
        paramContext["query"] = GetQueryString();
        
/*
    TODO
    6. if the query node specifies authRoles:
        6a: discover and parse out the authToken
        6b: verify that the user has a valid role for this query
*/

        // 7. gather parameters as indicated on the query_node
        std::vector<Json::Value> arguments;
        Json::Value paramsNode = queryNode["params"];
        if(paramsNode.isArray()){
            for(int i; i<paramsNode.size(); i++){
                Json::Value value = paramsNode[i];
                //TODO: trap fault here to signal back to user/developer
                std::string queryPath = value.asString();
                value = QueryObject(paramContext,queryPath);
                arguments.push_back(value);
            } 
        }

        // 8. execute the sql query
        std::string dbFile = QueryObject(paramContext,TokenSet({"DB","filename"})).asString();
        db.Open(dbFile);

        std::string query = queryNode["query"].asString();
        stmt.Prepare(db,query);

        // TODO: test argument set and number of bindings
        LogPrint("Bind count: %d Arguments: %d",stmt.BindCount(),arguments.size());        

        int column=0;
        FCGX_PutS("ARGUMENTS:\n",outStream);
        for(Json::Value value: arguments){
            stmt.Bind(column,value);
            column++;
        }

        // run the query and gather data
        Json::Value rowData;
        rowData.resize(0);
        while(stmt.Step()==SQLITE_ROW){
            rowData.append(stmt.GetRow());
        }

        WriteJson(rowData,errStream);

        // cleanup 
        stmt.Finalize();
        db.Close();     

        // 9. send response
        FCGX_PutS(STATUS_200,outStream); 
        WriteJson(rowData,outStream);
        return;

    // exception management
    } 
    catch (const std::exception& ex){
        //@breakpoint
        LogPrint("Exception: %s\n",ex.what());
        LogPrint("DB Message: %s\n",db.ErrorMessage());
        FCGX_PutS(ex.what(),outStream);
        FCGX_FFlush(outStream);
    }
    catch (const std::string& ex) {
        //@breakpoint
        LogPrint("Exception: %s\n",ex.c_str()); 
        FCGX_PutS(ex.c_str(),outStream);
        FCGX_FFlush(outStream);
    }
    catch (...) {
        //@breakpoint
        LogPrint("Generic Exception\n"); 
        FCGX_PutS("Generic Exception\n",outStream);
        FCGX_FFlush(outStream);
    }
}

}

////////////////////////////////////
#include "unittest.h"

int main(){
#ifdef UNITTEST
    unittest();
#endif

    minibar::Init();
    while (minibar::Accept() >= 0) {
        minibar::ProcessRequest();
    }
    minibar::Cleanup(); 
    return 0;
}

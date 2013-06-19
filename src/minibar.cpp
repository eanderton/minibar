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
#include <map>
#include <vector>
#include <memory>
#include <fstream>

#include "utils.h"
#include "minibar.h"
#include "cgi.h"
#include "config.h"
#include "database.h"

#include "sql.h"

namespace minibar{

void writeJson(Json::Value value){
    Json::StyledWriter writer;
    writeString(writer.write(value));
}

void logJson(Json::Value value){
    Json::StyledWriter writer;
    logString(writer.write(value));
}

Json::Value getRequestJson(){
    Json::Reader reader;
    Json::Value request;

    std::string data = getRequestContent();

    // set an empty object if there's no data
    if(data.length() == 0){
        data = "[]";
    }

    if(!reader.parse(data,request,false)){
        throw MinibarException(reader.getFormattedErrorMessages());
    }

    return request;

}

Json::Value getQueryJson(){
    return parseQueryString(getQueryString());
}

class ConfigCache{
private:
     std::map<std::string,Config*> cache;
public:
    ~ConfigCache(){
        auto iter = cache.begin();
        while(iter != cache.end()){
            delete iter->second;
            iter++;
        }
    }
    
    Config* getConfig(std::string filename){
        Config* config;

        auto iter = cache.find(filename);
        if(iter == cache.end()){
            config = new Config();
            config->loadConfig(filename);
            cache[filename] = config;
        }
        else{
            config = iter->second;
        }
        return config;
    }
};

ConfigCache cache;

void processRequest(){

    try{
        debugPrint("Handling Request");

        // parse configuration file and establish root JSON object

        Config* config = cache.getConfig(getConfigFilename());
 
        // compose the query path and get the query_node indicated by the path
        Json::Value pathValues; 
        RestNode* restNode = config->getRestNode(getRestTarget(),pathValues);         
        
        //TODO: auth/auth here

        Json::Value resultJson;

        // process special actions
        if(!restNode->specialAction.empty()){
            if(restNode->specialAction.compare("api")==0){
                // dump the sanitized config to JSON
                resultJson = config->toJson(); 
            }
            else{
                writeString(STATUS_400);
                std::string msg;
                msg += "Unknown special action: ";
                msg += restNode->specialAction;
                writeString(msg.c_str());
            }
        }
        else{
            // configure parameter evaluation context
            Json::Value paramContext;
            paramContext["conf"] = config->getRoot();
            paramContext["path"] = pathValues;
            paramContext["request"] = getRequestJson();
            paramContext["query"] = getQueryJson();
            
            // prepare the sql query
            Connection* con = restNode->database->getConnection();
            con->prepare(restNode->query);
            
            // gather parameters as indicated on the query_node
            for(QueryParameter param: restNode->parameters){ 
                Json::Value value = QueryObject(paramContext,param.path);
                if(param.name.empty()){
                    con->bind(value);  // positional
                }
                else{
                    con->bind(param.name,value);
                }
            }

            resultJson = con->execute();
            
            // debug
            logJson(resultJson);

            // cleanup
            con->close(); 
        }

        // send response
        writeString(STATUS_200);
        writeJson(resultJson);
        return;

    // exception management
    } 
    catch (const std::exception& ex){
        logException(ex);
    }
    catch (const std::string& ex) {
        logException(ex);
    }
    catch (...) {
        logException("Generic Exception"); 
    }
}

}

#ifdef UNITTEST
#include "gtest/gtest.h"
#include <iostream>
#include <fstream>

// Mock frontend
namespace minibar{

std::string logPrintResult;
void logPrint(const char* format,...){
    char buff[1024];
    va_list vptr;

    va_start(vptr,format);
    vsnprintf(buff,1024,format,vptr);
    logPrintResult = buff;
}

std::string _logStringResult;
void logString(std::string str){
    _logStringResult += str;
}

std::string _writeStringResult;
void writeString(std::string str){
    _writeStringResult += str;
}

std::string _configFilename;
std::string getConfigFilename(){
    return _configFilename;

}

std::string _requestContent;
std::string getRequestContent(){
    return _requestContent;
}

std::string _queryString;
std::string getQueryString(){
    return _queryString;
}

std::string _restTarget;
std::string getRestTarget(){
    return _restTarget;
}

std::string _logException;
void logException(const std::exception& ex){
    _logException = ex.what();
}

void logException(const std::string& ex){
    _logException = ex;
}

void _resetFrontend(){
    _logStringResult = "";
    _writeStringResult = "";
    _logException = "";
}

}

using namespace minibar;

TEST(Minibar,Unittest){
    _configFilename = "dist/test.mini";
    
    _resetFrontend();
    _restTarget = "GET/users/guest";
    _requestContent = R"({"username":"user"})";

    processRequest();
    ASSERT_EQ(_logException,""); 
    std::string result = 
"Status: 200 OK\r\nContent-type: application/json\r\n\r\n"
R"([
   {
      "password" : "password",
      "role" : "guest",
      "username" : "guest"
   }
]
)";
    ASSERT_EQ(_writeStringResult,result);

    

}

#endif


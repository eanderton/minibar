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

void ProcessRequest(){
    SqlDb db;
    SqlStatement stmt;
    Config config;

    try{
        debugPrint("Handling Request");

        // parse configuration file and establish root JSON object
        //TODO: cache this instead
        config.clear();
        config.loadConfig(getConfigFilename());
 
        // compose the query path and get the query_node indicated by the path
        Json::Value pathValues; 
        RestNode* restNode = config.getRestNode(getRestTarget(),pathValues);         

        //TODO: auth/auth here

        // configure parameter evaluation context
        Json::Value paramContext;
        paramContext["conf"] = config.getRoot();
        paramContext["path"] = pathValues;
        paramContext["request"] = getRequestJson();
        paramContext["query"] = getQueryJson();
        

        // gather parameters as indicated on the query_node
        std::vector<Json::Value> arguments;
        for(QueryParameter param: restNode->parameters){ 
            Json::Value value = QueryObject(paramContext,param.path);
            //TODO: handle positional and named arguments here
            arguments.push_back(value);
        }

        // prepare the sql query
        std::string dbFile = QueryObject(paramContext,TokenSet({"DB","filename"})).asString();
        db.Open(dbFile);
        stmt.Prepare(db,restNode->query);

        // TODO: test argument set and number of bindings
        debugPrint("Bind count: %d Arguments: %d",stmt.BindCount(),arguments.size());        

        // bind arguments
        int column=0;
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

        // debug
        logJson(rowData);

        // cleanup 
        stmt.Finalize();
        db.Close(); 

        // send response
        writeString(STATUS_200);
        writeJson(rowData);
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

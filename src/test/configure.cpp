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
#include "configure.h"

#include <fstream>

using namespace std;

namespace minibar {

static Json::ValueType getJsonScalarType(string name){
    if(name.compare("int")==0) return Json::ValueType::intValue;
    if(name.compare("uint")==0) return Json::ValueType::uintValue;
    if(name.compare("real")==0) return Json::ValueType::realValue;
    if(name.compare("string")==0) return Json::ValueType::stringValue;
    if(name.compare("bool")==0) return Json::ValueType::booleanValue;
    throw MinibarException("Expected a scalar type expression");
}

QueryParameter::QueryParameter(){
    //do nothing
}
    
QueryParameter::QueryParameter(const Json::Value& param){
    if(param.isObject() && !param.isNull()){
        path = param["path"].asString();
        if(param.isMember("name")){
            name = param["name"].asString();
        }
        if(param.isMember("type")){
            type = getJsonScalarType(param["type"].asString());
        }
        else{
            type = Json::nullValue;
        }
        if(param.isMember("default")){
            defaultValue = param["default"].asString();
        }
        if(param.isMember("validation")){
            validation = param["validation"].asString();
        }
    }
    else if(param.isString()){
        path = param.asString();
        type = Json::ValueType::nullValue;
    }
    else{
        throw MinibarException("Parameter config element must be an object or string");
    }
}

///////////////////

RestNode::RestNode(){
    //do nothing
}

RestNode::RestNode(Config* config,const std::string& path,const Json::Value& root){
    this->path = path;

    if(!root.isObject() || root.isNull()){
        throw MinibarException("REST node must be an object");
    }

    if(root.isMember("special") && root["special"].isString()){
        this->specialAction = root["special"].asString();
    }
    else{ 
        std::string dbName = root.get("database","default").asString();
        database = config->getDatabase(dbName);
        databaseName = dbName;
 
        query = root["query"].asString();
        
        Json::Value params = root["params"];
        for(Json::Value value: params){
            parameters.push_back(QueryParameter(value));
        }
    }
}

Json::Value RestNode::toJson(){
    Json::Value result;
    if(!specialAction.empty()){
        result["special"] = specialAction;
    }
    else{
        Json::Value params;
        for(auto param: parameters){
            Json::Value paramJson;
            paramJson["name"]       = param.name;
            paramJson["path"]       = param.path;
            paramJson["default"]    = param.defaultValue;
            paramJson["type"]       = param.type;
            paramJson["validation"] = param.validation;
            params.append(paramJson);
        }
        result["params"] = params;
        result["database"] = databaseName; 
    }
    return result;
}

///////////////////

Config::Config(){
    clear();
}

Config::~Config(){
    clear();
}

void Config::clear(){
    router.clear();
    root = Json::Value::null;

    for(auto pair: databases){
        delete pair.second;
    }
    databases.clear();
    
    for(auto it: routes){
        delete it;
    }
    routes.clear();
}

Json::Value Config::getRoot(){
    return root;
}

void Config::loadConfig(string filename){
    // load JSON data
    Json::Reader reader;

    std::ifstream inf(filename);
    std::string data((std::istreambuf_iterator<char>(inf)),
    std::istreambuf_iterator<char>());

    if(!reader.parse(data,root,false)){
        throw MinibarException(reader.getFormattedErrorMessages());
    }

    if(!root.isObject()){
        throw MinibarException("Root config element must be an object");
    }

    // debug mode
    debugMode = root.get("debug",false).asBool();

    // compile databases
    Json::Value dbNode = root["DB"];
    if(!dbNode.isObject()){
        throw MinibarException("DB config must be an object");
    }
    for(string key: dbNode.getMemberNames()){
        Json::Value value = dbNode[key];
        if(!value.isObject()){
            throw MinibarException("DB config child node must be an object");
        }
        databases[key] = Database::FactoryCreate(value);
    }

    // compile router
    Json::Value restNode = root["REST"];
    if(!dbNode.isObject()){
        throw MinibarException("REST config must be an object");
    }
    for(string key: restNode.getMemberNames()){
        Json::Value value = restNode[key];
        TokenSet tokens = tokenize(key,"/");

        router.addRoute(tokens,routes.size());
        routes.push_back(new RestNode(this,key,value));
    }
}

Database* Config::getDatabase(string name){

    if(databases.find(name) == databases.end()){
        std::string msg;
        msg += "Database " + name + " does not exist.";
        throw MinibarException(msg);
    }
    return databases[name];
}


RestNode* Config::getRestNode(string path,Json::Value& pathValues){
    int idx = router.matchRoute(tokenize(path,"/"),pathValues);
    if(idx == RouteNode::NO_ROUTE_MATCH){
        throw MinibarException(std::string("Unknown route ") + path);
    }
    return routes[idx];
}

Json::Value Config::toJson(){
    Json::Value result;

    for(auto dbPair: databases){
        auto name = dbPair.first;
        result["DB"][name] = name;
    }

    for(auto routeNode: routes){
        result["REST"][routeNode->path] = routeNode->toJson();
    }

    return result;
}

}

#ifdef UNITTEST

#include "gtest/gtest.h"

using namespace minibar;

static Json::Value operator"" _json( const char* str, size_t sz){
    Json::Reader reader;
    Json::Value value;
    reader.parse(str,value,false);
    return value;
}


TEST(MinibarConfig,QueryParameter){
    QueryParameter test;
    
    ASSERT_THROW(QueryParameter("[]"_json),MinibarException);
    
    test = QueryParameter(R"("foobar")"_json);
    ASSERT_EQ(test.name,"");
    ASSERT_EQ(test.path,"foobar");
    ASSERT_EQ(test.validation,"");
    ASSERT_EQ(test.defaultValue,Json::nullValue);
    ASSERT_EQ(test.type,Json::nullValue);

    test = QueryParameter(R"({})"_json);
    ASSERT_EQ(test.name,"");
    ASSERT_EQ(test.path,"");
    ASSERT_EQ(test.validation,"");
    ASSERT_EQ(test.defaultValue,Json::nullValue);
    ASSERT_EQ(test.type,Json::nullValue);

    test = QueryParameter(R"({"name":"foo","path":"bar"})"_json);
    ASSERT_EQ(test.name,"foo");
    ASSERT_EQ(test.path,"bar");
    ASSERT_EQ(test.validation,"");
    ASSERT_EQ(test.defaultValue,Json::nullValue);
    ASSERT_EQ(test.type,Json::nullValue);

    test = QueryParameter(R"({"name":"foo","path":"bar","validation":"val"})"_json);
    ASSERT_EQ(test.name,"foo");
    ASSERT_EQ(test.path,"bar");
    ASSERT_EQ(test.validation,"val");
    ASSERT_EQ(test.defaultValue,Json::nullValue);
    ASSERT_EQ(test.type,Json::nullValue);

    test = QueryParameter(R"({"name":"foo","path":"bar","validation":"val","default":"gorf"})"_json);
    ASSERT_EQ(test.name,"foo");
    ASSERT_EQ(test.path,"bar");
    ASSERT_EQ(test.validation,"val");
    ASSERT_EQ(test.defaultValue,"gorf");
    ASSERT_EQ(test.type,Json::nullValue);

    test = QueryParameter(R"({"name":"foo","path":"bar","validation":"val","default":"gorf","type":"string"})"_json);
    ASSERT_EQ(test.name,"foo");
    ASSERT_EQ(test.path,"bar");
    ASSERT_EQ(test.validation,"val");
    ASSERT_EQ(test.defaultValue,"gorf");
    ASSERT_EQ(test.type,Json::ValueType::stringValue);

    test = QueryParameter(R"({"type":"string"})"_json);
    ASSERT_EQ(test.type,Json::ValueType::stringValue);
    
    test = QueryParameter(R"({"type":"int"})"_json);
    ASSERT_EQ(test.type,Json::ValueType::intValue);
    
    test = QueryParameter(R"({"type":"uint"})"_json);
    ASSERT_EQ(test.type,Json::ValueType::uintValue);
    
    test = QueryParameter(R"({"type":"real"})"_json);
    ASSERT_EQ(test.type,Json::ValueType::realValue);
    
    test = QueryParameter(R"({"type":"bool"})"_json);
    ASSERT_EQ(test.type,Json::ValueType::booleanValue);
    
    ASSERT_THROW(QueryParameter(R"({"type":""})"_json),MinibarException);
    ASSERT_THROW(QueryParameter(R"({"type":"foobar"})"_json),MinibarException);
    ASSERT_THROW(QueryParameter(R"({"type":null})"_json),MinibarException);
}

TEST(MinibarConfig,RestNode){
    RestNode test;
    Config config;

    ASSERT_THROW(RestNode(&config,"","[]"_json),MinibarException);
    ASSERT_THROW(RestNode(&config,"",R"("foobar")"_json),MinibarException);
    ASSERT_THROW(RestNode(&config,"",R"(null)"_json),MinibarException);
    
    ASSERT_THROW(RestNode(&config,"",R"({})"_json),MinibarException); 

    //TODO: more tests
}

TEST(MinibarConfig,Config){
    //do nothing
}


#endif


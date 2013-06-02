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
#include "config.h"

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

    
QueryParameter::QueryParameter(Json::Value& param){
    if(param.isObject()){
        path = param["path"].asString();
        if(param.isMember("name")){
            name = param["name"].asString();
        }
        if(param.isMember("default")){
            defaultValue = param["default"].asString();
        }
        if(param.isMember("type")){
            type = getJsonScalarType(param["type"].asString());
        }
        if(param.isMember("validation")){
            validation = param["validation"].asString();
        }
    }
    else if(param.isString()){
        path = param.asString();
        type = Json::ValueType::stringValue;
    }
    else{
        throw MinibarException("Parameter config element must be an object or string");
    }
}

RestNode::RestNode(Config* config,Json::Value& root){
    //TODO: can we support a custom/special action here? "special":"schema" ?

    std::string dbName = root.get("database","default").asString();
    database = config->getDatabase(dbName);

    query = root["query"].asString();
    
    Json::Value params = root["params"];
    for(Json::Value value: params){
        parameters.push_back(QueryParameter(value));
    }
}

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
        routes.push_back(RestNode(this,value));
    }
}

Database* Config::getDatabase(string name){
    return databases[name];
}


RestNode* Config::getRestNode(string path,Json::Value& pathValues){
    int idx = router.matchRoute(tokenize(path,"/"),pathValues);
    return &(routes[idx]);
}



}

#ifdef UNITTEST

void minibar::Config::unittest(){
    //do nothing
}

#endif


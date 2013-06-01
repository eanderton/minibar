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



struct QueryParameter{
    std::string name;
    std::string path;
    Json::Value defaultValue;
    Json::ValueType type;
    std::string validation; // regex
    
    QueryParameter(Json::Value root){
        if(root.isObject()){
            //TODO: force types
            name = root.get("name",null);
            path = root["path"]; // required
            defaultValue = root.get("default",null);
            type = root.get("type",null);
            validation = root.get("validation",null);
        }
        else if(root.isString())){
            path = root;
    //TODO: set defaults
        }
        else{
            throw MinibarException("Parameter config element must be an object or string");
        }
    }
};

//TODO: move up and out
struct RestNode{
    std::vector<QueryParameter> parameters;
    std::string query;

    RestNode(Json::Value root){
        //TODO: can we support a custom/special action here? "special":"schema" ?
        query = root["query"];
        Json::Value params = root["params"];
        for(std::string value: params){
            parameters.push_back(QueryParameter(value));
        }    
    }
};

//TODO: gonna need a bigger boat
class Database{
    static Database FactoryCreate(Json::Value root){
        return new Database();
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
}

Json::Value Config::getRoot(){
    return root;
}

void Config::loadConfig(char* filename){
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
        throw minibarException("DB config must be an object");
    }
    for(string key: dbNode.getMemberNames()){
        Json::Value value = dbNode[key];
        databases[key] = Database::FactoryCreate(value);
    } 

    // compile router
    Json::Value restNode = root["REST"];
    if(!dbNode.isObject()){
        throw minibarException("REST config must be an object");
    }
    for(string key: restNode.getMemberNames()){
        Json::Value value = restNode[key];
        TokenSet tokens = tokenize(key,"/");

        router.addRoute(tokens,routes.size());
        routes.push_back(RestNode(value));
    }
}

// flip around to eval with json query eval context
Json::Value Config::getRestNode(string path,Json::Value& pathValues){
    int idx = router.matchRoute(tokenize(path,"/"),pathValues);
    return routes[idx];
}



}

#ifdef UNITTEST

void minibar::Config::unittest(){
    //do nothing
}

#endif


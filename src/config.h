#pragma once
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

#include "router.h"
#include "database.h"
#include "jsoncpp.h"
#include <map>
#include <functional>

using namespace std;

namespace minibar{

struct QueryParameter{
    string name;
    string path;
    Json::Value defaultValue;
    Json::ValueType type;
    string validation;

    QueryParameter(Json::Value& param);
};

class Config;

struct RestNode{
    Database* database;
    vector<QueryParameter> parameters;
    string query;

    RestNode(Config* config,Json::Value& root);
};



class Config{
    bool debugMode;
    Json::Value root;
    RouteNode router;
    vector<RestNode> routes;
    map<std::string,Database*> databases;
    
public:
    Config();
    ~Config();

    void clear();
    Json::Value getRoot();
    void loadConfig(string filename);

    Database* getDatabase(string name);
    RestNode* getRestNode(string path,Json::Value& pathValues);

#ifdef UNITTEST
static void unittest();
#endif
};

}

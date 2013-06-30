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

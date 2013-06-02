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

#include "utils.h"

namespace minibar{

TokenSet tokenize(const string& str,const string& delimiters,bool trimEmpty)
{
    TokenSet tokens;
    size_t pos, lastPos = 0;

    while(true){
        pos = str.find_first_of(delimiters, lastPos);
        if(pos == std::string::npos){
            pos = str.length();

            if(pos != lastPos || !trimEmpty){
                tokens.push_back(string(str.data()+lastPos,pos-lastPos));
            }
            break;
        }
        else{
            if(pos != lastPos || !trimEmpty){
                tokens.push_back(string(str.data()+lastPos,pos-lastPos));
            }
        }
        lastPos = pos + 1;
    }
    return tokens;
}

QueryException::QueryException(const char* text,TokenSet query,int errterm){
    this->text = text;
    this->query = query;
    this->errterm = errterm;
}

const char* QueryException::what() const throw(){
    std::string result;

    result += text;
    int i = 0;
    for(std::string term: query){
        if(i == errterm){
            result += ".>>";
            result += term;
            result += "<<";
        }
        else{
            result += ".";
            result += term;
        }
        i++;
    }
    return result.c_str();
}

// walks a set of query set tokens through a JSON object graph
// returns the value indicated by the query
Json::Value QueryObject(const Json::Value root,const TokenSet& query){
    int i = 0;
    Json::Value node = root;

    for(std::string term : query){
        if(!node.isObject()){
            throw QueryException("Query node must be an object: ",query,i);
        }
        // query this object to find our path element
        Json::Value next = node[term];
        if(next.isNull()){
            throw QueryException("Failed to find query element: ",query,i);
        }
        node = next;
        i++;
    }
    return node;
}

// overload of QueryObject that accepts a '.' delimited query expression
Json::Value QueryObject(const Json::Value root,const std::string& query){
    return QueryObject(root,tokenize(query,"."));
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

}

#ifdef UNITTEST
#include <assert.h>

bool tokensEqual(minibar::TokenSet& a,minibar::TokenSet& b){
    if(a.size() != b.size()) return false;

    auto ai = a.begin();
    auto bi = b.begin();
    while(ai != a.end()){
        if( ai->compare(*bi) != 0) return false;
        ai++;
        bi++;
    }

    return true;
}

void minibar::utilsUnittest(){
    TokenSet test = tokenize("foo/bar/baz","/");
    TokenSet test2 = {"foo","bar","baz"};

    assert(tokensEqual(test,test2));
}

#endif

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
#include "cgi.h"
#include "utils.h"
#include "minibar.h"

namespace minibar{

const char* STATUS_200 = "Status: 200 OK\r\nContent-type: application/json\r\n\r\n";
const char* STATUS_400 = "Status: 400 Bad Request\r\nContent-type: application/json\r\n\r\n";
const char* STATUS_401 = "Status: 401 Unauthorized\r\nContent-type: application/json\r\n";
const char* STATUS_405 = "Status: 405 Method Not Allowed\r\n";
const char* STATUS_500 = "Status: 500 Internal Server Error\r\nContent-type: application/json\r\n\r\n";

enum{ KEY, VALUE };

Json::Value parseQueryString(std::string query){
    Json::Value result(Json::ValueType::objectValue);
    int state = KEY;
    char* p = (char*)query.c_str();
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

}

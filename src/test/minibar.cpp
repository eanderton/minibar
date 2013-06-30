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
#include "minibar.h"
#include "cgi.h"
#include "configure.h"
#include "database.h"
#include "gtest/gtest.h"
#include <stdarg.h>
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
    _configFilename = "resources/test.mini";
    
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

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

#include "fcgi_config.h"
#include "fcgiapp.h"

#include <stdarg.h>

#include <string>
#include <exception>
#include <vector>
#include <memory>
#include <fstream>

#include "utils.h"
#include "minibar.h"

namespace minibar{

// global vars for working with FastCGI
FCGX_Stream *inStream, *outStream, *errStream;
FCGX_ParamArray envp;

#define BUFFER_SIZE 1024

std::string FCGX_ReadString(FCGX_Stream* stream){
    std::unique_ptr<char> buf(new char[BUFFER_SIZE]);
    std::string result;
    size_t amount;
    
    do {
        amount = FCGX_GetStr(buf.get(),BUFFER_SIZE,stream);
        result.append(buf.get(),amount);
    } while(amount == BUFFER_SIZE);

    return result;
}

void FCGX_WriteString(const std::string str,FCGX_Stream* stream){
    FCGX_PutStr(str.data(),str.length(),stream);
}

void logPrint(const char* format,...){
    va_list vptr;
    va_start(vptr,format);
    FCGX_VFPrintF(errStream,format,vptr);
    FCGX_FFlush(errStream);
}

void logString(string str){
    FCGX_WriteString(str,errStream);
}

void writeString(string str){
    FCGX_WriteString(str,outStream);
}

std::string getConfigFilename(){
    return FCGX_GetParam("SCRIPT_FILENAME",envp);
}

std::string getRequestContent(){
    return FCGX_ReadString(inStream);
}

std::string getQueryString(){
    return FCGX_GetParam("QUERY_STRING",envp);
}

std::string getRestTarget(){
    std::string restTarget;
    restTarget += FCGX_GetParam("REQUEST_METHOD",envp);
    restTarget += FCGX_GetParam("PATH_INFO",envp);
    return restTarget;
}

void logException(const std::exception& ex){
    //@breakpoint
    logPrint("Exception: %s\n",ex.what());
    FCGX_PutS(ex.what(),outStream);
    FCGX_FFlush(outStream);
}

void logException(const std::string& ex){
    //@breakpoint
    logPrint("Exception: %s\n",ex.c_str()); 
    FCGX_PutS(ex.c_str(),outStream);
    FCGX_FFlush(outStream);
}

}

int main(){
    while(FCGX_Accept(&minibar::inStream,&minibar::outStream,&minibar::errStream,&minibar::envp) >= 0){
        minibar::processRequest();
    }
    return 0;
}

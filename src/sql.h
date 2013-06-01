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

#include <stdarg.h>

#include <string>
#include <exception>
#include <vector>

#include "sqlite3.h"
#include "json/json.h"

// exception class for sqlite3
struct SqlException: public std::exception{
    const char* msg;
    
    SqlException(int errcode);
    SqlException(const char* text);
    const char * what () const throw ();
};


struct SqlDb{
    sqlite3* handle;
    
    SqlDb();
    ~SqlDb();
    const char* ErrorMessage();
    void Open(const std::string filename);
    void Close();
    sqlite3* operator&();
};

struct SqlStatement{
    sqlite3_stmt* handle;

    SqlStatement();
    ~SqlStatement();
    void Prepare(SqlDb db,std::string sql);
    int BindCount();
    int ColumnCount();
    void Bind(int col,Json::Value& value);
    int Step();
    Json::Value GetRow();
    void Reset();
    void Finalize();
};



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

#include "sql.h"


namespace minibar{
extern void LogPrint(const char* msg,...);
}

///////// 
SqlException::SqlException(int errcode){
    msg = (const char*)sqlite3_errstr(errcode);
}

SqlException::SqlException(const char* text){
    msg = text;
}
    
const char * SqlException::what () const throw (){
    return msg;
}


// exception-throwing wrapper for sqlite3 calls
static void sqlite3_fn(int result){
    if(result != SQLITE_OK) throw SqlException(result);
}



///////// 
SqlStatement::SqlStatement(){
    handle = NULL;
}

SqlStatement::~SqlStatement(){
    if(handle != NULL) Finalize();
}

void SqlStatement::Prepare(SqlDb db,std::string sql){
    //@breakpoint
    int result;
    result = sqlite3_prepare_v2(db.handle,sql.data(),sql.length(),&handle,NULL);
    sqlite3_fn(result);
}

int SqlStatement::BindCount(){
    return sqlite3_bind_parameter_count(handle);
}

int SqlStatement::ColumnCount(){
    return sqlite3_column_count(handle);
}

void SqlStatement::Bind(int col,Json::Value& value){
    std::string str;
    int idx = col+1;

    switch(value.type()){
    case Json::nullValue: 
        sqlite3_fn(sqlite3_bind_null(handle,idx));
        break;
    case Json::intValue:     
        sqlite3_fn(sqlite3_bind_int64(handle,idx,value.asInt()));
        break;
    case Json::uintValue:     
        sqlite3_fn(sqlite3_bind_int64(handle,idx,value.asUInt()));
        break;
    case Json::realValue:     
        sqlite3_fn(sqlite3_bind_double(handle,idx,value.asDouble()));
        break;
    case Json::stringValue:
        str = value.asString();
        sqlite3_fn(sqlite3_bind_text(handle,idx,str.c_str(),-1,SQLITE_TRANSIENT));
        break;
    case Json::booleanValue:
        sqlite3_fn(sqlite3_bind_int64(handle,idx,value.asInt()));
        break;
    case Json::arrayValue:  
        throw SqlException("Cannot bind column to JSON Array");    
    case Json::objectValue:
        throw SqlException("Cannot bind column to JSON Object");    
    }
}

int SqlStatement::Step(){
    minibar::LogPrint("handle: %p",handle);
    int result = sqlite3_step(handle);
    switch(result){
        case SQLITE_DONE:
        case SQLITE_BUSY:
        case SQLITE_ROW:
            return result;
        default:
            throw SqlException(result);
    }
}

Json::Value SqlStatement::GetRow(){
    Json::Value row;
    for(int i=0; i<ColumnCount(); i++){
        const char* name = sqlite3_column_name(handle,i);

        switch(sqlite3_column_type(handle,i)){
        case SQLITE_INTEGER:
            row[name] = (Json::Int64)sqlite3_column_int64(handle,i);
            break;
        case SQLITE_FLOAT:
            row[name] = sqlite3_column_double(handle,i);
            break;
        case SQLITE_TEXT:
            row[name] = (const char*)sqlite3_column_text(handle,i);
            break;
        case SQLITE_BLOB:
            throw SqlException("BLOB column data is not supported");
        case SQLITE_NULL:
            row[name] = Json::Value();
            break;
        default:
            row[name] = (const char*)sqlite3_column_text(handle,i);
            break;
        } 
    }
    return row;
}

void SqlStatement::Reset(){
    sqlite3_fn(sqlite3_reset(handle));
}

void SqlStatement::Finalize(){
    if(handle != NULL){
        sqlite3_finalize(handle);
        handle = NULL;
    }
}


///////////
SqlDb::SqlDb(){
    handle = NULL;
}
SqlDb::~SqlDb(){
    if(handle != NULL) Close();
}

const char* SqlDb::ErrorMessage(){
    if(handle != NULL){
        return sqlite3_errmsg(handle);
    }
}

void SqlDb::Open(const std::string filename){
    sqlite3_fn(sqlite3_open(filename.c_str(),&handle));
}


void SqlDb::Close(){
    if(handle != NULL){
        sqlite3_close_v2(handle);
        handle = NULL;
    }
}


sqlite3* SqlDb::operator&(){
    return handle;
}

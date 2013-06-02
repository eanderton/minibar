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

REGISTER_DB(sqlite,SqliteDb::Create);

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

SqliteDbConnection::SqliteDbConnection(std::string dbFile){
    handle = NULL;
    stmt = NULL;
    bindIndex = 0;
    sqlite3_fn(sqlite3_open(dbFile.c_str(),&handle));
}

SqliteDbConnection::~SqliteDbConnection(){
    close();
}

void SqlStatement::prepare(std::string query){
    //@breakpoint
    int result;
    result = sqlite3_prepare_v2(db,query.data(),query.length(),&stmt,NULL);
    sqlite3_fn(result);
}

void SqliteDbConnection::bind(int idx,Json::Value value){
    std::string str;
    switch(value.type()){
    case Json::nullValue: 
        sqlite3_fn(sqlite3_bind_null(stmt,idx));
        break;
    case Json::intValue:     
        sqlite3_fn(sqlite3_bind_int64(stmt,idx,value.asInt()));
        break;
    case Json::uintValue:     
        sqlite3_fn(sqlite3_bind_int64(stmt,idx,value.asUInt()));
        break;
    case Json::realValue:     
        sqlite3_fn(sqlite3_bind_double(stmt,idx,value.asDouble()));
        break;
    case Json::stringValue:
        str = value.asString();
        sqlite3_fn(sqlite3_bind_text(stmt,idx,str.c_str(),-1,SQLITE_TRANSIENT));
        break;
    case Json::booleanValue:
        sqlite3_fn(sqlite3_bind_int64(stmt,idx,value.asInt()));
        break;
    case Json::arrayValue:  
        throw SqlException("Cannot bind column to JSON Array");    
    case Json::objectValue:
        throw SqlException("Cannot bind column to JSON Object");    
    }
}

void SqliteDbConnection::bind(Json::Value value){
    bind(bindIndex+1,value);
    bindIdex++;
}

void SqliteDbConnection::bind(std::string name,Json::Value value){
    int idx = sqlite3_bind_parameter_index(stmt,name.c_str());
    if(idx != 0){
        bind(idx,value);
    }
}

int SqlStatement::queryStep(){
    int result = sqlite3_step(stmt);
    switch(result){
        case SQLITE_DONE:
        case SQLITE_BUSY:
        case SQLITE_ROW:
            return result;
        default:
            throw SqlException(result);
    }
}

Json::Value SqlStatement::queryGetRow(){
    Json::Value row;
    for(int i=0; i<sqlite3_column_count(stmt); i++){
        const char* name = sqlite3_column_name(stmt,i);

        switch(sqlite3_column_type(stmt,i)){
        case SQLITE_INTEGER:
            row[name] = (Json::Int64)sqlite3_column_int64(stmt,i);
            break;
        case SQLITE_FLOAT:
            row[name] = sqlite3_column_double(stmt,i);
            break;
        case SQLITE_TEXT:
            row[name] = (const char*)sqlite3_column_text(stmt,i);
            break;
        case SQLITE_BLOB:
            throw SqlException("BLOB column data is not supported");
        case SQLITE_NULL:
            row[name] = Json::Value();
            break;
        default:
            row[name] = (const char*)sqlite3_column_text(stmt,i);
            break;
        } 
    }
    return row;
}

Json::Value SqliteDbConnection::execute(){
    Json::Value rowData;
    rowData.resize(0);
    while(queryStep()==SQLITE_ROW){
        rowData.append(queryGetRow());
    }
    return rowData;
}

void SqliteDbConnection::close(){
    if(handle != NULL){
        sqlite3_finalize(stmt);
        handle = NULL;
    }
    if(handle != NULL){
        sqlite3_close_v2(handle);
        handle = NULL;
    } 
}

///////////
SqliteDb::SqliteDb(std::string dbFile){
    this->dbFile = dbFile;
}

SqliteDb::~SqliteDb(){
    //do nothing
}

Connection* SqliteDb::getConnection(){
    return new SqliteDbConnection(dbFile);
}

Database* SqliteDb::Create(Json::Value root){
    std::string dbFile = root["filename"];
    dbFile = QueryObject(paramContext,dbFile).asString();
    return new SqliteDb(dbFile);
}

}

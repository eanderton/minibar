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

// used to get the crypt(3) function
#define _XOPEN_SOURCE

#include <stdexcept>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "htpasswd.h"

namespace minibar{

REGISTER_DB(htpasswd,htpasswd::HtPasswdDb::Create);

}

namespace htpasswd{


///////// 

struct RAIILock{
    pthread_mutex_t* mutex;

    RAIILock(pthread_mutex_t* mutex){
        this->mutex = mutex;
        lock();
    }
    ~RAIILock(){
        unlock();
    }
    void lock(){
        pthread_mutex_lock(mutex);
    }
    void unlock(){
        pthread_mutex_unlock(mutex);
    }
};


///////// 

HtPasswdDbConnection::HtPasswdDbConnection(HtPasswdDb* db){
    bindIndex = 0;
    this->db = db;
}

HtPasswdDbConnection::~HtPasswdDbConnection(){
    // do nothing
}

void HtPasswdDbConnection::prepare(string query){
    if(query.compare("update") == 0){
        this->query = HTPASSWD_UPDATE;
    }
    else if(query.compare("insert") == 0){
        this->query = HTPASSWD_INSERT;
    }
    else if(query.compare("delete") == 0){
        this->query = HTPASSWD_DELETE; 
    }
    else{
        stringstream msg;
        msg << "Invalid htpasswd query: " << query;
        throw runtime_error(msg.str());
    }
}

void HtPasswdDbConnection::bind(int idx,Json::Value value){
    if(!value.isString()){
        throw runtime_error("Argument must be of type string.");
    }
    
    switch(idx){
    case HTPASSWD_USERNAME_IDX:
        username = value.asString();
        break;
    case HTPASSWD_PASSWORD_IDX:
        password = value.asString();
        break;
    default:
        stringstream msg;
        msg << "Invalid Parameter Index: " << idx;
        throw runtime_error(msg.str());
    }
}

void HtPasswdDbConnection::bind(Json::Value value){
    bind(bindIndex+1,value);
    bindIndex++;
}

void HtPasswdDbConnection::bind(string name,Json::Value value){
    if(name.compare("username")){
        bind(HTPASSWD_USERNAME_IDX,value);
    }
    else if(name.compare("password")){
        bind(HTPASSWD_PASSWORD_IDX,value);
    }
    else{
        stringstream msg;
        msg << "Invalid parameter name: " << name;
        throw runtime_error(msg.str());
    }
}

Json::Value HtPasswdDbConnection::execute(){
    Json::Value rowData;

    switch(query){
    case HTPASSWD_UPDATE:
        db->updateUser(username,password);
        break;
    case HTPASSWD_INSERT:
        db->insertUser(username,password);
        break;
    case HTPASSWD_DELETE:
        db->deleteUser(username);
        break;
    }

    rowData.resize(0);
    return rowData;
}

void HtPasswdDbConnection::close(){
    // do nothing
}

///////////

HtPasswdDb::HtPasswdDb(string dbFile,HashFn hashFn){
    // create recursive mutex to guard the cache
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_settype(&mutexAttr,PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex,&mutexAttr);

    // set vars
    this->dbFile = dbFile;
    this->hashFn = hashFn;
    this->modified = 0;

    // load data
    syncCache();
}

HtPasswdDb::~HtPasswdDb(){
    pthread_mutex_destroy(&mutex);
}

void HtPasswdDb::syncCache(){
    RAIILock lock(&mutex);
    
    // check the timestamp on the file
    struct stat fileAttr;
    stat(dbFile.c_str(),&fileAttr);
    time_t fileModified = fileAttr.st_mtime;

    // reload the cache if the file was touched
    if(fileModified > modified){
        ifstream infile(dbFile);
        users.clear();

        string line;
        while(getline(infile,line)){
            size_t idx = line.find(':');
            users[line.substr(0,idx)] = line.substr(idx+1);
        }    
        modified = fileModified;
        infile.close();
    }
}

void HtPasswdDb::flushCache(){
    RAIILock lock(&mutex);
    ofstream outfile(dbFile);
    for(auto &item: users){
        outfile << item.first << ':' << item.second << endl;
    }
    outfile.close();
}

#define CRYPT_SALT "ZZ"

string HtPasswdDb::hashCrypt(string &value){
    return string(crypt(value.c_str(),CRYPT_SALT));
}

string HtPasswdDb::hashMD5(string &value){
    //TODO:
    return value;
}

string HtPasswdDb::hashSH1(string &value){
    //TODO:
    return value;
}

minibar::Connection* HtPasswdDb::getConnection(){
    syncCache();
    return new HtPasswdDbConnection(this);
}

void HtPasswdDb::updateUser(string username,string password){
    RAIILock lock(&mutex);
    auto iter = users.find(username);
    if(iter == users.end()){
        stringstream msg;
        msg << "Invalid username: " << username;
        throw runtime_error(msg.str());
    }
    iter->second = hashFn(password);
    flushCache();
}

void HtPasswdDb::insertUser(string username,string password){
    RAIILock lock(&mutex);
    auto iter = users.find(username);
    if(iter != users.end()){
        stringstream msg;
        msg << "Invalid username: " << username;
        throw runtime_error(msg.str());
    }
    users[username] = hashFn(password);
    flushCache();
}

void HtPasswdDb::deleteUser(string username){
    RAIILock lock(&mutex);
    auto iter = users.find(username);
    if(iter == users.end()){
        stringstream msg;
        msg << "Invalid username: " << username;
        throw runtime_error(msg.str());
    }
    users.erase(iter);
    flushCache();
}

minibar::Database* HtPasswdDb::Create(Json::Value root){
    string dbFile = root["filename"].asString();
    
    HashFn hashFn;    
    string algo = root.get("algorithm","crypt").asString();
    if(algo.compare("crypt") == 0){
        hashFn = HtPasswdDb::hashCrypt;    
    }
    else if(algo.compare("md5") == 0){
        hashFn = HtPasswdDb::hashMD5;    
    }
    else if(algo.compare("sh1") == 0){
        hashFn = HtPasswdDb::hashSH1;    
    }
    else{
        stringstream msg;
        msg << "Invalid algorithm: " << algo;
        throw runtime_error(msg.str()); 
    }

    return new HtPasswdDb(dbFile,HtPasswdDb::hashCrypt);
}

}

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

#include <map>
#include "database.h"
#include <pthread.h>
#include <time.h>

using namespace std;

namespace htpasswd {

enum {
    HTPASSWD_UPDATE,
    HTPASSWD_INSERT,
    HTPASSWD_DELETE
};

enum {
    HTPASSWD_USERNAME_IDX=0,
    HTPASSWD_PASSWORD_IDX=1,
};

typedef map<string,string> UserMap;

class HtPasswdDb;

class HtPasswdDbConnection: public minibar::Connection{
    int bindIndex;
    int query;
    string username;
    string password;
    HtPasswdDb* db;

    void bind(int idx,Json::Value value);

public:
    HtPasswdDbConnection(HtPasswdDb* db);
    ~HtPasswdDbConnection();
    
    virtual void prepare(string query);
    virtual void bind(Json::Value value);
    virtual void bind(string name,Json::Value value);
    virtual Json::Value execute();
    virtual void close();
};

typedef string (*HashFn)(string &value);

class HtPasswdDb: public minibar::Database{
    string dbFile;
    time_t modified;
    pthread_mutex_t mutex;
    UserMap users;  
    HashFn hashFn;

    HtPasswdDb(string dbFile,HashFn hashFn);
    ~HtPasswdDb();

    void syncCache();
    void flushCache();

    static string hashCrypt(string &value);
    static string hashMD5(string &value);
    static string hashSH1(string &value);

public:
    virtual minibar::Connection* getConnection();

    void updateUser(string username,string password);
    void insertUser(string username,string password);
    void deleteUser(string username); 

    static Database* Create(Json::Value root);
};

}

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

#include <iostream>
#include <fstream>
#include <string>
#include "htpasswd.h"
#include "gtest/gtest.h"

using namespace minibar;
using namespace htpasswd;


class HtPasswdTest : public HtPasswdDb{
public:
    HtPasswdTest(string filename,HashFn fn): HtPasswdDb(filename,fn){
        //do nothing
    }
};

#define HTPASSWD_TEST_FILE "/tmp/minibar_test.htpasswd"

std::string readFile(std::string filename){
    ifstream file(filename);
    std::string data;
    while(file.good()){
        std::string line;
        getline(file,line);
        data += line;
    }
    file.close();
    return data;
}

TEST(HTPasswd,Unittest){
    ASSERT_NO_THROW(HtPasswdTest db(HTPASSWD_TEST_FILE,HtPasswdDb::hashCrypt));
}

TEST(HTPasswd,Insert){
    HtPasswdTest db(HTPASSWD_TEST_FILE,HtPasswdDb::hashCrypt);

    db.insertUser("username","changeme");

    ASSERT_TRUE(readFile(HTPASSWD_TEST_FILE).compare("username:ZZyOXxdk08WKY") == 0); 
}

TEST(HTPasswd,Update){
    HtPasswdTest db(HTPASSWD_TEST_FILE,HtPasswdDb::hashCrypt);

    db.updateUser("username","password");
    ASSERT_TRUE(readFile(HTPASSWD_TEST_FILE).compare("username:ZZKRwXSu3tt8s") == 0); 
}

TEST(HTPasswd,Delete){
    HtPasswdTest db(HTPASSWD_TEST_FILE,HtPasswdDb::hashCrypt);

    db.deleteUser("username");
    ASSERT_TRUE(readFile(HTPASSWD_TEST_FILE).compare("") == 0);
}

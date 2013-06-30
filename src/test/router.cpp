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

#include "router.h"
#include "gtest/gtest.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include <assert.h>

using namespace minibar;

void assert_throws(function<void()> x){
    bool result = false;
    try{
        x();
    }
    catch(...){
        result = true;
    }
    ASSERT_TRUE(result);
}

TEST(MinibarRouter,Unittest){
    RouteNode root;
    Json::Value pathValues;
    TokenSet test = tokenize("foo/bar/baz","/");

    // insert route
    root.addRoute(test,42);
    root.addRoute(tokenize("foo/*/gorf","/"),13);
    root.addRoute(tokenize("foo/bar/baz/:gorf","/"),29);

    // prevent duplicates
    ASSERT_THROW(root.addRoute(test,42),MinibarException);

    // matching
    pathValues.clear();
    ASSERT_TRUE(root.matchRoute(tokenize("x/y/z","/"),pathValues) == RouteNode::NO_ROUTE_MATCH);
    ASSERT_TRUE(pathValues.empty());

    pathValues.clear();
    ASSERT_TRUE(root.matchRoute(tokenize("foo/bar/baz","/"),pathValues) == 42);
    ASSERT_TRUE(pathValues.empty());

    pathValues.clear();
    ASSERT_TRUE(root.matchRoute(tokenize("foo/bar/baz/goat","/"),pathValues) == 29);
    ASSERT_TRUE(pathValues.isMember("gorf"));
    ASSERT_TRUE(pathValues["gorf"].asString().compare("goat") == 0);
}

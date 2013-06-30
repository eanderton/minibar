#include "router.h"
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

using namespace std;

namespace minibar{


static bool isWildcard(string tok){
    return tok.compare("*") == 0 || tok[0] == ':';
}

RouteNode::RouteNode(){
    matchId = NO_ROUTE_MATCH;
}

void RouteNode::clear(){
    for(RouteNode* child: children){
        delete child;
    }
    children.clear();
}

void RouteNode::addRoute(TokenSetIter begin,TokenSetIter end,int matchId){
    
    // get next token and advance   
    string tok = *begin;
    TokenSetIter next = begin;
    next++;

    // find next node
    for(RouteNode* child: children){
        if(child->name == tok){
            // match - are we out of tokens?
            if(next == end){
                throw RouteException("route already exists");
            } 
            child->addRoute(next,end,matchId);
            return;
        }
    }

    // no match, so add a new node
    RouteNode* node = new RouteNode();
    node->name = tok;
    node->matchId = matchId;
    children.push_back(node);

    // recurse
    if(next != end){
        node->addRoute(next,end,matchId);  
    }
}

int RouteNode::matchRoute(TokenSetIter begin,TokenSetIter end,Json::Value& pathValues){
    // are we out of tokens?
    if(begin == end){
        return matchId;
    }

    // get next token and advance   
    string tok = *begin;
    TokenSetIter next = begin;
    next++;

    // find next node
    for(RouteNode* child: children){
        bool isNamed = false;
        if(child->name[0] == ':'){
            isNamed = true;
        }
        else if(child->name.compare("*") != 0 && child->name != tok){
            // no exact or wildcard match - keep looking
            continue;
        }

        // go to the next node
        int result = child->matchRoute(next,end,pathValues);
        if(result != NO_ROUTE_MATCH){
            if(isNamed){
                //add named token to the value set
                pathValues[child->name.substr(1)] = tok;
            }
            return result;
        }
    }

    // no match 
    return NO_ROUTE_MATCH;
}


void RouteNode::addRoute(TokenSet tokens,int matchId){
    addRoute(tokens.begin(),tokens.end(),matchId);
}

int RouteNode::matchRoute(TokenSet tokens,Json::Value& pathValues){
    return matchRoute(tokens.begin(),tokens.end(),pathValues);
}

}

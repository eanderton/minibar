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

#include <openssl/evp.h>
#include "string.h"
#include "sqlite3.h"
#include "sqlext.h"


static void sqlite_ext_md5(sqlite3_context *context,int argc,sqlite3_value **argv){
        unsigned char md_value[EVP_MAX_MD_SIZE];
        int md_len,i;
        if ( argc != 1 ){
                sqlite3_result_null(context);
                return ;
        }
        EVP_MD_CTX ctx;
        EVP_MD_CTX_init(&ctx);
        if(EVP_DigestInit_ex(&ctx,EVP_md5(),NULL)){
                char * input = 0 ;
                char * output = 0 ;
                input = ( char * ) sqlite3_value_text(argv[0]);
                EVP_DigestUpdate(&ctx, input, strlen(input));
                EVP_DigestFinal_ex(&ctx,md_value,&md_len);
                output = ( char * ) malloc ( md_len * 2 + 1 );
                if ( !output ){
                        sqlite3_result_null(context);
                        EVP_MD_CTX_cleanup(&ctx);
                        return ;
                }
                for (i = 0; i < md_len; i++){
                        sprintf (&output[i*2],"%02x", (unsigned char)md_value[i]);
                }
                sqlite3_result_text(context,output,strlen(output),(void*)-1);
                free(output);
                output=0;
        }else{
                sqlite3_result_null(context);
        }
        EVP_MD_CTX_cleanup(&ctx);
}

int minibar_sqlite_ext_init(sqlite3 *dbcon){
  sqlite3_create_function(dbcon, "md5", 1, SQLITE_ANY, 0, sqlite_ext_md5, 0, 0);
  return 0;
}

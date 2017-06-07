#!/usr/bin/env cpx
#^ "../inc/cpx-unchanged.hpp"
#|
#(
//  Illustrates range loop over chars in string (opposing user of iterating const char* from string::c_str())
const char *name="mijstonen@hotmail.com";
string name_str(name);
INFO(VARVALHEX(sizeof(string)));

for(auto& c: name_str) 
{
#pragma GCC diagnostic push
    // given warning is harmless and specific to this loop, ignored to avoid polution of compilation resulty in the log
#pragma GCC diagnostic ignored "-Waddress"
    // Illustrates use of variable macros, notice hexadecimal representations and pointer(+hexadecimal) representations
    //   including the new VARVALS macro
    INFO(PTRVAL(&c),PTRCHRNUMHEX(&c),VARVAL(c),VARCHRNUMHEX(c),"VARVALS for multiple arguments",VARVALS(c,&c));
#pragma GCC diagnostic pop
}
  
#)


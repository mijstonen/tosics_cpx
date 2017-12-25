#!/usr/bin/env cpx

#|

#(
// test code generation calling and use, see hello_meta_world.cpp
cout
    << "string Greetings(char _char, int _count)\n"
    << "{\n"
    << "    string ret;\n"
    << "    for( auto i=0; i<_count; ++i ) {\n"
    << "        ret+=_char;\n"
    << "    }\n"
    << "    return ret;\n"
    << "}\n"
    ;
#)

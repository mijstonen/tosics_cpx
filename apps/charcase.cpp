#!/usr/bin/env cpx
#|
#(

// Nice illustration, but its use in fingerprint is obsolete


if ( ProgramArguments.size()!=2 ) {
    throw runtime_error("ERROR: commandline arguments, expected syntax:  charcase <char>...");
    exit(1);
}
for(char const *pc=ProgramArguments[1].c_str();*pc;++pc){ 
    cout<<"case '"<<*pc<<"':\n"; 
}
(void)"1";
#)

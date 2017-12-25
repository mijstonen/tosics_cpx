#!/usr/bin/env cpx

// produce string variable of repeated chars,used in hello_meta_world.cpp

#|

#(
//info_ProgramArguments();

if ( ProgramArguments.size()<3 ) {
    cerr<< "Usage: metagreetings.cpp <stringname> <char> <count>"<< endl;
    return EXIT_FAILURE;
}

string const& stringname= ProgramArguments[1];
char chr= ProgramArguments[2][0];
int  cnt= atoi(ProgramArguments[3].c_str());
//INFO(VARVAL(chr));

cout << "std::string "<< stringname<< "=\"";
for ( auto i=0; i<cnt; ++i ) {
    cout<< chr;
}
cout << "\";\n";
#)

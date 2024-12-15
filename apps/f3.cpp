#!/usr/bin/env cpx
#|

// TBD: transfere to util library

constexpr streamsize BUFF_SIZE = 32*1024;

char Buff[BUFF_SIZE];


void run(istream& _in) {

    SHA1 s;
    
    while (_in) {
        _in.read( Buff, BUFF_SIZE);
        s.processBytes(Buff,_in.gcount());
    }
    cout<< s.make_fingerprint() << endl;
}


// See tosics::util::SHA1 and TinySHA1.hpp
#(
    if ( ProgramArguments.size()>= 2 ) {// when there is a argument after the command, take input file path from the commandline
        ifstream inputFile(ProgramArguments[1]);
        if (! inputFile) {
            CERROR("Teminating","Failed to open inputFile from the commandline.", VARVAL(ProgramArguments));
            exit(EXIT_FAILURE);
        }
        else {
            run(inputFile);
        }
    }
    else {// use standard input
        run(cin);
    }
#)

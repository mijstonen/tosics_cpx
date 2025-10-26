#!/usr/local/bin/cpx
 // extract file extensions from find .... -type f where .... can be extra find options and directory to search
 // example: find ~ -type f | cpext.cpp
 // See also ~/bin/fndexp which was the prototype.
 // The algorithm is improved by minimizing the needed test conditions.

 static char LineBuffer[1024*1024];

    struct
 EndOfFileException
 {
 };

    char*
 getReverseLine() // throws EndOfFileException
 {
        char*
    p_current=LineBuffer-1
    ;
        auto
    getCinChar=[p_current]() -> char
    {
        if ( !cin ) {
            throw EndOfFileException();
        }
        //OTHERWISE its ok to read from cin
        return cin.get();
    }
    ;
    while ( ( *++p_current = getCinChar() )!='\n' );
    // expected lines returned by find to start with "./<rest of path>" or "/<rest of path>"
    ASSERT((LineBuffer[0]=='.'||LineBuffer[0]=='/') && '\n'==*p_current);
    return p_current-1; // position at lats character of the line as begin position to reserse scan the path (beginning with the extension, if any).
 }

#!
    set<string>
 extensions_set
 ;

 try {
     for(;;) {
            string
        extension
        ;
        for( char* p_rl=getReverseLine(); *p_rl!='/'; --p_rl) {
            if ('.'== *p_rl ) {
                if ( p_rl[-1]!='/' ) {
                    if ( !extension.empty() ) {
                        extensions_set.insert( extension);
                    }
                }
          //else hidden file, starts with '.' but no extension
                break;
            }
            //OTHERWISE prepend charecter
            extension.insert(0,1,*p_rl);
        }
     }
 }
 catch( EndOfFileException ) {
    for(const auto& extension: extensions_set) {
        cout<< '.'<<extension<< '\n';
    }
    cout.flush();
 }

#!/usr/bin/env cpx

#|

#(

if ( ProgramArguments.size()<2 ) {
    return EXIT_FAILURE;
}
ifstream in(ProgramArguments[1]);

char char_from_source=0, prior_cfs=0, next_cfs=0, last_to_target=0;
    struct 
EndOfSourceFileEvent
{
};

    auto 
get_char_from_source = [&]() -> char 
{
    // try to get next char
    prior_cfs= char_from_source;
    char_from_source=  in.get();
    if ( ! in ) {
        throwEvent(EndOfSourceFileEvent());
    }
    next_cfs=  in.peek();
    return char_from_source;
};
    auto 
isSeparator=[&](auto _c) -> bool
{
    if ( isspace(_c) ) {
        return true;
    }
        
    if ( ispunct(_c) ) {
        // true, except for _ $
        switch ( _c){
           case '_': case '$': return false;
        }
        return true;
    }
        
    return false;
};
//FAKE_USE(isSeparator);
    auto 
isIdentChar=[&](auto _c) -> bool
{
    if ( isalnum( _c) ) {
        return true;
    }
    
    switch ( _c){
      case '_': case '$': return true;
    }
    
    return false;
};
//FAKE_USE(is...
    auto 
isLineWs=[&](auto _c) ->bool
{
    switch (_c){
      case ' ': case '\t': return true;
    }
    return false;
};
FAKE_USE(isLineWs);
    auto
skip_single_line_comment=[&]()
{
    while ( get_char_from_source(), next_cfs!='\n' );
};
    auto 
skip_multi_line_comment=[&]()  // NOTE: Does not handle nested comments
{
    get_char_from_source(); // because /*/ is not complete, minimal /**/ is complete
    for(;;) {
        while ( get_char_from_source()!='/' );
        if ( prior_cfs=='*' ) {
            return;
        }
    }
};
    bool 
in_text=false;

try {
    for(;;){
        get_char_from_source();
        if ( char_from_source=='"' ) {
            // check for change of in_text status
            if ( in_text ) {
                if ( prior_cfs!='\\' ) {
                    // valid, not quoted by
                    in_text=false;
                }
                // else backquoted (aka \") shall not effect in_text
            }
            else {
                if ( !(prior_cfs=='\'' && next_cfs =='\'' ) ) {
                    // valid, not '"' in code
                    in_text=true;
                }
                // else '"' shall not effect in_text
            }
        }
        if ( !in_text ) {
            if ( char_from_source=='/' ){
                // could be start of // or /*
                auto keep_prior=prior_cfs;
                switch ( next_cfs ) {
                  case '/': //...
                    skip_single_line_comment();
                    char_from_source=keep_prior;
                    continue;
                  case '*': /*...*/
                    skip_multi_line_comment();
                    if ( isIdentChar(keep_prior) && isIdentChar(next_cfs)) {
                        // keep identifiers separated when removing multiline comment
                        char_from_source=' ';
                        cout<<' ';
                    }
                    else {
                        // keep separate identifiers separated
                        char_from_source=keep_prior;
                    }
                    continue;
                    
                  // default: / (not followed by / or *) is treated normally
                } //switch
            }
            if ( isspace( char_from_source) ) {
                // Reduce whitespace up to 0 when identifiers and separators are interleaving
                if ( !( isIdentChar(last_to_target)&& isIdentChar(next_cfs) ) ) {
                    // possibly more whitespaces then required, reduce them
                    // or same whitespace character is repeated, reduce them
                    if ( ( ( char_from_source!= '\n' )&&  ( isSeparator( prior_cfs)|| isSeparator( next_cfs) ) )|| 
                         ( ( prior_cfs== char_from_source )|| ( char_from_source== next_cfs ) )
                       ) {
                        // no need to print space between identifiers and keywords 
                        char_from_source= prior_cfs;
                        continue;
                    }
                }
                // else whitespace is surrounded by identifiers or keywords, keep appart
            }
            // no space character is treated normally
        }
        // else filtering is diabled inside program texts
        
        // Normal treatment.
        // Unless continu was called to filter spaces and comments
        // the source characters are copied here
        last_to_target= char_from_source;
        cout.put(last_to_target);
    }
} 
catch (EndOfSourceFileEvent){}

#)

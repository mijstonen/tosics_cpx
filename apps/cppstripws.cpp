/*OBSOLETE*/
#!/usr/bin/env cpx
#|

/*
Strips all comments and most of the whitespace without changing the meaning of source code with C++ syntax and
calculates a (SHA1) hash code. This can be used to detect changes to a source code that effects the functioning
of the code. However, most changes to whitespaces and comments do not effect the hash code. This is usefull when
sources are generated and may slightly vary in format or comments, but not in the essential source code.
A different hash causes recompilation of the source and that increases runtime. By stripping the source code
that effect is minimized.

build: CPX-build-app cppstripws
install: /home/michel/Projects/Kdevelop/build/cpx_apps/cppstripws
link: /usr/local/bin/cppstriws

Used by CPX-preproces_hash_compile.sh  (which is executed by cpx)
*/


#define SOURCE_NOT_HASH 0

class StripContext
{
    struct EndOfSourceFileEvent{};

    istream *pIn= &cin;
    char char_from_source=0, prior_cfs=0, next_cfs=0, last_to_target=0;
    bool in_text=false;
    SHA1 pure_src_sha;

  public:

        void
    selectInput(istream* _pIn)
    {
        pIn=_pIn;
    }

    ~StripContext()
    {
    }
        char
    get_char_from_source()
    {
        // try to get next char
        prior_cfs= char_from_source;
        char_from_source=  pIn->get();
        if ( ! ( *pIn ) ) {
            ThrowEvent( EndOfSourceFileEvent());
        }
        next_cfs=  pIn->peek();
        return char_from_source;
    };
        bool
    isSeparator(auto _c)
    {
        if ( isspace(_c) ) {
            return true;
        }

        switch ( _c){
            case '_': case '$': return false;
        }

        return ispunct(_c);
    };
        bool
    isIdentChar(auto _c)
    {
        if ( isalnum( _c) ) {
            return true;
        }
        switch ( _c){
        case '_': case '$': return true;
        }
        return false;
    };
        bool
    isLineWs(auto _c)
    {
        switch (_c){
        case ' ': case '\t': return true;
        }
        return false;
    };
        void
    skip_single_line_comment()
    {
        while ( get_char_from_source(), next_cfs!='\n' );
    };
        void
    skip_multi_line_comment()  // NOTE: Does not handle nested comments
    {
        get_char_from_source(); // because /*/ is not complete, minimal /**/ is complete
        for(;;) {
            while ( get_char_from_source()!='/' );
            if ( prior_cfs=='*' ) {
                return;
            }
        }
    };
        void
    output(char _c)
    {
        last_to_target= _c;
#if SOURCE_NOT_HASH
        cout.put(last_to_target);
#else
        pure_src_sha.processByte(last_to_target);
#endif
    };

        void
    parse()
    {
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
                                output(' ');
                            }
                            else {
                                char_from_source=keep_prior;
                            }
                            continue;
                        // default: / (not followed by / or *) is treated normally
                        } //switch
                    } //
                    if ( isspace( char_from_source) ) {
                        // Reduce whitespace up to 0 when identifiers and separators are interleaving
                        if ( !( isIdentChar(last_to_target) && isIdentChar(next_cfs) ) ) {
                            // possibly more whitespaces then required, reduce them
                            // or same whitespace character is repeated, reduce them
                            auto isSeparatorNoSingleQuote=[this](char _cfs)->bool
                            {
                                return (_cfs!='\'')&& isSeparator(_cfs);
                            };
                            if ((( char_from_source!= '\n' )&&
                                 ( isSeparatorNoSingleQuote( prior_cfs)|| isSeparatorNoSingleQuote( next_cfs) )
                                )||
                                (( prior_cfs==/*same whitespace as*/char_from_source )||
                                 (  next_cfs==/*same whitespace as*/char_from_source )
                                ))
                            {   // no need to print space between identifiers and keywords
                                char_from_source= prior_cfs;
                                continue;
                            }
                        }
                        // else whitespace is surrounded by identifiers or keywords, keep appart
                    }
                    // no space character is treated normally
                }
                // Normal treatment.
                // Unless continue was called to filter spaces and comments
                // the source characters are copied here
                output(char_from_source);
            } //for
        }
        catch (EndOfSourceFileEvent){
        }

#if SOURCE_NOT_HASH
        cout.flush();
#else
        sha1::SHA1::digest32_t digest;
        pure_src_sha.getDigest(digest);
        cout<<pure_src_sha.make_digest_string();
#endif
    }
} //class StripContext
;

#(


ifstream fromfile;
StripContext sc;

if ( ProgramArguments.size()<2 ) { // || ProgramArguments[1]=="--" ) {
    // assume standard input if argument is -- or there is no argument
    sc.selectInput( &cin);
}
else {
    fromfile.open(ProgramArguments[1]);
    // first argument gives filename
    if ( !fromfile ) {
        ThrowBreak("Failed to open source file",eBC_handled);
    }
    else {
        sc.selectInput( &fromfile);
    }
}

sc.parse();
#)

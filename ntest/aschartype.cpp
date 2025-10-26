#!/usr/bin/env cpx
#|

// put your globals, templates,class definitions and includes here

#!

#if 1
for (int c=0; c<255; ++c) {
    auto as_char= static_cast<unsigned char>(c);

    INFO(
        VARVALS(c,as_char,isalnum(c),iscntrl(c),isgraph(c),isprint(c),ispunct(c),isspace(c),isblank(c),isascii(c))
    );
}
#endif

char const* puncts="~`!;@$%^&*()+-={}[]|:\\\"'<>?/.#,";
  
#if 0
for (int c=0; c<255; ++c) {
    if ( !ispunct(c) ) continue;
    auto as_char= static_cast<unsigned char>(c);
    cout<< as_char;
}
#endif

for (int c=0; c<255; ++c) {
    if ( !ispunct(c) ) continue;
  
    auto as_char= static_cast<unsigned char>(c);
    
    bool found=false;
    for (auto pc=puncts;*pc;++pc){
        found= ( *pc==as_char );
        if ( found ) break;
    }

    if ( !found ) cout<< as_char;
}

INFO();



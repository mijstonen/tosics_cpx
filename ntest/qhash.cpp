#!/usr/local/bin/cpx
/*
__ description ________________________________________________________________________________________________________

 Proof of concept to turn short strings at compiletime into a numeric representation that can be converted
 back to the original short string and do this without translation map for the characters, this makes the encoding and decoding as fast as it can become. I discovered that the with a code range from 0..83 and a
 base of 84 the most  efficient encoding and decoding is possible.
*/
__research_____________________________________________________________________________________________________________
    void
 codesResearch()
 {
    size_t cnt_punct=0;
    size_t cnt_chars=1;
    for(uint8_t i=32;i<127;++i) {
        if (ispunct(i)) ++cnt_punct;
        switch (i) {
            // filter character range
            default: continue;      // skip body after the switch
            case '+' ... '}':break; // go on with body after the switch
        }
        auto c=static_cast<char>(i);
        INFO(VARVALS(cnt_chars,i,c));
        ++cnt_chars;
    }
    INFO(VARVALS(cnt_punct,cnt_chars));
 }
__helpers______________________________________________________________________________________________________________
    template<typename T>
 concept Hash_uintX_typename = std::same_as<T, uint128_t>
                            || std::same_as<T,  uint64_t>
                            || std::same_as<T,  uint32_t>
                             ;
    template<typename T>
    constexpr size_t
MaxLen=0
;
    template<>
    constexpr size_t
MaxLen<uint128_t> =20
;
    template<>
    constexpr size_t
MaxLen<uint64_t> =10
;
    template<>
    constexpr size_t
MaxLen<uint32_t> =5
;
    template<Hash_uintX_typename HASH_T>
    inline constexpr size_t
 isQHachable ( ///////////////////////// Returns 0 (false) if not isQHachable, if isQHachable it returns
       const char*                    // sizeof HASH_T. If not isQHashable, the hash can still be used,
    cstr                              // but without decodability guarantees. isQHachable is not running
 )  noexcept                          // inside encode or decode functions to not hinder performance.
 {
    if ( cstr ) {
        return 0ul;
    }
    if ( strlen(cstr)> MaxLen<HASH_T> ) {
        return 0ul;
    }
    while ( *cstr ) {
        switch ( *cstr ) {
            // g++ case range extension
            case '+' ... '}':
                // characters * ! " # $ % & ' ( ) ~  cannot be used
                break;
            default:
                return 0ul;
        }
        ++cstr;
    }
    return MaxLen<HASH_T>;
 }
__encode_______________________________________________________________________________________________________________
    template<Hash_uintX_typename HASH_T>
    inline constexpr
    HASH_T
 qHashEncode ( ////////////////////////// Converts a short string into a unsigned number
        const char*                    // charactes after MaxLen are ignored. The possible length depends on HASH_T,
    _cstr                              // any character above that length is ignored and only those within that length
 )  noexcept                           // are decoded.
 {
        auto
    convert=[]( char _c)
    {
        return static_cast<HASH_T>( static_cast<uint8_t>( _c) - 42ul);
    };
        size_t
    index= min(strlen(_cstr),MaxLen<HASH_T>)
    ;
    if ( !index ) {
        return 0ul;
    }
        HASH_T
    hash= convert(_cstr[ --index])
    ;
    while (index) {
        ( hash*= 84 )+= convert(_cstr[ --index]);
    }
    return hash;
 }
__decode_______________________________________________________________________________________________________________
    template<Hash_uintX_typename HASH_T>
    inline constexpr state_t
 qHashDecode ( ////////////////////////// Converts a number to a short string
       char*
    cstr_
 ,     HASH_T
    hash
 ) noexcept
 {
    //auto out=cstr_;
    if ( !cstr_ ) {
         return state_t{-1};
    }
    if ( !hash ) {
        return state_t{0};  // hash of an empty string
    }
    while ( hash ) {
            HASH_T
        first= hash% 84;
        ;
            auto
        decoded = static_cast<HASH_T>(42)+ first
        ;
        *cstr_ = static_cast<char>( decoded);

        * ++cstr_ = 0;
        //INFO(VARVALS(hex128str(first),hex128str(decoded),*cstr_,out));

        hash/= 84;
     }
     *cstr_='\0';
     return state_t{0};
 }
__tests________________________________________________________________________________________________________________
    void
 test( //////////////////////////////// Encodes and decodes only valid strings and compares them on equality.
        const char*
    _cstr
 ) noexcept
 {
        using
     Hash_uintX_t = uint128_t
     ;
     if ( isQHachable<Hash_uintX_t>( _cstr) ) {
            auto
        hash= qHashEncode<Hash_uintX_t>( _cstr)
        ;
            char
        decoded[32]
        ;
        memset(decoded,0,sizeof decoded);
        if ( STATEREPORT(qHashDecode(decoded , hash)) ) {
            INFO( "decoding",VARVALS(hash), "failed");
        }
        else {
            if ( strcmp(_cstr,decoded) ) {
                INFO(VARVALS(_cstr,hex128str(hash),decoded));
                ( strcmp(_cstr,decoded) )? INFO(HRED "FAILED"): INFO(HGREEN "PASSED");
                INFO(NOCOLOR);
            }
        }
     }
     else {
//          INFO(HYELLOW,VARVALS(_cstr), "does not meet the requirements for a qHash" NOCOLOR);
     }
 }
    void
 streamTest()
    noexcept
 {
    string word;
    while ( cin>> word ) {
        test( word.c_str() );
    }
 }
__main_________________________________________________________________________________________________________________
#!
 codesResearch();
 #if 0
 test("Michel");     // ok
 test("[\\]^_`");    // in between upper and lower case characters
 test("with space"); // fails
 test("good123bad"); // fails, should not have no digits
 test("~~~");        // above alpha-s fails
 test("@@@");        // below alpha-s fails
 test("tooMANYtooMANY"); // too long fails
 test("Toonen");     // ok
 #endif

 WITH_SCOPE_DECORATOR_LINES;
 streamTest();
__end__________________________________________________________________________________________________________________
//d


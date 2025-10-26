#!/usr/local/bin/cpx
#^ <string_view>
#|


using cstr2hash_return_t =  uint64_t;   // UNSOLVED:decltype(std::hash<std::string_view(const char*, size_t)>{}());

    inline constexpr uint64_t
charIdCode(char c)
{
   auto index = static_cast<uint64_t>(static_cast<unsigned char>(c));
   // index >= 32  when <32 is harmless but is undefined behavour and could cause collisions.
   return index- 32ull;
}


    inline constexpr cstr2hash_return_t
CstrHash(const char* s, size_t len)
{
    // nearest prime to (2^64)/(34693/359), this emulates bit rotaion, to serve longer words
    constexpr cstr2hash_return_t VALUE_FEEDBACK_DIVIDER = 190647870118074689ull;

    cstr2hash_return_t out{len};
    while EXPECT_true_FROM( len-- ) {
        cstr2hash_return_t cycled= ( out / VALUE_FEEDBACK_DIVIDER );
        // approzimates prime 97 (+96.63788... with) prime numbers and pi approximation
        out*= 34693;  out/= 359;

        out+= cycled? cycled: len;
        out+= charIdCode( *s ++ );
    }
    return out;
}

    inline constexpr cstr2hash_return_t
CstrHash(const char* _cstr)
{
    return CstrHash(_cstr,std::strlen(_cstr));
}

    inline consteval /*only for compiletime literals */ cstr2hash_return_t operator ""
_CstrHash(const char* _cstr,size_t _len)
{
    return CstrHash(_cstr,_len);
}

#(
    const char* name= "Jan";

    switch( CstrHash( name) ) {
        case "Jan"_CstrHash:
            INFO(FUNC_MSG("1st"));
            break;
        case "Pim"_CstrHash:
            INFO(FUNC_MSG("2nd"));
            break;
        case "Michel"_CstrHash:
            INFO(FUNC_MSG("OK"));
            break;
        default:
            ThrowBreak("No match");
            break;
    }
#)

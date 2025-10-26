#!/usr/local/bin/cpx
#|
    constexpr uint128_t inline
charIdCode(char c)
{
   // take full 256 bit 
   auto index = static_cast<uint128_t>(static_cast<unsigned char>(c));
   return index;   
}

//:Cstr2uint128    Convert 'at compile time' ASCII characters to a 128 bit integer.
//
    constexpr  uint128_t 
Cstr2uint128(char const* s, size_t len)
{
    constexpr uint128_t VALUE_FEEDBACK_DIVIDER_PRIME=71554072443048259;
    // 28949 /113 ≈ 256.18584...
    constexpr uint128_t RESOLUTION_COUNT_PRIME   = 28949;
    constexpr uint128_t RESOLUTION_DIVIDER_PRIME =   113;
    
    uint128_t out(len);
    while EXPECT_true_FROM( len-- ) {
        uint128_t cycled= ( out / VALUE_FEEDBACK_DIVIDER_PRIME ); 
        
        out*= RESOLUTION_COUNT_PRIME;  
        out/= RESOLUTION_DIVIDER_PRIME;
        
        out+= cycled? cycled: len;
        out+= charIdCode( *s ++ );
    }
    return out;
}

    inline uint128_t 
Cstr2uint128(char const* _s)
{
    return Cstr2uint128( _s, strlen(_s));
}
    inline constexpr uint128_t 
operator "" _word(const char* _s,size_t _l) {
    return Cstr2uint128( _s, _l);
}
    inline uint128_t /*runtime*/
Cstr2uint128(string const&_s)
{
    return Cstr2uint128( _s.c_str(), _s.length());
}
    constexpr  uint128_t
MemBlock2uint128(void const*_s, size_t _l)
{
    return Cstr2uint128( static_cast<const char*>(_s), _l);
}
    template<typename POD_T>
    constexpr  uint128_t
AnyPOD2uint128(POD_T const&_s)
{
    return MemBlock2uint128(&_s, sizeof _s);
}


map<uint128_t,string> Words;

template<size_t _Size>
class Line
{
    char m_line[_Size];
  public:
    Line()
    {
        memset( m_line, 0, _Size);
    }
        char const* 
    line() const
    {
        return m_line;
    }
        Line& 
    line(char *_line)
    {
        strncpy( m_line, _line, _Size); 
    }
};

void dumpWords()
{
    for ( auto const& [hash,word]: Words ) {
        INFO(VARVALS(hash,word));
    }
}

#(
    INFO(VARVAL(Cstr2uint128("")));
    
    Line<64> l;
    l.line("Praize my hash");
    INFO(VARVALS(l.line(),AnyPOD2uint128(l)));
    

    size_t collisions(0);
    string word;
    while (getline(cin,word)) {
        auto hash(Cstr2uint128(word));
        cout<<hash<<endl;
        
        if ( Words.find(hash)==Words.end() ) {
            Words[hash]= word;
     continue;/*while(getline...)*/;// new word was added
        }//else
        if ( Words[hash] == word ) 
     continue/*while(getline...)*/;// expected result
        //else
        ++collisions;
        CERROR("Collision:  ",VARVALS(collisions,word,hash,Words[hash]));
    }//while(getline...)
#if 0
    dumpWords();
#endif
INFO(VARVAL(collisions));
#)

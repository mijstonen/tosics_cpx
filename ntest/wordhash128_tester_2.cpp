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
    constexpr uint128_t VALUE_FEEDBACK_DIVIDER=(uint128_t(1)<<120);
(uint128_t(1)<<119) | (uint128_t(1)<<118) | (uint128_t(1)<<117) | (uint128_t(1)<<116) | 
(uint128_t(1)<<115) | (uint128_t(1)<<114) | (uint128_t(1)<<113) | (uint128_t(1)<<111) | (uint128_t(1)<<110) | 
(uint128_t(1)<<108) | (uint128_t(1)<<107) | (uint128_t(1)<<106) | (uint128_t(1)<<104) |  (uint128_t(1)<<99) | 
 (uint128_t(1)<<98) |  (uint128_t(1)<<95) |  (uint128_t(1)<<93) |  (uint128_t(1)<<92) |  (uint128_t(1)<<90) | 
 (uint128_t(1)<<89) |  (uint128_t(1)<<87) |  (uint128_t(1)<<84) |  (uint128_t(1)<<82) |  (uint128_t(1)<<80) | 
 (uint128_t(1)<<79) |  (uint128_t(1)<<73) |  (uint128_t(1)<<71) |  (uint128_t(1)<<69) |  (uint128_t(1)<<64) | 
 (uint128_t(1)<<61) |  (uint128_t(1)<<60) |  (uint128_t(1)<<59) |  (uint128_t(1)<<58) |  (uint128_t(1)<<57) | 
 (uint128_t(1)<<55) |  (uint128_t(1)<<54) |  (uint128_t(1)<<52) |  (uint128_t(1)<<51) |  (uint128_t(1)<<50) | 
 (uint128_t(1)<<47) |  (uint128_t(1)<<44) |  (uint128_t(1)<<41) |  (uint128_t(1)<<40) |  (uint128_t(1)<<39) | 
 (uint128_t(1)<<35) |  (uint128_t(1)<<34) |  (uint128_t(1)<<33) |  (uint128_t(1)<<32) |  (uint128_t(1)<<31) | 
 (uint128_t(1)<<29) |  (uint128_t(1)<<28) |  (uint128_t(1)<<24) |  (uint128_t(1)<<23) |  (uint128_t(1)<<22) | 
 (uint128_t(1)<<19) |  (uint128_t(1)<<18) |  (uint128_t(1)<<15) |  (uint128_t(1)<<14) |  (uint128_t(1)<<11) | 
  (uint128_t(1)<<7) |   (uint128_t(1)<<6) |   (uint128_t(1)<<4) |  (uint128_t(1)<<3)  |   (uint128_t(1)<<1);
   
    constexpr uint128_t RESOLUTION_COUNT_PRIME   = 29057;
    constexpr uint128_t RESOLUTION_DIVIDER_PRIME =   113;
    
    uint128_t out(len);
    while EXPECT_true_FROM( len-- ) {
        uint128_t cycled= out / VALUE_FEEDBACK_DIVIDER; // safe upper 8 bits to cycle them 
        
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
        
        if ( Words.find(hash)==Words.end() ) {
            Words[hash]= word;
            cout<<hash<<endl;
        }
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

#!/usr/local/bin/cpx
#@
unsigned char t[256];
    void
initTable(){
    memset(t,0, sizeof(t));
 // Disable the warning for this section of code.
 #pragma GCC diagnostic ignored "-Wchar-subscripts"
    t['.']=0; // consider any non assigned character be presented equal to the presentation of a '.'
    t['e']=1;
    t['t']=2;
    t['o']=3;
    t['a']=4;
    t['i']=5;
    t['n']=6;
    t['s']=7;
    t['r']=8;
    t['h']=9;
    t['d']=10;
    t['u']=11;
    t['l']=12;
    t['c']=13;
    t['m']=14;
    t['f']=15;
    t['w']=16;
    t['g']=17;
    t['y']=18;
    t['p']=19;
    t['b']=20;
    t['v']=21;
    t['k']=22;
    t['j']=23;
    t['q']=24;
    t['z']=25;
    t['x']=26;
    t['E']=27;
    t['T']=28;
    t['A']=29;
    t['O']=30;
    t['I']=31;
    t['N']=32;
    t['S']=33;
    t['R']=34;
    t['H']=35;
    t['D']=36;
    t['L']=37;
    t['U']=38;
    t['C']=39;
    t['M']=40;
    t['F']=41;
    t['W']=42;
    t['G']=43;
    t['Y']=44;
    t['P']=45;
    t['B']=46;
    t['V']=47;
    t['K']=48;
    t['J']=49;
    t['Q']=50;
    t['Z']=51;
    t['X']=52;
    t['0']=53;
    t['1']=54;
    t['2']=55;
    t['3']=56;
    t['4']=57;
    t['5']=58;
    t['6']=59;
    t['7']=60;
    t['8']=61;
    t['9']=62;
    t['-']=63;
    t['_']=63;
  #pragma GCC diagnostic warning "-Wchar-subscripts"
}

uint64_t encode(char _c)
{
    return t[static_cast<unsigned>(_c)];
}

/*
 1*0   6   0
 3*6  18   1..3    6
 4*5  20   4..7    5
 5*4  20   8..12   4
*/
    constexpr
    uint64_t
wordHash(const char* _cs)
{
        const unsigned
    REMAINDER_LEFTSHIFT= 3
    ;
        const unsigned
    KEEP_PRIOR_BITS= 64- /* Number of highest bits to rotate= */ REMAINDER_LEFTSHIFT
    ;
    if ( !strlen(_cs) ) {
        return 0ul;
    }
        const size_t
    SIGNIFFICAT_CHARS=13
    ;
        char
    cs[SIGNIFFICAT_CHARS+1]
    ;
    strncpy(cs,_cs,SIGNIFFICAT_CHARS);
        const char*
    pcs=cs
    ;
        unsigned short
    shifts[]={ 6,6,6, 5,5,5,5, 4,4,4,4,4}
    ;
        auto
    isf = &shifts[0]
    ;
        uint64_t
    hw= encode(*pcs++);
    ;
    while (*pcs) {
        hw<<=*isf++;
        hw+= encode(*pcs++);
    }
    //if ( strlen(_cs)<=SIGNIFFICAT_CHARS ) {        return hw;    }
    // otherwise

    // Do the rest (unsignifficat characters)
    size_t offset=pcs-cs;
    pcs= _cs+ offset;
    while (*pcs) {
        hw<<= REMAINDER_LEFTSHIFT;
        hw+= (( hw>> KEEP_PRIOR_BITS )+ encode(*pcs++) );
    }
    return hw;
}

    void
wordCode(const char* _cstr)
{
    auto wh=wordHash(_cstr);
    INFO(VARVALS(_cstr,wh));
}


#define WH(word)

bool lineFilter(const string& _s)
{
    for(auto c:_s) if (!encode(c) ) return true;
    return false;
}


#!
initTable();
wordCode("Michel0123456789");

#if 1
for (std::string w; cin;std::getline(cin, w)){
    if ( lineFilter(w) ) {
        // contains not allowed characters
        continue;
    }
    uint64_t wh=wordHash(w.c_str());
    const char* whbits=Bits2str(wh);
    INFO(VARVALHEX(wh),VARVALS(w,hex,wh,whbits));
}
#endif
//switch ()


#if 0
for(auto i: repeat(256)){
    INFO(VARVALS(i,static_cast<char>(i),static_cast<uint16_t>(t[i])));
}
#endif


/*OBSOLETE*/
#!/usr/bin/env cpx
#^ <TinySHA1.hpp>
#|

/*
 * SECOND VERSION CHANGE
 * Instead of reading a external created hash from commandline. 
 * Create the hash from standard input using from standard input.
 * This will be a drop-in replacement for makeHash in  
 * ...cpx/scripts/CPX-preproces_hash_compile.sh
 * replacing scripting to lower shell processing overhead. 
 */


/*
 Create a unique finger print from file content, independed of its original filename.
 A finger print is a name made of digits alphabethical characters and 2 
 punctuation characters which is more precise and more comapct then common checksums.

 This cpx app reads words from the standard input and
 
1. ignores any kind of whitespace (as defined by std::cin >> word_string )
2. translates hexadecimal feed to a packed stream of a 64 bit encoded character set
3. Eventually when other characters are encounterd, there ACSII value is used, but only if printable,
   in inclusive range 33..127. Else, it will abort.
4. Designed for typical use in: 
   f=<filname>; (sum $f;cksum $f|awk '{print $1, $2}';md5sum $f|awk '{print$1}';sha512sum $f|awk '{print$1}')|fingerprint.cpp
   where $f is the file for which the fingerprint is taken.
5. Later the app could be improved to use pstream to run hash programs (see 4).
   
   
ALGIRITHM
=========
(after 'other char expansion', asume only hexadecimal chars)

a 64 bit unsigned long is used as a shift buffer.
6 bytes (= 12 nible chars) or else what is left from the input is pushed in.
     except: the first time, 8 bytes are pushed in.

6 bytes make op 48 bits, which can be read as 6 bit (=0..63) values, 8 shift left of 6 bits operations.
those values are each translated by the NUMS_0_61_TO_CHAR table, which is put to the output.

FingerPrint is intended to have a very strong unique identification filename of a source file.

NOTE: Because this app has 1 single purpose and does it in one 'fast' way, it has no commandline arguments.

This is C++17 code.
*/

// translating numbers 0..63 to character in constant global string
    constexpr char 
NUMS_0_61_TO_CHAR[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
;
    constexpr uint64_t 
NIBLE_BASE=19            /* less dense < 16 < more dense */
;
    constexpr uint64_t 
MAX_UNSIGNED16BIT=NIBLE_BASE*NIBLE_BASE*NIBLE_BASE*NIBLE_BASE
;
    constexpr uint64_t 
LOW_6BIT_MASK = 0x3Full
;
    class
FingerPrint
{
        [[nodiscard]] static state_t 
    ExpandNonHexChars(string* _fragment_)
    {
        ostringstream new_fragment;
        for( char const *pchr= _fragment_->c_str(); *pchr; ++pchr ) {
            switch (*pchr) {
//#!=             printf "#|\n#(\nfor(char const *pc=\"0123456789ABCEFabcdef\";*pc;++pc){ cout<<\"              case '\"<<*pc<<\"':\"<<endl; }\n#)\n" | cpx
#!=             charcase.cpp 0123456789ABCEFabcdef
                new_fragment.put(*pchr);
                break;
              default:
                ostringstream hexvalue_oss;
                hexvalue_oss<<hex<<static_cast<unsigned>(*pchr);
                string chr2hex=hexvalue_oss.str();
                //INFO(VARVALS(*pchr,chr2hex));
                new_fragment<<chr2hex.c_str();
            }
        }
        ( *_fragment_ )= new_fragment.str();
        return State( 0);
    }
    /****************************************/
        string 
    m_hexInStr= ""
    ;
        string::size_type 
    m_hisPos= 0
    ;
        uint64_t 
    m_shiftbuffer= 0
    , 
    m_rest= 0
    ,
    m_sumAccuTail = 0
    ,
    // i.s.o 64 bit values, using 2 punctuators, only use 62 values
    // instead values 0, 62 and 63 are counted and their mod 62 value is added at the end as a sected char from NUMS_0_61_TO_CHAR
    m_border62=0, m_borderEnd=0, m_borderStart=0
    ;
        ostringstream
    m_fingerprintStream
    ;
    /****************************************/
        [[nodiscard]] state_t 
    getHexIn( istream& _is)
    {
        if ( !_is ) {
            return State(0);  // nothing to read
        }
        for/*almost ever*/(auto cnt=0;cnt<1'000'000;++cnt) {
                string 
            fragment = ""
            ;
            _is >> fragment;  // so far assume to have only nibble chars
            if ( !_is ) {
                return State(0);  // end reading
            }
            if (STATEREPORT(ExpandNonHexChars( &fragment ))) return State( -1);
            m_hexInStr += fragment;
        }
        return State( -1); // prevented hanging in endless loop  (however this will not avoid waiting for input)
    }
        [[nodiscard]] state_t
    getTinyHashIn( istream& _is)  
    {
        string line; 
        SHA1 s;
        while ( getline( cin,line ) ){
            s.processBytes(line.c_str(),line.length());
        }

        sha1::SHA1::digest32_t digest;
        m_hexInStr= s.make_digest_string();
        return State( 0);
    }
        [[nodiscard]] state_t
    bumpreadNibbles(string::size_type _nibbleChars)
    {
            char 
        readbuffer[17]
        ;
        
        ASSERT( m_hexInStr.length()>= m_hisPos ); // if not, remainingInputLength will be calculated incorrectly
        
            string::size_type
        remainingInputLength = m_hexInStr.length()- m_hisPos
        ;
            string::size_type
        charsToBeCopied= ( remainingInputLength< _nibbleChars ?remainingInputLength :_nibbleChars )
        ;
            string::size_type 
        copiedChars= m_hexInStr.copy(readbuffer , charsToBeCopied, m_hisPos)
        ;

        readbuffer[copiedChars]='\0'; // end of plain zero terminated C string
        
        // must have copied all (nibbleChars) or remaining at the end of the string, should also be true if m_hexInStr contains nothing
        ASSERT( ( copiedChars== charsToBeCopied )|| ( ( m_hisPos+ copiedChars )== m_hexInStr.size() ) ); 

        m_shiftbuffer= 0;
        if ( !copiedChars ){
            // m_hexInStr could be empty, which - here - is no error
            // treated as reading nummeric: 0
            // Usually, all nibbles where read before
            // notify caller
            return State( 1);
        }
        
        // bump
        m_hisPos+= copiedChars;
        
        // read: convert hexadecinal to nummeric
        if ( !(istringstream(readbuffer) >> hex>>m_shiftbuffer ) ) {
            return State( -1); // could not read hex value from readbuffer
        }
        
        return State(0);
    }
        [[nodiscard]] state_t 
    initial()
    {
        if ( STATEREPORT(bumpreadNibbles(16/*_nibbleChars*/)) ) return State( -1);
        return State( 0);
    }
        void
    writebump()
    {
        // write
            uint64_t
        oChrSel = m_shiftbuffer & LOW_6BIT_MASK
        ;
            char
        outChr='?'  // should aways be overwritten by 0..9,A..Z,a..z, '?' in output inidates a software error
        ;
        ASSERT( oChrSel < 64 );
        
        switch ( oChrSel ) {
            case 0: 
                ++m_borderStart;
                break;
            case 62:
                ++m_border62;
                break;
            case 63:
                ++m_borderEnd;
                break;
            default:
                --oChrSel;
                outChr= NUMS_0_61_TO_CHAR[ oChrSel];
                m_fingerprintStream.put( outChr);
        }
        
        // bump
        m_shiftbuffer>>= uint64_t(6);
    }
        [[nodiscard]] state_t
    processRemaining()
    {
        // Output chars till completion and no bits left in the shift buffer
        for/**almost ever*/(uint32_t cnt=0;cnt<1'000'000;++cnt){
            while ( m_shiftbuffer > MAX_UNSIGNED16BIT ) {
                show();
                writebump();
            }
            m_rest = m_shiftbuffer;  // preserve for use in next iteration, m_shiftbuffer get overwritten by call to bumpreadNibbles();
            ;
            switch ( STATEREPORT(/***/bumpreadNibbles(12/*_nibbleChars*/)/***/, SR_EXCLUDE_0_AND(1)) ) {
                case 0:
                    break;
                case 1:  // finish and discontinue loop 
                    ASSERT(  m_shiftbuffer== 0 ); // read failed, is no error, but end of input

                    // write out the rest (value in lowest 16 bit below MAX_UNSIGNED16BIT) plus collected shifbuffer summing
                    m_shiftbuffer+=m_sumAccuTail;
                    while ( m_shiftbuffer ) {
                        writebump();
                    }
                    m_fingerprintStream.put(NUMS_0_61_TO_CHAR[m_borderStart%62]); // put translated zero count
                    m_fingerprintStream.put(NUMS_0_61_TO_CHAR[m_border62%62]); // put translate 62, 63 counts  (becomes last char of output)
                    m_fingerprintStream.put(NUMS_0_61_TO_CHAR[m_borderEnd%62]); // put translate 62, 63 counts  (becomes last char of output)
                    
                    // OK, nmormal return
                    return State( 0);
                default:
                    return State( -1);
            }
            ASSERT( ( m_shiftbuffer& 0xFFFF000000000000ull )== 0ull ); // Here, upper 16 bits always have to be 0
            m_sumAccuTail += m_shiftbuffer;
            
            // put the rest bytes of prevous cycle into the upper 16 bits (byte 7 and 8) of the next shift buf value by shift right 6 bytes ( == 48 bits )
            m_shiftbuffer |= ( m_rest<< uint64_t(6*8) );
        }
        // practically infinite loop ended without reaching finish (see case 1)
        return State( -2); 
    }
        [[nodiscard]] state_t
    writeFingerPrintTo( ostream& os_ )
    {
        if ( !m_fingerprintStream ) return State( -1);
        if ( !os_ ) return State( -2);
        os_ << m_fingerprintStream.str()<< endl;
        if ( !os_ ) return State( -3);
        return State( 0);
    }
        void 
    show() const
    {
        //INFO(VARVALS(m_hexInStr,m_hisPos,m_shiftbuffer,m_rest));
    }
    
    /******************************************************************/
  public:
        [[nodiscard]] state_t 
    main()
    {
        show();
        if ( STATEREPORT(getTinyHashIn( cin)) ) return State( -1);
        if ( STATEREPORT(initial()) ) return State( -2);
        if ( STATEREPORT(processRemaining()) ) return State( -3);
        show();
        if ( STATEREPORT(writeFingerPrintTo( cout)) ) return -4;
        return State( 0);
    }
} 
Services
;
#(
    if ( STATEREPORT(Services.main() ) ) return State( -1);
  
    return State( 0);
#)

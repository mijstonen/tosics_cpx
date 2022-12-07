#!/usr/bin/env cpx
#|

#if 0
    union 
Endian 
{
  private:
    const uint64_t i=0x7061524334251607;
    const uint8_t  cc[8];

    uint8_t const* begin() const {return cc;} 
    uint8_t const* end()   const {return begin()+ ( sizeof cc );}

    
  public:
    
      Endian(){}
      
    void showLayout() {
        for( auto& c : cc ) {
            INFO(PTRCHRNUMHEX(&c));
        }
    }
      
       
    bool is_Big() const
    {
        decltype(cc) dd={0x70,0x61,0x52,0x43,0x34,0x25,0x16,0x07};
        return ! memcmp(cc,dd,sizeof cc);
    }

    bool is_Little() const
    {
        decltype(cc) dd={0x07,0x16,0x25,0x34,0x43,0x52,0x61,0x70};
        return ! memcmp(cc,dd,sizeof cc);
    }
}
;
#endif

static char IndentSpaces[]="<?php echo str_repeat(' ',200);?>";
static char EndMarker[]="<?php echo str_repeat('<>',60);?>";

char const* make_hex_str(uint32_t _v) 
{
    /*
     * Formatting print with old style snprintf easier 
     * then with iostreams and strings, use simple static 
     * buffer.
     * But let the outputstream align the values.
     */
    static char Hex32bitStr[16];  // need 8 * char + '0x' +'\0'

    snprintf( Hex32bitStr, sizeof Hex32bitStr, "0x%08x", _v);
    return Hex32bitStr;
}


//  Note: code is borrowed and modified from ~/Public/git/busybox/libbb/crc32.c (I did NOT invent this)
void crc32_little_endian_table(char const* _tableName)
{
    cout<< IndentSpaces<< "uint32_t "<< _tableName<< "[256] = {\n"
          "        ";
    uint32_t polynomial = 0xedb88320;
    cout << IndentSpaces;
    for (unsigned  i = 0;/*condition and ....*/ ; /*.... iteration inside loop*/) {
        uint32_t c(i);
        for (unsigned j = 8; j; j--) c = (c&1) ? ((c >> 1) ^ polynomial) : (c >> 1);
        
        // print caclulated item
        cout << setw( 10)<< make_hex_str( c);
        
        if (++i >255) {
            //  terminate
            cout<<'\n'<< IndentSpaces<< "};" << endl;
            break;
        }
      //else next i
        cout<< ", ";
        if ( i & 0xF ) continue;
      //else every i = 16, 32, 48 ...
        cout<< "\n        "<< IndentSpaces;
    }
}

//  Note: code is borrowed and modified from ~/Public/git/busybox/libbb/crc32.c (I did NOT invent this)
void crc32_big_endian_table(char const* _tableName)
{
    cout<< IndentSpaces<< "uint32_t "<< _tableName<< "[256] = {\n"
          "        ";
    uint32_t polynomial = 0x04c11db7;
    cout << IndentSpaces;
    for (unsigned  i = 0;/*condition and ....*/ ; /*.... iteration inside loop*/) {
        uint32_t c(i << 24);
        for (unsigned j = 8; j; j--) c = (c&0x80000000) ? ((c << 1) ^ polynomial) : (c << 1);
        
        // print caclulated item
        cout << setw( 10)<< make_hex_str( c);
        
        if (++i >255) {
            //  terminate
            cout<<'\n'<< IndentSpaces<< "};" << endl;
            break;
        }
      //else next i
        cout<< ", ";
        if ( i & 0xF ) continue;
      //else every i = 16, 32, 48 ...
        cout<< "\n        "<< IndentSpaces;
    }
}

#!= curl -s cpx-server:6789/inc_serve.php


/* ----------------- PHP INJECTION
<?php 
printf( "running Php: %s\n", phpversion()); 
?>
----------------- PHP INJECTION */

#(
    unsigned how_many_IndentSpaces= 0;
    
    //TheEndian.showLayout();
    //INFO(VARVALS(TheEndian.is_Big(),TheEndian.is_Little()));
    //INFO(VARVALS(Endian().is_Little()));

    //INFO(VARVALS(ProgramArguments,how_many_IndentSpaces));
    
    INFO(VARVAL(Is_Little_Endian));

    if ( ProgramArguments.size() > 1 ) {
        istringstream arg1(ProgramArguments[1]);
        if ( !(arg1 >> how_many_IndentSpaces) ) {
            cerr<< "Failed to read unsigned int number of indentation spaces from first program argument";
        }
    }

    if ( EXPECT_true_FROM( how_many_IndentSpaces< ( sizeof IndentSpaces ) ) ) {
        // make string termination occure soner
        IndentSpaces[ how_many_IndentSpaces]='\0';

    }
  //else UNPEXPECT a too large number is silently ignored

  
    
//  if ( TheEndian.is_Little() ) {
//      ASSERT( !TheEndian.is_Big() ); 
      crc32_little_endian_table("little_endian_table");    
//  }
//  else {
//      ASSERT( TheEndian.is_Big() );
      crc32_big_endian_table("big_endian_table");
//  }
      
    cout<<EndMarker<<endl;       
#)

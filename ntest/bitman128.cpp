#!/usr/bin/env cpx

  //typedef unsigned int qword_t __attribute__((mode(TI)));

  typedef unsigned __int128 qword_t __attribute__((mode(TI)));

union u128 {
    
     
    struct PACKED b128 {
        qword_t  W:128;
    } b128_view;
    
    struct PACKED h64l64 {
        uint64_t L:64
        ,        H:64
        ;
    } h64l64_view;
};

/* This should be the intended 64 bit (aka 8 byte) structure
defined by bitfields. However, the compiler makes up its own mind
and uses 12 bytes, unacceptable.
*/

struct PACKED q64node {
    unsigned typebits:2;
    unsigned childInParent:2;
    signed parent :12;
    signed child_A:12;
    signed child_B:12;
    signed child_C:12;
    signed child_D:12;
};

/* Situation here is even worse. This structure shoudl be 128 bit (aka 16 byte).
Its even impossible to have the correct size. When incrementally lowering the bits
the size goes fromn 20 to 12 bytes.
 */ 

struct PACKED q128node {
    unsigned typebits:1;
    unsigned childInParent:2;
    signed  parent:25;
    signed child_A:25;
    signed child_B:25;
    signed child_C:25;
    signed child_D:25;
};
/*
 Conclusion:
 Bitfield manipulation must be done with arimetric integerfield operations.
 This is previously already done on for 64 bit. This test program investigates
 the abillity (with gcc) to manipulate 128 bit directly ( or better: allow 
 cross 64 bit bounday bitfield manipulations ).
*/



ostream& operator<<(ostream& os_, u128 __i)
{
    return ( os_<<setfill('0')<<setw(16)<<hex<<__i.h64l64_view.H
            <<':'<<setfill('0')<<setw(16)<<hex<<__i.h64l64_view.L );
}

//ostream& operator<<(ostream& os_,unsigned __int128 __i)
//{
//    uint64_t h= __i>>64, l=__i&&0xFFFFFFFFFFFFFFFF;
//    return ( os_<<setfill('0')<<setw(16)<<hex<<h<<' '<<setfill('0')<<setw(16)<<hex<<l );
//}
// put your globals, templates,class definitions and includes here

#!

/****************** empty template for startinhg a new cpx script *********************/

// put you (main script)  code here

// hint: See cpx-core and use ProgramArguments for commandline arguments

// The std and util namespaces are standard enabled, see cpx-file-begin

//@{ script
//@} script
//unsigned __int128 bm=0;
//bm=0xF000F000F000F000; // F000F000F000F000;
//bm<<=64;
INFO(VARVALS(sizeof(q64node),sizeof(q128node)));
INFO(VARVAL(sizeof(u128)));

u128 u;
u.h64l64_view.H= 0xF000'F000'F000'F000;
u.h64l64_view.L= 0x0000'1000'2000'3000;
INFO(VARVALS(u));
u.b128_view.W >>= 32;
u.b128_view.W <<= 32;
INFO(VARVALS(u));
u.b128_view.W|=0x10;
INFO(VARVALS(u));
u.b128_view.W|=(qword_t)0x0000'0000'0001'0100<<(qword_t)48;
INFO(VARVALS(u));
u128 f;
f.b128_view.W= (u.b128_view.W>>56);
INFO(VARVALS(f));
f.b128_view.W = f.b128_view.W & (qword_t)0xFFF;
INFO(VARVALS(f));
/* buggy:*/// cout<<"my:"<<hex<<u.b128_view.W<<endl;
#define BUILDREFRESH 2


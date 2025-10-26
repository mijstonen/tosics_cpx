#!/usr/local/bin/cpx

#!

        auto
    mtoNum="T24jul23_162549266"_s128lh
    ;
        unsigned char
    decoded[18+1]
    ;
        state_t
    unexpected= uint128ToSmallCstr(decoded , 18, mtoNum)
    ;
    if (unexpected) {
        INFO(HCYAN, VARVALS(unexpected),"state" NOCOLOR);
    }
    if ( unexpected< 0 ) {
        INFO(HRED "Decoding of hash failed!", NOCOLOR);
    }
    INFO(VARVALS(mtoNum,decoded));

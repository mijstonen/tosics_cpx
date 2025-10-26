#!/usr/local/bin/cpx

#!

        auto
    mtoNum=";;;;;;;;;;;;;;;;;;"_s128lh
    //mtoNum="Michel"_slh
    ;
        unsigned char
    decoded[18+1]
    ;


    if ( !STATEREPORT(hashToSmallCstr(decoded, mtoNum)) ) {
        INFO(VARVALS(mtoNum,decoded));
    }

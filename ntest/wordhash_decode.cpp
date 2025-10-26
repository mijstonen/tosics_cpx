#!/usr/local/bin/cpx

#!
        auto
    mtoNum="Michel"_slh
    ;
        unsigned char
    decoded[10]
    ;
        state_t
    unexpected= uint64ToSmallCstr(decoded , strlen("Michel"), mtoNum)
    ;
    if (unexpected) INFO(HCYAN, VARVALS(unexpected),"state" NOCOLOR);
    if (unexpected<0) {
        INFO(HRED "Decoding of hash failed!", NOCOLOR);
    }
    INFO(VARVALS(mtoNum,decoded));

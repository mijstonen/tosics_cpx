#!/usr/local/bin/cpx

/* Broken

   U1='?' U2='?' U3='?' U4='?'
   divider= '?'
   u='?' M='?' v='?' w='?'
   2

*/



// checking 128 bit arimetrics

constexpr uint128_t U1=uint128_t(1);
constexpr uint128_t U2=uint128_t(1)<<33;
constexpr uint128_t U3=uint128_t(1)<<66;
constexpr uint128_t U4=uint128_t(1)<<99;

#!

//uint128_t divider=190647870118074689ull;
uint128_t divider=uint128_t(1)<<64;

INFO(VARVALS(U1,U2,U3,U4));
INFO(VARVAL(divider));

uint128_t u= 1;
uint128_t M= uint128_t(1)<<127;

auto v=M;
v /= divider;
auto w=v;
w *= divider;
INFO(VARVALS(u,M,v,w));

#if 0
while (u) {
    190647870118074689ull
    StringFrom128bitUInt u_str(u);
    INFO(VARVAL(u_str));
}
#endif

switch ( U2 ) {
    case U1:
        INFO(1);
        break;
    case U2:
        INFO(2);
        break;
    case U3:
        INFO(3);
        break;
    case U4:
        INFO(4);
        break;
    default:
        INFO('?');
}


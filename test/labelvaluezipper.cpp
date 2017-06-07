#!/usr/bin/env cpx
#^ "../inc/cpx-unchanged.hpp"
#|
#(
auto n=1;
auto m=2;
float f=33.333;
auto c='M';
std::string s="Toonen";
auto lsum=[](auto a, auto b) { return a+b; };
INFO(VARVALS(n,m,f,c,lsum(n,m),s,lsum(s,s)));
#)
//

#!/usr/bin/env cpx
#^ "../inc/cpx-unchanged.hpp"
#|

// Test overloading output parameters

class Out1
{
public:
    int o;
};

class Dirived1: public Out1
{
public:
    int m;
};

class Out2
{
public:
    int z;
};


class Multi
: public Out1
, public Out2
{
};


void update(Out1* o_)
{
    INFO(__PRETTY_FUNCTION__);
    o_->o=1;
}

void update(Out2* o_)
{
    INFO(__PRETTY_FUNCTION__);
    o_->z=2;
}

void update(Dirived1* o_)
{
    INFO(__PRETTY_FUNCTION__);
    o_->o=1;
    o_->m=2;
}


#(
INFO("OK");
Out1 o1;
Out2 o2;
Dirived1 d1;
Multi m1;
update(&o2);
update(&o1);
update(&d1);
// update(&m1); // who wins ? abbigious
update(static_cast<Out1*>(&m1)); // choose
update((Out2*)&m1); // who wins
return 0;
#)
//force rebuild by changing the file by incrementing number 4

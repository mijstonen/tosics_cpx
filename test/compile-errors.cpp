#!/usr/bin/env cpx

#|

#(

// Deliberately fail to compile to test the cpx behaviour.

auto msg= "This should not run, but cause a nasty compile error!";

#if 1

// Deliberately cause compile error(s)
cout << msg << end;

#else 

// Correct version
cout << msg << endl;

#endif

#)

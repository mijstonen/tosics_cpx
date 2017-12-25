#!/usr/bin/env cpx
#|

// Testing Append_splitted(), with special options for Append_splitted() and Append_joined()
// to handle custom situations.
// NOTE: Minimal testing frame (to demands), extension candidate in future.

#(

ASJ_special asjs;

asjs.m_allow_empty= false;

//string in("'12 2' 3");
string in("\",\\\",,\",\"_\",,,''");

vector<string> out;
INFO(VARVAL(in));
STATEREPORT(Append_splitted( &out, in,",",asjs));
for( auto ostr:out ) INFO(VARVAL(ostr));

#)

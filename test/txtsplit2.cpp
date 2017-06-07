#!/usr/bin/env cpx
#|


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

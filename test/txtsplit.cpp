#!/usr/bin/env cpx

// Testing Append_splitted()

#^ "../inc/cpx-unchanged.hpp"
#|
#(


string in{"   a,bb   ,,,,c     cc,dd dd"};
vector<string> out;


STATEREPORT(  Append_splitted( &out, in, ","),'A' );
// Use requires linkage with util.cpp

STATEREPORT(  Append_splitted( &out, in, ",")  );
STATEREPORT(  Append_splitted( &out, in, " ")  );
//STATEREPORT(  Append_splitted( &out, in, " ",true)  );
//STATEREPORT(  Append_splitted( &out, in, ",",true)  );
//STATEREPORT(  Append_splitted( &out, "P,Q,RE", ",",true)  );

// mini test, can I get the type name of a lambda
// INFO(VARVAL(type_name_of(out)));
// ---------

INFO(VARVAL(in));
for( auto ostr:out ) INFO(VARVAL(ostr));

string back;


STATEREPORT(  Append_joined( &back, out, '|')    );

CERROR(VARVAL(back));


return 0;
#)
//force rebuild by changing the file by incrementing number 4

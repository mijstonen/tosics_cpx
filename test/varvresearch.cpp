#!/usr/bin/env cpx

// Testing Append_splitted() and (in util) VSTRINS macro cases, during its development.

#|

#(


//namespace fs = std::experimental::filesystem;

using namespace experimental;
namespace tu=tosics::util;

optional<tu::ASJ_special> optAsj;


string s = VSTRINGS(XPR{ kaasboer, T(map, '99', p) },'q,q\', r', ",,,",f(a, g(b)), 1, 10, foo<r, r, -9.3>, endit);
//string s = __VSTRINGS(X{k,T(map,99,p)},q,r,f(a,g(b)),1,10,foo<r,r,-9.3>, e);
//Unbalanced:  string s = "(a({[<M>])}b,c),a({[<M>]})b,c)";
//string s = __VSTRINGS(a({[<Michel>]})b,{},c,a({[<M>]})b,c);
//string s = __VSTRINGS(a<M>b,c);
INFO(VARVAL(s));
vector<string> vss;

tu::ASJ_special asj;

#if 0
auto ul= asj.m_up.length();
auto dl= asj.m_down.length();
assert( ul==dl );
for ( decltype(ul) i=0; i<ul; ++i ) {
    INFO(VARVAL(asj.m_up[i]),VARVAL(asj.m_down[i]));
}
#endif
// asj.m_up= asj.m_down="";
if ( !STATEREPORT(  Append_splitted(&vss, s)  ) ) {
    auto max = vss.size();
    INFO(VARVALHEX(sizeof(string)));
    for (decltype(max) i = 0; i < max; ++i) {
        INFO(VARVAL(i),PTRVAL(&vss[i]));
    }
}



#if 0
// test error checking
string nothing; //= __VSTRINGS();
STATEREPORT(  append_splitted(&vss, nothing, ",", asj)  );

vector<string> * undefined = nullptr;
STATEREPORT(  append_splitted(undefined, s, asj)  );
STATEREPORT(  append_splitted(undefined, s, " ,;/-")  );
#endif


#)

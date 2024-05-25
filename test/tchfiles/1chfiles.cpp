#!/usr/local/bin/cpx
#|
#(
// creates single character files, intended to research valid use of special chars in certain cases (like as valid bash shell variable value e.g)
for (int i=33; i<128;++i){
    char nm[2]={(char)i,0};
    ofstream nmos(nm);
    nmos<<i;
}
#)

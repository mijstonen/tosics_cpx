#!/usr/local/bin/cpx

void ala(size_t _s)
{
    //char allla[_s];
    char * allla = (char*)alloca(_s);
    memset(allla,'_',_s-1);
    allla[_s-1]=0;
    cout << allla << endl;
}

#!
ala(200);


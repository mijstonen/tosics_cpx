#!/usr/local/bin/cpx
#!
INFO("start",VARVALS(ProgramArguments[0]));
||  parallel num_threads(8)
    {
        double t=0.0;
        for(;;){
            t+=1.0e-6;
        }
    }

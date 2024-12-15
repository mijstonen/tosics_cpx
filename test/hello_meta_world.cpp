#!/usr/bin/env cpx

// test meta commands

#|

// #!=pure-circle.cpp pie 60 150 3 11



#!=cpx greetings.cpp

#!=metagreetings.cpp compileTimeString c 20

// depricated 
// #!!circle.cpp circle.h pie 60 150 3 10


#(

//#!?sleepdatetime.cpp


#!=metagreetings.cpp local_compileTimeString l 30

    
#!=pure-circle-v2.cpp pie 0 360 15 100
    
    string message="Hello meta world";
    INFO(VARVALS(message,::Greetings('*',10),compileTimeString,local_compileTimeString));
    
    for ( auto ad: pie ) {
        INFO(VARVALS(ad.angle,ad.radius,ad.x,ad.y));
    }

    INFO("Hello meta world");
#if 0
    {
//#!?sleepdatetime.cpp one 1
        INFO(VARVAL(one));
    }
    {
//#?sleepdatetime.cpp one 1
        INFO(VARVAL(one));
    }
#endif    
#)
    

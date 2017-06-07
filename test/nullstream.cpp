#!/usr/bin/env cpx

#|


/*
Researching implementation of s output stream that does not output anything, the software variant of /dev/null.
This is very usefull to suppress output on moments you do not want it being shown.
*/


// Because allocations in the pre-main program startup phase should be avoided,
//  these pointers are assigned to objects in  main().
ostream  *pLogStream= nullptr, *pNullStream= nullptr;


ostream& logIt(bool _enable)
{
    ASSERT( pLogStream && pNullStream ); // will fail if logIt() somehow would be called 
                                         // 'before' (the initialization in) 'main'() .
    return *( _enable? pLogStream: pNullStream );
}

#(
    pLogStream= &cout;
    onullstream sink;  // make hole inside to hide it from user (this test code)
    
    
    
    pNullStream= &sink;

    // TEST
    string something=  STREAM2STR( HWHITE << "  Something to or not to be."<< NOCOLOR );
    
    bool log_switch=true;
    
    INFO(GREEN,"logging turned ON",NOCOLOR,VARVALS(log_switch,something));
    logIt(log_switch)<< 1 << something<< endl; 
    
    INFO(RED,"logging turned OFF",NOCOLOR,VARVALS(!log_switch,something));
    logIt(!log_switch)<< 2 << something<< endl;
#)

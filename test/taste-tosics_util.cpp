#!/usr/bin/env cpx
#|

// put your globals, templates,class definitions and includes here

#(

/****************** empty template for startinhg a new cpx script *********************/

// put you (main script)  code here

// hint: See cpx-core and use ProgramArguments for commandline arguments

// The std and util namespaces are standard enabled, see cpx-file-begin

//@{ A taste of tosics_util, replace it
    string tester="Testring"; 
    int jun=42; 
    auto almost_pi=3126535000000000/995207;
    
    INFO(GREEN);
    INFO(FUNC_MSG("Here is a VARVALS demo"));
    INFO(VARVALS(tester,jun,almost_pi,typeid(almost_pi).name()));
    
    cout<<RED<<"\nOr write manually with ostream (takes much longer to do)\n";
    cout<< "tester='"<<tester<<"' jun='"<<jun
        <<"' almost_pi='"<<almost_pi
        <<"' typeid(almost_pi).name()='"<<typeid(almost_pi).name()<<"'";
    cout<<NOCOLOR<<endl;
    
//@} A taste of tosics_util, replace it

#)

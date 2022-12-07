#!/usr/bin/env cpx
#|
#(
    // This is a mini feature tester of cpx.
    // However it also has become a super fast tester for STATEREPORT.
    // And later it is extended again to work on correct source file
    // numbering that is affected by PHP preprocessing.
    // Note that there is not much function in the code except for
    // the supposed testing.

    // Workarround because gdb has difficulties to inspect certain globals
    INFO(FUNC_MSG("start"));
    vector<string> &args = ProgramArguments;
    FAKE_USE(args);

    Info_ProgramArguments();
    cout<< DateTime();
    if ( ProgramArguments.size() < 2 ) {
 PHP_BEGIN
    foreach( repeat(10) as $index) if($index&1)
        echo "cout << \"PHP $index\"<< endl;\n";
 PHP_END
        INFO(FUNC_MSG("can't run without arguments 1"));
 PHP_BEGIN
    foreach( repeat(10) as $index ) {
        if ( !($index & 1) ) {
            echo "cout << \"PHP $index\"<< endl;\n";
        }
    }
 PHP_END
        INFO(FUNC_MSG("can't run without arguments 2"));
        cerr<< "Provide return value as argument 1 ."<<endl;
        return EXIT_FAILURE;
    }
    auto status= State( stoi(ProgramArguments[1]) );
    INFO(VARVAL(STATEREPORT(status, 'E'))); // Handle notifications
    INFO(VARVAL(STATEREPORT(status, SR_EXCLUDE_0_AND(-3,-1,1,3),'E')  ));
    INFO(VARVAL(status));
    return status;
#)

#!/usr/bin/env cpx
#|
#(
    // This is a mini feature tester of cpx.
    // However it also has become a super fast tester for STATEREPORT

    // Do not recompile when adding documenation
    // Not even now

    // Workarround because gdb has difficulties to inspect certain globals
    vector<string> &args = ProgramArguments;
    FAKE_USE(args);

    Info_ProgramArguments();
    cout<< DateTime();
    if ( ProgramArguments.size() < 2 ) {
        cerr<< "Provide return value as argument 1 ."<<endl;
        return EXIT_FAILURE;
    }
    auto status= State( stoi(ProgramArguments[1]) );
    STATEREPORT(status, 'E');
    STATEREPORT(status,SR_EXCLUDE_0_AND(-3,-1,1,3));
    INFO(VARVAL(status));
    return status;
#)
//8

#!/usr/bin/env cpx

// A bit pointless functionallity, maybe. But it is used to test #!? meta command
//  (Query meta command) that will aways execute because the effect of the script execution
// can't be predicted based on its name and arguments.
// This cpx script has different results every time its called and also a system effect in
// that it causes a sleep for a while. Using #!= would produce wrong results and discard
// the sleep.

#^ <unistd.h>

#|

#(

if ( ProgramArguments.size()<3 ) {
    cerr<< "Usage: sleepdatetime <string_id> <sleep_seconds>"<< endl;
    return EXIT_FAILURE;
}

string const& id=ProgramArguments[1];
int sleep_seconds=atoi(ProgramArguments[2].c_str());
//INFO(VARVAL(sleep_seconds));
ASSERT(sleep_seconds>=0);
sleep( sleep_seconds);
string dtexnl=tosics::util::DateTime();

// remove newline at the end, see man ctime
dtexnl.pop_back();

cout<< "std::string "<< id<< "= \""<< dtexnl<< "\";"<< endl;

#)

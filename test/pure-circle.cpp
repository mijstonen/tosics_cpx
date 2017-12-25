#!/usr/bin/env cpx
#|
#+generator.cpp
#(
// Genarate table representing circle positions.
// This script is superseded by pure-circle-v2.cpp


//info_ProgramArguments();
//INFO(VARVAL(getenv("CPX_WORK_DIR")));
//INFO(VARVAL(cpx::Work_dir()));

(void)cpx::Work_dir();

if ( ProgramArguments.size()<6 ) {
    cerr<< "Usage: pure-circle.cpp <id> <begin_angle> <end_angle> <angle_delta> <radius>"<< endl;
    return EXIT_FAILURE;
}

string const& id=ProgramArguments[1];
double begin_angle= atof(ProgramArguments[2].c_str());
double   end_angle= atof(ProgramArguments[3].c_str());
double angle_delta= atof(ProgramArguments[4].c_str());
double      radius= atof(ProgramArguments[5].c_str());

ASSERT(end_angle>begin_angle);

// avoid division by zero (good enough for example code)
ASSERT(abs(angle_delta)>double(1.0e-6));

auto steps= 1+ ( ( end_angle- begin_angle )/ angle_delta );

cout<<"\
struct AngleData\n\
{\n\
    double angle, radius, x, y;\n\
};\n"
<< "AngleData "<< id<< "["<< steps<<"]={\n";
for ( decltype(steps) i=0; i<steps; ++i ) {
    if ( i ) {
        // separate items;
        cout<< ",\n";
    }
    cout<< "{"<< begin_angle+i*angle_delta<< ","<< radius<< ","<< "/*TODO: x,y */ 0, 0 }";
}

cout<<"\n};\n";


#)



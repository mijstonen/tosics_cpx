#!/usr/bin/env cpx
#^ <cmath>
#|

#(

//info_ProgramArguments();

//INFO(VARVAL(getenv("CPX_WORK_DIR")));

if ( ProgramArguments.size()<7 ) {
    cerr<< "Usage: circle.cpp <include_name> <id> <begin_angle> <end_angle> <angle_delta> <radius>"<< endl;
    return EXIT_FAILURE;
}


string const& id=ProgramArguments[2];
double begin_angle= atof(ProgramArguments[3].c_str());
double   end_angle= atof(ProgramArguments[4].c_str());
double angle_delta= atof(ProgramArguments[5].c_str());
double      radius= atof(ProgramArguments[6].c_str());


ASSERT(end_angle>begin_angle);

// avoid division by zero (good enough for example code)
ASSERT(abs(angle_delta)>double(1.0e-6));


INFO_TO(cerr);
INFO(VARVALS(begin_angle,end_angle,angle_delta,radius));

char const* include_path_name_cstr= getenv("CPX_WORK_DIR");
if ( Is_null( include_path_name_cstr)) {
    throwBreak("environment variable CPX_WORK_DIR is not defined. "
               "NOTE: circle.cpp is intended to be called within the CPX system to generate code."
               ,eBC_handled
              );
}
string include_path_name= include_path_name_cstr + ProgramArguments[1];
//INFO(VARVAL(include_path_name));
cout<< "#include \""<< include_path_name<< "\" //    #!!  ";
ofstream generatedCodeInHeader(include_path_name);
if ( !generatedCodeInHeader ) {
    throwBreak("Failed opening destination header file for the generated code.",eBC_handled);
}


auto steps= 1+ ( ( end_angle- begin_angle )/ angle_delta );

generatedCodeInHeader<<"\
struct AngleData\n\
{\n\
    double angle, radius, x, y;\n\
};\n\
\n"
<< "AngleData "<< id<< "["<< steps<<"]={\n";


for ( decltype(steps) i=0; i<steps; ++i ) {
    if ( i ) {
        // separate items;
        generatedCodeInHeader<< ",\n";
    }
    generatedCodeInHeader<< "{"<< begin_angle+i*angle_delta<< ","<< radius<< ","
                         << "/*TODO: x,y */ 0, 0 }";
}

generatedCodeInHeader<<"\n};\n";

#)



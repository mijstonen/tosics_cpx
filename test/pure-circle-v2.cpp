#!/usr/bin/env cpx
#|
#+generator.cpp
#(
// Genarate table representing circle positions.
// Code generation by use of INFO, use 1 item per token to make it easier to parameterize items and improve readability.

//info_ProgramArguments();
//INFO(VARVAL(getenv("CPX_WORK_DIR")));
//INFO(VARVAL(cpx::Work_dir()));

constexpr const double Pi= std::acos(-1);
constexpr const double Angle2RadianFactor= Pi/180.0;
INFO("//   ",VARVALS(Pi,Angle2RadianFactor));


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

INFO("#ifndef","STRUCT_ANGLEDATA");
INFO("#define","STRUCT_ANGLEDATA",1);
INFO("struct","AngleData");
INFO('{');
INFO(' ',"double","angle",';');
INFO(' ',"double","radius",';');
INFO(' ',"double","angle_radians",';');
INFO(' ',"double","x",';');
INFO(' ',"double","y",';');
INFO('}',';');
INFO("#endif","//","STRUCT_ANGLEDATA");
INFO();
INFO("AngleData",id,'[',steps,']','=');
#if 1
for ( decltype(steps) i=0; i<steps; ++i ) {
    auto angle= begin_angle+ ( i* angle_delta ) ;
    auto angle_radians= Angle2RadianFactor* angle;
    double x= radius* sin(angle_radians);
    double y= radius* cos(angle_radians);
    INFO( i?',':'{',' ','{',angle,',',radius,',',angle_radians,',',x,',',y,'}');
}
INFO('}',';',"//",id);
#endif
#)

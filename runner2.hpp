#pragma once
//runner2.hpp
extern std::string Work_Dir;
extern std::ofstream *PLogStream;

extern bool ForceRebuild;
extern std::string ErrorMsg;

void test_it();
//int runner(int _argc, char const* _argv[]);
int runner();



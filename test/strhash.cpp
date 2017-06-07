#!/usr/bin/env cpx
#^ "../inc/cpx-unchanged.hpp"
// use std::hash on std::string
#|
#(
INFO(VARVALHEX(hash<string>{}(string("Michel"))));
#)

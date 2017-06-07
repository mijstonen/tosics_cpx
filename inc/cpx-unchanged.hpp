#pragma once
//cpx-unchanged.hpp
#ifndef CPX_UNCHANGED_HPP_
#define CPX_UNCHANGED_HPP_ 1


// headers included here are are included with -include to the compiler and bypass the hash phase
// this shortens the hashing time and up to date binaries are executed faster

// when makeing changes to the util library, build the util library first
// cpx will detect date changes in linked util library.

#include "cpx-file-begin.hpp"

#endif // CPX_UNCHANGED_HPP_

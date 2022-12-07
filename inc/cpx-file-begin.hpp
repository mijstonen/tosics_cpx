#pragma once
//cpx-file-begin.hpp
#ifndef CPX_FILE_BEGIN_HPP
#define CPX_FILE_BEGIN_HPP 1

// From here, any source in prehash will be php preprocessed.
// The environment must define: CPX_HASH_COMPILE=CPX-precmdproces_hash_compile.sh
#include "cpx-precmdproces_hash_compile-invoked.hpp"
// Get global php utilities from file cpx_std.php . See ~/.tosics_custom for environment variables, its asumed they are defined
#include "cpx_stdphp.hpp"

// namespace fs = std::filesystem;
#if 1
using namespace std;
#endif

namespace tu=tosics::util;
#if 1
using namespace tu;
#endif

#include "cpx-core.cpp"


#endif  // CPX_FILE_BEGIN_HP

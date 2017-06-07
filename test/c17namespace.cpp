#!/usr/bin/env cpx
#|

// Try C++17 namespace syntax.

namespace tosics::util {
    
    void hello()
    {
        INFO(FUNC_MSG("tosics::util"));
    }
}  

#(

//@{ script
namespace tu=tosics::util;
tu::hello();
//@} script

#)

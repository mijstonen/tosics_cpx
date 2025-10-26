#!/usr/local/bin/cpx
 // C++23 backtrace


 struct Other_exception {};

 // Function to simulate an error
    void
 function_that_throws(size_t _level)
 {
    if ( _level> 0u ) {
        function_that_throws( _level- 1 );
    }
 #if 1
    ThrowBreak(runtime_error("too deep recursion"));  // tosics::util call that creates a stacktrace and will throw.
 #else
    ThrowBreak(Other_exception());
 #endif
 }
___________________________________________________________
#!

 function_that_throws(10);

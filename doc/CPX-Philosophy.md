.........|.........|.........|.........|.........|.........|.........|.........|.........|.........|.........|.........|
CPX Philosophy
==============
CPX shortcut stands for ComPile and eXecute. CPX is the core building block for code generation based meta programming.
It's inspired by the disadvantages of GAPP 
(See file:///home/michel/Projects/Kdevelop/SeqIntra/gapp/docs/en/index-1.html#ss1.1).
GAPP came out of the disadvantages of the C++ preprocessor. Due to IDE background parsing and syntax highlighting, GAPP 
came less attractive. Also the use of relatively unknown scripting language S-lang got its disadvantages (altough 
technically the right tool for the job). Out of a long thought and investigations its concluded that in-fact the best 
meta programming language in C++ (aka: S-lang for GAPP) is C++ it self for some major reasons:
1. Syntax highlighting and background parsing.
2. All of the meta programming languages need library bindings that can become quite complex and needs a lot of effort. 
With C++ this aspect disappears completely, library bindings are natuarally aviable for C and C++.
3. No need to learn and switch between language syntaxes.
4. The abillity to debug it is a big plus, many other scripting languages have no or limited debugging support.

But there are more differences. With CPX there should be no direct integration in the source code anymore. Instead, 
generated code is included, using #include . CPX scripts (we explain this term later) are stored in separate but project
specific sources. CPX can still do multilevel meta programming (like GAPP) but differs in the method.

But CPX is more, it can be used for several purposes. Its like a system programming glue with many many applications

1. Ultra fast (system)software prototyping (virtually 0 seconds compile time, fysical below 1 second).
2. C++ scripting. Put a hashbang on the fistline (aka:#!/usr/bin/env cpx)
3. (Web) server accelleration.
4. Program caching....
5. Automatic dependency analyzing build system.....
6. Multicompiler project building....
7. Testing compiles and research fastest possible compilation, not comprimizing runtime performance.
8. Try doing things with tosics_util.
9. Link your own library, test and experiment with it. Likewhise (dynamicly) link any library for (almost) interactive
   testing.
10. A test suite for compilers to indicate what are the shortes possible achivable compilation times.

N. ......


CPX objectives
==============
Functionalillity development of CPX is focussed on code generation. That is (besides it could do many other things) that
after doing some specific custom task is produces code and data in a machine readable form, or more specifically, C++
source code. This code then is integrated in the total


Waht is CPX not?
================
It's not replacing your development environment. It's not a build system replacement. It is not intended or capable 
handling complex software systems. Better think of it as a rapid interactive development tool with the sortest fastest 
thinkable compiltation times and low system complixity surrounding your source. Typically strong traits of interactive
scripting languages like python, ruby etc. Change, try run, change, try run, change, try run, as fast as you can. It's
fun doing that with C++.

When I have to solve a problem, what I do is isolate the problem from the application intoi a separate test program and
first solve it there. This way the problem gets decoupled from the application behaviour and can developped in 
isolation. Cpx is the perfect tool to do just that. Isolate the problem, research partial and subsolutions, develop 
interactively a prove of concept using cpx and when its working there, clean it up, formalize it and integrate it in 
the application. The realy nice thing about this working method is that during this development the application is not 
changed at all. Only when in isolation the concept is proven, then the coding - integrating the solution - in the 
application starts.

CPX however is a valuable asset in a application build system, where it can be called in one of the building phases. Its
also designed such that is very easily integrates with shellscripts like php, python and perl. My big picture is that
CPX will be a essential component in a PHP code generator and build system evnvironment (discussion of is outside the
scope of this document.


CPX basics
==========


How CPX works
=============
Despite the princieples of CPX look desceptively simple, it is not easy to get it done right.

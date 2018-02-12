CPX Philosophy
==============
CPX shortcut stands for ComPile and eXecute. CPX is the core building block for code generation based meta programming.
It's inspired by the disadvantages of GAPP
(See file:///home/michel/Projects/Kdevelop/SeqIntra/gapp/docs/en/index-1.html#ss1.1).
GAPP came out of the disadvantages of the C++ preprocessor. Due to IDE background parsing and syntax highlighting, GAPP
came less attractive. Also the use of relatively unknown scripting language S-lang got its disadvantages (although
technically the right tool for the job). Out of a long thought and investigations its concluded that in-fact the best
meta programming language in C++ (aka: S-lang for GAPP) is C++ it self for some major reasons:
1. Syntax highlighting and background parsing.
2. All of the meta programming languages need library bindings that can become quite complex and needs a lot of effort.
With C++ this aspect disappears completely, library bindings are natarally avialable for C and C++.
3. No need to learn and switch between language syntax's.
4. The ability to debug it is a big plus, many other scripting languages have no or limited debugging support.

But there are more differences. With CPX there should be no direct integration in the source code anymore. Instead,
generated code is included, using #include . CPX scripts (we explain this term later) are stored in separate but project
specific sources. CPX can still do multilevel meta programming (like GAPP) but differs in the method.

But CPX is more, it can be used for several purposes. Its like a system programming glue with many many applications

1. Ultra fast (system)software prototyping (virtually 0 seconds compile time, physical below 1 second).
2. C++ scripting. Put a hash bang on the fist line (aka:#!/usr/bin/env cpx)
3. (Web) server acceleration.
4. Program caching....
5. Automatic dependency analyzing build system.....
6. Multi compiler project building....
7. Testing compiles and research fastest possible compilation, not compromising run-time performance.
8. Try doing things with tosics_util, being the 'interactive' test tool for it. i.s.o. creating wrappers for python.
9. Link your own library, test and experiment with it. Likewise (dynamiclly) link any library for (almost) interactive
   testing.
10. A test suite for compilers to indicate what are the shortest possible achievable compilation times.
11. cpx can be used to create advanced systems that are constructed from (relative) low complexity although advanced
    cpx apps as building blocks. This in contradiction to a single large application.
    All the IPC, database and network power that is avialable in custom C(++) libraries can be put to use, to let these
    cpx apps interact.
N. ......


CPX objectives
==============
Functionalillity development of CPX is focused on code generation. That is (besides it could do many other things) that
after doing some specific custom task is produces code and data in a machine readable form, or more specifically, C++
source code. This code then is integrated in the total.

What is CPX not?
================
It's not replacing your development environment. It's not a build system replacement. It is not intended or capable
handling complex software systems. Better think of it as a rapid interactive development tool with the sortest fastest
thinkable compilation times and low system complexity surrounding your source. Typically strong traits of interactive
scripting languages like python, ruby etc. Change, try run, change, try run, change, try run, as fast as you can. It's
fun doing that with C++.

Conceptional use
================
When I have to solve a problem, what I do is isolate the problem from the application into a separate test program and
first solve it there. This way the problem gets decoupled from the application behavior and can developed in
isolation. Cpx is the perfect tool to do just that. Isolate the problem, research partial and sub solutions, develop
interactively a prove of concept using cpx and when its working there, clean it up, formalize it and integrate it in
the application. The really nice thing about this working method is that during this development the application is not
changed at all. Only when in isolation the concept is proven, then the coding - integrating the solution - in the
application starts.
CPX however is a valuable asset in a application build system, where it can be called in one of the building phases. Its
also designed such that is very easily integrates with shellscripts like php, python and perl. My big picture is that
CPX will be a essential component in a PHP code generator and build system environment (discussion of is outside the
scope of this document.

cpx and util
============
The util library is not specific for cpx. It is a general purpose collection and extraction of small definitions,
macro's as well (prefered and mostly inline or generic templated) plain C++ code. However, over time code from cpx will
be migrated to util. The role of cpx to util is to provide a emperical, experimental proving ground for those
definitions in util.

cpx in scripts
==============
Cpx is designed to coorporate with other scripting languages. It reads the sourcecode from standard if no source name
is specified or the source name = '--' which is a common concept for unix commands. This also makes it easy to use
"HERE" documents in shell scripts, php, perl etc. to let cpx perform a specific "C++" task while the "HERE" document
may contain scrip variables that are substituted according the rules of the acting scripting language.
Cpx option -o is used to write the executable with specified name (or source file name derived name). So a script could
"prepare" small cpx executables and use them repeatly.


CPX basics
==========
A cpx script is just a cpp file, but with

     #! /usr/bin/env cpx

on the first line. Cpx minimally has to pass the source, but without the first line. But since we already have the
power to parse per character and a micro preprocessor command is already starting with # in the first column of the
line, why not add some more useful commands?

    #|
includes the gory details any C++ program source requires.

    #{
    #}
    #[
    #]
    #(
    #)
define entry and exit code in which you enclose your script code.

    #(
    #)
Will do the job in almost all cases, the other wrappers where historically implemented first.
They allow more detailed control.

The rest you can read in micro_preprocessor.txt (which less then 100 lines long). In their you find additional commands
that proved to be useful and desired.

We use cpp as extension because the syntax highlightning will be the same as in C++ files. But we set the execution
bit (aka chmod +x <source>.cpp).

In test/ you find cpx scripts, start with dumm.cpp, cpx-empty.cpp and rnr_source2.cpp .
In scripts/ you find several helper scripts and the customizable CPX_preproces_hash_compile.sh which is part of the
cpx execution.
Most useful are CPX-sher, CPX-shrink-runner, CPX-common.sh, CPX_preproces_hash_compile and CPX-build-app.



cpx command line syntax
=======================
(from options loop in runner2.cpp:int runner())
Option processing is deliberately kept very simple. In general, by processing
each option it and eventually its argument(s) is/are shifted from the option list.
So with next iteration, next option is processed.

<description>     describing argument
<description>0    optional describing arg
<description>1    precisely one arg
<description>0+   optional to multiple args
<description>1+   one or more args

    >             command prompt
                  ((( Right oriented instruction(s) for interactive actions )))


general:

    cpx <options>0+ <..arguments..>0+


    cpx

Runs code from the command-line. However to make this practical, you need some preprocessor commands
that handle standard definitions and declarations.

    #|
will include the std namespace and the tosics::util namespace and most common std functions
and all util functionalility.

    #(
and
    #)
each starting at first column of the line wrap your code from the standard input within entry and exit code,
so you do not need to write it.

**example**
```
> cpx
#|
#(
cout<<"Plain simple iostream printing"<<endl;
#)
Plain simple iostream printing
>
```

This mode is particulair useful when the code is generated in a scripting system (aka sh, php, python, perl scripts)

    cpx --

Is the same, but you explicit indicate to use stdin as code.


Compiling to named target, target may not start with '-'

    cpx <options>0+ -o  <relative target path>1 <source|-->0+      (do not put <option>0+ after <source|-->)


Compile and execute

    cpx <options>0+ <-->0+

    cpx <cpx options> <source>1 <arguments>0+

Or enter the source (if execute bit is set by chmod...+x...) with its arguments.

    <source_path> <arguments>

Other options

    -f
   Force compilation, even no changes where detected. For example, when you want to recompile after editing comments
   or white space in the source file.

    -v
   Be verbose in the logging output,this is by default.

    -q
   Be quieter in the logging output, only show what's really essential.


CPX internals
=============
Despite the principles of CPX look deceptively simple, it is not easy to get it done right. The 2 mayor components
are:
    * main.cpp and runner2.cpp are the C++ coded portions
    * scripts/CPX-preproces_hash_compile.sh deals with deep compiler related preprocessing up to compilation.

After initialization and command-line processing, cpx will place intermediate files in CPX_WORK_DIR (usually
<user directory>/tmp/cpx ). The source name can be any relative path to the current directory (that is where cpx is
started), but the path name is translated to a universal (platform and directory in-depended) name in order to
create a unique name in the CPX_WORK_DIR.
Then the micro preprocessing is applied. For the micro-preprocessing commands, see micro_preprocessor.txt .
This micro preprocessor is inspired by the hash bang syntax that needs to be the first line of the source, like with
any UNIX script.
The objective is

1. To lift the burden of standard definitions and declarations from the script source.
2. To do some specific advanced meta operations. Scrips can generate code for execution in the enclosing script.
3. Do some tuning to minimize compilation times. Like not preprocessing heavy boost includes, which are not touched
   anyway and a dependency check is not desired.

These operations are in strong correlation with the files in the sub directory inc/ . Often it is just a matter of
generating #include lines that includes one of these files. Having these files in-dependedly from the cpx executable
allows further customization, aka, integrating your own libraries or tools.

After that, the source is handed over to CPX-preproces_hash_compile.sh, which has 3 tasks

1. Collect all sources ( except those included with '#^', see micro_preprocessor.txt ) into a single file.
2. Create a hash code and produce the filenames for the sources and executable from that hash code.
3. If the executable - which name is CPX-<hashcode>.cpp.x does not exist then compile and link it. In the end
   there will always be a executable, of cause; unless there where compile errors.

Because the results are preserved, next time the same source is handled by cpx, it will not compile but pass
back the already existing executable to cpx. The process up to that detection or not of a existing (binary)
executable is substatianally faster then the compilation it self. In fact you can CPX_WORK_DIR a compiler
cache directory.
Because this is just a (although medium complicated) bash script, you can tweak it to your needs, change compiler
options. Tweak fast compilation, fast execution e.g. . Besides the sources involved, it also has tricks to
detect changes in directories to trigger recompilation (see LS_PATTERNS).

REMARKS
=======
cpx is smart, but not smarter as it needs to be. It will skip the rest of the first line when #! is detected, but
it will not check the syntax of the content of the first line. If you start creating you script by copying
cpx-empty.cpp, what could go wrong?
Neither cpx has any clue about C++, is does not know and not check it. Its the compiler beneath that is doing the
checking. Usually you can customize compiler options CPX-preproces_hash_compile.sh . Checking for all warnings and
errors is the default, but that's up to you.
Equally, do not expect C++ programming is becoming simpler. Cpx removes repetitions, not complexities.
If you have compiler errors (and you will!), check $CPX_WORK_DIR/CPX-runner.log, with 'less -R'. CPX-sher is a
convenience script that will show you the last compilation. CPX-runner.log has some ANSI coloring for more ergonomic
reading. The rest is up to the compiler that is used. I use gcc (aka g++) all the time, because it is the most
progressive compiler, which I happen to know in detail.
If you are a C++ beginner, I recommend to use a online source compiler tool instead. If you are you are a mediate level
or professional (Linux) C++ programmer, you will quickly find your use for cpx.
Cpx is rather tightly coupled with my util library. You do not need to use it, but is has some useful things,
however that is not discussed and documented here. Most of the util library is documented with doxygen.

THANKS TO
=========
I did not invent all the cpx wheels, many thanks to SAURAV MOHAPATRA for TinySHA1.hpp and certainly for pstream.h
by Jonathan Wakely, without it, cpx would - at least - a lot harder to realize. See deps/ .

FINALLY
=======
Sure, cpx has bugs ;-) please report them when you find them. And sure, I dislike some parts of the (empirical)
coding style me my self. It was the required method to get the job done ASAP. I intend to refactor it, plans to do
that are made right now. I would like to read any positive or (respectfull) negative feedback.
All suggestions and questions are very welcome, mail to mijstonen@hotmail.com with CPX in the subject field.




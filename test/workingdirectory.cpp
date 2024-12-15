#!/usr/local/bin/cpx
#^ <filesystem>
#|
// Show in which directory does compile and where it runs.

using std::filesystem::current_path   ;

#(
    const string runtime_current_working_directory = current_path();
 PHP_BEGIN
    $directory = getcwd();
    if ( ! $directory ) /* teminate* by */ PHP_ERROR_DIE("php: getcwd() failed");

/* otherwise continued with */
    echo "const string compiletime_current_working_directory="
       . '"'.getcwd().'"'
       . ";\n";
    echo "INFO(VARVALS(compiletime_current_working_directory,runtime_current_working_directory));";
 PHP_END
    /*
        The directories should not be the same, the compilation uses a standard working directory
        (actually a ramdisk) where as at runtime the execution should be in the same directory as
        where cpx did start it. This is reflected by the return value;
        Ofcause, the exception is this cpx script is deliberately started in the
        standard working directory. This is an abnormal case and is not treated.
        So we can use it to use it to pro that this test works.
    */
    return  ( compiletime_current_working_directory!= runtime_current_working_directory )
                ? EXIT_SUCCESS : EXIT_FAILURE ;
#)

#pragma once
//cpx-core.cpp
#ifndef CPX_CORE_CPP_
#define CPX_CORE_CPP_ 1

#include "cpx-common-impl.cpp"


namespace cpx
{
    int main();

        [[noreturn]]void
    catch_with( string _label, exception const* optional_std_exception)
    {
        std::string human_readable_typename(" <<<UNKNOWN>>>");
        // works for clang and gcc
        const std::type_info* tinfo = __cxxabiv1::__cxa_current_exception_type();
        // MSVC++ hints Use __std_exception_ptr_current_exception to get the exception type const
        //   std::type_info* tinfo = __std_exception_ptr_current_exception();
        if (tinfo) {
            human_readable_typename= boost::core::demangle(tinfo->name());
        }
        INFO(_label,"Catched with:",VARVALS(human_readable_typename));
        if ( optional_std_exception ) {
            INFO(VARVALS(optional_std_exception->what()));
        }
        exit( EXIT_FAILURE);
    }
} // nameapce cpx


PHP_BEGIN
    function CatchWithGenerate(string $_catch_signature)
    {
        if (preg_match('/std::exception/',$_catch_signature)) {
            echo 'catch('.$_catch_signature.'){ cpx::catch_with("1st catch signature match:",&_e); }'.PHP_EOL;
        }
        // else if ... preg_match ..$_catch_signature  # for other exception tree
        else {
           // catch all other
            echo 'catch(...){ cpx::catch_with("Unknown signature match, catched with (...) .",nullptr); }'.PHP_EOL;
        }
    }
PHP_END
#define CATCHED_WITH( _CATCH_SIGNATURE ) <? CatchWithGenerate(#_CATCH_SIGNATURE) ?>



    int
main( int _argC, char const* _argV[] )
{
    /* defined later in cpx script source file */void app_debugging_main_entry();
    app_debugging_main_entry();  // should be void function defined in the cpa app, to enter debugging in main

    namespace tu=tosics::util;

    int return_value=EXIT_FAILURE;  // shall be set by cpx::main() unless a exception is thrown

    try {
        CPX_VALIDATE_WITH_SOURCE;
        tu::Initialize( _argC, _argV);
        return_value = cpx::main();
    } // try
    catch ( char const* _msg ) {
        std::cerr<< "ERROR: message: "<< _msg << std::endl;
    }
    catch ( int _errno ) {
        std::cerr<< "ERROR: status: "<< _errno << std::endl;
        exit( _errno);
    }
    CATCHED_WITH(std::exception const& _e)
    CATCHED_WITH( ... )

    return return_value;
}
#endif // CPX_CORE_CPP_ 1

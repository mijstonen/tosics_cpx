#pragma once
//cpx-core.cpp
#ifndef CPX_CORE_CPP_
#define CPX_CORE_CPP_ 1

#include "cpx-common-impl.cpp"

namespace cpx
{
    int main();
} // nameapce cpx

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
    catch ( std::exception const& _e ) {
        std::cerr<< "ERROR: "<< ( type_name(_e) )<<" : "<< _e.what()<< std::endl;
    }
    catch ( ... ) {
        std::string human_readable_typename(" <<<UNKNOWN>>>");

        // works for clang and gcc
        const std::type_info* tinfo = __cxxabiv1::__cxa_current_exception_type();
        // MSVC++ hints
        // Use __std_exception_ptr_current_exception to get the exception type
        // const std::type_info* tinfo = __std_exception_ptr_current_exception();
        if (tinfo) {
            human_readable_typename= boost::core::demangle(tinfo->name());
        }
        INFO("catch (...) excption: ",VARVAL(human_readable_typename));
    }

    return return_value;
}
#endif // CPX_CORE_CPP_ 1

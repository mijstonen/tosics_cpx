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
    CPX_VALIDATE_WITH_SOURCE;

    namespace tu=tosics::util;

    int return_value=EXIT_FAILURE;  // shall be set by cpx::main() unless a exception is thrown

    try {
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
        std::cerr<< "ERROR: "<< ( typeid(_e).name() )<<" : "<< _e.what()<< std::endl;
    }
    catch ( ... ) {
        std::cerr<< "ERROR: catched unknown exception"<< std::endl;

        // For call of terminate() possibly providing information about the exception
        throw;
    }

    return return_value;
}
#endif // CPX_CORE_CPP_ 1

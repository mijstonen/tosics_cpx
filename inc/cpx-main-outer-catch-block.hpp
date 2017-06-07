#pragma once
//cpx-main-outer-catch-block.hpp
    } // try
    catch ( std::exception const& _e ) {
        std::cerr<< "ERROR: exception: "<< _e.what()<< std::endl;
        exit(-1);
    }
    catch ( char const* _msg ) {
        std::cerr<< "ERROR: message: "<< _msg << std::endl;
        exit(-1);
    }
    catch ( int _errno ) {
        std::cerr<< "ERROR: status: "<< _errno << std::endl;
        exit( _errno);
    }
    catch ( ... ) {
        std::cerr<< "ERROR: catched unknown exception"<< std::endl;
    }

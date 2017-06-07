#include <tosics_util.hpp>
#include "cpx-config.h"
#include "runner2.hpp"

namespace tu = tosics::util;  // prepare for renamiming util to tosics::util

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:main:// entry function for runner, setup runtime environment and last resort exception handling


int main(int _argc, char const* _argv[])
{  
    namespace fs = std::experimental::filesystem;
// temp. solution, filesystem should deal with it

    int status = EXIT_FAILURE;
    std::ofstream logfile;
    
    int Cpx_StackLevelCount=0;
    char const* Env_Work_Dir= getenv( ENV_WORK_DIR);
    char const* Env_Cpx_StackLevelCount= getenv(ENV_CPX_STACKLEVELCOUNT);
    
    
    try {
        tu::Initialize(_argc,_argv);
        
        // reserve memory, so if a error message is created it should not need to allocate from heap again
        // (presuming message size is not exceeding the specified size here).
        ErrorMsg.resize(1024);
        
        if ( tu::Is_null(Env_Work_Dir) ) {
            Env_Work_Dir= DEFAULT_WORK_DIR;
        }
        
        //@{ Determine working directory
        Work_Dir= Env_Work_Dir;
        
        if ( !fs::exists(Work_Dir) ) {
            ErrorMsg= STREAM2STR("Non existing working directory for cpx: "<< Work_Dir<< 
                ". Export CPX_WORK_DIR from the environment and make it R/W available.");
            tu::ThrowBreak(ErrorMsg.c_str(),tu::eBC_handled);
        }
        Work_Dir= fs::path(Work_Dir).c_str();
        
        // Ensure that Work_Dir ends with a directory path separator, this asumed in the remain software.
        if ( Work_Dir.back()!=fs::path::preferred_separator ) {
            Work_Dir+= fs::path::preferred_separator;
        }
        
        // Write it back to the environment sanatized value, for use in script(s) being called
        if ( STATEREPORT(setenv( ENV_WORK_DIR, Work_Dir.c_str(), /* overwrite= */true)) ) {
            perror("setenv() failed");
            ErrorMsg= STREAM2STR("Unable to set environment variable "<< ENV_WORK_DIR);
            tu::ThrowBreak(ErrorMsg.c_str());
        }     
        //@} Determine working directory
        
        
        //@{ Determine Env_Cpx_StackLevelCount
        if ( !tu::Is_null(Env_Cpx_StackLevelCount) ) {
            Cpx_StackLevelCount=std::atoi( Env_Cpx_StackLevelCount)+1;
            if ( Cpx_StackLevelCount> MAX_CPX_STACKLEVELCOUNT ) {
                ErrorMsg= STREAM2STR("Cpx terminated due to execeeding MAX_CPX_STACKLEVELCOUNT");
                tu::ThrowBreak(ErrorMsg.c_str());
            }
            ASSERT( Cpx_StackLevelCount>0 );
        }
        Env_Cpx_StackLevelCount= std::to_string( Cpx_StackLevelCount).c_str();
        if( STATEREPORT(setenv(ENV_CPX_STACKLEVELCOUNT,Env_Cpx_StackLevelCount, /* overwrite= */true))) {
            perror("setenv() failed");
            ErrorMsg= STREAM2STR("Unable to set environment variable "<< ENV_CPX_STACKLEVELCOUNT);
            tu::ThrowBreak(ErrorMsg.c_str());
        }
        //@} Determine Env_Cpx_StackLevelCount
        
        //@{ open log file for appending
        std::string LogFile_name= ( Work_Dir+ WORK_PATH_PREFIX )+ LOGFILE_NAME_IDENTIFICATION;
        if ( Cpx_StackLevelCount ) {
            LogFile_name+="-";
            LogFile_name+=Env_Cpx_StackLevelCount;
        }
        LogFile_name+= LOGFILE_EXTENSION; 
         logfile.open(LogFile_name, std::ios::app);
          PLogStream = &logfile;

        if ( !( *PLogStream ) ) {
            tu::ThrowBreak("Unable to perform logging.");
        }
        //@} open log file for appending
        
        ///// THE MAIN ACTION IS IN runner() /////
        //test_it();
        //status = runner(_argc, _argv);
        status = runner();
        //////////////////////////////////////////
        
    } // try
    // Notice: All cathes cause main() to return EXIT_FAILURE
    catch (std::exception const& _e) {
        std::cerr << tu::HRED << "ERROR: exception: " << _e.what() << tu::NOCOLOR << std::endl;
    } //
    catch (char const* _msg) {
        std::cerr << tu::HRED << "ERROR: message: " << _msg << tu::NOCOLOR << std::endl;
    } //
    catch (int _errno) {
        std::cerr << tu::HRED << "ERROR: status: " << _errno << tu::NOCOLOR << std::endl;
    } //
#if 1
    catch (...) {
    std::cerr << "ERROR: catched unknown exception" << std::endl;
    }
#endif
    if ( PLogStream ) {
        ( *PLogStream ) << "\n--------" << std::endl; // note that this endmaker is used for 'generation' counting by CPX-preproces_hash_compile.sh
        PLogStream->close();
    }
    // else there was a error related to the logfile, so we do not write to it

    // returning exit status of the executed client (or failed compilation)
    return status;
}

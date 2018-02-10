// syntax
//  runner       (READ FROM STDIN)
//       | -o <targetname> source|--    (implicit -v)
//       | source|-- arguments...
//       | -v source
//

//#include "info.hpp"
//#include "util.hpp"
//#include "preserve.hpp"
#include <tosics_util.hpp>


namespace fs = std::experimental::filesystem;
namespace tu = tosics::util;  // prepare for renamiming util to tosics::util

#include "cpx-config.h"
#include "runner2.hpp"


//#include <pstream.h>
//#include <TinySHA1.hpp>


#define IS_COMPILED 64


//@{  Global program data

// shared with main.cpp
std::ofstream* PLogStream = nullptr;  // see logref()
std::ostream* PNullStream = nullptr;  // see logref()
std::string Work_Dir;
bool ForceRebuild=false;
//:ErrorMsg:// must persist against scopes due to its use for exceptions and it needs to outlive function main()
std::string ErrorMsg;

//@}  Global program data




namespace {
//:LoggingEnabled://
// Bad & Ugly! Global. Please refactor!
// TODO: get from envinonment variable CPX_LOGGING_DEFAULT
// + if false executable scrips mimimally log (possibly prefered in production environment),
// turn on with -v, however that is only possible if by explicitely use cpx  (aka cpx -v dummy.cpp i.s.o. dummy.cpp)
// + if true it is easier to lean cpx and take advantage of logged data, turn off with -q
// turn off with -q, however that is only possible if by explicitely use cpx  (aka cpx -q dummy.cpp i.s.o. dummy.cpp)
bool LoggingEnabled=true;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:logref:// log activity of compiling and executing to desdicated stream (aka
// file), distinguisch messages with label
std::ostream& logref(char const* _label = nullptr)
{
    if ( !LoggingEnabled ) {
        static tu::onullstream sink;
        return sink;
    }
  //else

#if 1
    if ( !PLogStream ) {
        tu::ThrowBreak("PLogStream (pointer to logfile) must be initialized (aka in "
                              "main()) before use.",
                              tu::eBC_assertion_failed);
    }
#endif
    if (_label) {
        ( *PLogStream ) << std::endl
                        << tu::HWHITE << std::setw(20) << std::left << _label
                        << tu::WHITE << " : " << tu::DateTime() << tu::NOCOLOR;
    }
    ( *PLogStream ) << tu::YELLOW << "    "
                    << "|" << tu::WHITE << tu::NOCOLOR;
    return ( *PLogStream );
}

//////////////////////////////////////////////////      ////////////////////////////////////////////////////////////////
//:mkname:// substitute characters that make bad file names
int mkname(std::string* name_, char const* _path_in_arg, char _subst = '^')
{
    *name_ = WORK_PATH_PREFIX;
    bool convert = false;
    for (char const* pc = _path_in_arg; *pc; ++pc) {
        switch (*pc) {
        case '_':
            convert = false;
            break;
        case '.':
            convert = ( ( pc[1] )!= 'c'); // keep name.c and name.cpp in tact
            break;
        default:
            convert = ispunct(*pc) || isspace(*pc);
        }
        if (convert) {
            int c_as_num(*pc);
            *name_ += _subst;
            *name_ += std::to_string(c_as_num);
        } //
        else {
            *name_ += *pc;
        }
    }

    int return_value = (name_->length() ? 0 : -1);
    return return_value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:valid_phc_status:// Check status of Preprocess Hash Compile, true for IS_COMPILED or EXIT_SUCCESS, otherwhise false
    bool
valid_phc_status(int _phc_exit_status_)
{
    switch (_phc_exit_status_) {
      case EXIT_SUCCESS:
      case IS_COMPILED:
        return true;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:logged_popen:// run subprocess, log it whats executed, deliver its output to callback and its exit status
    template <typename LAMBDA_T>
    int
logged_popen(std::vector<std::string>* outlines_, std::string const& _allargs, LAMBDA_T output_dest_,
    char const* _label = nullptr)
{
    // LAMBDA_T output_dest_ should be a function or lambda that returns a ostream&, aka: ;logref()
    //
    logref(_label) << _allargs << "\n";
    redi::ipstream popen_out(_allargs);
    if (!popen_out) {
        tu::ThrowBreak("logged_popen(): coud not open stream to process, see log.", tu::eBC_assertion_failed);
    } // else
    if (outlines_) {
        for (std::string line; std::getline(popen_out, line);) {
            //output_dest_() << line << '\n';
            outlines_->push_back(line);
        }
    } // if
    else {
        for (std::string line; std::getline(popen_out, line);) {
            output_dest_() << line << '\n';
        }
    }
    popen_out.close();
    int tmp = popen_out.rdbuf()->status();
    int child_process_exit_status = static_cast<int>(static_cast<signed char>(WEXITSTATUS(tmp)));

    {LOCAL_MODIFIED(LoggingEnabled);
    // when something went wrong, logging will be enabled to present the output of the
    // child process in case of compiling it will make the compilation errors visable.
    if ( !valid_phc_status(child_process_exit_status) ) {
        LoggingEnabled= true;
    }

    if ( outlines_ ) {
        for(std::string line: *outlines_) {
            output_dest_()<< line << '\n';
        }
    }
    } //LOCAL_MODIFIED
    return child_process_exit_status;
}
    template <typename LAMBDA_T>
    int
logged_popen(std::vector<std::string>* outlines_, std::vector<std::string> const& _cmd_args, LAMBDA_T output_dest_,
    char const* _label = nullptr)
{
    //std::ostringstream allargs;
    //std::ostream_iterator<std::string> outItr(allargs, " ");
    //copy(_cmd_args.begin(), _cmd_args.end(), outItr);
    std::string program_and_args;
    tu::Append_joined( &program_and_args, _cmd_args);
    return logged_popen(outlines_, program_and_args, output_dest_, _label);
}
    template <typename LAMBDA_T>
    int
logged_popen(std::string const& _cmd_args, LAMBDA_T output_dest_,
    char const* _label = nullptr)
{
    return logged_popen(nullptr, _cmd_args, output_dest_, _label);
}
    template <typename LAMBDA_T>
    int
logged_popen(std::vector<std::string> const& _cmd_args, LAMBDA_T output_dest_, char const* _label = nullptr)
{
    return logged_popen(nullptr, _cmd_args, output_dest_, _label);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:preproces_hash_compile:// run script to preprocess then hash and eventually compile using pstream popen mechanism
    int
preproces_hash_compile(std::string* target_prog_, std::string const& _source, fs::path const& _orginal_source_dir)
{
    char const* phc_script = getenv( ENV_CPX_HASH_COMPILE);
    if ( tu::Is_null( phc_script) ) {
        phc_script= DEFAULT_HASH_COMPILE;
    }
    fs::path cpx_script_dir(CPX_SCRIPTS_DIR);
    // if phc_script is a absolute file path, cpx_script_dir is not prepended
    fs::path phc_script_path= ( *phc_script== fs::path::preferred_separator )? phc_script: cpx_script_dir / phc_script;

    if (!fs::exists(phc_script_path)) {
        ErrorMsg = "preproces_hash_compile(): can not find preproces_hash_compile "
                   "script: "
            + phc_script_path.native();
        tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
    }
    /* clang-format off */
    std::vector<std::string> cmd_args =
    {   phc_script_path
    ,   _source
    ,   _orginal_source_dir
    ,   (LoggingEnabled? "on": "off")
    ,   (ForceRebuild? "force": "auto")
    };
    /* clang-format on */
    // Run it
    std::vector<std::string> outlines;
    int return_value = logged_popen(&outlines, cmd_args, []() -> std::ostream& { return logref(); }
                        , WORK_PATH_PREFIX LOGFILE_NAME_IDENTIFICATION);

    // get return_status_ and target_prog_
    if ( valid_phc_status(return_value) ) {
        (*target_prog_) = "";
        if (outlines.size() < 1) {
            ErrorMsg = phc_script;
            ErrorMsg += " unsufficient lines, must provide <compiling|available> <target_prog> on the "
                        "last line of its output";
            tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
        }

        // Get status ( compiling | available ) and target name from the last line
        std::istringstream lastOutputLine(outlines[outlines.size() - 1]);
        std::string status_word;
        int same_as_return_value=-1;
        if (!(lastOutputLine >> status_word >> (*target_prog_) >> same_as_return_value)) {
            ErrorMsg = phc_script;
            ErrorMsg += " failed reading <compiling|available> <target_prog> on the "
                        "last line of its output";
            tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
        }

        // check consistancy in return_value and status_word reported on the last output line of the script
        // so we are absolutely sure we sent back correct data from the script to the application. Otherwhise
        // the script is likely to misoperate or fail. It probably has a operational error in it and needs
        // to be fixed.
        int selector=-1;
        bool consistant=( return_value == same_as_return_value );
        if (consistant) { // do more checks, note that it can change 'consistant' to false
            char const* anyStatusWord[]={status_word.c_str(),"available","compiling"};
            selector=/*number of*/ITEMS_IN(anyStatusWord);
            tu::FindIndex( &selector, anyStatusWord);
            switch ( selector ) {
              case 1:
                consistant= ( return_value== IS_COMPILED );
                break;
              case 2:
                consistant= ( return_value== EXIT_SUCCESS );
                break;
              default:
                  // No status word found, selector MUST be 0.
                  // It might indicate that not all status_word values are checked.
                ASSERT( !selector );
                consistant= false;
            } //switch
        }
        if ( !consistant ) {
            LOCAL_MODIFIED(INFO_STREAM_PTR); std::stringstream smsg; INFO_TO(smsg);

            INFO("Software corporation error. Last line of script is inconsistant",
                 VARVALS(selector,phc_script,return_value,status_word,same_as_return_value));
            ErrorMsg= std::move(smsg.str());
            tu::ThrowBreak(std::runtime_error(ErrorMsg), tu::eBC_fatal);
        }

    } // if valid_phc_status
    else {
        CERROR("Compilation failed! ", VARVAL(phc_script), VARVAL(return_value));
    } // else valid...

    return return_value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:execute:// Execute given program where its arguments is offset _start to the given argument list.
    int
execute(std::ostream* pOs_, std::string const& _program_and_args)
{
    // Run it
    int return_value = logged_popen( _program_and_args, [pOs_]() -> std::ostream& { return ( *pOs_ ); }, "execute");
    return return_value;
}
    int
execute(std::ostream* pOs_, std::vector<std::string> const& _program_and_args_vec)
{
    // Run it
    int return_value = logged_popen(_program_and_args_vec, [pOs_]() -> std::ostream& { return ( *pOs_ ); }, "execute");
    return return_value;
}
    int
execute(std::string const& _program_and_args_str)
{
    return execute( &(std::cout), _program_and_args_str);
}
    int
execute(std::vector<std::string> const& _program_and_args_vec)
{
    return execute( &(std::cout), _program_and_args_vec);
}

// depricated
   int
execute(std::string const& _program, int _argc, char const* _argv[], int _start = 0)
{
    std::vector<std::string> program_and_args_vec = { _program };
    for (int i = _start; i < _argc; ++i) {
        program_and_args_vec.emplace(program_and_args_vec.end(), _argv[i]);
    }
    return execute( program_and_args_vec);
}













////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:runner:// main functional routine. prepares the files, compiles and executes with arguments

// TODO: split runner(), make a class out of it.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  lines as here above mark locatioons where to break up runner() is separate methods.

int runner()
{
#if DEBUG  // provide local that is a reference to global, this is a gdb cxx11:abi globals problem work arround
    decltype(tu::ProgramArguments) &prog_args= tu::ProgramArguments;
    FAKE_USE(prog_args);
#endif
    auto start = std::chrono::system_clock::now();

    std::istream* psource = nullptr; // to switch input from stdin or from file
    std::unique_ptr<std::ifstream> fromfile;
    std::string source_name="";
    fs::path orginal_source_dir;

    std::string named_target="";
    int status = EXIT_FAILURE;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // Log commandline arguments parcing
    std::ostringstream args_stream;

    for( auto arg: tu::ProgramArguments ) {
        args_stream<< arg<< ' ';
    }

    logref("cpx-start")<< args_stream.str()<< std::endl<< std::flush;

    for ( bool process_more_options=true; process_more_options && tu::ProgramArguments.size()> 1 ;
         /* tu::ProgramArguments changes in body*/) {
        char const* anyOfOptions[]={tu::ProgramArguments[1].c_str(),"-o","-q", "-v" ,"-f"/*,"-",*/};
        int selector=/*number of*/ITEMS_IN(anyOfOptions);
        tu::FindIndex( &selector, anyOfOptions);
        switch ( selector) {
          case 1: // -o <targetname> Save compiled target output to given destination (relative to current directory)
            if ( tu::ProgramArguments.size()< 3) {
                tu::ThrowBreak(std::runtime_error("After arg1 is -o expecting arg2 is target "
                                        "name and (optional) arg3 is source name!"),
                    tu::eBC_handled);
            } // else
            if ( tu::ProgramArguments.size()> 4) {
                std::cerr << std::endl
                    << "WARNING: When -o <target name> ..., then "
                       "the options after the source will be ignored !" << std::endl;
            }
            named_target = std::move( tu::ProgramArguments[2]);
            if ( named_target[0]=='-' ) {
                ErrorMsg= STREAM2STR( "Target name seem to start with '-' this conflicts with the option syntax!");
                tu::ThrowBreak( ErrorMsg.c_str(), tu::eBC_handled);
            }
            if ( ! named_target.length() ) {
                ErrorMsg = "with option -o: Target name length is 0, "
                           "the target name should be made of at least of one character, but not start with '-' .";
                tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
            }
            tu::ProgramArguments.erase( tu::ProgramArguments.begin()+ 1 , tu::ProgramArguments.begin()+ 3 );
            break;

          case 2: // -q quiet mode, mimimize logging, except for compiler errors and warnings (also errors)
            LoggingEnabled=false;
            tu::ProgramArguments.erase( tu::ProgramArguments.begin()+ 1 );
            break;

          case 3: // -v :  opposite of -q, when running script as executable, and logging would be off by default
            LoggingEnabled=true;
            tu::ProgramArguments.erase( tu::ProgramArguments.begin()+ 1 );
            break;

          case 4: // -f : Force rebuild
            ForceRebuild=true;
            tu::ProgramArguments.erase( tu::ProgramArguments.begin()+ 1 );
            break;

          default:

            // If no (more) option(s) found, the selector must me zero.
            if ( selector ) {
                ErrorMsg= "Expecting selector being zero, but got "+std::to_string(selector) +
                            ". Check commandline argument processing";
                tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_default);
            }
            //else {
                // It's OK, there where no (explicit) commandline options
                // LoggingEnabled=  ... see declaration above or get it from environment variable
                // if there was no ProgramArguments[1], source code is red from stdin
            //}
            source_name = std::move( tu::ProgramArguments[1] );
            if ( source_name.length() ) {
                if ( source_name[0]== '-' && source_name!= "--"  ) {
                    ErrorMsg= STREAM2STR("Invalid source name: '"<< source_name<<
                                        "' , starting with:'-'  conflicts with the option syntax!");
                    tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
                }
            }
            else {
                ErrorMsg = "The source name length is 0, but it has to be made of one or more characters";
                tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
            }
            tu::ProgramArguments.erase( tu::ProgramArguments.begin()+ 1 );
            process_more_options= false;
        } //switch
    } // for process_more_options

    // determine origin of *psource, either from stdin or from a named file, if
    // read from stdin but need arguments to read, then use '--' as sourcefile name replacement
    if ( source_name=="" || source_name=="--" ) {
        psource = &std::cin;
        pid_t pid= getpid();  // pid_t =  (likely)  int
        source_name = STREAM2STR("standard_input_"<< pid<< ".cpp");
        orginal_source_dir = fs::current_path();
    } //
    else {
        //source_name = tu::ProgramArguments[1].c_str();

        fromfile.reset(new std::ifstream(source_name));
        if (!(*fromfile)) {
            ErrorMsg=STREAM2STR("Failed to open source file: '"<< source_name<< "'");
            tu::ThrowBreak( ErrorMsg.c_str(), tu::eBC_handled);
        }
        psource = fromfile.get();
        fs::path source_path = source_name;
        orginal_source_dir = fs::canonical( source_path.parent_path());
    }

    // append orginal source dir to child process executions PATH, create PATH=orginal_source_dir when needed.
    char const* current_path= getenv( ENV_PATH);
    std::string new_path( orginal_source_dir);
    if ( !tu::Is_null( current_path) ) {
        new_path+= ':';
        new_path+= current_path;
    }
    if( STATEREPORT(setenv( ENV_PATH, new_path.c_str(), /* overwrite= true*/1))) {
        perror("setenv() failed");
        ErrorMsg= STREAM2STR("Unable to set environment variable "<< ENV_PATH);
        tu::ThrowBreak(ErrorMsg.c_str());
    }


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  input name sanitation
    std::string work_input_name;
    mkname( &work_input_name, source_name.c_str());

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  source conversion with micro preprocessor
    fs::path work_input_path= Work_Dir + work_input_name;

    std::ofstream work_input(work_input_path);
    if (!work_input) {
        tu::ThrowBreak("failed to open work_input_path");
    }

    std::string unchanged_includes_name= work_input_name + UNCHANGED_INCLUDES_POSTFIX;
    fs::path unchanged_includes_path= Work_Dir + unchanged_includes_name;

    std::ofstream unchanged_includes(unchanged_includes_path);
    if (!unchanged_includes) {
        tu::ThrowBreak("failed to open unchanged_includes");
    }
    unchanged_includes<<"#pragma once\n";

    //:EndOfSourceFileEvent:// Thrown to leave the char processing loop
    struct EndOfSourceFileEvent  { };

    char char_from_source = '\0'; // assigned by calling get_char_from_source()
    uint32_t line = 1,col = 0;             //

    //:preprocess_char_copy://Copy char to dest detect #! (hashbang) #| .... which
    // insert a cpx specific include(s)
        auto
    preprocess_char_copy = [&]() -> void
    {
        //:get_char_from_source:// ~ and calc column, line and checksum, throwing
        // EndOfSourceFileEvent ends file reads
            auto
        get_char_from_source = [&]() -> char
        {
            // try to get next char
            psource->get(char_from_source);
            if (!(*psource)) {
                tu::ThrowEvent(EndOfSourceFileEvent());
            }
            // update prosition data used for mini preprocessor command detection and
            // check sum calculation
            if (char_from_source == '\n') {
                ++line;
                col = 0;
            } //
            else {
                ++col;
            }
            return char_from_source;
        }; // get_char_from_source

            auto
        metaCommand=[&](bool _predicatble )
        {
            std::string cmd_and_args;

            for (;;) {
                if (get_char_from_source() == '\n') {
                    break;
                } // else

                cmd_and_args+= char_from_source;
            }
            std::string command;
            std::istringstream(cmd_and_args)>> command; // Note: Command should not be quoted or contain whitespace(s).
            fs::path command_canonical_path;
            std::time_t command_file_time=0; // undetermined
            auto directories= new_path.c_str();
            switch ( STATEREPORT(tu::FileInPATH( &command_canonical_path, command, directories),SR_EXCLUDE_0_AND(-3)) ) {
              case 0: // Ok
                if ( STATEREPORT( tu::PathWriteTime( &command_file_time, command_canonical_path) ) ) {
                    ErrorMsg= STREAM2STR("Unable to determine last write time for "<< command_canonical_path);
                    tu::ThrowBreak( ErrorMsg.c_str());
                }
                break;

              case -3: // Possible error, see ErrorMsg
                ErrorMsg= STREAM2STR("Command: "<<command<<" could not be found in PATH with directories: "<<directories);
                tu::ThrowBreak( ErrorMsg.c_str(), tu::eBC_handled);

              default: // All other are unexpected
                ErrorMsg= STREAM2STR("tu::FileInPATH( ,"<<command<<", "<<new_path<<") failed");
                tu::ThrowBreak( ErrorMsg.c_str());
            }

            //check previous pure argument depended results
            sha1::SHA1 s;
            s.processBytes( &command_file_time, sizeof command_file_time);
            s.processBytes( cmd_and_args.c_str(), cmd_and_args.length());
            sha1::SHA1::digest32_t digest;

            if ( !_predicatble ) {
                // ensure unique include file names by including path and line number into the hash
                char const* work_input_path_name= work_input_path.c_str();
                s.processBytes( work_input_path_name, strlen(work_input_path_name));
                s.processBytes( &line, sizeof line);
            }

            s.getDigest(digest);
            fs::path execute_results_path=
                STREAM2STR( Work_Dir<< WORK_PATH_PREFIX<< std::hex<< std::setfill('0')<<
                    std::setw(8)<< digest[0]<< std::setw(8)<< digest[1]<< std::setw(8)<< digest[2]<<
                    std::setw(8)<< digest[3]<<std::setw(8)<< digest[4]<< ".inc" );
            if ( ForceRebuild || !( _predicatble && fs::exists( execute_results_path) ) ) {
                std::ofstream execute_results( execute_results_path);
                if ( !execute_results ) {
                    ErrorMsg = STREAM2STR("Metacommand (#!=) running '"<< cmd_and_args<< "' at line:"<< (line-1)<<
                                          " failed to open execute results file:"<< execute_results_path );
                    tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
                }
                status = execute( &execute_results, cmd_and_args); // what to do with stderr of cmd... ?
                if ( !valid_phc_status(status) ) {
                    ErrorMsg = STREAM2STR( "Metacommand (#!=) running '"<<cmd_and_args<< "' at line:"<< (line-1)<<
                                           " failed and returned "<<status );
                    tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
                }
            }
            work_input<<"#include "<< execute_results_path<< "    /*    #!="<< cmd_and_args<< "    */"<< std::endl;
        }; // metaCommand

        // BEGIN micro preprocessor
        // After a '#' on the first column check for a single char command.
        // See case's in switch below for explation of each command.
        // Reuire: Not to change the linecount of the compiler.
        switch (get_char_from_source()) {
          case '#':
            if (col > 1) {
                work_input << '#';
                break;
            }
            // preprocesssing based on next char
            switch (get_char_from_source()) { // unhandled EndOfSourceFileEvent, file
                                              // closed without writing.
              case '!': // #!   script hash bang
                if ( line==1 ) /* && col==2 ) */ {
                    work_input << "//    #!";
                }
                else {
                    // Meta programming tokens
                    switch(get_char_from_source()) {
                      case '!': // plain execution (depricated, puts to much responsibility in called executable)
                        {//OLD
                            std::string cmd_and_args;

                            if ( !std::getline( *psource, cmd_and_args) ){
                                tu::ThrowEvent(EndOfSourceFileEvent());
                            }
                            ++line;
                            col=0;

                            //execute
                            status = execute( &work_input, cmd_and_args); // what to do with stderr of cmd... ?
                            if ( !valid_phc_status(status) ) {
                                ErrorMsg = STREAM2STR( "Metacommand (#!!) running '"<< cmd_and_args
                                                    <<"' at line:"<< (line-1)
                                                    <<" failed and returned "<<status
                                                     );
                                tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
                            }
                        }//OLD
                        break;
                      case '?': // querying meta command, every call might produce different results despite equal arguments, alayas execute
                        metaCommand(/* _predicatble= */false);
                        break;
                      case '=': // pure execution if previously arguments and generated output file differ
                        metaCommand(/* _predicatble= */true);
                        break;
                      case '&': // reserved for asynchronus querying meta command  ( might be difficult to implement, no prio)
                      case '.': // wait for completion of asynchronus executing metacommands
                          ErrorMsg = STREAM2STR( "Metacommand #!"<< char_from_source<< " is not implemented"
                                              << " found at line:"<< line
                                               );
                          tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
                        break;
                      default:
                        work_input<< "#!"<< char_from_source;
                    }
                }
                break;
#if 0
                // This is likely to become too complicated to realize and uneconomical
                // (since it redoing the functionality of #!= and #!?)
                // Though striving to it uncovers some demanded functionallity, now to be builtin tosics::util.
              case '=': // recurse cpx pure script
                break;
              case '?': // recurse cpx query script
                break;
#endif

              case '|': // replace by preprogrammed file begin
                work_input << "/* #|   '-include <cpx-unchanged.hpp>' added to compiler options */";
                unchanged_includes<< "#include <cpx-unchanged.hpp>\n";
                break;
              case '(': // second generation, fewer tags and better encapsulation,
                      // replaces #{ + #[ see cpx-core.cpp and tu::cpx_main()
                      // vector<string> tu::ProgramArguments i.s.o argc and argv
                work_input << "#include " << '"' << "cpx-all-before-script.hpp" << '"' << " /*    #(    */";
                break;
              case ')': // replaces #} + #] see case '(':
                work_input << "#include " << '"' << "cpx-all-after-script.hpp" << '"' << "    /*    #)    */";
                break;
              case '{': // replace by start of main declaration
                work_input << "#include " << '"' << "cpx-main-definition-signature.hpp" << '"' << " /*    #{    */";
                break;
              case '}':
                work_input << "#include " << '"' << "cpx-main-and-file-end.hpp" << '"' << "    /*    #}    */";
                break;
              case '[': // start most outer try block of the program
                work_input << "#include " << '"' << "cpx-main-outer-try.hpp" << '"' << "    /*    #[    */";
                break;
              case ']': // end most outer try block of the program and handle standard
                      // catch situations
                work_input << "#include " << '"' << "cpx-main-outer-catch-block.hpp" << '"' << "    /*    #]    */";
                break;
              case '+': // customized cpx include #+name turns into #include "cpx-name"
                work_input << "#include " << '"' << "cpx-";
                for (;;) {
                    if (get_char_from_source() == '\n') {
                        work_input << '"' << "    /*    #+    */\n";
                        break;
                    } // else
                    work_input.put(char_from_source);
                }
                break;
              case '^': // -include directily to compilation, bypassing hash phase
                work_input<< "//#^";
                unchanged_includes<<"#include";
                for(;;) {
                    get_char_from_source();
                    work_input.put(char_from_source);
                    unchanged_includes.put(char_from_source);
                    if ( char_from_source=='\n' ) {
                        break;
                    }
                }
                break;

              default:
                // anything else is taken 1:1 from the source
                work_input<< '#'<< char_from_source;
            } // inner switch c
            break;

          default:
            work_input<< char_from_source;
        } // outer switch c
        // END micro preprocessor
    }; // preprocess_char_copy

    try {
        for (;;) {
            preprocess_char_copy();
        }
    } catch (EndOfSourceFileEvent&) {
    }

    unchanged_includes.close();
    work_input.close();
    if (psource == fromfile.get()) {
        fromfile->close();
    } // else psource == &cin  it is not closed.

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  start script (to decide) to (re)compile or not
    std::string target_name;

    // every string that gives a path with a directory should end on /
    std::string orginal_source_dir_name= orginal_source_dir;
    orginal_source_dir_name+= fs::path::preferred_separator;

    status = preproces_hash_compile(&target_name, work_input_name, orginal_source_dir_name);

    // report "Needed" time
    auto phc_endtime = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = phc_endtime - start;
    logref("preproces_hash_compile")
        << tu::YELLOW << "Needed " << tu::HYELLOW
        << (diff.count() * 1000) << tu::YELLOW
        << " miliseconds for "
        << tu::HYELLOW<< work_input_name << tu::NOCOLOR
        << " to complete." << tu::NOCOLOR << "\n";

    if ( !valid_phc_status(status) ) {
        ErrorMsg = "Something went wrong during compilation of " + work_input_name + ". Check logfile.";
        tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
    }
#if 0 // maybe changed due to compiling (a unknown process), checked again
    if (!fs::exists(target_name)) {
        throwBreak("Failed to retrieve compiled target binary", tu::eBC_assertion_failed);
    }
#endif
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  copy binary or execution
    if (named_target.length()) {
        fs::path named_target_path = orginal_source_dir / named_target;
        fs::copy(target_name, /*to*/ named_target_path, fs::copy_options::overwrite_existing);
        // notice: remaining arguments are ignored
        status= EXIT_SUCCESS;

        logref("output") << "Coping (executable) " << target_name << " to " << named_target << "\n";
    } //
    else {
        // replace first argument of the resulted ProgramArguments by the created target_name
        tu::ProgramArguments[0]= target_name;
        status= execute( tu::ProgramArguments);
    }
    return status;
} // runner()

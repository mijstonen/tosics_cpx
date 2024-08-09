//// s  y n t a x
//  runner       (READ FROM STDIN)
//       | -o <targetname> source|--    (implicit -v)
//       | source|-- arguments...
//       | -v source
//

//#include "info.hpp"
//#include "util.hpp"
//#include "preserve.hpp"
#include <tosics_util.hpp>


namespace fs = std::filesystem;
namespace tu = tosics::util;

#include "inc/cpx-config.h"
#include "runner2.hpp"




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

namespace OccasionallyModified
{
    //:LoggingEnabled://
    bool LoggingEnabled=true;
    //:FlushLogging://
    bool FlushLogging=false;
}

namespace
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:logref:// log activity of compiling and executing to desdicated stream (aka
// file), distinguisch messages with label
    std::ostream&
logref(char const* _label = nullptr)
{
    if ( !OccasionallyModified::LoggingEnabled ) {
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
                        << HWHITE << std::setw(20) << std::left << _label
                        << WHITE << " : " << tu::DateTime() << NOCOLOR;
    }
    ( *PLogStream ) << YELLOW << "/**/"
                    /*<< "|" */ << WHITE << NOCOLOR;
    return ( *PLogStream );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:mkname:// substitute characters that make bad file names
    tu::state_t
mkname(std::string* name_, char const* _path_in_arg, char _subst = '^')
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

    return tu::State( name_->length() ? 0 : -1 );
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
    if ( /* true || "byCpxProgramArgumen(?)" || */ OccasionallyModified::LoggingEnabled && OccasionallyModified::FlushLogging ) {
        logref().flush();
    }
    /*
     * Or close logfile (and null PLogStream) and reopen it set search tag:OpenLogFileAgain
     */
    redi::ipstream popen_out(_allargs);
    if (!popen_out) {
        tu::ThrowBreak("logged_popen(): coud not open stream to process, see log.", tu::eBC_assertion_failed);
    } // else
    if (outlines_) {
        for (std::string line; std::getline(popen_out, line);) {
            //output_dest_() << line << '\n';
            outlines_->push_back( line);
        }
    } // if
    else {
        for (std::string line; std::getline(popen_out, line);) {
            output_dest_() << line << '\n';
        }
    }
    popen_out.close();
    int tmp = popen_out.rdbuf()->status();
    int  child_process_exit_status= static_cast<int>(static_cast<signed char>(WEXITSTATUS(tmp)));

    {LOCAL_MODIFIED(OccasionallyModified::LoggingEnabled);
    // when something went wrong, logging will be enabled to present the output of the
    // child process in case of compiling it will make the compilation errors visable.
    if ( !valid_phc_status(child_process_exit_status) ) {
        OccasionallyModified::LoggingEnabled= true;
    }

    // tag:OpenLogFileAgain
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
    std::string program_and_args;
    STATEREPORT(tu::Append_joined( &program_and_args, _cmd_args));
    return logged_popen(outlines_, program_and_args, output_dest_, _label);
}
    template <typename LAMBDA_T>
    int
logged_popen(std::string const& _cmd_args, LAMBDA_T output_dest_,char const* _label = nullptr)
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
//:extract variable that represents a filesystem object (dir,file,(sym)link, pipe,...) that must exist
    std::string
getPathFromEnvironment(const char* _environmentVariable)
{
    const char* path_in_envar=getenv(_environmentVariable);
    if ( tu::Is_null(path_in_envar) ) {
       ErrorMsg= STREAM2STR("Unknown environment variable:'"<< _environmentVariable<<"'");
       tu::ThrowBreak(ErrorMsg.c_str());
    }
    //otherwise
    if ( !fs::exists(path_in_envar) ) {
       ErrorMsg= STREAM2STR("The path:'"<< path_in_envar<<"' does not match a existing path (dir,file,(sym)link, pipe,...)."
                            "Check environment variable:'"<< _environmentVariable<<"'");
       tu::ThrowBreak(ErrorMsg.c_str());
    }
    //otherwise
    logref("getPathFromEnvironment")<<"Retrievd environment varariable:'"<<_environmentVariable<<"'  providing path:'"<<path_in_envar<<"'";
    return path_in_envar;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:preproces_hash_compile:// run script to preprocess then hash and eventually compile using pstream popen mechanism
    int
preproces_hash_compile(std::string* target_prog_, std::string* runtimeValidationHash_,
                       std::string const& _source, fs::path const& _orginal_source_dir,
                       std::string const& _build_config_name)
{
    char const* phc_script = getenv( ENV_CPX_HASH_COMPILE);
    if ( tu::Is_null( phc_script) ) {
        phc_script= DEFAULT_HASH_COMPILE;
    }

    fs::path phc_script_path;
    if ( *phc_script== fs::path::preferred_separator ) {
        // phc_script is a absolute file path, cpx_script_dir is not prepended
        phc_script_path= phc_script;
    }
    else {
        fs::path cpx_script_dir(getPathFromEnvironment(ENVNAME_CPX_SCRIPTS_DIR));
        phc_script_path= cpx_script_dir/ phc_script;
    }

    if (!fs::exists(phc_script_path)) {
        ErrorMsg = "preproces_hash_compile(): can not find preproces_hash_compile "
                   "script: "
            + phc_script_path.native();
        tu::ThrowBreak(ErrorMsg.c_str());
    }
    /* clang-format off */
    std::vector<std::string> cmd_args =
    {   phc_script_path
    ,   _source
    ,   _orginal_source_dir
    ,   (OccasionallyModified::LoggingEnabled? "on": "off")
    ,   (ForceRebuild? "force": "auto")
    ,   _build_config_name
    };
    /* clang-format on */
    // Run it
    std::vector<std::string> outlines;
    int return_value = logged_popen(&outlines, cmd_args, []() -> std::ostream& { return logref(); }
                        , WORK_PATH_PREFIX LOGFILE_NAME_IDENTIFICATION);

    // get return_status_ and target_prog_
    if ( valid_phc_status( return_value) ) {
        (*target_prog_) = "";
        if (outlines.size() < 1) {
            ErrorMsg = phc_script;
            ErrorMsg += " unsufficient lines, must provide <compiling|available> <target_prog> on the "
                        "last line of its output";
            tu::ThrowBreak(ErrorMsg.c_str());
        }

        // Get status ( compiling | available ) and target name from the last line
        std::istringstream lastOutputLine(outlines[outlines.size() - 1]);
        std::string status_word;
        int same_as_return_value=-1;
        if (!(lastOutputLine >> status_word >> (*target_prog_) >> (*runtimeValidationHash_) >> same_as_return_value )) {
            ErrorMsg = phc_script;
            ErrorMsg += " failed reading <compiling|available> <target_prog> on the "
                        "last line of its output";
            tu::ThrowBreak(ErrorMsg.c_str());
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

            INFO("Software cooporation error. Last line of script is inconsistant",
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

    char const*
colored( char const* _color, char const* _label )
{
    /* BAD AND UGGLYYYYY */
    static char buf[256];
    buf[0]=0;
    return strcat(strcat(strcat(buf,_color),_label),RESET);
}

//:execute:// Execute given program where its arguments is offset _start to the given argument list.
    int
execute(std::ostream* pOs_, std::string const& _program_and_args)
{
    // Run it
    int return_value = logged_popen( _program_and_args, [pOs_]() -> std::ostream& { return ( *pOs_ ); }, colored(HGREEN,"execute"));
    return return_value;
}
    int
execute(std::ostream* pOs_, std::vector<std::string> const& _program_and_args_vec)
{
    // Run it
    int return_value = logged_popen(_program_and_args_vec, [pOs_]() -> std::ostream& { return ( *pOs_ ); }, colored(HGREEN,"execute"));
    return return_value;
}
    int
execute(std::vector<std::string> const& _program_and_args_vec)
{
    return execute( &(std::cout), _program_and_args_vec);
}


} // namespace





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//:runner:// main functional routine. prepares the files, compiles and executes with arguments

// TODO: split runner(), make a class out of it.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  lines as here above mark locatioons where to break up runner() is separate methods.

    int
runner()
{
#if DEBUG  // provide local that is a reference to global, this is a gdb cxx11:abi globals problem work arround
//    decltype(tu::ProgramArguments) &prog_args= tu::ProgramArguments;
//    FAKE_USE(prog_args);
#endif
    auto start = std::chrono::system_clock::now();

    std::istream* psource = nullptr; // to switch input from stdin or from file
    std::unique_ptr<std::ifstream> fromfile;
    std::string source_name="";
    fs::path orginal_source_dir;

    std::string named_target="", build_config_name="";
    int status = EXIT_FAILURE;

    // Detects use of -o and -p options and .?., for either option, only ' ' is allowed, others are incorrect
    char one_of_the_options=' ';

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // Log commandline arguments parcing
    std::ostringstream args_stream;

    for( auto arg: tu::ProgramArguments ) {
        args_stream<< arg<< ' ';
    }
    logref("cpx-start")<< args_stream.str()<< std::endl<< std::flush;
    for ( bool process_more_options=true; process_more_options && tu::ProgramArguments.size()> 1 ;
         /* tu::ProgramArguments changes in body*/) {
        char const* anyOfOptions[]={tu::ProgramArguments[1].c_str(),
            "-o","--output",
            "-q","--quiet",
            "-v","--verbose",
            "-f","--force",
            "-n","--named-stdin",
            "-C","--build-config",
            "-p","--print-target",
            "-a","--preprocessor-arguments",
            "-E","--expanded-source"
            // ,"-t", "--test" // --test {<indentifier>|=cpx::main()}
            // runs the app, stores output in <identifier>.stdout and <identifier>.stderr and returns
            // the exit status. The test did succeed when if cpx --test <ident> <app> ; then executes.
            // --test <...> can be specified multiple times.
            // ,"-tt","--test-all" // {<!identifier>}" //run all tests, optionally specify which to exclude.
            // -tt is a place holder, recommend to always use --test-all .
            // -tx --test-all-then-execute {<!identifier>} // after completing tests succesfully run cpx::main
        };
        int selector=/*number of*/ITEMS_IN(anyOfOptions);
        tu::FindIndex( &selector, anyOfOptions);

        if ( selector ) { // Found
            ASSERT(!strcmp(anyOfOptions[selector],anyOfOptions[0])); // Check that search and found fo match
            logref("process argument")<<selector<<' '<<anyOfOptions[selector]<<"\n";
        }
        else {
            if ( **anyOfOptions== '-' ) {
                std::string valid_options=[anyOfOptions]{std::ostringstream oss;
                    for(size_t i=1;i<ITEMS_IN(anyOfOptions);++i) oss<<anyOfOptions[i]<<' '; return oss.str();}();
                logref("process argument")<< HRED "Invalid cpx option:"<< anyOfOptions[selector]<< " NOT-FOUND!\n" NOCOLOR
                       "Try one of: "<<valid_options<<'\n';
                ErrorMsg= STREAM2STR("Invalid cpx option: '"<< anyOfOptions[selector]);
                tu::ThrowBreak(ErrorMsg.c_str());
            }
            // else, possibly filename and/or other argument(s) not starting considered an cpx option
        }

        decltype( tu::ProgramArguments.size() ) option_param_count=0;
        switch (selector) {
        //_____________________________________________________________________________________________________________
          case 1: case 2:// -o <targetname> Save compiled target output to given destination (relative to current directory)
            if ( tu::ProgramArguments.size()< 3) {
                tu::ThrowBreak(std::runtime_error("Expecting -o <target_name> <source_name>, but only got -o")
                    );
            } // else
            if ( one_of_the_options!= ' ' ) {
                if ( one_of_the_options == 'o' ) {
                    std::cerr << std::endl
                        << "WARNING: option: -o <target name> detected more then once !"
                        "Any other -o <target name> then the first - and, so this one - is ignored !" << std::endl;
                }
                break; // the case 1
            }
            if ( tu::ProgramArguments.size()> 4) {
                std::cerr << std::endl
                    << "WARNING: When -o <target name> ..., then "
                       "the options after the source will be ignored !"
                       " Best is to put all option in front of -o" << std::endl;
            }
            // get target name, if you forgot but added the source name, it may go (horribly) wrong
            named_target = std::move( tu::ProgramArguments[2]);
            if ( named_target[0]=='-' ) {
                ErrorMsg= STREAM2STR( "Target shouldn't start with '-'."
                " Expecting -o <target_name> <source_name>,"
                " it looks -o was followed by another option (starting with '-')");
                tu::ThrowBreak( ErrorMsg.c_str());
            }
            if ( ! named_target.length() ) {
                ErrorMsg = "with option -o: Target name length is 0, "
                           "the target name should be made of at least of one character, but not start with '-' .";
                tu::ThrowBreak(ErrorMsg.c_str());
            }
            one_of_the_options= 'o';
            option_param_count=1;  // for the target name
            break;
        //_____________________________________________________________________________________________________________
          case 3: case 4: // -q quiet mode, mimimize logging, except for compiler errors and warnings (also errors)
            OccasionallyModified::LoggingEnabled=false;
            break;
          case 5: case 6: // -v :  opposite of -q, when running script as executable, and logging would be off by default
            OccasionallyModified::LoggingEnabled=true;
            break;
          case 7: case 8: // -f : Force rebuild
            ForceRebuild=true;
            break;
          case 9: case 10: // - : source is standard input  --named-stdin report.cpp
              // this command does (jet) nothing, presuming correct commandline construction with source comming from standard input
              // When standard input is named, the compiled result can be cached and parsing works as with a file.
              tu::ThrowBreak("-n | --named-stdin : This option is not implemented.");
            break;
        //_____________________________________________________________________________________________________________
          case 11: case 12: // -C <build_configuration_name>
                // Here: tu::ProgramArguments[1]=="-C"
          {
                bool build_cfg_default = false;
                if ( tu::ProgramArguments.size()< 3 ) {
                    std::cerr<< "WARNING: build configuration name missing, expected -C <build_configuration_name>, nothing came after -C"<<std::endl;
                    build_cfg_default= true;
                }
                else if ( tu::ProgramArguments[2][0]=='-' ) {
                    std::cerr << "WARNING: build configuration name shouldn't start with '-'."
                    " Expecting -C <build_configuration_name> <source_name>,"
                    " it looks -C was followed by another option (starting with '-')"<<std::endl;
                    build_cfg_default= true;
                }
                if ( build_cfg_default ) {
                    std::cerr << "WARNING: -C was specified but could not get <build_configuration_name> after -C ."
                    " So, using the default configuration to build."<< std::endl;
                    build_config_name= ""; // a empty string is passed for the default configuration
                    // shift -C
                }
                else {
                    build_config_name = std::move( tu::ProgramArguments[2]);
                    // shift -C <build_configuration_name>
                    option_param_count=1; // for the build_configuration_name
                }
                // Technical design
                //   -C <default|debugging|deployed|...>
                // The build config parameter in is used to call a script functionality with the case word in [ [(] pattern [ | pattern ]  in CPX-preproces_hash_compile.sh
                //  this is a very general approach to do almost anything.
                // The intend is to set CPP_FLAGS kind of variables that determine what kind of executable is build,
                //   default: fast edit compile execute cycle, no size and speed optimizations, with debug info
                //            also, if no --build-config option is given, this is the selected configuration
                //   debugging: more to debugging focussed compiling and load executable in debugger, probably a suited debugging gui, aka: kdbg
                //   deploy: for production use, optimize in one way, depending on needs, for size or speed
                //   deploy_max_speed | deploy_min_size
                //   profiled | sanatized | ... other compiler build assistance configurations.
                //   default | debugging |deploy  fullfill immediate needs, other build configurations may follow, maybe as spinoff from other work.
                //
                // The buildconfig should be somewhat restricted because, always keep in mind that this remains intended for research, proof of concepts and tiny 'single purpose' apps.
                // Complexity should not explode or else consider a more mature industrial level development enviromnment backed by vendors.
          }// case 11 case 12
            break;
        //_____________________________________________________________________________________________________________
          case 13: case 14: // -p    instead of executing, print target after compilation. Intendec use: APP=$(cpx -p myApp.cpp);later use it (multiple times): $APP
                if ( one_of_the_options== ' ' ) {
                    one_of_the_options='p';
                }
                else {
                    if ( one_of_the_options == 'p' ) {
                        std::cerr << std::endl
                        << "WARNING: option: -p detected more then once !"
                        "Any other -p then the first - and, so this one - is ignored !"
                        << std::endl;
                    }
                    else {
                        ErrorMsg= STREAM2STR( "option -p conflicts with option -"<<one_of_the_options
                        <<" choose one, hey cant be used both at the sane time");
                        tu::ThrowBreak( ErrorMsg.c_str());
                    }
                }
              break;
        //_____________________________________________________________________________________________________________
          case 15: case 16: // TBD: -a  'preprocessor arguments passed as single argument'  ( --arguments (for preprocessing) )
                  // check tu::ProgramArguments.size()
                  // preprocessor_arguments = std::move( tu::ProgramArguments[2]);
                  // option_param_count= 1;
              break;

        //_____________________________________________________________________________________________________________
          case 17: case 18: // TBD: -E (ake: line gcc -E) output transformed source i.s.o compiling and running it.
              // Spec
              //   This option is intended to integrate cpx into larger projects with an industry standard build system.
              //   The task for cpx is then to work as code generator with compile and execution validation.
              // -E --expanded-source does compile but not execute.
              // If the compilation fails, no output is presented.
              // If it succeeds, it dumps the intermediate file, that was the translation unit for the compiler, to stdout
              //    the option can be combined with --output <output file>
              // If (also) combined with --test then the app runs as a test
              //    and only if it succeeds it will provide the expanded source
              break;

          default:
            // If no (more) option(s) found, the selector must me zero.
            if (EXPECT_false_FROM(selector>0)) {
                ErrorMsg= "Expecting selector being zero, but got "+std::to_string(selector) +
                            ". Check commandline argument processing";
                tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_default);
            }
            //else {
                // HERE: selector == 0
                // It's OK, there where no (explicit) commandline options left to be processed.
                // anyOfOptions[0]==tu::ProgramArguments[1].c_str()
                // LoggingEnabled=  ... see declaration above or get it from environment variable
                // if there was no ProgramArguments[1], source code is red from stdin
            //}
            source_name = std::move( tu::ProgramArguments[1] );
            if ( source_name.length() ) {
                if ( source_name[0]== '-' && source_name!= "--"  ) {
                    ErrorMsg= STREAM2STR("Invalid source name: '"<< source_name<<
                                        "' , starting with:'-'  conflicts with the option syntax!");
                    tu::ThrowBreak(ErrorMsg.c_str());
                }
            }
            else {
                ErrorMsg = "The source name length is 0, but it has to be made of one or more characters";
                tu::ThrowBreak(ErrorMsg.c_str());
            }
            tu::ProgramArguments.erase( tu::ProgramArguments.begin()+ 1 );
            process_more_options= false;
        } //switch
        if (process_more_options) {
            auto issueState=STATEREPORT(tu::LeftShiftOut_First_ProgramArgument(option_param_count));
            FAKE_USE(issueState);
            ASSERT(!issueState);
        }
    } // for process_more_options

    // be sure to pass as a valid commandline argument.
    if ( !( build_config_name.size() ) ) {
        build_config_name= "default";
    }

    // determine origin of *psource, either from stdin or from a named file, if
    // read from stdin but need arguments to read, then use '--' as sourcefile name replacement
    if ( source_name=="" || source_name=="--" ) {
        ErrorMsg=STREAM2STR("Running source code from standard input is not supported");
        tu::ThrowBreak(ErrorMsg.c_str());

        // Use of unget is needed for modern parsing, but you cannot unget from stdin
        // psource = &std::cin;
        // pid_t pid= getpid();  // pid_t =  (likely)  int
        // source_name = STREAM2STR("standard_input_"<< pid<< ".cpp");
        // orginal_source_dir = fs::current_path();
    } //
    else {
        //source_name = tu::ProgramArguments[1].c_str();
        ASSERT(!source_name.empty());
        fromfile.reset(new std::ifstream(source_name));
        if ( !( *fromfile ) ) {
            ErrorMsg= STREAM2STR("Failed to open source file: '"<<source_name<<"'");
            tu::ThrowBreak( ErrorMsg.c_str());
        }
        psource= fromfile.get();
        fs::path source_path = source_name;
        fs::path souce_path_parent = source_path.parent_path();
        if ( souce_path_parent.empty() ) {
            orginal_source_dir = fs::current_path();
        }
        else {
            orginal_source_dir = fs::canonical( souce_path_parent);
        }
    }

    // append orginal source dir to child process executions PATH, create PATH=orginal_source_dir when needed.
    char const* current_path= getenv( ENV_PATH);
    std::string new_path(orginal_source_dir);
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

    bool close_main_at_end_of_file=false;
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
    uint32_t line_nr=1, col_nr=0, chars=0;
    bool preprogrammed_done=false;    // #| is only working once. Any repeated is silently ignored, such that new (auto) include of |# cause explicit mentioning it to be ignored.
    bool enabled_pragma_omp=true;

    //:preprocess_char_copy://Copy char to dest detect #! (hashbang) #| .... which
    // insert a cpx specific include(s)
        auto
    preprocess_char_copy = [&] (auto /*recurse*/preprocess_char_copy) mutable -> void
    {
            auto
        throwIfEndOfSourceFile= [&]() -> void
        {
            if (!(*psource)) {
                chars+=col_nr;
                tu::ThrowEvent(EndOfSourceFileEvent());
            }
        };

        //:get_char_from_source:// ~ and calc column, line and checksum, throwing
        // EndOfSourceFileEvent ends file reads
            auto
        get_char_from_source = [&]() -> char
        {
            psource->get(char_from_source );

            if (EXPECT_false_FROM( char_from_source== '<' )) {
                    auto
                instead_inject=[&](const char* _word)
                {
                    work_input<< _word;
                    col_nr+=3;
                    // asume its ok not the check for a token immediately after having found one.
                    psource->get(char_from_source );
                };

                // think char_from_source is char a
                char  b='\0'; psource->get(b );
                char  c='\0'; psource->get(c );

                if ( c== '>' ) {
                    switch (b) {
                      case '{':  //  <{>
                        // leave line marked PHP code region
                        // enter PHP controlled (C/C++) code block
                        instead_inject(" { PHP_MARKSRCLINE ?> ");
                        goto skip;
                      case '}':  //  <}>
                        // leave PHP controlled (C/C++) code block
                        // enter and leave line marked PHP code region
                        instead_inject(" <?php } PHP_MARKSRCLINE ?> ");
                        goto skip;
                      case '?':  //  <?>
                        // enter line marked PHP code region
                        instead_inject(" <?php PHP_MARKSRCLINE ");
                        goto skip;
                      // case !
                      // case ;
                    } // switch b
                } // if c
                psource->unget(); // undo c
                psource->unget(); // undo b
                //{                    const char abc[]={char_from_source,b,c,'\0'};
                //    instead_inject(abc);                }
              skip:;
            }
            //else no special token was started

            throwIfEndOfSourceFile();
            // update position data used for mini preprocessor command detection and
            // check sum calculation
            if ( char_from_source== '\n' ) {
                ++line_nr;
                chars+=col_nr;
                col_nr = 0;
            }
            else {
                ++col_nr;
            }

            return char_from_source;
        }; // get_char_from_source

        enum getline_mode
        {
            GLM_no_trimming=0,
            GLM_left_trimmed=1,
            GLM_right_trimmed=2,
            GLM_left_and_right_trimmed=3
        };

            auto
        // return line without the newline at the end
        get_line_from_source=[&]( getline_mode glmMode=GLM_no_trimming) -> std::string
        {
            std::string line;

            //#define

            if ( glmMode & GLM_left_trimmed ) {
                // trim spaces and tabs before
                while( get_char_from_source() ){
                    switch (char_from_source) {
                      case ' ':
                      case '\t':
                          continue;
                      case '\n':
                          return line;  // probably not desired empty (or only whitespaces) line
                      default:
                          line+= char_from_source;
                          goto done_trim_before;
                    }
                }
                done_trim_before:;
            }

            while (get_char_from_source()!='\n') {
                line+= char_from_source;
            }

            if ( glmMode& GLM_right_trimmed ) {
                // trim spaces and tabs after, also
                while ( !line.empty() ) {
                    switch ( line.back() ) {
                      case ' ':
                      case '\t':
                          line.pop_back();
                          continue;
                      default:
                          // keep all that is not a white space
                          goto done_trim_after;
                    }
                }
                done_trim_after:;
            }
            return line;
        };

            auto
        metaCommand=[&](bool _predicatble )
        {
            std::string cmd_and_args(get_line_from_source());

            std::string command;
            std::istringstream(cmd_and_args)>> command; // Note: Command should not be quoted or contain whitespace(s).
            fs::path command_canonical_path;
            std::time_t command_file_time=0; // undetermined
            auto directories= new_path.c_str();
            // determine
            switch ( STATEREPORT(tu::FileInPATH( &command_canonical_path, command, directories),SR_EXCLUDE_0_AND(-3)) ) {
              case 0: // Ok, determine command_file_time
                if ( STATEREPORT( tu::PathWriteTime( &command_file_time, command_canonical_path) ) ) {
                    ErrorMsg= STREAM2STR("Unable to determine last write time for "<< command_canonical_path);
                    tu::ThrowBreak( ErrorMsg.c_str());
                }
                break;

              case -3: // Possible error, see ErrorMsg
                ErrorMsg= STREAM2STR("Command: "<<command
                                   <<" unrecognized! Could not be found in PATH with following directories: "
                                   <<directories);
                tu::ThrowBreak( ErrorMsg.c_str());
                break;

              default: // All other are unexpected
                ErrorMsg= STREAM2STR("tu::FileInPATH( ,"<<command<<", "<<new_path<<") failed");
                tu::ThrowBreak( ErrorMsg.c_str());
            }

            //check previous pure argument depended results
            tu::SHA1 content_hash;
            content_hash.processBytes( &command_file_time, sizeof command_file_time);
            content_hash.processBytes( cmd_and_args.data(), cmd_and_args.length());
            content_hash.processString( command_canonical_path);

            if ( !_predicatble ) {
                // ensure unique include file names by including path and line number into the hash
                char const* work_input_path_name= work_input_path.c_str();
                content_hash.processBytes( work_input_path_name, strlen(work_input_path_name));
                content_hash.processBytes( &line_nr, sizeof line_nr);
            }

            //{@ TODO: test metaCommand usage again
            fs::path execute_results_path=
                STREAM2STR( Work_Dir<< WORK_PATH_PREFIX<< content_hash.make_digest_string()<< ".inc" );
            //}@

            if ( ForceRebuild || !( _predicatble && fs::exists( execute_results_path) ) ) {
                std::ofstream execute_results( execute_results_path);
                if ( !execute_results ) {
                    ErrorMsg = STREAM2STR("Metacommand (#!=) running '"<< cmd_and_args<< "' at line:"<< (line_nr-1)<<
                                          " failed to open execute results file:"<< execute_results_path );
                    tu::ThrowBreak(ErrorMsg.c_str());
                }
                status = execute( &execute_results, cmd_and_args); // what to do with stderr of cmd... ?
                if ( !valid_phc_status(status) ) {
                    ErrorMsg = STREAM2STR( "Metacommand (#!=) running '"<<cmd_and_args<< "' at line:"<< (line_nr-1)<<
                                           " failed and returned "<<status );
                    tu::ThrowBreak(ErrorMsg.c_str());
                }
            }
            work_input<<"#include "<< execute_results_path<< "    /*    #!="<< cmd_and_args<< "    */"<< std::endl;
        }; // metaCommand
            auto
        sourceLineMarking=[&line_nr,&source_name](int32_t _lnr_offset=0)
        {
            return STREAM2STR("\n#line "<< ( static_cast<int32_t>(line_nr)+ _lnr_offset )<< " \""<< source_name<< "\"\n");
        };
            auto
        insert_preprogrammed=[&]()
        {
            if (preprogrammed_done) {
                return;
            }
            // otherwise
            unchanged_includes<< "#include <cpx-unchanged.hpp>\n";
            work_input<< "#include \"cpx-file-begin.hpp\"  /* #|   '-include <cpx-unchanged.hpp>' added to compiler options, is include before this */\n";
            work_input<< "void app_debugging_main_entry(){/* set breakpoint here*/}\n";
            preprogrammed_done=true;
        };
            auto
        pragma_omp=[&]()
        {
            // look ahead to enable/disable pragma omp
            {
                char plusmin;
                psource->get(plusmin );
                throwIfEndOfSourceFile();
                switch (plusmin) {
                  case '+':
                    enabled_pragma_omp= true;
                    ++col_nr;
                    work_input.put(' ');
                    break;
                  case '-':
                    enabled_pragma_omp= false;
                    ++col_nr;
                    work_input.put(' ');
                    break;
                  default:
                    psource->unget();
                    if (enabled_pragma_omp) {
                        work_input<<" #pragma omp ";
                    }
                }
            }
            if (enabled_pragma_omp) {
                for(;get_char_from_source()!='\n';) {
                    switch (char_from_source) {
                      case '\\':
                        //handle next line continuation and double backquote
                        switch ( get_char_from_source() ) {
                          case '\n': // line continuation
                            work_input.put('\\');
                            work_input<<'\n';  // maintain the same number of lines
                            break;
                          case '\\': // double backquote
                            work_input<<"\\\\";
                            break;
                          default:  // just a singleback quote
                            work_input<<'\\'<< char_from_source;
                        }
                        break;
                      default:
                        work_input<< char_from_source;
                    } // switch char_from_source
                } // for
            }
            else {
                // ignore all except newlines from next line continuations
                while( get_line_from_source().back()=='\\' ){
                    work_input.put('\n');
                }
            }
            work_input<<'\n';  // maintain the same number of lines
        };

            auto
        source_inner=[&]()
        {
            fs::path current_working_directory(fs::current_path());

            std::string inner_source_name=get_line_from_source( GLM_left_and_right_trimmed);
            // SHALLOW recursion check, notice that indirect deep infinite recursions are not prevented this way.
            // That is more complex to implement. Just, don't be silly ;-)
            if ( inner_source_name==source_name ) {
                ErrorMsg= STREAM2STR("Failed to inner source: "<< inner_source_name<<"    cpx prevented infinite recursion!");
                tu::ThrowBreak( ErrorMsg.c_str());
            }


            fs::path inner_source_path(inner_source_name);
            fs::path inner_souce_path_directory(inner_source_path.parent_path());
            fs::path inner_souce_filename=inner_source_path.filename();

            try {
                // Change to directory of inner source, such that its sources files are relative are:  seen_from_inner
                tu::ScopedDirectory seen_from_inner(inner_souce_path_directory);

                fs::path inner_current_working_directory(fs::current_path());
                std::ifstream sourcing_file(inner_souce_filename); // 1st attempt

                if ( !sourcing_file ) {
                    // Note: Trying the alternative is ONLY for backwards compatablity with prior #+ implementation.
                    std::string cpx_inner_source_name=
                        getPathFromEnvironment(ENVNAME_CPX_INCLUDES_DIR)+"/cpx-"+inner_souce_filename.generic_string();
                    fs::path cpx_inner_source_path(cpx_inner_source_name);
                    logref("cpx-WARNING")<<inner_souce_filename<<" in "<<inner_current_working_directory<<" failed to open. "
                                        "Instead trying cpx default: "<< cpx_inner_source_path;
                    sourcing_file.open(cpx_inner_source_path);
                    if (!sourcing_file) {
                        ErrorMsg= STREAM2STR("Failed to open inner_source "<< inner_source_path<< " and alternative "<< cpx_inner_source_path);
                        logref("cpx-ERROR")<<HRED<<ErrorMsg<<NOCOLOR;
                        tu::ThrowBreak( ErrorMsg.c_str());
                    }
                    //otherwise
                    inner_source_name=std::move(cpx_inner_source_name);
                }
                logref("cpx-inner")
                    << current_working_directory.generic_string()<<'/'<<source_name<< "  line:"<<line_nr<<" col:"<<col_nr<< ' '
                    << "Parsing inner source file: "<< inner_source_name;

                LOCAL_MODIFIED(source_name, psource,line_nr, col_nr,   chars, char_from_source,enabled_pragma_omp);
                            source_name=inner_source_name;
                                            psource= &sourcing_file;
                                                    line_nr=1;col_nr=0;chars=0;
                                                                            char_from_source='\0';
                                                                                            // enabled_pragma_omp in enclosing file does not change

                work_input<<sourceLineMarking();
                try {
                    for (;;) preprocess_char_copy(preprocess_char_copy);
                }
                catch (EndOfSourceFileEvent&) {
                    logref("cpx-inner")<< "Parsing inner source: "<< inner_source_name<<" completed "<< line_nr-(col_nr?0:1) << " lines, "<< chars<< " characters.\n";
                }
            }
            catch (const tu::DirectoryChanger::changeDir_error& e) {
                ErrorMsg= e.what();
                tu::ThrowBreak( ErrorMsg.c_str());
            }
        };

        // BEGIN micropreprocessor
        // After a '#' on the first column check for a single char command.
        // See case's in switch below for explation of each command.
        // Reuire: Not to change the linecount of the compiler.
            bool
        do_mark_sourceline=false;
        ;
        get_char_from_source();
        switch ( col_nr )
        {
          case 0: // line ended by a newline
            work_input<< std::endl;
            return;

          case 1:  // continue after switch statement, any detected uPP command should trigger sourceline marking
            do_mark_sourceline= true;
            break;

          default: //for all other columns preprocessing is not triggered.
            work_input.put( char_from_source);
            return;
        }
        // otherwise only when col_nr==1
        switch (char_from_source) /*1*/ {
          case '#':
            // preprocesssing based on next char
            get_char_from_source();
            switch (char_from_source) /*2*/ {
              case '!': // #!   script hash bang
                 /* col==2 */
                if ( line_nr==1 )  {
                    work_input << "//    #! "<< get_line_from_source()<<'\n';  // outcommenting what is on the first line (the hash bang to execute cpx)
                    insert_preprogrammed();
                    break;
                }
                // otherwise line_nr!=1
                // Meta programming tokens
                get_char_from_source();
                switch (char_from_source) /*3*/ {
                  case '?': // querying meta command, every call might produce different results despite equal arguments, always execute
                    metaCommand(/* _predicatble= */false);
                    break;
                  case '=': // pure execution if previously arguments and generated output file differ
                    metaCommand(/* _predicatble= */true);
                    break;

                  default: // found '#!' after line_nr==1 at col_nr==1
                    close_main_at_end_of_file= true;
                    work_input << "#include \"cpx-all-before-script.hpp\" /* #!  starts main */";
                    work_input.put(char_from_source);
                    if ( '\n'!= char_from_source ) {
                        work_input<< get_line_from_source()<< '\n';
                    }

                // here add other 3 character uPP tokens to process

                }// switch *3*
                break;

              case '+':
                source_inner();
                break;

              case '^': // -include directily to compilation, bypassing hash phase, multiple includes are separated by '^'
                work_input<< "// #^";
                unchanged_includes<<"#include ";
                do  {
                    get_char_from_source(); // when '\n' then col_nr==0 and the loop terminates
                    if ( char_from_source=='^' ) {
                        unchanged_includes<< "\n#include ";
                    }
                    else {
                        unchanged_includes.put( char_from_source);
                    }
                    work_input.put( char_from_source);
                }   while (col_nr);
                break;

              case '@': // explicit micropreprocessor invoked sourceline marking
                // Sometimes, there is no other alternative them explicit fix the line numering by \n#line <currentline number +1>
                // The disadvantage is that every time the source text changes you need to modify these numbers too, that's too
                // cubersome to accept. Instead, you can use #@ micropreprocessor command that will do it for you, but with the
                // advantage that it's result changes accordingly to source changes.
                // This ONLY works in the cpx script, in (somehow) included files, other solutions need to be provided.

                // NEW: This won;t be needed anymore, since any executed uPP command will trigger source line maring and
                // '__' can be used for this instead.
                // These reason this is still here is that there are scripts that use it.
                break;

              case '|': // replace by preprogrammed file begin
                insert_preprogrammed();
                break;

# if OBSOLETE
              case '(': // second generation, fewer tags and better encapsulation,
                    // replaces #{ + #[ see cpx-core.cpp and tu::cpx_main()
                    // vector<string> tu::ProgramArguments i.s.o argc and argv
                work_input << "\n#include " << '"' << "cpx-all-before-script.hpp" << '"' << " /*    #(    */";
                break;
              case ')': // replaces #} + #] see case '(':
                work_input << "\n#include " << '"' << "cpx-all-after-script.hpp" << '"' << "    /*    #)    */";
                break;
                //( BEGIN depricated (functioning) micropreprocessor commands
                //       These still should work but a are no further maintained.
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
                //) END depricated micropreprocessor commands
# endif
              default:
                do_mark_sourceline= false;
                // anything else is taken 1:1 from the source
                work_input<< '#'<< char_from_source;
              break;
            } // switch *2*
            break;

          case '|':
            get_char_from_source();
            switch (char_from_source) /*2*/ {
              case '|':
                pragma_omp();
                break;
              default:
                do_mark_sourceline= false;
                work_input<< '|'<< char_from_source;
            } // switch /*2*/
            break;

          case '_':
            get_char_from_source();
            switch (char_from_source) /*2*/ {
              case '_': // ignore this line (use for comment or high level tool control), except the newline
                (void)get_line_from_source();
                ASSERT( !col_nr );
                work_input.put('\n');
                break;
              default:
                do_mark_sourceline= false;
                work_input<< '_'<< char_from_source;
            }
            break;

          default:
            ASSERT('\n'!=char_from_source);  // prior control flow will have avoided this to happen
            work_input.put(char_from_source);
            do_mark_sourceline= false;
            break;
        } // switch /*1*/
        if ( do_mark_sourceline ) {
            work_input<< sourceLineMarking();
        }

        // END micropreprocessor
    }; // preprocess_char_copy

    try {
        for (;;) preprocess_char_copy(preprocess_char_copy);
    }
    catch (EndOfSourceFileEvent&) {
        logref("cpx-micropreprocessor")<< "completed "<< line_nr-(col_nr?0:1)<< " lines, "<< chars<< " characters."<< std::endl<< std::flush;
    }

    unchanged_includes.close();
    if ( close_main_at_end_of_file ) {
        work_input << "\n#include " << '"' << "cpx-all-after-script.hpp" << '"' << "    /* closing main that was started with #!  */";
    }
    work_input.close();
    if (psource == fromfile.get()) {
        fromfile->close();
    } // else psource == &cin  it is not closed.

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  start script (to decide) to (re)compile or not
    std::string target_name, runtimeValidationHash;

    // every string that gives a path with a directory should end on /
    std::string orginal_source_dir_name= orginal_source_dir;
    orginal_source_dir_name+= fs::path::preferred_separator;

    status = preproces_hash_compile( &target_name, &runtimeValidationHash,
                                    work_input_name, orginal_source_dir_name, build_config_name);

    // report "Needed" time
    auto phc_endtime = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = phc_endtime - start;
    logref("preproces_hash_compile")
        << HGREEN << "Needed " << HYELLOW
        << (diff.count() * 1000) << HGREEN
        << " miliseconds for "
        << HYELLOW<< work_input_name << NOCOLOR
        << " to complete." << NOCOLOR << "\n";

    if ( !valid_phc_status(status) ) {
        ErrorMsg = "Something went wrong during compilation of " + work_input_name + ". Check logfile.";
        tu::ThrowBreak(ErrorMsg.c_str());
    }
 // maybe changed due to compiling (a unknown process), checked again
    if (!fs::exists(target_name)) {
        tu::ThrowBreak("Failed to retrieve compiled target binary", tu::eBC_assertion_failed);
    }
    if ( runtimeValidationHash.length()<30 ){
        tu::ThrowBreak("Failed retrieving runtimeValidationHash, tu::eBC_assertion_failed");
    }
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if( STATEREPORT(setenv( ENV_CPX_VALIDATION_HASH, runtimeValidationHash.c_str(), /* overwrite= true*/1))) {
        perror("setenv() failed");
        ErrorMsg= STREAM2STR("Unable to set environment variable "<< ENV_CPX_VALIDATION_HASH);
        tu::ThrowBreak(ErrorMsg.c_str());
    }

//  copy binary or execution
    if (named_target.length()) {
        assert( one_of_the_options== 'o' ); // output target was specified, so ensure -o was detected
        fs::path named_target_path = orginal_source_dir / named_target;
        fs::copy(target_name, /*to*/ named_target_path, fs::copy_options::overwrite_existing);
        // notice: remaining arguments are ignored
        status= EXIT_SUCCESS;

        logref("output") << "Coping (executable) " << target_name << " to " << named_target << "\n";
    } //
    else if ( one_of_the_options=='p' ) {
        // print target
        std::cout<< target_name<< std::endl;
    }
    else {
        LOCAL_MODIFIED(OccasionallyModified::FlushLogging);
        OccasionallyModified::FlushLogging = true;
        // replace first argument of the resulted ProgramArguments by the created target_name
        tu::ProgramArguments[0]= target_name;
        status= execute( tu::ProgramArguments);
    }
    return status;
} // runner()

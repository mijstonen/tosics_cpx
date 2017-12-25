#!/usr/bin/env cpx

#|


namespace fs=std::experimental::filesystem;

state_t file_in_path( fs::path* canonical_path_,  fs::path _filename)
{
    char const* envvar_path_content= getenv("PATH");
    if ( tu::Is_null(envvar_path_content) ) {
        return State(-1); // Could not retrieve the PATH environment variable
    }
    vector<string> directories;

    state_t as= Append_splitted( &directories, string(envvar_path_content), ":");
    if ( STATEREPORT(as) ) {
        return State(-2); // Error during splitting.
    }

    for ( auto dir : directories ) {
        fs::path gues_path= fs::path(dir) / _filename;
        INFO(VARVALS(gues_path,fs::exists(gues_path)));

        if ( fs::exists(gues_path) ) {
            ( *canonical_path_ )= fs::canonical( gues_path);
            return State(0);
        }
    }
    return State(-3); // File not found in any of the directories of the PATH environment variable.
}


//NOTICE: state_t PathWriteTime(std::time_t *time_,__TU_FS::path _path); replaces this local function.
std::time_t pathWriteTime(fs::path _path)
{
    auto ftime = fs::last_write_time( _path);
    return decltype(ftime)::clock::to_time_t(ftime);
}

#(

#if 1

if ( ProgramArguments.size()<2 ) {
    cerr<< "Usage: "<<ProgramArguments[0]<<" <filename>"<< endl;
    return EXIT_FAILURE;
}

fs::path file_canonical_path;
if ( STATEREPORT(file_in_path( &file_canonical_path, ProgramArguments[1])) ) {
    return -1;
}
INFO(VARVALS(ProgramArguments[1],file_canonical_path));
std::time_t cftime = pathWriteTime( file_canonical_path);
INFO(VARVAL(cftime));
std::cout << "File write time is " << std::asctime(std::localtime(&cftime)) << '\n';


#endif


#)

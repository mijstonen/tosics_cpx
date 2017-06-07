

// default directories
#define PRJ_PARENT_DIR "/home/michel/Projects/Kdevelop/"
#define CPX_SCRIPTS_DIR PRJ_PARENT_DIR "cpx/scripts/"
#define CPX_INCLUDES_DIRS PRJ_PARENT_DIR "cpx/inc/"


// or from environment, fall back on defines above
#define ENVNAME_CPX_SCRIPTS_DIR "CPX_SCRIPTS_DIR"

// separated list of directories
#define ENVNAME_CPX_INCLUDES_DIRS "CPX_INCLUDES_DIRS"

// default prefix for all files created by cpx
#define WORK_PATH_PREFIX "CPX-"

#define UNCHANGED_INCLUDES_POSTFIX ".unchanged.hpp"

#define ENV_PATH "PATH"
#define ENV_WORK_DIR "CPX_WORK_DIR"
#define DEFAULT_WORK_DIR "/tmp/cpx/"
#define ENV_CPX_STACKLEVELCOUNT "CPX_STACKLEVELCOUNT"
#define ENV_CPX_HASH_COMPILE "CPX_HASH_COMPILE"
#define DEFAULT_HASH_COMPILE "CPX-preproces_hash_compile.sh"
#define LOGFILE_NAME_IDENTIFICATION "runner"
#define LOGFILE_EXTENSION ".log"

// limit process recursion, higher levels are likely indicate incorrect functioning of cpx
#define MAX_CPX_STACKLEVELCOUNT 16

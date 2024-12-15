# must be sh (man 1 dash) compatible
# substituted by CPX-precmdproces_hash_compile.sh which incorporates PHP preprocessing
# Currently (B0106) progressions aren't tested with this script.
########################################################

# files and directories / depended settings
PRJ_PARENT_DIR="/home/michel/Projects/Kdevelop"
 CPX_INCLUDES_DIR="$PRJ_PARENT_DIR/cpx/inc"
 CPX_BIN_DIR="$PRJ_PARENT_DIR/build/cpx"
 UTILS_BIN_DIR="$PRJ_PARENT_DIR/build/util"
 UTILS_INCLUDES_DIR="$PRJ_PARENT_DIR/util"
 CPX_SCRIPTS_DIR="$PRJ_PARENT_DIR/cpx/scripts"
 source "$CPX_SCRIPTS_DIR/CPX-common.sh"
########################################################

# compiler tools selection
HASH_PREPROCESSOR=g++
#compiler configuration
CPPCOMPILER=g++
#Notice that there are some small issues with clang++ but I got it working with small workarrounds
#CPPCOMPILER=clang++
WRAPPER="nice -20 ccache"  # use empty value if not used, ccache effectiveness needs to increase before using it
#WRAPPER="nice -20"  # use empty value if not used
# STD_OF_CPP="c++14" # uncomment desired standard selection, c++14 should be default
#STD_OF_CPP="c++1z"   # see man gcc , search: -std   near line 817
STD_OF_CPP="gnu++1z"  # experimental c++17

########################################################

# commandline argument processing
CMDARGS_ERROR=16
# initialize from commandline arguments
if test $# -ne 5
then printf "$0: commandline argument retrieval failed. \
              ${HRED}Got $# arguments, but expected 5 arguments: \
          <sourcefile> <source directory> \
          <logging ( = on|off ) > <rebuild ( = auto|off )>  <build_configuration_name>$NOCOLOR"
    exit "$CMDARGS_ERROR"
fi
SOURCE_FILE="$1"
TMP_varibale="$2"
SOURCE_DIR=${TMP_varibale%/}
LOGGING="$3"
REBUILD="$4"
CPX_BUILD_CONFIG="$5"

########################################################

# set 'default' values
PRE_PPSRC_OPTIONS="-P -H -nostdinc -march=native -O0 -std=$STD_OF_CPP"  # -P (no include directives)  -C (keep comments)
PRE_SRC_OPTIONS="-march=native -save-temps -ggdb -H -O0 -DDEBUG -std=$STD_OF_CPP -fconcepts -fopenmp -Wall -Wextra -fmax-errors=50 -fdiagnostics-color=always"
LINK_LIBRARIES_OPTIONS="-lutil -lstdc++fs -lpthread -ldl -latomic"

# selecting configuration
case $CPX_BUILD_CONFIG in
 default)
 ;;
 debugging) # TBD not executing, instead load app into debugger, for now it behaves like 'default' and no error message is triggered
 ;;
 release)
    # note: tbd: release version link of util library, turning off DEBUG might fail, util:CMakeLists.txt is only made for debugging
    PRE_PPSRC_OPTIONS="-P -nostdinc -march=native -H -s -Ofast -DDEBUG -std=$STD_OF_CPP"  # -P (no include directives)  -C (keep comments)
    PRE_SRC_OPTIONS="-march=native -save-temps -H -s -Ofast  -DDEBUG -std=$STD_OF_CPP -fconcepts -fopenmp -Wall -Wextra -fmax-errors=50 -fdiagnostics-color=always"
    LINK_LIBRARIES_OPTIONS="-lutil -lstdc++fs -lpthread -ldl -latomic"
 ;;
 *)
    printf "$HRED Aborted $0: CPX_BUILD_CONFIG has invalid (not implemented) value '$CPX_BUILD_CONFIG' $NOCOLOR"
    exit -1
 ;;
esac

PRE_TARGET_OPTIONS="-rdynamic -fuse-ld=gold -Winvalid-pch"
PREPROC_INCLUDE_DIR="$UTILS_INCLUDES_DIR"
PREPROC_INCLUDE="stdinc.hpp"
 PREPROC_INCLUDE_PATH="$PREPROC_INCLUDE_DIR/$PREPROC_INCLUDE"
LINK_LIBRARIES_OPTIONS="-lutil -lstdc++fs -lpthread -ldl -latomic"

#cpx specifics
LOG_FILE="CPX-runner.log"
TMP_varibale="${CPX_WORK_DIR:-/tmp/cpx}"
WORK_DIR=${TMP_varibale%/}
 LOG_PATH="$WORK_DIR/$LOG_FILE"
 WORK_PATH_PREFIX="$WORK_DIR/CPX-"
WORK_INPUT="$SOURCE_FILE";
 WORK_INPUT_UNCHANGED_HPP="$WORK_INPUT.unchanged.hpp"
 TMP_PP_WORK_PATH="$WORK_INPUT.prehash"

EXIT_VALUE=99 # set to some invalid value and MUST be overwritten before exitting
IS_COMPILED=64 # exit value when there is no need to compile (the value should NOT be returned by the compiler)

##### Functions #####

maybe_echo() {
    test "$LOGGING" = "on" && echo "$*"
}

maybe_printf() {
    test "$LOGGING" = "on" && printf "$*"
}

# TBD: seems overwriting CPX-common.sh version
logged_eval () {
    local label
    label=$1
    shift 1
    # the raw command is presented above the label line. To debug, execute it from the commandline.
    if test "$LOGGING" = "on"
    then maybe_printf "$YELLOW---[$label]------------------------------------------------------$NOCOLOR\n"
    else echo "$label"
    fi
    maybe_echo "$*"
    maybe_echo ""
    # eval output is presented in the log below the 'label' line
    eval "$*"
}

CPX_cleanup () {
  rm CPX-*temp* CPX-*.o  2>/dev/null  # CPX-*.s
  cd - 2>&1 > /dev/null
}

OnExit () {
  trap "" EXIT
  CPX_cleanup
  return "$EXIT_VALUE"
}

trap OnExit EXIT

dumpvars ()
{
    local color
    local vars
    local cmd
    echo ''
    color=$1
    vars=$2
    set -f  # disable filename expansion
    for n in $vars
    do
        cmd="maybe_echo $n=\$color\\'\$$n\'"
        eval $cmd
    done
    set +f # re-enable filename expansion
    echo ''
}


beatify_ii() # $1 is finame  Beatify raw preprocessed C++ code
{
    local ii="${1%.*}.ii"
    maybe_printf "beatified  $HCYAN$ii$NOCOLOR  to check raw preprocessed C++ code\n"

    # customize beatify here, Chooseing speed and simplicity over beatified style, but ensure pretty readable identing
    # indent "$ii" & disown   # error: cant handle C++>=11 syntax (lambda e.g)
    astyle --quiet --style=stroustrup -c "$ii" & disown # ii file is not used so run a concurrent detached subshell
}


print_included_files_without_missing_guards_warning()  # $1 = filename
{
    awk '/Multiple include guards may be useful for:/ {nextfile} {print}' "$1"
}

#
# end of definitions and functions
#
########################################################
########################################################
########################################################
#
# From here the action of the script statrts
#

if test -f "$LOG_PATH"
then
    ACTION_CNT=$(gawk '
    BEGIN {
        cnt=1
    }
    /^-{8,8}[^-]*$/ {
        ++cnt
    }
    END {
        print cnt
    }' "$LOG_PATH" )

else
    ACTION_CNT="1"
fi

start_dir=$(pwd)
maybe_echo "Current directory: $start_dir"

cd $WORK_DIR

# if given configuration changes, it has to trigger a rebuild, conviently let serveral editors/vieuwers select correct syntax highlight
echo "#define CPX_BUILD_CONFIG \"$CPX_BUILD_CONFIG\" /* -*- C++ -*- syntax highlight */" > "$TMP_PP_WORK_PATH"

# construct Preprocessor CoMmanD
PCMD="$WRAPPER $HASH_PREPROCESSOR -Werror -DSTDINC_PRECOMPILED $PRE_PPSRC_OPTIONS \
 -I $SOURCE_DIR -I $CPX_INCLUDES_DIR -I $UTILS_INCLUDES_DIR \
 -E $WORK_INPUT >> $TMP_PP_WORK_PATH 2>$TMP_PP_WORK_PATH.errors"

# run and handle errors
logged_eval "hash phase" "$PCMD"
PREPROC_STATUS=$?
if test "$PREPROC_STATUS" -ne 0
then
    maybe_printf "$HRED=== Preprocessing failed ===$NOCOLOR\n"
    cat "$TMP_PP_WORK_PATH.errors"
    CPX_cleanup
    exit "$PREPROC_STATUS"
else
    print_included_files_without_missing_guards_warning "$TMP_PP_WORK_PATH.errors"
fi

# create unique source specific hash by which it detects existance of prior created executable for that source
LS_PATTERNS="$0 $UTILS_BIN_DIR/libutil.a $CPX_BIN_DIR/cpx $UTILS_INCLUDES_DIR/* $PREPROC_INCLUDE_PATH* $CPX_INCLUDES_DIR/*"
 nice -11 ls -lrta --full-time $LS_PATTERNS >> $TMP_PP_WORK_PATH
HASH=$("$PRJ_PARENT_DIR/build_cpx_apps/fingerprint3" < "$TMP_PP_WORK_PATH")

if test "$?" -ne 0
then
    maybe_printf "$HRED=== failed to produce HASH ===$NOCOLOR\n"
    exit 1
fi

TARGET_PROG="$WORK_PATH_PREFIX$HASH.cpp.x"

maybe_printf "$YELLOW---------------------------------------------------------------------$NOCOLOR\n"

if true
then
    dumpvars '' "SOURCE_FILE SOURCE_DIR CPX_WORK_DIR WORK_DIR LOGGING REBUILD CPX_BUILD_CONFIG \
        PRJ_PARENT_DIR CPX_INCLUDES_DIR UTILS_INCLUDES_DIR UTILS_BIN_DIR \
        PREPROC_INCLUDE_PATH CPX_STACKLEVELCOUNT WORK_INPUT WORK_INPUT_UNCHANGED_HPP \
        LOG_PATH HASH TARGET_PROG ACTION_CNT LS_PATTERNS \
        PRE_PPSRC_OPTIONS PRE_SRC_OPTIONS LINK_LIBRARIES_OPTIONS"
fi

W_ERROR=""  # " -Werror "

if test "$REBUILD" = "force" ||  test ! -f "$TARGET_PROG"
then
    maybe_echo "=== Compiling $SOURCE_FILE ==="
    maybe_printf "$HBLUE(~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~$NOCOLOR\n"
    CCMD="\
 $WRAPPER $CPPCOMPILER $W_ERROR -DSTDINC_PRECOMPILED \
 $PRE_SRC_OPTIONS \
 -include  $PREPROC_INCLUDE_PATH \
 -include $WORK_INPUT_UNCHANGED_HPP \
 -I $SOURCE_DIR -I $CPX_INCLUDES_DIR -I $UTILS_INCLUDES_DIR \
 $PRE_TARGET_OPTIONS\
 -o $TARGET_PROG -x c++ $WORK_INPUT \
 -L $UTILS_BIN_DIR $LINK_LIBRARIES_OPTIONS 2>$WORK_INPUT.errors"

    logged_eval "build phase" "$CCMD"
    COMPILE_RESULT=$?

    beatify_ii "$WORK_INPUT"
    # CPX_cleanup
    maybe_printf "$HBLUE)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~$NOCOLOR\n"

    if test "$COMPILE_RESULT" -ne 0
    then
       cat  "$WORK_INPUT.errors"
    else
        print_included_files_without_missing_guards_warning  "$WORK_INPUT.errors"
    fi

    echo "compiling $TARGET_PROG $COMPILE_RESULT $ACTION_CNT"
    EXIT_VALUE=$COMPILE_RESULT
    exit $COMPILE_RESULT
fi
# there was no compilation required.
# However it is not an error and we want to distinguish
# between error values ogf the compilation and this case.
# (this presumes that chache g++ ... never returns 64
# So if it is not required to use this value, do not!)
# CPX_cleanup
maybe_printf "$HGREEN=====================================================================$NOCOLOR\n"
echo "available $TARGET_PROG $IS_COMPILED $ACTION_CNT"
EXIT_VALUE=$IS_COMPILED
exit $IS_COMPILED

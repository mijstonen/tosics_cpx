# must be sh (man 1 dash) compatible
########################################################

# files and directories / depended settings
PRJ_PARENT_DIR="/home/michel/Projects/Kdevelop/"
 CPX_INCLUDES_DIR="${PRJ_PARENT_DIR}cpx/inc/"
 CPX_BIN_DIR="${PRJ_PARENT_DIR}build/cpx/"
 UTILS_BIN_DIR="${PRJ_PARENT_DIR}build/util/"
 UTILS_INCLUDES_DIR="${PRJ_PARENT_DIR}util/"
 CPX_SCRIPTS_DIR="${PRJ_PARENT_DIR}cpx/scripts/"
 . ${CPX_SCRIPTS_DIR}CPX-common.sh

CMDARGS_ERROR=16
# initialize from commandline arguments
if test $# -ne 4
then echo "$0: commandline argument retrieval failed." \
          "    ${HRED}Got $# arguments, but expected 4 arguments:" \ 
          "<sourcefile> <source directory>" \
          "<logging ( = on|off ) > <rebuild ( = auto|off )>${NOCOLOR}"
    exit "$CMDARGS_ERROR"
fi
SOURCE_FILE="$1"
SOURCE_DIR="$2"
LOGGING="$3"
REBUILD="$4"


########################################################

HASH_PREPROCESSOR=g++
#compiler configuration
CPPCOMPILER=g++ 
#Notice that there are some small issues with clang++ but I got it working with small workarrounds
#CPPCOMPILER=clang++
WRAPPER="nice -20 ccache"  # use empty value if not used, ccache effectiveness needs to increase before using it
#WRAPPER="nice -20"  # use empty value if not used
PRE_PPSRC_OPTIONS="-C -H -nostdinc -march=native -O0 -std=c++14"  # -P (no include directives)  -C (keep comments)

PRE_SRC_OPTIONS="-march=native -save-temps -DDEBUG -ggdb -H -O0 -DDEBUG -std=c++14 -Wall -Wextra -fdiagnostics-color=always"
# -Wno-unused

PRE_TARGET_OPTIONS="-rdynamic -fuse-ld=gold -Winvalid-pch"
  PREPROC_INCLUDE_DIR="${UTILS_INCLUDES_DIR}"
PREPROC_INCLUDE="stdinc.hpp"
 PREPROC_INCLUDE_PATH="${PREPROC_INCLUDE_DIR}${PREPROC_INCLUDE}"
LINK_LIBRARIES_OPTIONS="-lutil -lstdc++fs -lpthread -ldl"

# NOTE: if util is renewed, recompilation will be issued, see gawk (below) how DepsProces effects the HASH

#cpx specifics
LOG_FILE="CPX-runner.log"
 WORK_DIR="${CPX_WORK_DIR:-/tmp/cpx/}"
  LOG_PATH="${WORK_DIR}${LOG_FILE}"
 
WORK_PATH_PREFIX="${WORK_DIR}CPX-"
WORK_INPUT="${SOURCE_FILE}";
WORK_INPUT_UNCHANGED_HPP="$WORK_INPUT.unchanged.hpp"
TMP_PP_WORK_PATH="${WORK_INPUT}.prehash"

EXIT_VALUE=99 # set to some invalid value and MUST be overwritten before exitting
IS_COMPILED=64 # exit value when there is no need to compile (the value should NOT be returned by the compiler)

##### Functions #####

maybe_echo() {
    test "$LOGGING" = "on" && echo "$*"
}

maybe_printf() {
    test "$LOGGING" = "on" && printf "$*"
}

logged_eval () {
    local label
    label=$1
    shift 1
    # the raw command is presented above the label line. To debug, execute it from the commandline.
    if test "$LOGGING" = "on"
    then maybe_printf "${YELLOW}---[$label]------------------------------------------------------${NOCOLOR}\n"
    else echo "$label"
    fi
    maybe_echo "$*"
    maybe_echo ""
    # eval output is presented in the log below the 'label' line
    eval "$*"
}

CPX_cleanup () {
  local tempFiles
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


# ansi color management

  PCMD=" \
$WRAPPER ${HASH_PREPROCESSOR} -Werror -DSTDINC_PRECOMPILED $PRE_PPSRC_OPTIONS \
 -fdirectives-only \
 -I ${SOURCE_DIR} -I ${CPX_INCLUDES_DIR} -I ${UTILS_INCLUDES_DIR} \
 -C -E $WORK_INPUT > $TMP_PP_WORK_PATH 2>$TMP_PP_WORK_PATH.errors"
logged_eval "hash phase" "$PCMD"
PREPROC_STATUS=$?

if test "$PREPROC_STATUS" -ne 0
then
    maybe_printf "${HRED}=== Preprocessing failed ===${NOCOLOR}\n"
    cat "$TMP_PP_WORK_PATH.errors"
    CPX_cleanup
    exit "$PREPROC_STATUS"
else
    cat "$TMP_PP_WORK_PATH.errors"
fi

# if test "$LOGGING" = "on"
# then KEEP_HASH_INPUT="tee ${WORK_INPUT}-$PPID.hash | "
# else KEEP_HASH_INPUT=""
# fi

LS_PATTERNS="$0 ${UTILS_BIN_DIR}libutil.a ${CPX_BIN_DIR}cpx ${PREPROC_INCLUDE_PATH}* ${CPX_INCLUDES_DIR}*"
 HASH=$( nice -12 gawk -v HashProces="nice -11 cppstripws" \
             -v DepsProces="nice -11 ls -lrtha --full-time ${LS_PATTERNS}" \
    '
    #-1-------- disable P for non-locals injected by cpp or any unchanging include file directory
        $1=="#" && $2 ~ "^[[:digit:]]+$" { #1
            P=!($3 ~ /"<.*>"|"\/usr\/include\/.*"|\/Projects\/Kdevelop\/cpx\/inc\/cpx-|\/Projects\/Kdevelop\/util\// )
        } #1

    
    #-3-------- if it is not a directive, pass line to hash proces if P is enabled
        P { #3
            if (  ($1 !~ "^#.*") || ($1 ~ "^#.*" && $2 !~ "^[[:digit:]]+$") ) {
                print $0 | HashProces
            }
        } #3

    #---- debugging output
    #{   printf("%-2d:%s\n",P,$0) > "awk_dbg.txt"
    #}
    
    
    END {
        while ( (DepsProces | getline )>0 ) {
           print | HashProces
        }
    }
    ' "$TMP_PP_WORK_PATH"
)
# gawk: What is used to calc the hash: -v HashProces="tee input_to.hash | md5sum | cut -c -32" 
# gawk: extra arg when enabling #-2- -v discarded_before_hash="discarded_before.hash" \
    #-2-------- copy line to file if P is disabled
    #    { #2
    #        if(!P) {
    #            print $0 > discarded_before_hash
    #         }
    #    } #2
 #
# Uncomment below to check outcome of the gawk process, see log outpu
# echo "$HASH"
# exit 3


if test "$?" -ne 0
then
    maybe_printf "${HRED}=== gawk failed to produce HASH ===${NOCOLOR}\n"
    CPX-cleacleanup
    return 1
fi

SOURCE2COMPILE="${WORK_PATH_PREFIX}${HASH}.cpp"
TARGET_PROG="${SOURCE2COMPILE}.x"



maybe_printf "${YELLOW}---------------------------------------------------------------------${NOCOLOR}\n"
if true
then
    dumpvars '' "CPX_WORK_DIR WORK_DIR SOURCE_FILE \
        SOURCE_DIR PRJ_PARENT_DIR CPX_INCLUDES_DIR \
        UTILS_INCLUDES_DIR UTILS_BIN_DIR PREPROC_INCLUDE_PATH \
        CPX_STACKLEVELCOUNT WORK_INPUT WORK_INPUT_UNCHANGED_HPP 
        LOG_PATH HASH TARGET_PROG ACTION_CNT LS_PATTERNS"
fi

if test "$REBUILD" = "force" ||  test ! -f "${TARGET_PROG}"
then
    maybe_echo "=== Compiling ${SOURCE_FILE} ==="
    maybe_printf "${HBLUE}(~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~${NOCOLOR}\n"
    CCMD="\
 $WRAPPER ${CPPCOMPILER} -Werror -DSTDINC_PRECOMPILED \
 $PRE_SRC_OPTIONS \
 -include  $PREPROC_INCLUDE_PATH \
 -include $WORK_INPUT_UNCHANGED_HPP \
 -I ${SOURCE_DIR} -I ${CPX_INCLUDES_DIR} -I ${UTILS_INCLUDES_DIR} \
 $PRE_TARGET_OPTIONS\
 -o $TARGET_PROG -x c++ ${WORK_INPUT} \
 -L ${UTILS_BIN_DIR} $LINK_LIBRARIES_OPTIONS 2>&1"

    logged_eval "build phase" "$CCMD"
    COMPILE_RESULT=$?
    # CPX_cleanup
    maybe_printf "${HBLUE})~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~${NOCOLOR}\n"
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
maybe_printf "${HGREEN}=====================================================================${NOCOLOR}\n"
echo "available $TARGET_PROG $IS_COMPILED $ACTION_CNT"
EXIT_VALUE=$IS_COMPILED
exit $IS_COMPILED

    # must be sh (man 1 dash) compatible
########################################################

# files and directories / depended settings

THIS_FILE=$(basename $0)

#PRJ PARENT DIR and CPX INCLUDES DIR set in $HOME/.tosics_custom
  CPX_BIN_DIR="$PRJ_PARENT_DIR/build/cpx"
  CPXAPPS_BIN_DIR="$PRJ_PARENT_DIR/build_cpx_apps"
  UTILS_BIN_DIR="$PRJ_PARENT_DIR/build/util"
  UTILS_INCLUDES_DIR="$PRJ_PARENT_DIR/util"
  CPX_SCRIPTS_DIR="$PRJ_PARENT_DIR/cpx/scripts"
   source "$CPX_SCRIPTS_DIR/CPX-common.sh"
  PHP_INI_PATH="$PRJ_PARENT_DIR/cpx/etc/cpx_php.ini"
  BEATIFY_CONFIG_PATH="$PRJ_PARENT_DIR/cpx/etc/clang-format"

########################################################

# compiler tools selection
HASH_PREPROCESSOR=g++
#compiler configuration
CPPCOMPILER=g++
#Notice that there are some small issues with clang++ but I got it working with small workarrounds
#CPPCOMPILER=clang++
#WRAPPER="nice -20 ccache"  # use empty value if not used, ccache effectiveness needs to increase before using it
#WRAPPER="nice -20"  # use empty value if not used
WRAPPER=' '
# STD_OF_CPP="c++14" # uncomment desired standard selection, c++14 should be default
# deprecated STD_OF_CPP="c++1z"   # see man gcc , search: -std   near line 817
# STD_OF_CPP="c++14"
# STD_OF_CPP="gnu++17"  # experimental c++17
STD_OF_CPP="gnu++23"  # experimental c++23

HASH_PROG="$PRJ_PARENT_DIR/build_cpx_apps/hasher"
# use same symlink as on commandline, sourcelines link can be replaced by a test versions instead of the apps version

# normal use, outcomment when 'CPX-build sourcelines'
SOURCELINES_PROG="/usr/local/bin/sourcelines"

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
export CPX_SOURCE_FILE="$1"
TMP_variable="$2"
export SOURCE_DIR=${TMP_variable%/}
LOGGING="$3"
REBUILD="$4"
CPX_BUILD_CONFIG="$5"
# TBD: PRE_ARGS="$6"

########################################################

# exerimental options (should usually be blanc)
TRY_OPTIONS=""
#TRY_OPTIONS="-fpermissive"

# set 'default' values
PRE_PPSRC_OPTIONS="-v -H -nostdinc -march=native -O0 -std=$STD_OF_CPP"  # -P (no include directives)
PRE_SRC_OPTIONS="-v -march=native -save-temps -ggdb -H -O0 -DDEBUG -std=$STD_OF_CPP $TRY_OPTIONS -ftemplate-depth=900 -fconcepts -fopenmp -Wall -Wextra -fmax-errors=50 -fdiagnostics-color=always"
LINK_LIBRARIES_OPTIONS="-lutil -lstdc++fs -lpthread -ldl -latomic"
PHP_INCLUDE_PATH="$SOURCE_DIR:$CPX_INCLUDES_DIR:/usr/share/php8:/usr/share/php/PEAR"

# selecting configuration
case $CPX_BUILD_CONFIG in
 default)
 ;;
 debugging) # TBD not executing, instead load app into debugger, for now it behaves like 'default' and no error message is triggered
 ;;
 release)
    # note: tbd: release version link of util library, turning off DEBUG might fail, util:CMakeLists.txt is only made for debugging
    PRE_PPSRC_OPTIONS="-nostdinc -march=native -H -s -Ofast -DDEBUG -std=$STD_OF_CPP"  # -P (no include directives)
    PRE_SRC_OPTIONS="-march=native -save-temps -H -s -Ofast  -DDEBUG -std=$STD_OF_CPP -fconcepts -fopenmp -Wall -Wextra -fmax-errors=50 -fdiagnostics-color=always"
    LINK_LIBRARIES_OPTIONS="-lutil -lstdc++fs -lpthread -ldl -latomic"
    #SOURCELINES_PROG="$PRJ_PARENT_DIR/build_cpx_apps/sourcelines --wordmap"
    # outcommented normally, use when 'CPX-build sourcelines'
    SOURCELINES_PROG="$PRJ_PARENT_DIR/build_cpx_apps/FALLBACK_sourcelines --wordmap"
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
  # create unique source specific hash by which it detects existance of prior created executable for that source
  LS_PATTERNS="$0 $SOURCE_DIR $UTILS_BIN_DIR $CPX_BIN_DIR $CPXAPPS_BIN_DIR $UTILS_INCLUDES_DIR $CPX_INCLUDES_DIR"
LINK_LIBRARIES_OPTIONS="-lutil -lstdc++fs -lpthread -ldl -latomic"

#cpx specifics
LOG_FILE="CPX-runner.log"
TMP_varibale="${CPX_WORK_DIR:-/tmp/cpx}"
WORK_DIR=${TMP_varibale%/}
 LOG_PATH="$WORK_DIR/$LOG_FILE"
 WORK_PATH_PREFIX="$WORK_DIR/CPX-"
WORK_INPUT="$CPX_SOURCE_FILE";
 WORK_INPUT_UNCHANGED_HPP="$WORK_INPUT.unchanged.hpp"
 TMP_PP_WORK_PATH="$WORK_INPUT.prehash"
 TMP_TRANSFORMED_WORK_PATH="$WORK_INPUT.transformed"

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

amalgamate() # $1 is input ii file, remove most of manual formatting before automated beautify
{
    awk '
    BEGIN { priorEmptyLine=0;
    }
    NF==0 { # skip multiple empty lines, allow only one
        if (!priorEmptyLine) printf("\n");
        ++priorEmptyLine;
        next;
    }
    # othwhise
        { # NF!=0
       priorEmptyLine = 0;   # resetted
    }
    $1 == "#" { # suppres line directives, but not compiler messaging directives
        next
    }
    /"/ { # Unmodified print of lines with look to have string literals
        print
        next
    }
    { # lines unhandled so far are printed with reduced white spaces in the line
        for (i=1;i<NF;++i) {
            printf("%s ",$i)
        }
        printf("%s\n",$i)
    }
    ' "$1"
}

beautify_ii() # $1 is fiename  Beatify raw preprocessed C++ code
{
    ( # Do not delay cpx execution, this part runs in parallel while cpx continues to executing the cpx C++ script
      # Note that time benefits depend on (current) available (processor) resources.
        sleep 0.5         # give the parent proces time to continue and complete before starting.
        cd $WORK_DIR
        local ii="${1%.*}.ii"
        local output="$ii.beautified"
        ii="$(basename $TARGET_PROG)-$ii"  # Compiler seems to prefix intermediate names with the target name.
        # maybe_printf "beautified $HCYAN $ii $NOCOLOR \n to $HCYAN $output $NOCOLOR to check raw preprocessed C++ code\n"
        # ensure access to the clang format configuration file, create the symlink if it was is not there
        if  [ ! -L .clang-format ]; then
            ln -sf "$BEATIFY_CONFIG_PATH" .clang-format
        fi
        echo "// -*- C++ -*- " > "$output"  # triggers syntax coloring for several editors and viewers
        astyle --quiet --style=stroustrup -c < "$ii" >> "$output"
        # Hot fixed (activated astyle), need clang 12, incompatible format configuration file
        # amalgamate "$ii" |  clang-format >> "$output" &
        $SOURCELINES_PROG < "$ii" > "$CPX_SOURCE_FILE.sourcelines" &
    ) & disown -ahr   # decouple from parent by the disown command
}


print_included_files_without_missing_guards_warning()  # $1 = filename
{
    awk '
    BEGIN {
        guards_warning=0
    }
    /Multiple include guards may be useful for:/ {
        guards_warning=1;
        next;
    }
    $1 ~ "^/usr/" { # a bit of a fake solution, but supresses the message for system include files.
        if ( guards_warning==1 ) {
            next;
        }
        #else
        print "Missing guards in: " $0;
    }
    {
        print
    }
   ' "$1"
}

function getHashInputData {
    cat "$TMP_PP_WORK_PATH"
    ls -lrta --full-time $LS_PATTERNS
}

function runHashProg {
    # set variable in context of running the hash program
    CPX_VALIDATION_POLICY=disabled "$HASH_PROG"
}

injectPriorToSourceCode()  # written to C/++ output file, by encapsulating it into a function, it can be done with a single file redirection
{
    export CPX_VALIDATION_HASH=$(rev "$WORK_INPUT" | runHashProg )
    echo "<? declare(strict_types=1); namespace Cpx; \$PHP_started=microtime(true); ?> //-*- C++ -*-"
    echo "# 3 \"$THIS_FILE\""

    # Here below is only required if runtime validation is needed (aka: a lot of different cpx binaries in the cache over a long time)
    # The code written to cpx-precmdproces_hash_compile-invoked.hpp is only meant for checking the generated code.
    # It is not good enough for including, then better write to standard out so it will be specific for the compiled app.
    (
    echo "#define CPX_BUILD_CONFIG \"$CPX_BUILD_CONFIG\" "
    echo "#define CPX_SOURCE_FILE \"$CPX_SOURCE_FILE\" "
    echo "#ifdef CPX_VALIDATION_HASH"
    echo "# undef CPX_VALIDATION_HASH"
    echo "#endif"
    if [ "$CPX_BUILD_CONFIG" != 'release' ]
    then
        echo "#define CPX_VALIDATION_HASH \"$CPX_VALIDATION_HASH\" "
    else
        echo "/* 'release' configuration: no CPX VALIDATION HASH */"
    fi
    ) > "cpx-precmdproces_hash_compile-invoked.hpp"
    # to make it work, link from a Dir include path to cpx-precmdproces_hash_compile-invoked.hpp in $CPX_WORK_DIR
    # heh??? or simply remove braces and file redirection. What's up?
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

# pathlines
echo $PATH|while read -d ':' d; do echo $d;done

dumpvars '' "0    PRE_ARGS    CPX_HASH_COMPILE    CPX_SOURCE_FILE SOURCE_DIR \
    PHP_INCLUDE_PATH    PHP_INI_PATH    BEATIFY_CONFIG_PATH    CPX_WORK_DIR  \
    WORK_DIR    LOGGING    REBUILD    CPX_BUILD_CONFIG    PRJ_PARENT_DIR \
    CPX_INCLUDES_DIR    UTILS_INCLUDES_DIR    UTILS_BIN_DIR    PREPROC_INCLUDE_PATH \
    CPX_STACKLEVELCOUNT    WORK_INPUT    TMP_PP_WORK_PATH    TMP_TRANSFORMED_WORK_PATH \
    WORK_INPUT_UNCHANGED_HPP    LOG_PATH LS_PATTERNS    PRE_PPSRC_OPTIONS \
    PRE_SRC_OPTIONS    LINK_LIBRARIES_OPTIONS    HASH_PROG    SOURCELINES_PROG"

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


injectPriorToSourceCode > "$TMP_PP_WORK_PATH"

# construct Preprocessor CoMmanD
PCMD="($WRAPPER $HASH_PREPROCESSOR -Werror -DSTDINC_PRECOMPILED -C $PRE_PPSRC_OPTIONS \
       -I $CPX_WORK_DIR -I $SOURCE_DIR -I $CPX_INCLUDES_DIR -I $UTILS_INCLUDES_DIR \
       -E $WORK_INPUT \
       )>> $TMP_PP_WORK_PATH 2>$TMP_PP_WORK_PATH.errors"

# run and handle errors
logged_eval "hash phase" "$PCMD"
PREPROC_STATUS=$?
if test "$PREPROC_STATUS" -ne 0
then
    maybe_printf "$HRED=== Preprocessing failed ===$NOCOLOR\n"
    #cat "$TMP_PP_WORK_PATH.errors"
    print_included_files_without_missing_guards_warning "$TMP_PP_WORK_PATH.errors"
    CPX_cleanup
    exit "$PREPROC_STATUS"
else
    print_included_files_without_missing_guards_warning "$TMP_PP_WORK_PATH.errors"
fi

HASH=$(getHashInputData | runHashProg)

if test "$?" -ne 0
then
    maybe_printf "$HRED=== failed to produce HASH ===$NOCOLOR\n"
    exit 1
fi

TARGET_PROG="$WORK_PATH_PREFIX$HASH.cpp.x"

maybe_printf "$YELLOW---------------------------------------------------------------------$NOCOLOR\n"
dumpvars '' "HASH    TARGET_PROG    ACTION_CNT    CPX_VALIDATION_HASH"
W_ERROR=""  # " -Werror "

if test "$REBUILD" = "force" ||  test ! -f "$TARGET_PROG"
then
    # Run php with default cli except for specific configuration on commandline
    maybe_printf  "$HGREEN=== Transforming $CPX_SOURCE_FILE with php ===$NOCOLOR\n"
    (
        cd $start_dir
        php --no-php-ini --php-ini "$PHP_INI_PATH" \
            --no-chdir --no-header -d variables_order="E" -d short_open_tag=On \
            -d include_path="$PHP_INCLUDE_PATH" "$WORK_DIR/$TMP_PP_WORK_PATH"
    ) 2> "$TMP_TRANSFORMED_WORK_PATH.errors" 1> "$TMP_TRANSFORMED_WORK_PATH"
    TRANSFORM_STATUS=$?
    cat $TMP_TRANSFORMED_WORK_PATH.errors # can also contain php warnings or trace messages and should be printed always
    if test "$TRANSFORM_STATUS" -ne 0
    then
        maybe_printf "$HRED=== transformation with php failed ===$NOCOLOR\n"
        CPX_cleanup
        exit "$TRANSFORM_STATUS"
    fi

    maybe_echo "=== Compiling $CPX_SOURCE_FILE ==="
    maybe_printf "$HBLUE(~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~$NOCOLOR\n"
    CCMD="\
      $WRAPPER $CPPCOMPILER $W_ERROR -DSTDINC_PRECOMPILED \
      $PRE_SRC_OPTIONS \
      -include  $PREPROC_INCLUDE_PATH \
      -I $CPX_WORK_DIR -I $SOURCE_DIR -I $CPX_INCLUDES_DIR -I $UTILS_INCLUDES_DIR \
      -include $WORK_INPUT_UNCHANGED_HPP \
      $PRE_TARGET_OPTIONS \
      -o $TARGET_PROG -x c++ $TMP_TRANSFORMED_WORK_PATH \
      -L $UTILS_BIN_DIR $LINK_LIBRARIES_OPTIONS 2>$WORK_INPUT.errors"

    logged_eval "build phase" "$CCMD"
    COMPILE_RESULT=$?

    beautify_ii "$TMP_TRANSFORMED_WORK_PATH"
    # CPX_cleanup
    maybe_printf "$HBLUE)~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~$NOCOLOR\n"

    if test "$COMPILE_RESULT" -ne 0
    then
       maybe_printf "$HRED=== failed to compile ===$NOCOLOR\n"
    fi
    print_included_files_without_missing_guards_warning  "$WORK_INPUT.errors"

    echo "compiling $TARGET_PROG $CPX_VALIDATION_HASH $COMPILE_RESULT $ACTION_CNT"
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
echo "available $TARGET_PROG $CPX_VALIDATION_HASH $IS_COMPILED $ACTION_CNT"
EXIT_VALUE=$IS_COMPILED
exit $IS_COMPILED

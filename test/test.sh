# All programs here should compile and run
# they them self do not neccesarely produce correct output.

# set -e
# files and directories / depended settings
PRJ_PARENT_DIR="/home/michel/Projects/Kdevelop/"
 CPX_SCRIPTS_DIR="${PRJ_PARENT_DIR}cpx/scripts/"
 . ${CPX_SCRIPTS_DIR}CPX-common.sh

# NOTICE: the currect directory has to be in the path (use alias: cupas)
WORK_DIR="${CPX_WORK_DIR:-/tmp/cpx/}"
 WORK_DIR=$(heal_DIR WORK_DIR)
  LOGFILE="${WORK_DIR}CPX-runner.log"

CPX_WITH_OPTIONS=""

run_scripts () {
    # reference standard
    ${CPX_WITH_OPTIONS} dummy.cpp
    ${CPX_WITH_OPTIONS} cpx-empty.cpp
    ${CPX_WITH_OPTIONS} rnr_source2.cpp 3

    # mini programs
    ${CPX_WITH_OPTIONS} strhash.cpp
    ${CPX_WITH_OPTIONS} labelvaluezipper.cpp
    ${CPX_WITH_OPTIONS} boocircbuf.cpp
    ${CPX_WITH_OPTIONS} arg_output.cpp
    ${CPX_WITH_OPTIONS} strangeloop.cpp
    ${CPX_WITH_OPTIONS} outptroverload.cpp
    ${CPX_WITH_OPTIONS} varvresearch.cpp
    ${CPX_WITH_OPTIONS} txtsplit.cpp

    # programs with test input 
    # 1
    ${CPX_WITH_OPTIONS} bracecount.cpp <<EOF
{{{ }}
EOF
    ${CPX_WITH_OPTIONS} bracecount.cpp <<EOF
{{{ }}}}
EOF
    ${CPX_WITH_OPTIONS} bracecount.cpp <<EOF
{{{ }}}
EOF
    # 2
    ${CPX_WITH_OPTIONS} cppstripws.cpp  < cppstripws.cpp > cppstripws.test-output 

    # what is to be known about a pointer with a null value
    ${CPX_WITH_OPTIONS} stream_nullptr.cpp
    
    # taste
    # ${CPX_WITH_OPTIONS} taste-tosics_util.cpp
    
    # test what happens when compilation fails
    ${CPX_WITH_OPTIONS} compile-errors.cpp
    # deliberately fails to test backtrace
    echo ""
    echo "-------------------------------------------------------------------------------------"
    echo "forcefail: testing segmentation error"
    ${CPX_WITH_OPTIONS} forcefail.cpp
    echo "-------------------------------------------------------------------------------------"
    echo ""
    ${CPX_WITH_OPTIONS} hello_meta_world.cpp
    ${CPX_WITH_OPTIONS} colors.cpp
}


cycle () {
    CPX-clean-WORK_DIR

    # compile and run
    run_scripts

    # already compiled, only run
    run_scripts
}

from_commandline() {

# TODO: CPX_LOGGING_DEFAULT="on"

# as if CPX_LOGGING_DEFAULT="off"
    cpx <<EOF
#|
#(
    INFO("from_commandline: Default (logging off)");
#)
EOF


    cpx -q <<EOF
#|
#(
    INFO("from_commandline: Quiet (logging off)");
#)
EOF

if false
then
    cpx -v <<EOF
#|
#(
    INFO("from_commandline: Verbose (full logging)");
#)
EOF

fi


}


### RUN ###

echo $LOGFILE

if true
then
    # put in marker to simplify finding beginning of last test in logfile, back search _test_test_test_
    eval "echo \"testtesttest $(date)\" >> $LOGFILE"
    CPX_WITH_OPTIONS="cpx "
    cycle
    CPX_WITH_OPTIONS="cpx -q"
    cycle
fi

from_commandline

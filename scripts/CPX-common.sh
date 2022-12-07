# CPX-common.sh  sh compatible shell definitions, to be used (in the shell process) of other CPX-... scripts
# to load it use the following on top of the using script. This prestates that this script should be
# looked up in the same directory as the using script.
#
# |SCRIPTS_DIR="$(dirname $(realpath -P $0))/"
# |. "${SCRIPTS_DIR}CPX-common.sh"

PATH_SEPARATOR="/"
CPX_COMMOM_ERROR=240  # aka -16

# coloring constants
NOCOLOR="\033[00m"
RED="\033[00;31m"
GREEN="\033[00;32m"
CYAN="\033[00;36m"
YELLOW="\033[00;33m"
BLUE="\033[00;34m"
WHITE="\033[00;37m"

HRED="\033[01;31m"
HGREEN="\033[01;32m"
HCYAN="\033[01;36m"
HYELLOW="\033[01;33m"
HBLUE="\033[01;34m"
HWHITE="\033[01;37m"

RESET="\033[00m"

is_empty_var () {
    eval "test -z \"\$$1\""
}

warn_if_var_is_empty () {
    if is_empty_var $1
    then
        echo -e "${HYELLOW}WARNING: warn_if_var_is_empty(): variable $1 is empty${NOCOLOR}" >&2
    fi
}

heal_DIR () {
# Not supported on all shells, works with bash requires Parameter Expansion form: ${parameter: offset}

    local varname="$1"   # must be a environment variable name
    warn_if_var_is_empty $varname
    local cmd

    cmd="
    if [ '$PATH_SEPARATOR' != \"\${$varname: -1}\" ];
    then export $varname=\"\${$varname}/\" ;
    fi;
    echo \"\${$varname}\"
    "
    val_DIR=$(eval "$cmd")
    if [ "$val_DIR" = "$PATH_SEPARATOR" ]
    then
        echo -e "${HRED}ERROR in $0: heal_DIR: set $varname to / " \
                "(aka: The root directory) is considered invalid.${NOCOLOR}" >&2
        exit "$CPX_COMMOM_ERROR"
    fi

    if [ ! -d "$val_DIR" ]
    then echo -e "${HRED}ERROR in $0: heal_DIR: missing directory '${val_DIR}' ${NOCOLOR}" >&2
        exit "$CPX_COMMOM_ERROR"
    fi

    echo "$val_DIR"
}

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
        cmd="echo -e $n=\$color\\'\$$n\'"
        eval $cmd
    done
    set +f # re-enable filename expansion
    echo -e "$NOCOLOR"
}

logged_eval () {
    local label
    label=$1
    shift 1
    # the raw command is presented above the label line. To debug, execute it from the commandline.
    if test "$LOGGING" = "on"
    then printf "${YELLOW}---[$label]------------------------------------------------------${NOCOLOR}\n"
    else echo "$label"
    fi
    echo "$*"
    echo ""
    # eval output is presented in the log below the 'label' line
    eval "$*"
}


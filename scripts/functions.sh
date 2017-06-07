# functions.sh

showBoolOf() {
    eval "$*" && echo "True" || echo "False"
}



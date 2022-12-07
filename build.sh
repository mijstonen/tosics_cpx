
#
# Build with CMake generated makefile.
#
# VERBOSE=1 causes the makefile outputting the compilation command.
# That is used sometimes to replay the compilation from the console.
# Mostly to substitute some options by -E so we get the preprocessing output.
# This because sometimes compile errors can only be understood with all
# preprocessor macros expanded.
#
# Ofcause and in general   it is nice not to depend on a IDE for compilation.
# So when the repo is cloned, you can build that easily too.
# cmake and make suffices and this script lets you issue a customized
# build from the command line.
#
# See also:  compile-alias
#


prj_dir_name=$(basename $(pwd))

build_in_other_dir()
{
  cd "../build/$prj_dir_name"
  VERBOSE=1 ninja -v -j 8  $1
  ls -lrth --full-time --color=always
  cd -
}

mkdir -p ../build/$prj_dir_name/logs/
BUILDLOG="../build/$prj_dir_name/logs/build-$(timestamp).log"
build_in_other_dir 2>&1 | tee $BUILDLOG
less -R $BUILDLOG
ls -lrth build*.log | tail




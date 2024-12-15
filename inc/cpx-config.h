#ifndef CPX_CONFIG_H
#define CPX_CONFIG_H 1


  // or from environment, fall back on defines above
  #define ENVNAME_CPX_SCRIPTS_DIR "CPX_SCRIPTS_DIR"

  // separated list of directories
  #define ENVNAME_CPX_INCLUDES_DIR "CPX_INCLUDES_DIR"

  // default prefix for all files created by cpx
  #define WORK_PATH_PREFIX "CPX-"

  #define UNCHANGED_INCLUDES_POSTFIX ".unchanged.hpp"

  #define ENV_HOME_DIR "HOME"
  #define ENV_PATH "PATH"
  #define ENV_WORK_DIR "CPX_WORK_DIR"
  #define DEFAULT_WORK_DIR "/tmp/cpx/"
  #define ENV_CPX_STACKLEVELCOUNT "CPX_STACKLEVELCOUNT"
  #define ENV_CPX_HASH_COMPILE "CPX_HASH_COMPILE"
  #define ENV_CPX_VALIDATION_HASH "CPX_VALIDATION_HASH"

  // Optional, if set to a value of ApplicationValidator::validation_policy it is assigned to ApplicationValidator::myValidationPolicy
  #define ENV_CPX_VALIDATION_POLICY "CPX_VALIDATION_POLICY"

  #define DEFAULT_HASH_COMPILE "CPX-preproces_hash_compile.sh"
  #define LOGFILE_NAME_IDENTIFICATION "runner"
  #define LOGFILE_EXTENSION ".log"

  // limit process recursion, higher levels are likely indicate incorrect functioning of cpx
  #define MAX_CPX_STACKLEVELCOUNT 16

#endif //CPX_CONFIG_H

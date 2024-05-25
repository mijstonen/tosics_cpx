#!/usr/local/bin/cpx
#|
// Backtick PHP subprocess execution demo.
// have preprocessor (no func) macro inside embedded script


# define WRAP(txt) #txt
#(
    // In the embedded (awk) script; avoid comments (starting with #) at the beginning of the line since it is picked up by the cpp C/++ preprocessor.
    // $ES_COMMENT passes though the preprocessor but is expanded by PHP so it invokes the # in the awk and bash scripts to mark comments (or out commented code).
    // Unfornally most syntax highlightning won't play allong, keep C++ syntax highlitning or else "Normal"
    // Hint to improve script output in CPX-runner.log
    // 1 align scripts with heredoc tags (in this file AWKSCRIPT  and COMMANDS)
    // surround script(s) with newlines beginning after and ending before heredoc tags, so the (expanded) script lines are on separate lines in CPX-runner.log .

 PHP_BEGIN

    $ES_COMMENT = '#';
    $inputFile = "boocircbuf.cpp";

    $awkResults = subProcessScript(
        "word WRAPper", "awk"
      , // The script parameter string is stored in a temporary file so it  does not need escaping script start or termination characters
        <<< AWKSCRIPT

            NF==0 {
                next
            }
            \$1!="#" {
                for(i=1;i<=NF;++i){
                    printf(WRAP(<) "%s" WRAP(>),\$i);
                }
                print("'''");
            }
            $ES_COMMENT // Passing double quote remains nasty, it needs to be quoted 1. for php 2. for awk 3. for C/++
            END {
                printf("/* I like to use \\\\\\"\\\\\\"\\\\\\"\\\\\\" */");
            }

            AWKSCRIPT
      , // commands (usually, most likely only one) are in a shell command string ( sh -c '...commnds...' )
       <<< COMMANDS

           $ES_COMMENT cp -a %{<<<script} 2>/dev/null 1>/dev/null .   # preserve the awkscript, otherwise its deleted (except in CPX-runner.log)
            awk -f %{<<<script} %s $ES_COMMENT | tee awkResults.asc    # PHP_CMDFTSTR_SCRIPT_TAG-s are replaced by the temporary path of the (awk) script

           COMMANDS,                                       # The heredoc script string functions like a printf format string,
           $inputFile                                      # also substituted variables are subjected to that replacement
                                                           // Better put the variables in the varariable argument list (...), than it can't happen.
    ); // awkResults
    // generate C++ code
    echo "auto lines=",C_initializerListFrom(explode(PHP_EOL,$awkResults)),";",PHP_EOL;
 PHP_END

    // show the results
    for(auto line:lines){
        cout<< line<< '\n';
    }
#)

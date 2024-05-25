#!/usr/local/bin/cpx
#|
// Backtick PHP subprocess execution demo.
// have preprocessor (no func) macro inside embedded script

$AWK_SCRIPTOGLE="'";
# define AWK_END $AWK_SCRIPTOGLE
# define AWK_BEGIN awk $AWK_SCRIPTOGLE

# define WRAP(txt) #txt
#(

 PHP_BEGIN
    $awkResults = subProcess("awk word WRAPper",
        <<<EOS
            AWK_BEGIN

                NF==0 {
                    next
                }

                \$1!="#" {
                    for(i=1;i<NF;++i){
                        printf(WRAP(<) "%s" WRAP(>),\$i);
                    }
                    print(\$NF);
                }

            AWK_END /home/michel/Projects/test/bbc.cpp
           EOS);
    // generate C++ code
    echo "auto lines=",C_initializerListFrom(explode(PHP_EOL,$awkResults)),";",PHP_EOL;
 PHP_END

    // show the results
    for(auto line:lines){
        cout<< line<< '\n';
    }
#)

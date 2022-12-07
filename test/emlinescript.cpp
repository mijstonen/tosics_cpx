#!/usr/local/bin/cpx
#|
// have preprocessor (no func) macro inside embedded script
#define WRAP(txt) #txt
# define SINGLE_QUOTE "'"
#(
 PHP_BEGIN
    $AWK_SCRIPTOGLE=SINGLE_QUOTE;
    $lines=explode(PHP_EOL,shell_exec(<<< EOT
        awk $AWK_SCRIPTOGLE
            NF==0{
                next
            }
            \$1!="#"{
                for(i=1;i<NF;++i){
                    printf(WRAP(<) "%s" WRAP(>),\$i);
                }
                print(\$NF)
            }
        $AWK_SCRIPTOGLE /home/michel/Projects/test/bbc.cpp
        EOT));
    // PHP_VARDUMP_COMMENT(lines);
    echo "auto lines=",C_initializerListFrom($lines),";",PHP_EOL;
 PHP_END
    for(auto line:lines){
        cout<< line<< '\n';
    }
#)


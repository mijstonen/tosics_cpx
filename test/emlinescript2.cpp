#!/usr/local/bin/cpx
#|
// have preprocessor (no func) macro inside embedded script
#define WRAP(txt) #txt
#define SINGLE_QUOTE "'"

#define AWK_BEGIN awk $AWK_SCRIPTOGLE
#define AWK_END $AWK_SCRIPTOGLE
#(

 PHP_BEGIN
    $AWK_SCRIPTOGLE=SINGLE_QUOTE;
    $awkResults =`
            AWK_BEGIN
                NF==0{
                    next
                }
                \$1!="#"{
                    for(i=1;i<NF;++i .){
                        printf(WRAP(<) "%s" WRAP(>),\$i);
                    }
                    print(\$NF)
                }
            AWK_END /home/michel/Projects/test/bbc.cpp`
    ;
    if ( !$awkResults) {
        PHP_ERROR_DIE("Embedded awk script failed.");
    }

    //otherwhise
    $lines=explode(PHP_EOL,$awkResults);
    PHP_VARDUMP_COMMENT(lines);
    echo "auto lines=",C_initializerListFrom($lines),";",PHP_EOL;
 PHP_END

    for(auto line:lines){
        cout<< line<< '\n';
    }
#)


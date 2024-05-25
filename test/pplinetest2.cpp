#!/usr/local/bin/cpx
#|
 PHP_BEGIN
    CPPLineFileAndTokes( <<<EOT
#line   __LINE__   __FILE__
EOT);
    ob_start();
    foreach(repeat(10) as $i){

        echo "cout<< $i << \" times\\n\";";

    }
    $ar=ob_get_clean();
 PHP_END
#(


    INFO(FUNC_MSG(""));
 PHP_BEGIN
    print( <<<EOT
#@
    auto line=$CPP_LINE;
    auto fname=$CPP_FILE;
    INFO(FUNC_MSG(VARVALS(line,fname)));
    $ar

EOT);PHP_END
#)

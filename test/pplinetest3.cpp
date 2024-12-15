#!/usr/local/bin/cpx
#|
 PHP_BEGIN
    CPPLineFileAndTokes( <<<EOT
#line   __LINE__   __FILE__
EOT);
#@
    CPPLineFileAndTokes("# ". __LINE__ ." __FILE__ 3 4 5 6");
    ob_start();
    foreach($GLOBALS['CPP_TOKENS'] as $i){
        echo "cout<< $i << \"\\n\";";
    }
    $ar=ob_get_clean();
    if (true){ // (en|dis)able dump of globals in comment of intermediate file when when needed.
        echo "\n/* var_dump here\n";
        var_dump($GLOBALS['CPP_TOKENS']);
        echo "\n*/\n";
    }
 PHP_END
#(
    INFO(FUNC_MSG(""));
 PHP_BEGIN
    $php2c='Cpx\C_initializerListFrom';
    $optional_tokens_argument=count($GLOBALS['CPP_TOKENS'])>0?',tokens':'';
    print( <<<EOT
    auto line=$CPP_LINE;
    auto fname=$CPP_FILE;
    auto tokens={$php2c($GLOBALS['CPP_TOKENS'])};
#@  INFO(FUNC_MSG(VARVALS(line,fname$optional_tokens_argument)));
    $ar
EOT);
 PHP_END
#)

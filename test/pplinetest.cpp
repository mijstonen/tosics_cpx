#!/usr/local/bin/cpx
#|


<?
CPPLineFileAndTokes( <<<EOT
#line __LINE__ __FILE__
EOT
);
ob_start();
foreach(repeat(10) as $i){
    echo "//$i times\n";
}
$ar=explode("\n",ob_get_contents());
/*
 * var_dump
var_dump($ar);
*/
?>

#(
auto line=<?=$CPP_LINE?>;
auto fname=<?=$CPP_FILE?>;


INFO(VARVALS(line,fname));
#)

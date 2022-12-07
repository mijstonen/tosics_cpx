<?php
$keywords=array();
foreach(explode( PHP_EOL,file_get_contents("cplusplus_keywords.asc"))  as $w) {
    if (strlen($w)) {
        $keywords[]=$w;
    }
}
echo "{";
foreach( $keywords as $itemValue ){
    echo PHP_EOL.',"'.$itemValue.'"';
    if (--$itemsLeft) {
          echo ',';
    }
}
echo PHP_EOL.'};'.PHP_EOL;

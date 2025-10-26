#!/usr/local/bin/cpx
 // Proof of concept, using awk via php for compiletime processing.
#!
 <?>
 $AwkBegin="'";
 $AwkEnd="'";
 $awkResults = shell_exec(<<< EOT
 awk  $AwkBegin
   NF==0{next}
   \$1!="#" {for(i=1;i<NF;++i){
       printf("%s ",\$i);
  }
  print(\$NF)}
 $AwkEnd bbc.cpp
 EOT);
  PHP_VARDUMP_COMMENT(awkResults);
  // generate C++ code
    echo "auto lines=",C_initializerListFrom(explode(PHP_EOL,$awkResults)),";",PHP_EOL;
 <;>
    // show the results
    for(auto line:lines){
        cout<< line<< '\n';
    }

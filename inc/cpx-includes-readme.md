# CPX-INCLUDES

What are cpx includes ?

First, what's not! It is NOT the place to put your include files for your application.
Use standard #include "my own app specific file"

The include files here should be used to enhance and customize for __ALL__ C++ scripts
running through **cpx**. The main purpose is to avoid boilerplate code that could polute scripts.
So the first place you take counter measures against it.

CPX includes are drawn in automatically and by the micro preprocessor tags 
(see runner2.cpp //:runner:// -> // BEGIN micro preprocessor ).

The micro preprocessor does not inject code it self. Instead it includes those cpx includes 
and delegates the code generation. Also for a practical reason, the replacing code should 
not increase the number of lines in the script. 

CPX includes also differ in their form from normal includes. They may inject fragments of sources, 
depending on their needs.

## Example: main() 

```
#{
//  do your main expressions here

.....

#}

The mpp commands #{ and #} are replaced:

#include "cpx-main-definition-signature.hpp" //    #{ 
//  do your main expressions here

.....

#include "cpx-main-and-file-end.hpp"    //    #}
```

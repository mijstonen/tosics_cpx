#!/usr/local/bin/cpx


namespace 
{

struct EndOfStream{};


char isChar;
void get_char(istream& is)
{
    if ( !is ) throw EndOfStream();
    is.get(isChar);
}

} // namespace


#define ANY_SUFFICIENT_LARGE_ASCII_FILE "./getunget_testdata.txt" 

#!


 try {
 // PHP code generation repeats the inner code block several times
 //   each iteration is a new and independend test for $maxunget gets and ungets
 //   The range is from 1 to N (not starting at 0) because we want the inner loop running at least once.
 <?> for($maxunget=41;$maxunget<=100;++$maxunget) <{>
    {   
    INFO("\n<?=$maxunget?>\n----------\n"); 
    isChar=0xFF;  // clear current value so we are sure not to reuse it.
    ifstream testfile(ANY_SUFFICIENT_LARGE_ASCII_FILE);
    testfile.seekg(22);
    stack<char> char_stack;

    int count=0;
    for (; count< <?=$maxunget?>; ++count ) {
        get_char(testfile);
        char_stack.push(isChar);
        INFO(VARVALS(count),char_stack.top());
    }
    isChar=0xFF;  // clear current value so we are sure not to reuse it.
    char_stack.push(isChar);  // hack,to have one stack push pop difference to have the first char pushed on the stack remains
    for(; count>0; --count) {
        INFO(VARVALS(count),char_stack.top());
        if ( !testfile.unget() ) throw "unget() failed.";
        char_stack.pop();  // undo same amount of stack
    }
    // at prior position after maxunget gets and ungets
    get_char(testfile);
    ASSERT(!char_stack.empty());
    INFO(VARVALS(count,char_stack.top(),isChar),"  should be equal");
    }
<}>
 }
 catch ( EndOfStream ) {
    INFO("Unexpected EndOfStream");
    return EXIT_FAILURE;  // arguable, but in this case, we do not expect to get to the end of file at all,nor parse all the characters of the file.
 }
 

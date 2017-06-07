#!/usr/bin/env cpx
#|


#(
    char char_from_source=0, prior_cfs=0, next_cfs=0;
    struct EndOfSourceFileEvent{};
    auto get_char_from_source = [&]() -> char
    {
        // try to get next char
        prior_cfs= char_from_source;
        char_from_source= cin.get();
        if ( !cin ) {
            ThrowEvent(EndOfSourceFileEvent());
        }
        next_cfs= cin.peek();
        return char_from_source;
    };

    int64_t count=0;

    try {
        for(;;){
            get_char_from_source();
            switch (char_from_source) {
                case '{':
                    ++count;
                    break;
                case '}':
                    --count;
            }
        }
    }
    catch (EndOfSourceFileEvent){}

    INFO(HBLUE,DateTime(),NOCOLOR,VARVAL(count));

#)
/*11*/

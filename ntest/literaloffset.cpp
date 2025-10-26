#!/usr/local/bin/cpx

/*
    Researching of generating compile time enum values being associated with string items (words) in a 
     compile time array of string literals. 
    The aim is to be able to have a string representation derived from the created enum value.
*/

REPEAT_MAKE(test_words,black|white|red|green|blue)

// Concatenated string of all words
    constexpr const char* 
Words[] = 
{   
    "null" REPEAT_FOREACH(test_words,tested_word),
    REPEAT_ITEM_STRING(tested_word)
    REPEAT_END    
}
;
    constexpr size_t 
Words_size()
{
    return ( sizeof Words )/ ( sizeof Words[0] );
}
    constexpr size_t 
find_word_index( const char* const words[], size_t size, const char* const word) 
{
    for (size_t i = 0; i < size; ++i) {
        if ( strcmp(words[i],word) ) {
            continue;
        }
        return i;
    }
    return size;  // Return size if the word is not found
}
    constexpr size_t
operator "" _cstr2u64(const char* const _s,size_t) 
{
    return find_word_index(Words, Words_size() , _s);
}

<? function enum_literalized($item) { return $item.'= "'.$item.'"_cstr2u64'; } ?>

#define REPEAT_ITEM_ENUM_LITERALIZE(item) <?=enum_literalized(${#item})?>
#define             ENUM_LITERALIZE(item) <?=enum_literalized(  #item )?>

    enum class 
color : size_t
{   
    ENUM_LITERALIZE(null) 
REPEAT_FOREACH(test_words,tested_word),
    REPEAT_ITEM_ENUM_LITERALIZE(tested_word)
REPEAT_END
};

#!
  INFO(VARVALS(Words));
  INFO(VARVALS(Words_size()));

 REPEAT_FOREACH(test_words,tested_word) size_t REPEAT_ITEM(tested_word)_offset = find_word_index(Words, Words_size(), REPEAT_ITEM_STRING(tested_word));
  INFO(VARVALS(REPEAT_ITEM(tested_word)_offset));
  INFO(VARVALS(Words[static_cast<size_t>(color::REPEAT_ITEM(tested_word))]));
  
 REPEAT_END

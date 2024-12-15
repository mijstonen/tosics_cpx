#!/usr/local/bin/cpx
#|
//______________________________________________________________________________________________________________________
// Due to the source hashing, touching the file cant guarantee rebuilding the binary.
// Best is to enforce a file change transparent to the preprocessor.
// The actual number is irrelevant, as long it reflects change in the file,
// usually by increasing it by one.
const static int ForceRebuildChangeNumber=3;

namespace tu = tosics::util;



namespace {

    string
//:ErrorMsg:// must persist against scopes due to its use for exceptions and it needs to outlive function main()
ErrorMsg
;

struct DirectiveLine
{
        uint32_t
    // The directive line
    m_LineNumber
    ;
        string
    // The path from which the following source
    m_pathStr
    ;
        vector<int32_t>
    // A variable number of tokens for directive interpretation
    m_tokens
    ;

    DirectiveLine()
    {
        m_tokens.reserve(2);
    }
        tu::state_t
    getFields(string _line)
    // get the fields on the (directive) line, separated by whitespaces
    // ensure that _line is not empty or only contains whitespace, has_contents() servis thet purpose
    {
        istringstream directiveStream(_line);
        string directive;
        if ( !(directiveStream >> directive) ) {
            return tu::State(-1); // reading error
        }
        if ( directive != "#" ) {
            if ( directive[0]=='#' ) {
                return tu::State(2); // not # but starts with # (aka #pragma)
            }
            return tu::State(1); // not a directive line, should be handled by the caller
        }
        if ( !(directiveStream >> m_LineNumber >> m_pathStr) ) {
            return tu::State(-2); // reading error and need both mandatory fields
        }
        m_tokens.clear();
        int32_t nextToken{-128};
        while ( directiveStream >> nextToken ) {
            if ( nextToken<0 ) {
                // there should only be small positive numbers, anything else is likely to be
                // a reading error but maybe its correct and we did not see it before,
                // in most cases it will not affect the operation
                return tu::State(2); // recommend notification by caller for analyzes
            }
            m_tokens.emplace_back(nextToken);
        }
        // success
        return tu::State(0);
    }
};//DirectiveLine

    bool
contents_in( string const& _line) noexcept
// detects when the line has content that are interpretable as fields by getFields()
// that is it should not be empty or only contain whitespaces
{
    for( auto line_itr = begin(_line),line_end= end(_line);/*always true*/;++line_itr) {
        if ( isspace( *line_itr) ) {
            continue;
        }
        return ( line_itr<line_end );
    }
}

#define SPACECOUNT 2*40
char Spaces[SPACECOUNT+2]= <? echo '"'.str_repeat(' ',SPACECOUNT+1).'"'; PHP_MARKSRCLINE?>;

void indentPrint(uint32_t indents)  noexcept
{
    char *tmp_EndOfString= Spaces+ min(indents<<1,uint32_t(SPACECOUNT));
    *tmp_EndOfString='\0';
    printf( Spaces);
    *tmp_EndOfString=' ';
}


 PHP_BEGIN
    // load words frome file with each one word, handle inperfect input and sord the words,
    // making it suited for (fast) binary searchable
    function sortedWordsFile(string $fileName) : array
    {
        $words=array();
        foreach(explode( PHP_EOL,file_get_contents($fileName)) as $w) {
            trim($w);
            if (strlen($w)) {
                $words[]= $w;
            }
        }
        sort($words,SORT_STRING);
        return $words;
    }
    echo "const char*C_plus_plus_sorted_keywords[]"  . C_initializerListFrom(sortedWordsFile("cplusplus_keywords.asc")) . ";\n";
    echo "const char*Stdinc_common_sorted_symbols[]" . C_initializerListFrom(sortedWordsFile("custom-stdinc-keywords.asc")) . ";\n";
 PHP_END

void VerifySorted(const char* _words[], size_t _words_size)
// In case ASSERT is_sorted fails (look like a bug???), this can be used to check it and find the cause (if any)
{
    if ( !_words_size )
        return;  // a empty array is considered ok

    const char* prior{_words[0]};
    for( size_t siCnt=1; siCnt<_words_size  ; ++siCnt ) {
        const char* sym = _words[ siCnt];
        string_view lhs{prior}, rhs{sym};
        ++siCnt;
        if ( lhs<rhs ) {
            prior= sym;
            continue; // OK
        }
        CERROR("at",VARVAL(siCnt),"expects",VARVAL(prior),"<",VARVAL(sym));
        ErrorMsg= STREAM2STR(FUNC_MSG("expected  native array of words meeting sorting criteria, but it does not"));
        ThrowBreak(std::runtime_error(ErrorMsg));
    }
} // VerifySorted()
} // namepsace


//______________________________________________________________________________________________________________________

#(
    atexit([]()
        {
            printf(RESET ".\n");
            fflush(stdout);
            fflush(stderr);
        }
    );

    VerifySorted(C_plus_plus_sorted_keywords,ITEMS_IN(C_plus_plus_sorted_keywords));
    VerifySorted(Stdinc_common_sorted_symbols,ITEMS_IN(Stdinc_common_sorted_symbols));
    #define SPACES_PER_INDENT 4
    #define MAX_INDENTS 100
    #define UNDEFINED_FILE "????????????????????????????????"
    #define UNDEFINED_LINE_NUMBER uint32_t(-(1<<30))

    // extra colors selectred from the 256 color pallet
    // define BG_BLACK "\033[48;5;0m"
    //
    // for symbols that should not be publicly used ( start with _ or digit, not stored to wordMap )
    # define INTERNALCOLOR XLIGHTSTEELBLUE1
    //
    // deeper symmetry level symbol, possibly local or argument symbol without _ prefix, consider prefixing with _ , may or may not publicly usable
    # define DEEPSYMBOLCOLOR XLIGHTYELLOW3
    //
    // symbols for variables or constants defined by include file for available use when file is included
    # define SYMBOLCOLOR XLIGHTCYAN1
    //
    // Extra attention to this ( used for = and ; and directives to compiler aka #pragma
    # define HIGHMARKCOLOR XGREENYELLOW
    //
    // Oppsit of attention, make it less visable, being used for backgound details
    # define DIMMEDGREYCOLOR XGREY35
    //
    // (possible) keyword detected from stdinc, local sources should avoid redefining them locally
    # define STDINCSYMBOLCOLOR XPINK1
    // Quoted texts (strings and characters) also have modified backgound color.
    # define YELLOW1_ON_GREY15 "\033[38;5;226m\033[48;5;237m"
    # define BRIGHT_CYAN_ON_BLUE "\033[38;5;81m\033[48;5;17m"
    # define WHITE_ON_GRAY "\033[38;5;223m\033[48;5;235m"



        char const*
    formats[]   {"? %d "
                ,"\\%d  "
                ," %d/    "
                ," %d() "
           /*4*/," %d|  "
                ," %dC) "
                ," %d=  "
                }
    ;
    uint32_t fmtIndex{0};
    uint32_t includeLevel{0};
    string line;
    DirectiveLine dl;
    uint32_t current_line_number{UNDEFINED_LINE_NUMBER};
    string current_file{UNDEFINED_FILE};
    size_t linesread_count{0};
    tu::state_t howLineIsRed{-1000}; // force error when not overwritten with valid>=0 value but far from the normal error range.
    int32_t symmetry{0};  // count matching symmetrical charater tokens
    stack<decltype(symmetry)> symmStack;
    map<string,uint32_t> wordMap;
    // count found words and nicely print them colored at the bottom, in most used first order
    bool addWordMap{tu::ProgramArguments.size()>=2 && tu::ProgramArguments[1]=="--wordmap" };
    // addWordMap output unusable for extracting the words for further processing,
    // therefor a extended option dumps the words only and without colorting, but in sorted order
    // note that this is a quick hack for the output, altough not shown, the words are counted

    bool sortDumpWords{addWordMap && tu::ProgramArguments.size()>=3 && tu::ProgramArguments[2]=="--sortdump"};

        auto
    symmetryCount{ [&](string const& _line) -> string
    // ensure that there are no C/++ comments in the file
    // cant de < and > of template because these also work indiviually as less and more then, in expressions
    {
        bool sqtogle{false};
        bool dqtogle{false};
        bool dsq_enabled{true}; // with \ in a string or character sequence, it gets false to ignore \" and \'
        ostringstream decorated_line;
        string word="";

            auto
        flush_word{[&]()
        {
            if ( !word.size() )
                return;

            char const* wordColor{INTERNALCOLOR};
            bool c_plus_plus_keyword{ranges::binary_search( C_plus_plus_sorted_keywords, word)};
            bool stdinc_common_word{!c_plus_plus_keyword&& ranges::binary_search( Stdinc_common_sorted_symbols, word)};

                auto
            is_symbol{[&]()->bool
            {
                if ( c_plus_plus_keyword )
                    // known language keyword
                    return false;
                // otherwhise
                if ( stdinc_common_word )
                    // not specific to this source, a common word from includes
                    return false;
                // otherwhise
                if ( strchr("_0123456789", word[0]) )
                    // (part of) nummeric value
                    return false;
                // otherwhise
                return true;
            }};

            if ( c_plus_plus_keyword ) {
                wordColor= HWHITE;
            }
            else if ( isdigit(word[0]) ) {
                wordColor= HMAGENTA;
            }
            else if ( stdinc_common_word ) {
                wordColor= STDINCSYMBOLCOLOR;
            }

            if ( is_symbol() ) {
                if ( symmetry<5 ) {
                    // collect and count symbols
                    if ( addWordMap ) {
                        ++wordMap[word];
                    }
                    wordColor= SYMBOLCOLOR;
                }
                else {
                    wordColor= DEEPSYMBOLCOLOR;
                }
            }
            decorated_line<< wordColor<< word<< NOCOLOR;
            word="";
        }};


        for(auto c: _line) {
            if ( dqtogle && !(dsq_enabled && c=='"') ) {
                // char in string
                decorated_line<< c;
                dsq_enabled = ( c=='\\' )?  !dsq_enabled: true;
                continue;
            }
            // otherwise
            if ( sqtogle && !(dsq_enabled && c=='\'') ) {
                // char in character sequence
                decorated_line<< c;
                dsq_enabled = ( c=='\\' )?  !dsq_enabled: true;
                continue;
            }
            // otherwise
            if ( !sqtogle ) {
                if ( c=='"' ) {
                    flush_word();
                    if ( dqtogle  ) {
                        decorated_line<< c << NOCOLOR;
                        dqtogle= false;
                    }
                    else {
                        decorated_line<< WHITE_ON_GRAY<< c;
                        dqtogle= true;
                    }
                    continue;
                }
            }
            // otherwise
            if ( !dqtogle ) {
                if ( c=='\'' ) {
                    flush_word();
                    if ( sqtogle  ) {
                        decorated_line<< c << NOCOLOR;
                        sqtogle= false;
                    }
                    else {
                        decorated_line<< YELLOW1_ON_GREY15<< c;
                        sqtogle= true;
                    }
                    continue;
                }
            }
            // otherwise
            if ( strchr("=;",c) ) {
                flush_word();
                decorated_line<< HIGHMARKCOLOR<< c<< NOCOLOR;
                continue;
            }
            // otherwise
            if ( strchr("({[",c) ) {
                flush_word();
                decorated_line<< HCYAN<< c<< NOCOLOR;
                ++symmetry;
                continue;
            }
            // otherwise
            if ( strchr(")}]",c) ) {
                flush_word();
                decorated_line<< HCYAN<< c<< NOCOLOR;
                --symmetry;
                continue;
            }
            // otherwise
            if ( strchr("<>",c) ) {
                flush_word();
                decorated_line<< HRED<< c<< NOCOLOR;
                continue;
            }
           // otherwise
           if ( strchr(",+-*&.|\\/?:~!$%^" ,c) ) {
                flush_word();
                decorated_line<< HYELLOW<< c<< NOCOLOR;
                continue;
            }
            if ( isspace(c) ) {
                flush_word();
                decorated_line<< NOCOLOR<< c;
                continue;
            }
           // otherwise
            if ( strchr("#\"'<>(){}[]`@",c) ) {  // these charactes should not appear here at all or because the have been captured before
                printf("\n" HYELLOW "WARNING: Software error. Unexpected character '%c' being added to word buffer." NOCOLOR "\n",c);
            }
            word+= c;
        } // for c
        flush_word();


        auto return_value=decorated_line.str();
        //INFO(VARVAL(return_value));
        return return_value;
    }};

    //:EndOfSourceFileEvent:// Thrown to leave the char processing loop
    struct EndOfSourceFileEvent  { };

        auto
    getLineTillEndOfFile{[&]()
    {
        if (not getline( cin, line) ) {
            throw EndOfSourceFileEvent();
        }
        ++linesread_count;
        if ( howLineIsRed>0 ) {
             if ( current_line_number!= UNDEFINED_LINE_NUMBER ) {
                 ++current_line_number;
             }
        }
    }};

    try {
        for(;/* terminated by throw catch */;) {
            getLineTillEndOfFile();

            if ( not contents_in( line) ) {
                // skipped empty current_line needs to be counted
                howLineIsRed=tu::state_t{3};
                do {
                    getLineTillEndOfFile();
                } while ( not contents_in( line) );

                // only print a simgle empty line for multiple empty lines
                printf("\n");
            }

            printf(DIMMEDGREYCOLOR "%6zu " GREEN,linesread_count);
            indentPrint(includeLevel);
            printf(formats[fmtIndex],includeLevel);
            howLineIsRed= STATEREPORT(dl.getFields(line),SR_EXCLUDE_0_AND(1,2));

            switch ( howLineIsRed ) /* 1 */ {
                case 1: // Not a directive. Do something else with the line
                    fmtIndex=4;
                    printf("%05d: " CYAN "%4d "  WHITE "%s" GREEN , current_line_number,  symmetry, symmetryCount( line).c_str());
                    break;
                case 2: // print like 0 but do not interprete the meaning of the line
                    fmtIndex=6;
                    printf("%05d: " HIGHMARKCOLOR "%s" GREEN , current_line_number, line.c_str());
                    break;
                case 0: // Directive. Do the directive line thing
                    fmtIndex=6;
                    printf("%05d: " YELLOW "%s" GREEN , current_line_number, line.c_str());
                    if ( dl.m_pathStr != current_file ) {
                        //indentPrint(includeLevel);
                        printf("  _____FileChange_____ from: %s",current_file.c_str());
                        current_file = dl.m_pathStr;
                    }
                    current_line_number = dl.m_LineNumber;
                    current_file = dl.m_pathStr;
                    if (dl.m_tokens.size()>0) {
                        switch (dl.m_tokens[0]) /* 2 */ {
                            case 1:
                                ++includeLevel;
                                symmStack.push(symmetry);
                                fmtIndex=1;
                                break;
                            case 2:
                                --includeLevel;
                                ASSERT(static_cast<int32_t>(includeLevel)>= static_cast<int32_t>(0));
                                fmtIndex=2;
                                if ( symmetry!= symmStack.top() ) {
                                    INFO(HYELLOW,VARVAL(symmetry)," != ",VARVAL(symmStack.top()),"Detected asymmetric source code");
                                    symmetry = symmStack.top();
                                }
                                symmStack.pop();
                                break;
                            case 3:
                                // check expected behavior of fallthrough
                                if ( dl.m_tokens.size()<2 ) {
                                    fmtIndex=3;
                                    break;
                                }
                                if ( dl.m_tokens[1]==4 ) {
                                    fmtIndex=5;
                                    break;
                                }
                            [[fallthrough]];
                            default: //INFO(VARVAL(dl.m_tokens[0]),"case I dont know ?");
                                printf("\n" HRED "  ????????????I_DONT_KNOW????????????? tokens: %d %d" GREEN,dl.m_tokens[0],dl.m_tokens[1]);
                                fmtIndex=0;
                                break;
                        } /* 2 switch*/
                    }
                    break;
                default:
                    ErrorMsg = STREAM2STR("Line read error: State "<< howLineIsRed);
                    tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
                    return howLineIsRed;
            } /* 1 switch */
            printf("\n");
        }//for
    }//try
    catch ( EndOfSourceFileEvent ) {
    }


    if ( symmStack.size() ) {
        INFO(HRED VARVAL(symmStack.size()), "expected 0, symmStack_not_empty" NOCOLOR);
    }
    else {
        INFO(HGREEN VARVAL(symmStack.size()), "symmStack_is_empty" NOCOLOR);
    }

    if ( addWordMap ) {
        printf("\n");
        vector<pair<string,int>> wordVec;
        std::ranges::move(wordMap, std::back_inserter<std::vector<pair<string,int>>>(wordVec));
        if ( sortDumpWords ) {
            std::ranges::sort(wordVec,
                    [](const pair<string,int> &l, const pair<string,int> &r)
                    {
                        if ( l.first > r.first ) return false;
                        if ( l.first < r.first ) return true;
                        // otherwhise l.first == r.first  sort on first
                        return l.second < r.second;
                    }
            );
            // Print, suited for further processing (aka with awk)
            for (auto const &symcntPair: wordVec) {
                printf("%s %d\n",symcntPair.first.c_str(),symcntPair.second);
            }
        }
        else {
            std::ranges::sort(wordVec,
                    [](const pair<string,int> &l, const pair<string,int> &r)
                    {
                        if ( l.first < r.first ) return false;
                        if ( l.first > r.first ) return true;
                        // otherwhise l.second == r.second  sort on first
                        return l.second < r.second;
                    }
            );
            printf(NOCOLOR "\n\n"
                "wordMap, potentianlly global defined symbols useable in "
                "application source sorted from most to least occurrences.\n"
                "--------------------------------------------------------------------------------------------------------------"
                "--------------------------------------------------------------------------------------------------------------\n"
            );

            const int8_t COLS=int8_t(4);
            int8_t nlc=COLS;
            for (auto const &symcntPair: wordVec) {
                printf("%6d: "  SYMBOLCOLOR "%-40s" NOCOLOR, symcntPair.second, symcntPair.first.c_str());
                char const*  fmt="\n";
                if ( --nlc ) fmt="     ";
                else nlc=COLS;
                printf(fmt);
            }
        }
    }
#)


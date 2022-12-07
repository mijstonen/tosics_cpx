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


}//namepsace
//______________________________________________________________________________________________________________________

#(
    // load (at compiletime) C/++ keywords from asci file
    PHP_BEGIN
        $keywords=array();
        foreach(explode( PHP_EOL,file_get_contents("cplusplus_keywords.asc"))  as $w) if (strlen($w)) $keywords[]=$w;
        sort($keywords,SORT_STRING);
    PHP_END

        const vector<string>
    C_plus_plus_sorted_keywords=
        <?=C_initializerListFrom($keywords)?>
    ;

    ASSERT(ranges::is_sorted(C_plus_plus_sorted_keywords));

    #define SPACES_PER_INDENT 4
    #define MAX_INDENTS 100
    #define UNDEFINED_FILE "????????????????????????????????"
    #define UNDEFINED_LINE_NUMBER uint32_t(-(1<<30))

    // extra colors selectred from the 256 color pallet
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
    # define HIGHMARKCOLOR XPINK1

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
    tu::state_t howLineIsRed{0};
    int32_t symmetry{0};  // count matching symmetrical charater tokens
    stack<decltype(symmetry)> symmStack;
    map<string,uint32_t> wordMap;

        auto
    symmetryCount{ [&](string const& _line) -> string
    // ensure that there are no C/++ comments in the file
    // cant de < and > of template because these also work indiviually as less and more then, in expressions
    {
        bool sqtogle{false};
        bool dqtogle{false};
        ostringstream decorated_line;
        string word="";

            auto
        flush_word{[&]()
        {
            if ( !word.size() )
                return;

            char const* wordColor{INTERNALCOLOR};
            bool c_plus_plus_keyword{ranges::binary_search( C_plus_plus_sorted_keywords, word)};

                auto
            is_symbol{[c_plus_plus_keyword,&word]()->bool
            {
                if ( c_plus_plus_keyword )
                    // known language keyword
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

            if ( is_symbol() ) {
                // collect and count symbols
                wordColor= symmetry>4 ?DEEPSYMBOLCOLOR :SYMBOLCOLOR  ;
                ++wordMap[word];
            }
            decorated_line<< wordColor<< word<< NOCOLOR;
            word="";
        }};


        for(auto c: _line) {
            if ( dqtogle && c!='"' ) {
                decorated_line<< c;
                continue;
            }
            // otherwise
            if ( sqtogle && c!='\'' ) {
                decorated_line<< c;
                continue;
            }
            // otherwise
            if ( c=='"' ) {
                flush_word();
                if ( dqtogle  ) {
                    decorated_line<< c << NOCOLOR;
                    dqtogle= false;
                }
                else {
                    decorated_line<< HBLUE<< c;
                    dqtogle= true;
                }
                continue;
            }
            // otherwise
            if ( c=='\'' ) {
                flush_word();
                if ( sqtogle  ) {
                    decorated_line<< c << NOCOLOR;
                    sqtogle= false;
                }
                else {
                    decorated_line<< HMAGENTA<< c;
                    sqtogle= true;
                }
                continue;
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

    while( getline( cin, line) ) {
        ++linesread_count;
        if ( contents_in( line) ) {
            printf(GREEN "%6zu " GREEN,linesread_count);
            indentPrint(includeLevel);
            printf(formats[fmtIndex],includeLevel);
            howLineIsRed= STATEREPORT(dl.getFields(line),SR_EXCLUDE_0_AND(1,2));
//            printf("%05d: %s" "%s" GREEN , current_line_number, ( howLineIsRed ) ?WHITE :HYELLOW,line.c_str());

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
                                    INFO(RED,VARVAL(symmetry)," != ",VARVAL(symmStack.top()),"Detected asymmetric source code");
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
                        } /* 2 switch*/
                    }
                    break;
                default:
                    ErrorMsg = STREAM2STR("Line read error: State "<< howLineIsRed);
                    tu::ThrowBreak(ErrorMsg.c_str(), tu::eBC_handled);
                    return howLineIsRed;
            } /* 1 switch */
            printf("\n");
        } // if
        if ( howLineIsRed ) {
            if ( current_line_number!= UNDEFINED_LINE_NUMBER ) {
                ++current_line_number;
            }
        }
    } // while getline

    if ( symmStack.size() ) {
        INFO(HRED VARVAL(symmStack.size()), "expected 0, symmStack_not_empty" NOCOLOR);
    }
    else {
        INFO(HGREEN VARVAL(symmStack.size()), "symmStack_is_empty" NOCOLOR);
    }

    printf(NOCOLOR);
    vector<pair<string,int>> wordVec;
    std::ranges::move(wordMap, std::back_inserter<std::vector<pair<string,int>>>(wordVec));
    std::ranges::sort(wordVec,
            [](const pair<string,int> &l, const pair<string,int> &r)
            {
                if ( l.second < r.second ) return false;
                if ( l.second > r.second ) return true;
                // otherwhise l.second == r.second  sort on first
                return l.first < r.first;
            }
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
    printf(RESET "\n");
    fflush(stdout);
#)


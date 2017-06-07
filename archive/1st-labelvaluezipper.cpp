#!/usr/bin/env cpx
#|
#include "statereport.hpp"
using namespace util;


// to info.hpp

state_t zipLeftsAndRightsJoin2Str(
  std::string *out_
, std::vector<std::string> const& _lefts
, char const* _left_right_separator
, std::vector<std::string> const& _rights
, char const* _zippedSeparator
);


template <typename NextValue_T>
void argValues2StrVector( std::vector<std::string>* values_, NextValue_T const& _next_value_t )
{
    values_->push_back(custom_quote(_next_value_t));
}

template<typename FirstValue_T, typename... RemainingValues_T>
void argValues2StrVector( std::vector<std::string>* values_, FirstValue_T const& _first_value, RemainingValues_T const&...  _remaining_values)
{
    argValues2StrVector( values_,_first_value);
    argValues2StrVector( values_,_remaining_values...);
}

template <typename... VA_ARGS_T>
std::string varvals(char const * _args_str, VA_ARGS_T const&... _va_args)
{
    // extract lables
    std::vector<std::string> labels;
    state_t lastCalled_returns_no_0= append_splitted( &labels, _args_str /*, asj*/);

    char const * hereErrorMsgPart="From VARVALS(...) macro, varvals()/";
    if ( lastCalled_returns_no_0 ){
        throw std::logic_error(
            std::string(hereErrorMsgPart) + "append_splitted() returned State(" +
            std::to_string(lastCalled_returns_no_0) +
            ") : Failed splitting argument name list into separate arument names");
    }


    // extract values
    std::vector<std::string> values;
    argValues2StrVector( &values,  _va_args...);

    // zip all labels and values to string
    std::string return_value;
    lastCalled_returns_no_0= zipLeftsAndRightsJoin2Str( &return_value, labels,"=", values, ObjectSeparation());

    if ( lastCalled_returns_no_0 ) {
        throw std::logic_error(
            std::string(hereErrorMsgPart) + "zipLeftsAndRightsJoin2Str() returned State(" +
            std::to_string(lastCalled_returns_no_0) +
            ") : Failed zipping argument names and argument values together");
    }

    return return_value;
}
#define VARVALS(...) varvals(VSTRINGS(__VA_ARGS__),__VA_ARGS__)

// to util.cpp
state_t zipLeftsAndRightsJoin2Str(
  std::string *out_
, std::vector<std::string> const& _lefts
, char const* _left_right_separator
, std::vector<std::string> const& _rights
, char const* _zippedSeparator
)
{
    state_t return_value= State(0);
    //Preconditions
    if ( !_lefts.size() ) {
        // no labels, must have something to be zipped
        return State(-1);
    }
    if ( !_rights.size() ) {
        // no values, must have something to be zipped
        return State(-2);
    }
    if ( _rights.size()!=_lefts.size() ) {
        // number of values must be equal to number of labels to prefix each value with a label
        return State(-3);
    }
    if ( !out_ ) {
        // output is mandatory
        return State(-4);
    }
    if ( out_->length() ) {
        // out_ will be overwritten,
        // it is no error but that might be not intended
        // the proces continues
        return_value= State(1);
    }

    std::vector<std::string> to_be_joined;
    for( decltype( _lefts.size()) i=0; i<_lefts.size(); ++i ) {
        std::vector<std::string> label_and_value= { move( _lefts[i]), move( _rights[i])};
        std::string itemStr; // reused and reinitialized each cycle
        if ( STATEREPORT(   append_joined( &itemStr, label_and_value, _left_right_separator)   ) ) {
            return State(-5);
        }
        to_be_joined.push_back( move( itemStr));
    }

    if ( STATEREPORT(   append_joined( out_, to_be_joined, _zippedSeparator)   ) ) {
        return State(-6);
    }

    return return_value;
}





#(

//LabelValueZipper lvz;

auto n=1;
auto m=2;
float f=33.333;
auto c='M';
auto s="Toonen";


#if 0
lvz.commaSeparatedExpressions2StrVector( VSTRINGS(n,m,m+n,f/(m+n),c,s));
lvz.argValues2StrVector(n,m,m+n,f/(m+n),c,s);


// lvz.info();

string zippedStr="";

STATEREPORT(   lvz.zipAndJoin2Str( &zippedStr)   );
INFO(zippedStr);
#endif

// throw logic_error("try throwing a logic_error");


//INFO(ENDLINES(3),VARVALS(n,m,foo(foo(m,n)*foo(m,n),n),8.1234,f,c,"Ka,al",s));

INFO(ENDLINES(3),VARVALS(n,m,f,c,"Kaal",s));

#)
//.9

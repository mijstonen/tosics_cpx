#!/usr/bin/env cpx
#|

#define STATE(_intValue) (_intValue)
// better: int State( int _state );    usage   return util::State(-3);  [...](...){ ...;  throw util::State(-1);

#include "statereport.hpp"
#define __VSTRINGS(...) #__VA_ARGS__



namespace util {

    //:state_t:// The CPU natuaral integer (can vary per processor, only use small (..est possible) value
    using state_t = long; 

    //:stateLiteralArg_t:// not passing a integer literal should be detected by the compiler
    using stateLiteralArg_t = state_t&&;

    //:State:// The intend is to make searches easy where a state is thrown or returned. Aka State(0) State(-1) State(12)
    inline state_t State(stateLiteralArg_t _state ) 
    {
        return _state;
    }

//:append_splitted:// Research for VARVALS implementation, to split string with arguments
template <typename CONTAINER_T = std::vector<std::string> >
state_t append_splitted( CONTAINER_T* strs_, std::string const& _str, ASJ_special const& _asj, char const* _seps = ",")
{
    /* HowTo statically check
     * *strs::value_type == string
     */

    if (!strs_) {
        return State(-1); // strs_ output is mandatory and to be modified without further checks
    }
    auto pc = _str.c_str();
    if (!pc) {
        return State(-2); // pc must be valid to perform
    }
    auto str_len = strlen(pc);
    if (!str_len) {
        return State(1); // no input to split, but result is valid because nothing would change,
                  // however: no input possibly is not intended
    }

    auto start = pc; // [start...pc> is range of chars copied

    bool allow_empty = false;
    char const* up   = "([<{\"'`/";
    char const* down = ")]>}\"'`/"; 
    uint16_t encap_pair_index = str_len; // still valid index in up or down
    std::vector<decltype(encap_pair_index)> epi_stack;

    auto append_substr = [&]() mutable -> void 
    {
        if (start > pc) {
            throw State(-3); // calculated substr_len >= 0  -> catch and return from local function
        }
        auto substr_len = pc - start;
        if (allow_empty || substr_len) {
            // from start, len chars are copied in the string emplaced back in strs_
            strs_->emplace_back(start, substr_len);
        }
    };

    auto pair_index = [&](char const* _updn) 
    {
        char const* found;
        found = strchr(_updn, *pc);
        if (found) {
            encap_pair_index = found - _updn;
            if (encap_pair_index < 0) {
                throw State(-6);
            }
            if (encap_pair_index > str_len) {
                throw State(-7);
            }
            return true;
        }
        return false;
    };

    //initialize from control info
    allow_empty = _asj.m_allow_empty;
    up = _asj.m_up.c_str();
    down = _asj.m_down.c_str();

    //scan
    try {
        while (*pc) {
            //auto push_to_epi= [&](auto _encap_pair_index)
            if (epi_stack.size()) {
                if (pair_index(up)) {
                    epi_stack.push_back(encap_pair_index);
                    INFO("push => ", VARVAL(up[encap_pair_index]),VARVAL(epi_stack.size()));
                } 
                else if (pair_index(down)) {
                    auto up_encap_pair_index = encap_pair_index;
                    encap_pair_index = epi_stack.back();
                    epi_stack.pop_back();
                    if ( epi_stack.size() && ( encap_pair_index != up_encap_pair_index) ) {
                        epi_stack.push_back(encap_pair_index);
                        INFO("push => (undo invalid pop)",VARVAL( down[encap_pair_index]));
                    }
                    INFO( "pop <= ", VARVAL( up[up_encap_pair_index]), VARVAL( down[encap_pair_index]), VARVAL( epi_stack.size()));
                }
            } 
            else {
                auto old_encap_pair_index = encap_pair_index;
                if (pair_index(up)) {
                    epi_stack.push_back(old_encap_pair_index);
                    INFO("push => ", old_encap_pair_index);
                }
            }
            if ( epi_stack.size() || !strchr(_seps, *pc) ) {
                INFO("OK",VARVAL(epi_stack.size()),VARVAL(*pc),"char is encapsulated or is not a separator char");
            } // if
            else { // match found with a separator
                append_substr();
                start = pc + 1;
            } // else
            INFO(HYELLOW,*pc,NOCOLOR,VARVAL(epi_stack.size()),VARVAL(encap_pair_index),(char)(up[encap_pair_index]?up[encap_pair_index]:'?'),(char)(down[encap_pair_index]?down[encap_pair_index]:'?'),ENDL);
            ++pc;

        } // while *pc
        // last word preceeds  '\0' termination of _txt
        append_substr();

        auto estk_size = epi_stack.size();
        INFO(VARVAL(estk_size), VARVAL(encap_pair_index));
        if (estk_size > 1) {
            return State(-8);
        }
        if (encap_pair_index != str_len) {
            return State(-9);
        }
    } // try
    catch (int from_local_lambda) {
        return from_local_lambda;
    }

    if (epi_stack.size()) {
        // Possibly unintended result, encapsulation is symetric chars are not matching
        return State(-2);
    }
    return State(0); // succeeded
}
}


#(
string s = __VSTRINGS(XPR{ kaasboer, T(map, 99, p) }, q, r, f(a, g(b)), 1, 10, foo<r, r, -9.3>, endit);
//string s = __VSTRINGS(X{k,T(map,99,p)},q,r,f(a,g(b)),1,10,foo<r,r,-9.3>, e);
//Unbalanced:  string s = "(a({[<M>])}b,c),a({[<M>]})b,c)"; 
//string s = __VSTRINGS(a({[<Michel>]})b,{},c,a({[<M>]})b,c);
INFO(VARVAL(s));
vector<string> vss;

util::ASJ_special asj;

auto ul= asj.m_up.length();
auto dl= asj.m_down.length();
assert( ul==dl );
for ( decltype(ul) i=0; i<ul; ++i ) {
    INFO(VARVAL(asj.m_up[i]),VARVAL(asj.m_down[i]));
}

STATEREPORT(append_splitted(&vss, s, asj, ","));
auto max = vss.size();
for (decltype(max) i = 0; i < max; ++i) {
    INFO(VARVAL(vss[i]));
}
#)

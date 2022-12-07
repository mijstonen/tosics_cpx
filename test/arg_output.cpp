#!/usr/bin/env cpx

#|

/*
Researching a Output class to make function arguments that act as a output from a function explicit during the call.
Intended to follow the 'zero overhead abstraction' policy. This should be a dropin replacement for &output var.
So foo( &outvar,... ) becomes foo( OUT(outvar),...).

Output automates checking the (optional) output.
Additionally I implemented INFO_FUNC and FUNC_MSG macro's in tosics_util and they are used/tested here.


DETAILS
  Output contains a pointer to the actual variable - likewhise & would pass a address of the variable.
  Output is a temporary instance at the caller site. This is proven by the messages to the console.
  Inside the function that provides the output, a move operation (...Output<int>&& sum_...) takes the
  created pointer and no additional copies are made.

  The Output template class it self hides all the pointer operation access.

*/


namespace tosics::util {

template <typename OUT_T>
class Output
{
    OUT_T* const m_output;

  public:
    using out_type = OUT_T;

    // template CONCRETE_T ....
        explicit
    Output( out_type& _output_reference)
    : m_output(&_output_reference)
    {
        INFO(FUNC_MSG("===construct==="));
        assert(m_output);
    }
        explicit
    Output()
    : m_output(nullptr)
    {
        INFO(FUNC_MSG("===dropped==="));
    }
        virtual
    ~Output()
    {
        INFO(FUNC_MSG("===destruct==="));
    }
        out_type const&
    operator=( out_type const& _from)
    {
        if ( m_output ) {
            ( *m_output )= _from;
        }
        return ( *m_output );
    }
        template <typename LAMBDAFUNC_T, typename... ARGS_T>
        void
    manipulate( LAMBDAFUNC_T _actions_on_output, ARGS_T ... _args_)
    {
        if ( m_output ) {
            return _actions_on_output(m_output, forward<ARGS_T>(_args_)...);
        }
    }

} //  template  <typename OUT_T>class Output
;


}//util

#define OUT(ActualOutput) Output<decltype(ActualOutput)>(ActualOutput)

#undef DROP_OUTPUT
#define DROP_OUTPUT(OUT_T) Output<OUT_T>()

void foo(Output<int>&& sum_, vector<int> values)
{
    INFO(FUNC_MSG("==== enter ===="));
    int adding_result=0;
    for( auto value: values) {
        INFO("Addeing",VARVALS(adding_result,value));
        adding_result+= value;
    }
    sum_= adding_result;

    int spy=0;  // POSSIBLE BUT DISCOURAGED

    sum_.manipulate(
            [&](int * const output_, int* spy_ /*POSSIBLE BUT DISCOURAGED*/, char const* _msg)
            {
                ++( *output_);
                INFO(FUNC_MSG("===Forward test---> "),VARVALS(_msg));
                ( *spy_ )= ( *output_ );
            }
            , &spy // POSSIBLE BUT DISCOURAGED
            , "forwarded"
        );

    INFO(VARVAL(spy),FUNC_MSG("==== leaving ===="));
}

#(
INFO_FUNC;
int sumtest{0};
////
int *psumtest=&sumtest;
FAKE_USE(psumtest);
INFO(VARVAL(sumtest));
foo( OUT(sumtest), {1,2,3,4,5,10} );
INFO(VARVAL(sumtest));

INFO(ENDL,"---DROP---");
foo( DROP_OUTPUT(int),{8,9,10});
INFO(VARVAL(sumtest));

INFO(HBLUE,DateTime(),NOCOLOR);
#)

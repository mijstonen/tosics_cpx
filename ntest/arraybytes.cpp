#!/usr/bin/env cpx
#|


// SImple containter content printing test
// MTO-D0811 regresion: it is currently not working anymore.
// output: ..... my_array='?' m='?'   both failing to print the content


    template <size_t N, typename T>
    void
foo(T const (&v)[N] )
{
    INFO(VARVALS(N,v, sizeof v));
    _operator_shiftleft_body( cout,begin(v),end(v));
    INFO(ENDLINES(3));
}


#!
  int v[5]{1,2,3,4,5}; 
  foo(v);
  INFO(VARVAL(v));
  std::array<long,2> my_array{10,20};
  map<int,int> m{{1,10},{2,20}};
  INFO(VARVALS(sizeof my_array,begin(my_array),end(my_array),my_array,m));


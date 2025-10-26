#!/usr/bin/env/cpx
#|



struct Foooo
{
};

inline ostream& operator << (ostream &o_, Foooo const&) { return o_; }

struct Ooof
{
};

inline istream& operator >> ( istream& _i, Ooof& ) { return _i; }

template <typename Item_T>
void stdiostreamclass_Streamability()
{
    INFO(FUNC_MSG("-----------------------------------------"));
    INFO(VARVALS(is_from_stream_readable<istream,Item_T&>::value));
    INFO(VARVALS(is_to_stream_writable<ostream,Item_T>::value));
    INFO(VARVALS(is_to_stream_writable<ostream,Item_T const&>::value));
    INFO(VARVALS(is_to_stream_writable<ostream,Item_T &&>::value));
    INFO(VARVALS(is_from_to_stream_readable_writable<iostream,Item_T>::value));
}

#(
stdiostreamclass_Streamability<int>();
stdiostreamclass_Streamability<Foooo>();
stdiostreamclass_Streamability<Ooof>();
#)

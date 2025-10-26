#!/usr/local/bin/cpx

    template<typename T>
    inline void
 showtraits_of_helper()
 {
    //string type_name = boost::core::demangle( typeid(T).name());
    INFO(HWHITE,type_name<T>(),WHITE);

    INFO(VARVALS(is_iterable<T>::value ));
    INFO(VARVALS(is_appendable_at_end<T>::value ));
    INFO(VARVALS(is_to_stream_writable<ostream,T>::value ));
    INFO(VARVALS(is_from_stream_readable<istream,T&>::value ));
    INFO(VARVALS(is_from_to_stream_readable_writable<iostream,T>::value ));
    INFO();
 }
    template<typename... P>
    inline void
 showtraits_of()
 {
    (showtraits_of_helper<P>(),...);    // C++17 folding
 }


#!
 INFO(HGREEN,"===============================================",WHITE);
    showtraits_of
    < vector<int>
    , deque<int>
    , list<int>
    , forward_list<int>
    , array<int,5>
    , map<int,int>
    , set<int>
    , string
    >();

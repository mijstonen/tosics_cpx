#!/usr/bin/env cpx
#|


struct A {
    int fun(int) const&;
    int kaa;
};

template<typename>
struct Ptr2Member_traits {};

template<class T, class U>
struct Ptr2Member_traits<U T::*> {
    using member_type = U;
};

template<class MF>
struct is_member_function
: is_function< typename Ptr2Member_traits<MF>::member_type >
{
    using memberfunc_type = typename Ptr2Member_traits<MF>::member_type;
};





int f();


#(
    INFO(VARVALS(boolalpha));
    INFO(VARVALS(is_function<A>::value));
    INFO(VARVALS(is_function<int(int)>::value));
    INFO(VARVALS(is_function<decltype(f)>::value));
    INFO(VARVALS(is_function<int>::value));

    //using T = Ptr2Member_traits<decltype(&A::fun)>::member_type; // T is int() const&
    INFO(VARVALS(is_function<Ptr2Member_traits<decltype(&A::fun)>::member_type>::value));

    INFO("Query member function:",VARVALS(is_member_function<decltype(&A::fun)>::value));
    INFO("Query member function signature:",VARVALS(is_same<is_member_function<decltype(&A::fun)>::memberfunc_type,int(int) const&>::value));

    INFO("Query data member:",VARVALS(is_member_function<decltype(&A::kaa)>::value));
#)

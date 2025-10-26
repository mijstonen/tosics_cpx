#!/usr/bin/env cpx

// Possibly broken. TODO: make it work


#if 0
    template <typename T, typename = void>
    struct
is_iterable
: std::false_type
{
};
    template <typename T>
    struct
is_iterable<
    T,
    std::void_t<
        decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end())
        >
    >
: std::true_type
{
};
#endif

#if 0
    template <
        typename Container_T
    >
    void
foo( Container_T const& _c )
{
    FAKE_USE(_c);
    INFO(FUNC_MSG(""));
}

#endif


#if 1
    template <
        class Container_T,
        typename = std::enable_if_t< is_iterable<Container_T>::value>
    >
    void
foo( vector<int>& )
{
    //FAKE_USE(_c);
    INFO(FUNC_MSG(""));
}
#endif


#if 0
    template<
        typename T
    >
    auto
get_Value(T t)
{
    if constexpr (is_pointer_v<T>)
        return *t;
    else
        return t;
}
#endif

#!
    INFO(VARVAL( is_iterable< vector<int> >::value ));

    vector<int> v{1,2,3};
    INFO(VARVAL(v));
    foo(v);


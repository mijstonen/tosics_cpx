#!/usr/bin/env cpx
#|

    template<typename T, typename _ = void_t<>>
    struct
is_container
: false_type
{
};

    template<typename T>
    struct
is_container<
        T,
        void_t<
            typename T::value_type,
            typename T::size_type,
            typename T::allocator_type,
            typename T::iterator,
            typename T::const_iterator,
            decltype(declval<T>().size()),
            decltype(declval<T>().begin()),
            decltype(declval<T>().end()),
            decltype(declval<T>().cbegin()),
            decltype(declval<T>().cend()),
            decltype(declval<T>().push_back(declval<typename T::value_type>())),
            decltype(declval<T>().emplace_back())  // must have typename T::value_type constructor with zero arguments
                                                   // emplace_back(Args...) is not resolved.
        >
>
: public true_type
{
};

#(
auto vector_string_is_container= is_container<vector<string>>::value;
INFO(VARVAL(vector_string_is_container));
#)

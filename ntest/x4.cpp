#!/usr/bin/env cpx
#|
#^ <experimental/type_traits>

using namespace experimental;

template<class T>
using copy_assign_t = decltype(std::declval<T&>() = std::declval<const T&>());

struct Meow { };
struct Purr { void operator=(const Purr&) = delete; };

static_assert(is_detected<copy_assign_t, Meow>::value,
              "Meow should be copy assignable!");
static_assert(!is_detected_v<copy_assign_t, Purr>,
              "Purr should not be copy assignable!");
static_assert(is_detected_exact_v<Meow&, copy_assign_t, Meow>,
              "Copy assignment of Meow should return Meow&!");

template<class T>
using diff_t = typename T::difference_type;

template <class Ptr>
using difference_type = detected_or_t<std::ptrdiff_t, diff_t, Ptr>;

struct Woof { using difference_type = int; };
struct Bark {};

static_assert(std::is_same<difference_type<Woof>, int>::value,
              "Woof's difference_type should be int!");
static_assert(std::is_same<difference_type<Bark>, std::ptrdiff_t>::value,
              "Bark's difference_type should be ptrdiff_t!");


    template <template <class ...> class C, class ...A>
    struct
Container_Properties
{
    constexpr bool value = false;
}
;
    template <template <class ...> class C, class ...A>
    struct
Container_Properties
{
    using container_type = C<A...>;
    using appendable_t =
            void_t<
                decltype(declval<C>().push_back(declval<typename container_type::value_type>())),
                decltype(declval<C>().emplace_back())
            >
            ;
    constexpr bool value = true;
}
;





#(



//Fake_use(a);

//INFO( is_detected<cp::appendable_t,map<string,string>>::value);
#)

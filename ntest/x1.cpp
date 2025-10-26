#!/usr/bin/env cpx
#|


#define HAS_MFN(MFN_NAME)                                                                                            \
template<typename, typename MFN_T>                                                                                   \
struct has_##MFN_NAME {                                                                                              \
    static_assert(                                                                                                   \
        integral_constant<MFN_T, false>::value,                                                                      \
        "Second template parameter needs to be of (member) function type.");                                         \
};                                                                                                                   \
    template<typename Class_T, typename Ret_T, typename... Args_P>                                                   \
    struct                                                                                                           \
has_##MFN_NAME<Class_T, Ret_T(Args_P...)>                                                                            \
{                                                                                                                    \
  private:                                                                                                           \
        template<typename T>                                                                                         \
        static constexpr auto                                                                                        \
    check(Class_T*)-> typename is_same<                                                                              \
                        decltype( declval<Class_T>().MFN_NAME( declval<Args_P>()... ) ),                             \
                        Ret_T                                                                                        \
                    >::type                                                                                          \
    ;                                                                                                                \
        template<typename>                                                                                           \
        static constexpr false_type                                                                                  \
    check(...)                                                                                                       \
    ;                                                                                                                \
    using type = decltype(check<Class_T>(nullptr));                                                                  \
public:                                                                                                              \
    static constexpr bool value = type::value;                                                                       \
};

HAS_MFN(serialize)
HAS_MFN(quadro)
HAS_MFN(push_back)


template <typename CONAINTER_T=vector<string>>
class Holder
{
    CONAINTER_T words;
};

static_assert( has_push_back<Holder.words,void(string const&)>::value );

#(

struct X
{
     int serialize(const std::string&, char) { return 42; }
};

struct Y : X
{
     double quadro(int,char,float) { return 3.11; }
};

auto X_has_ser=has_serialize<X,double(const std::string&, char)>::value;
INFO(VARVAL(X_has_ser));
auto Y_has_quadro=has_quadro<Y,double(int,char,float)>::value;
INFO(VARVAL(Y_has_quadro));

#)

#!/usr/bin/env cpx

/***
 Researching binary writing to and binary reading from a stream.
 Instead of defining our own stream types we reuse the standard streams but
 provide operators '<' and '>' for unformatted writing and reading.

 This lets us reuse existing streams and levarage any extension to existing streams.
 The operator overloading is template based and not bound to specific stream types,
 altough typically stream functionallity is separated, like streams for reading and
 streams for writing.
 We can also exange the data erea via std string types. These can store binary data
 but should then not be used forformatted data and zero terminated C string
 representations.

 This binary stream research does not fullfil serialization requirements

 Binary streams are usefull for internal program communication inside a single program.


***/

// TBD: to utils


// Defining binarary reading and writing on streams.
// The generic template can work for value types, classes with visible or invisable pointers in the state need customized serialization.
// Simplified: the size of the entire object value is the total of sizes of its members.
// It is recommended to make a internal member method implementing the details and call it via a template specialized binary streaming operator.
//
// Instead of using dedicated streams, the existing standard streams of the C++   runtime are used but the binaray
// write and read are represented by > and <  iso >> and << .
// For both, read and write it is the caller responsibility to
// * prepare the object for reading or writing, including memory allocations.
// * error/exception handling.
//
// hint: some heap allocated types might be used if they have a overloaded sizeof operator and address operator;


#if 1
//  pre C++20 , current older compiler does have non standard usable variant, I use C++20 name with 'my' prefix so it should be easy 's/my_/std::/g'
//  do  '#if 0 ...' to query compile for all of them.

    template< class Object_T, std::size_t N>
    inline constexpr bool
my_is_array_known_bounds_v =
    std::__is_array_known_bounds<Object_T[N]>::value;
#endif


// Separate traits from enable_if so it is easier to adjust.
// Possibly will be extended compiling traits with boolean logic.
    template< class Object_T>
    inline constexpr bool
Dflt_Bin_Stream_Operator_Traits_v =
    std::is_trivially_copyable_v<Object_T> &&
    std::is_trivially_default_constructible_v<Object_T>;

    template< class Object_T, std::size_t N>
    inline constexpr bool
Dflt_Bin_Stream_Operator_bounded_array_Traits_v =
    std::is_trivially_copyable_v<Object_T> &&
    std::is_trivially_default_constructible_v<Object_T> &&
    my_is_array_known_bounds_v<Object_T,N>;

//    template< class Object_T>
//    inline constexpr bool
// Dflt_Bin_Stream_Operator_Traits_In_Iterable_v


#if 1
/* Notice: , before allows it to desolve into nothingness, in #else */
/* Make use of double braces as in: ENABLE_IF((...))) a habit because the preprocessor will otherwise break type lists as preprocessor arguments*/
# define ENABLE_IF(traits) ,typename std::enable_if_t<traits,int> = 0
#else
# define ENABLE_IF(trais)
#endif



//:operator< output stream intrinsic type or simple POD struncture
    template <class OutputStream_T, typename Object_T    ENABLE_IF((Dflt_Bin_Stream_Operator_Traits_v<Object_T>)) >
    OutputStream_T&
operator < ( OutputStream_T& os_, const Object_T& _object )
{
    std::streamsize sizeOfObject= sizeof _object;
    auto objectAddress = reinterpret_cast <typename OutputStream_T::char_type const*>(&_object);

    os_.write( objectAddress, sizeOfObject);
    return os_;
}

//:operator< output stream intrinsic type array or simple POD structure array, size not stored, must read back excact same size
    template<
        typename OutputStream_T,
        typename Object_T,
        std::size_t N   ENABLE_IF((Dflt_Bin_Stream_Operator_bounded_array_Traits_v<Object_T,N>)) >
    OutputStream_T&
operator< (OutputStream_T& os_,  Object_T const (&_container)[N] )
{
    /*static_*/assert( Items_in(_container) == N );
    /*static_*/assert( sizeof _container == N * sizeof(Object_T) );
#if 0
    INFO(FUNC_MSG("stream out intrinsic array:"));
    INFO("\"\"",VARVALS(type_name(_container),type_name(_container[0]),Items_in(_container),sizeof _container[0], sizeof _container));
    INFO("\"\"",VARVALS(std::end(_container),std::begin(_container),(const char*)(&_container[N])-(const char*)(&_container[0])));
#endif
    assert( (const char*)(&_container[N])-(const char*)(&_container[0]) == sizeof _container);
    std::streamsize sizeOfObjects = sizeof _container;
    auto objectAddress = reinterpret_cast <typename OutputStream_T::char_type const*>(std::begin(_container));

    os_.write( objectAddress, sizeOfObjects);
    return os_;
}

//:operator< output stream intrinsic type std::array or simple POD structure std::array, size not stored, must read back excact same size
    template<
        typename OutputStream_T,
        typename Object_T,
        std::size_t N    >
    OutputStream_T&
operator < (OutputStream_T& os_,  std::array<Object_T,N>const& _container)
{
    static_assert( _container.size() == N );
    std::streamsize sizeOfObjects = N* sizeof(Object_T);
    auto objectAddress = reinterpret_cast <typename OutputStream_T::char_type*>(_container.data());

    os_.write( objectAddress, sizeOfObjects);
    return os_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WIP
//:operator< output stream intrinsic type std::vector or simple POD structure std::vector, size stored
    template<
        typename OutputStream_T,
        typename Object_T
    >
    OutputStream_T&
operator < (OutputStream_T& os_,  std::vector<Object_T>const& _container)
{
    // Almost as array, except, the number of objects is dynamic and also needs to be written
    auto number_of_objects{_container.size()};
    os_ < number_of_objects;

    std::streamsize sizeOfObjects = number_of_objects* sizeof(Object_T);
    auto objectAddress = reinterpret_cast <typename OutputStream_T::char_type*>(_container.data());

    os_.write( objectAddress, sizeOfObjects);
    return os_;
}

/***
Before defining binary stream operators for strings, first consider generalized type iterable container.
For every container the number of items is counted and the values are copied to a vector.

(not scalable if the container is (very) big, then we would have to do chunks. However, these kind
of parameter stream (very) big containers should not be considered, rather pass a addres (aka pointer|reference)
for large (container) objects. We only need a solution for relative (say <= 1K) of container items.)

When the vector is ready, first write the items count and than the items at once in one chunk.
Ofcause if the container is already a vector we can skip that preperation phase.

For reading, the reverse should happen.
***/

    template<
        typename OutputStream_T,
        typename Char_T,
        std::size_t N
    >
    OutputStream_T&
operator < (OutputStream_T& os_,  std::basic_string<Char_T>const& _str)
{
    // Same result, but relaying on documented (cppreference) string continues characters layout with zero termination
    auto number_of_chars{_str.length()};
    os_ < number_of_chars;
    std::streamsize sizeOfChars = number_of_chars* sizeof(Char_T)-1;

    auto objectAddress = reinterpret_cast <typename OutputStream_T::char_type*>(_str.cbegin());
    os_.write( objectAddress, sizeOfChars);
    return os_;
}

    template<
         class OutputStream_T,
         template<
             typename _Object_T,
             typename ..._R
         >
         class CONTAINER_T,
         typename Object_T,
         typename... R         ENABLE_IF((is_iterable_v<CONTAINER_T<Object_T,R...>>))
    >
    OutputStream_T&
operator <  (OutputStream_T& os_,  CONTAINER_T<Object_T,R...>const& _container)            // and for map... s ???
{
    vector<Object_T> objects_vector(_container.begin(),_container.end());
    os_ < objects_vector;
    return os_;
}

namespace {
    template<
        typename OS_T,
        typename TU_T,
        size_t... I
    >
    void
_write_tuple(OS_T& os_, TU_T const& _tup, std::index_sequence<I...>) // requires C++17 index_sequence and folding
{
    (..., (os_ < std::get<I>(_tup)) );
}

} //namespace


    template<
        typename OS_T,
        typename ... T
    >
    OS_T&
operator < (OS_T& os_, std::tuple<T...> const& _tup)
{
    _write_tuple( os_, _tup, std::index_sequence_for<T...>{});
    return os_ ;
}


    template <class InputStream_T, class Object_T    ENABLE_IF(  Dflt_Bin_Stream_Operator_Traits_v<Object_T>) >
    InputStream_T&
operator > ( InputStream_T& _is, Object_T& object_ )
{
    std::streamsize sizeOfObject= sizeof object_;
    auto objectAddress = reinterpret_cast <typename InputStream_T::char_type*>(&object_);

    _is.read( objectAddress, sizeOfObject);
    return _is;
} // operator >

struct ATrivial {
    int m,n;
    void *context;
};

struct ANonTrivial: ATrivial {
    virtual void foo(){};
};


template <typename T>
void triviallity()
{
    INFO(std::boolalpha,VARVALS(tosics::util::type_name<T>(),std::is_trivially_copyable<T>::value));
}

#define TRIVIALLITY_OF(type) triviallity<type>()

void trivTest() {
    TRIVIALLITY_OF(int);
    TRIVIALLITY_OF(std::string);
    TRIVIALLITY_OF(ATrivial);
    TRIVIALLITY_OF(ANonTrivial);
}


uint64_t Context=10101010l;

void test() {
    ostringstream toPack;

    char ch{'a'}, dh{'?'}; // int n; float f;
    int k{9}, j{-1};
    ATrivial ant, anth;
    ant.m=ant.n=11;
    ant.context = &Context;
    int nulplaceholder{0};

    std::string m{"Can use >> and << instead"}, mh{""};
    toPack < nulplaceholder < ch < k < -1 < 2 < -3 < ant; //< word;
    toPack << "litteral chars";

    istringstream fromPack, fromPackAgain;
    auto const& ref= toPack.str();
    auto &c = toPack.view().front();
    //assert( &c == toPack.str().c_str() );
    INFO(PTRVAL(ref.c_str()),PTRVAL(&c));
    fromPack.str( &c);
    fromPackAgain.str( &c);

    nulplaceholder = 0;
    int z[]{7,7,7};
    fromPack >  nulplaceholder >  dh > j > z[0] > z[1] > z[2]; //> drow;

    // obscure values to test second fromPackAgain retrival
    dh='#', j=z[0]=z[1]=z[2]=0;
    ant.m=ant.n=0;

    fromPackAgain >  nulplaceholder >  dh > j > z[0] > z[1] > z[2] > anth; //> drow;

    INFO( VARVALS(nulplaceholder),
          VARVALS(toPack.str().size(),toPack.tellp(),fromPackAgain.str().size(),fromPackAgain.tellg()),
          VARVAL(ch),"to",VARVAL(dh),"and",VARVAL(k),"to",VARVAL(j),
          VARVAL(m),"to",VARVAL(mh),
          VARVALS(z[0],z[1],z[2]),
          VARVAL(&Context),
          VARVALS(anth.m,anth.m,anth.context)
        );

    //dh='?', j=-1;  // destroy  values red from copied stream

    //toPack > dh > j ;
    //INFO("org:",VARVALS(dh,j));
    /*std::array<int,6>*/ int nums[6]{1,2,3,4,5,6};


    INFO(VARVAL(toPack.str().length()));

    // stream out native integer array at once
    toPack < z;

    INFO(VARVAL(toPack.str().length()));
    INFO(VARVAL(is_array_v<decltype(nums)>));

}

#!
INFO_FUNC;
test();
//trivTest();


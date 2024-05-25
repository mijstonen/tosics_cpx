#pragma once
//cpx-common-impl.cpp
#ifndef CPX_COMMON_IMPL_CPP
#  define CPX_COMMON_IMPL_CPP 1

/* This file can be included to be compiled with the cpx script source code
 * It is as if this is in front of your scripting code at file scope.
 *
 * This file is included in cpx-main-definition-signature.hpp
 * ( Usage: #+common-impl.cpp )
 */

    namespace
fs = std::filesystem
;
    namespace
cpx {
// #include ...
// global functions and classes


//_____________________________________________________________________________________________________________________
    class
ScopeDecoratorLines
{
    // Simple decoration helper, allows fast inspection of a cpx script executing well, without need to
    // read from screen. Cpx reorts compilation failure in red. So if all is OK, you just look for geen
    // or red. The cpx script output is in between the scope decorator lines.
    // Usage:
    // {
    //    WITH_SCOPEDECORATORLINES;
    //    ....
    // }
  public:
        static void
    Line(char const* _color=GREEN, char _ch='-', size_t _repeat=120)
    {
        std::cout<< _color<< string(_repeat,_ch)<< RESET<< std::endl;
    }
CPP_MARKSRCLINE
  public:
    ScopeDecoratorLines()    { Line(GREEN,'<'); }
    ~ScopeDecoratorLines()   { Line(GREEN,'>'); }
};

#define WITH_SCOPE_DECORATOR_LINES cpx::ScopeDecoratorLines AUTO_ID

//_____________________________________________________________________________________________________________________
#ifdef CPX_VALIDATION_HASH

#ifndef CPX_VALIDATION_HASH
# error CPX_VALIDATION_HASH is not defined
#endif
    class
ApplicationValidator
{
        enum class
    validation_policy : int
    // Could be FindIndex result, can't be set but behavior is kept deterministic.
    {   notfound = 0
    // For runtime testing ApplicationValidator.
    ,   failtest
    // Be more strict then normal, enforce correct behavior and warn for legal but unrecommended or corner cases.
    ,   strict
    // Disable it at runtime, the will be no validation checks.
    ,   disabled
    // Will not abort execution when CPX_VALIDATION_POLICY wasn't found in runtime environment, the message bacomes a warning instead.
    ,   relaxed
    // Standard behavior, cpx apps can not run without CPX_VALIDATION_HASH environment varaiable matching.
    ,   normal
    }
    ;
        validation_policy
    myValidationPolicy = validation_policy::relaxed
    ;
        char const*
    m_compiletimeValidationHash = nullptr
    ;
        char const*
    m_runtimeValidationHash = nullptr
    ;
        void
    cstr2validationPolicy(char const* _validation_policy)
    {
            static char const*
        policyNames[]=
        {static_cast<char const*>(nullptr) /* <-- set on each call*/
            ,"failtest"
            ,"strict"
            ,"disabled"
            ,"relaxed"
            ,"normal"
        };
        int policyAsIndex=/*number of*/ITEMS_IN(policyNames);
        policyNames[0]=_validation_policy;
        FindIndex( &policyAsIndex , policyNames);
        if ( !policyAsIndex ) {
            INFO(HRED);
            INFO("Error:",VARVAL(_validation_policy),
                 "is not matching any policy and is ignored. "
                 "Try one of: 'disabled','relaxed','normal','strict' or 'failtest'"
                );
            INFO(NOCOLOR);
            return;
        }
        myValidationPolicy = static_cast<validation_policy>(policyAsIndex);
    }
  public:
    ApplicationValidator(char const *_compiletimeValidationHash) noexcept
    : m_compiletimeValidationHash(_compiletimeValidationHash)
    , m_runtimeValidationHash(getenv(ENV_CPX_VALIDATION_HASH))
    {
    }
  protected:
        void
    get_optional_runtime_policy()
    {
        if ( myValidationPolicy== validation_policy::disabled ) {
            return;
        }
        char* cpx_validation_policy=getenv(ENV_CPX_VALIDATION_POLICY);

        if ( cpx_validation_policy== NULL ) {
            // I hope you did not misspell it (see ENV_CPX_VALIDATION_POLICY) because setting it is optional and the
            // hardcoded will be used otherwise. There is a bit of rescue for developers if they recompile with
            // myValidationPolicy== validation_policy::strict.
            if ( myValidationPolicy== validation_policy::strict ) {
                INFO(HYELLOW);
                INFO("Warning:",VARVAL(ENV_CPX_VALIDATION_POLICY),"not retrieved from environment.");
                INFO(NOCOLOR);
            }
            return;
        }
        //else
        // dont use the default but whats retrieved here.
        cstr2validationPolicy( cpx_validation_policy);
    }
        bool
    fields_are_set() const
    {
        bool returnValue = true;
        if ( !m_compiletimeValidationHash ) {
            INFO(HRED);
            INFO("No usable compiletime validation hash, check ...ApplicationValidator instantiation.");
            INFO(NOCOLOR);
            returnValue = false;
        }
        if ( !m_runtimeValidationHash ) {
            INFO( ( myValidationPolicy== validation_policy::relaxed ) ?HYELLOW :HRED);
            INFO("Failed at runtime to retrieve enviromment variable:",VARVAL(ENV_CPX_VALIDATION_HASH),',');
            INFO("  it usually is defined when the cpx app is started via cpx or as executable source (starting with #!...cpx)");
            INFO("Resolve or set/export",ENV_CPX_VALIDATION_POLICY "=disabled");
            INFO("When building in release mode (for unrelated free binary executable), use cpx -C release ...");
            INFO(" runtime validation checking is omitted from the source and no special environment variables are required.");
            returnValue = false;
            INFO(NOCOLOR);
        }
        switch ( myValidationPolicy ) {
            case validation_policy::failtest:
                INFO(GREEN,"failtest: OK. cpx app was forced to fail the validation.",NOCOLOR);
                return false;
            case validation_policy::disabled:
                return true;
            case validation_policy::relaxed:
                if ( !m_runtimeValidationHash ) {
                    return true;
                }
                [[fallthrough]];
            case validation_policy::normal:
                [[fallthrough]];
            case validation_policy::strict:
                [[fallthrough]];
            default: // could be    _notfound
                return returnValue;
        }
    }
        int
    isValidationHashMismatching() const // returns non zero int when mismatching
    {
        switch ( myValidationPolicy ) {
            case validation_policy::failtest:
                return !0;
            case validation_policy::disabled:
                    return 0;
            case validation_policy::relaxed:
                if ( !m_runtimeValidationHash ) {
                    return 0;
                }
                [[fallthrough]];
            case validation_policy::normal:
                [[fallthrough]];
            case validation_policy::strict:
                [[fallthrough]];
            default:// could be _null
                return strcmp( m_compiletimeValidationHash, m_runtimeValidationHash);
        }
    }
//        operator
//    bool() const  // returns true if mathing otherwise false
//    {
//        return isValidationHashMismatching()? false: true;
//    }

public:
        void
    handle_validationHash_checking(ostream* pOs_=nullptr)
    {
        if ( myValidationPolicy == validation_policy::disabled ) {
            return;
        }

        LOCAL_MODIFIED(INFO_STREAM_PTR);
        INFO_TO(*pOs_);

        get_optional_runtime_policy();
        if ( myValidationPolicy == validation_policy::disabled ) {
            return;
        }
        if ( !fields_are_set() ) {
            tu::ThrowBreak("Missing application validation fields.");
        }
        if ( isValidationHashMismatching() ) {
            if ( pOs_ ) {
                char const* source = CPX_SOURCE_FILE;
                INFO("No maching validation hashes!",VARVALS(source,m_compiletimeValidationHash,m_runtimeValidationHash));
                INFO("CPX found a matching hash executable in its cache and asumed that it could use it, skipping compiling the source.");
                INFO("The validation Hashes are there to ensure that only the correct binary that belonged to the source is executed.");
                INFO("This excutable, does not match, it is from another source and therefor had to stop executing.");
            }
            tu::ThrowBreak("Executable, has same binary hash, but the validation Hashes (to verify it is the right executable) do not match.");
        }
    }
} //class ApplicationValidator
;
//_______________________________________________________________________________________

 PHP_BEGIN
    $CompileTimeValidationHash = getenv_or_die('CPX_VALIDATION_HASH',
        'Php transformation terminated because the compiletime requirement was not met.');
 PHP_END

# define CPX_VALIDATE_WITH_SOURCE (cpx::ApplicationValidator("<?= $CompileTimeValidationHash ?>").handle_validationHash_checking(&cerr ))
#else
# define CPX_VALIDATE_WITH_SOURCE
#endif // CPX_VALIDATION_HASH

} // namespace cpx

#endif // CPX_COMMON_IMPL_CPP

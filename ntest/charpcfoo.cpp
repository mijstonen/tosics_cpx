#!/usr/local/bin/cpx

constexpr long COLORBASE_ltrl = 9'000'000'000'000'000'000l;
constexpr long RED_ltrl    = COLORBASE_ltrl+1;
constexpr long GREEN_ltrl  = COLORBASE_ltrl+2;
constexpr long BLUE_ltrl   = COLORBASE_ltrl+3;

        template <long COLOR_ltrl>
        void
    foo()
    {
        INFO(HWHITE "NO Match" NOCOLOR);
    }
        template<>
        void
    foo<RED_ltrl>()
    {
        INFO( RED "foo" NOCOLOR );
    }
        template<>
        void
    foo<GREEN_ltrl>()
    {
        INFO(GREEN "foo" NOCOLOR);
    }
        template<>
        void
    foo<BLUE_ltrl>()
    {
        INFO(BLUE "foo" NOCOLOR);
    }

    void runtimeSelect(const long _color)
    {
        switch ( _color ) {
        case RED_ltrl:
            return foo<RED_ltrl>();
        case GREEN_ltrl:
            return foo<GREEN_ltrl>();
        case BLUE_ltrl:
            return foo<BLUE_ltrl>();
        }
        return foo<0l>();  // foo with any non matching number
    }

#!
    foo<BLUE_ltrl>();
    foo<235423523l>(); // fails

    for (auto v : {RED_ltrl,BLUE_ltrl,RED_ltrl,BLUE_ltrl,RED_ltrl,GREEN_ltrl}) {
        runtimeSelect(v);
    }



#!/usr/bin/env cpx
#^<ranges>
// See https://en.cppreference.com/w/cpp/ranges/range
// TODO #/is_iterable_v/is_const_itreable




#!
    INFO("range tests");
    INFO(VARVALS
    (   is_iterable_v<vector<int>>
    ,   ranges::range<vector<int>>
    ));

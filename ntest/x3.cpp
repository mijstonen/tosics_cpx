#!/usr/bin/env cpx
#|

    template <
        template <
            class ITEM_T,
            class... OPTS_P
        >
        class CONTAINER_TPL,

        class ITEM_T,
        class... OPTS_P
    >
    struct
ortho_container_items
{
    using container_type = CONTAINER_TPL<ITEM_T,OPTS_P...>;
};


#(
    ortho_container_items<vector,string,allocator<string>>::container_type vs_words={"Hello","vector","string"};
    ortho_container_items<list,string,allocator<string>>::container_type ls_words={"Hello","list","string"};
    ortho_container_items<deque,string,allocator<string>>::container_type ds_words={"Hello","deque","string"};

    Fake_use(vs_words,ls_words,ds_words);

        auto
    print=[](auto const & _container)
    {
        for( auto item : _container ) {
            INFO(VARVAL(item));
        }
    };

    print(vs_words);
    INFO();
    print(ls_words);
    INFO();
    print(ds_words);

#)

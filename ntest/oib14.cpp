#!/usr/local/bin/cpx

/*
 Generalization of signal management data handling. For signal data it is key to avoid fine grained per iten allocations
 and avoid copying of data to every
 */

// BUG in tosics::util. is_to_stream_writable does not return true for the function below
    template<
        typename _DataT
    >
    std::ostream&
operator<< (std::ostream& os_, const std::optional<_DataT>& _opt)
{
    if ( _opt ) {
        os_<< *_opt;
    }
    return os_;
}

//_____________________________________________________________________________________________________________________
class ObservedDataBagException_Base //
// besides being an exception it optionally can reroute the data.
: public std::length_error
{
  public:
        using
    std::length_error::length_error
    ;
        virtual std::byte const*
    optionalDataAddress()
    {
        return nullptr;
    }
        virtual
    ~ObservedDataBagException_Base()
    {
    }
} // ObservedDataBagException_Base
;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     template<
        typename _DataT
    >
class ObservedDataBagException //
// besides being an exception it optionally can reroute the data.
: public ObservedDataBagException_Base
{
    std::optional<_DataT> m_optional_data;

  public:
    ObservedDataBagException(const char* _what_arg, _DataT const* _pdata=nullptr)
    : ObservedDataBagException_Base(_what_arg)
    {
        if ( _pdata ) {
            // physical copy
            this->m_optional_data = *_pdata;
        }
    }
    ObservedDataBagException(const std::string& _what_arg, _DataT const* _pdata=nullptr)
    : ObservedDataBagException(_what_arg.c_str(),_pdata)
    {
    }
    ~ObservedDataBagException()  override
    {
    }
        decltype(m_optional_data) const &
    optional_data() const
    {
        return this->m_optional_data;
    }
        std::byte const*
    optionalDataAddress()
    {
        return ByteAddress_cast( &(this->m_optional_data) );
    }
} // ObservedDataBagException
;
//_____________________________________________________________________________________________________________________
    template<typename _ControlFieldT>
class ObservedDataBag_Base
;
//_____________________________________________________________________________________________________________________
    template<
        typename _ControlFieldT
    >
class ManagedData_Base //
{
        static constexpr _ControlFieldT
    RESERVED_CONTROLFIELD_VALUES=0xF
    ;
    // keep RESERVED_CONTROLFIELD_VALUES number space above MAX_REFCNT to detect invalid values caused by unbalanced
    // refCnt decrements or increments and if the intension is to make it undefined (aka for checking it is or is
    // not initialized), make it MAX+1 and we have about RESERVED_CONTROLFIELD_VALUES
    // possible special handling enummerations available.


  public:
        static constexpr _ControlFieldT
    MAX_REFCNT=numeric_limits<_ControlFieldT>::max() - RESERVED_CONTROLFIELD_VALUES;
    ;
        static constexpr _ControlFieldT
    UNDEFINED_REFCNT = MAX_REFCNT+1
    ;
        static constexpr _ControlFieldT
    DEFINED_REFCNT0 = MAX_REFCNT+2
    ;
        static constexpr _ControlFieldT
    MAX_SELFINDEX=numeric_limits<_ControlFieldT>::max() - RESERVED_CONTROLFIELD_VALUES;
    ;
        static constexpr _ControlFieldT
    UNDEFINED_SELFINDEX = MAX_SELFINDEX+1
    ;
  private:
        _ControlFieldT
    m_refCnt = UNDEFINED_REFCNT
    ;
        _ControlFieldT
    m_selfIndex = UNDEFINED_SELFINDEX
    ;
  public:
        constexpr
    ManagedData_Base()
    : m_refCnt{0} // uninitialized m_selfIndex and m_data
    {
    }
        virtual
    ~ManagedData_Base()
    {
    }
        ManagedData_Base<_ControlFieldT>&
    refCnt( decltype(m_refCnt) const& _refCnt)
    {
        ASSERT(_refCnt<MAX_REFCNT || _refCnt==DEFINED_REFCNT0);
        this->m_refCnt= _refCnt;
        return *this;
    }
        constexpr
        ManagedData_Base<_ControlFieldT>&
    unchecked_selfIndex( decltype(m_selfIndex) const& _selfIndex)
    {
        this->m_selfIndex= _selfIndex;
        return *this;
    }
        constexpr
        ManagedData_Base<_ControlFieldT>&
    selfIndex( decltype(m_selfIndex) const& _selfIndex)
    {
        ASSERT(_selfIndex<MAX_SELFINDEX);
        return this->unchecked_selfIndex(_selfIndex);
    }
        decltype(m_refCnt)
    refCnt() const
    {
        return this->m_refCnt;
    }
        bool
    isFree() const
    {
        return ( this->refCnt()== 0 );
    }
        bool
    isShared() const
    {
            auto
        refcnt=this->refCnt()
        ;
        return
            ( refcnt>0 )
            ? refcnt!=DEFINED_REFCNT0
            : false
            ;
    }
        void
    increase_refCnt()
    {
            auto
        refcnt=this->refCnt()
        ;
        if ( refcnt== DEFINED_REFCNT0 ){
            this->refCnt( 1);
        }
        else {
            this->refCnt( refcnt+ 1 );
        }
    }
        void
    decrease_refCnt()
    {
        auto refcnt=this->refCnt();
        if ( !refcnt
           || refcnt== DEFINED_REFCNT0 ){
            return;
        }
        if ( refcnt== 1 ) {
            this->refCnt( ManagedData_Base<_ControlFieldT>::DEFINED_REFCNT0);
        }
        else {
            this->refCnt( refcnt- 1 );
        }
    }
        void
    share()
    {
        this->increase_refCnt();
    }

        ObservedDataBag_Base<_ControlFieldT>*
    outerBase()
    ;
        void
        // protocol
    unshare()
    {
        this->decrease_refCnt();
        if ( this->isShared() ) {
            return;
        }
        // unshare() might be called from descructor,
        // hence throwing an exception is considered inappropiate
        // instead fallback to STATEREPORT to signal the error to
        // the user but with (unfortunate) less dramatic
        // program flow effects.
        STATEREPORT(this->outerBase()->deleteData( this));
    }

        constexpr
        decltype(m_selfIndex)
    unchecked_selfIndex() const
    {
        return this->m_selfIndex;
    }
        constexpr
        decltype(m_selfIndex)
    selfIndex() const
    {
        ASSERT(this->m_selfIndex<MAX_SELFINDEX);
        return this->unchecked_selfIndex();
    }
        virtual void
    throwData() const = 0
    ;
        virtual void
    setToDeletedState()  = 0
    ;
        virtual ManagedData_Base<_ControlFieldT> const*
    selfToBegin() const  = 0
    ;
        std::byte const*
    selfToOuter() const
    ;

} // ManagedData_Base
;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    template<typename _DataT,size_t  _SizeL,typename _ControlFieldT>
class ObservedDataBag
;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    template<
        typename _DataT,
        typename _ControlFieldT
    >
class ManagedData //
: public ManagedData_Base<_ControlFieldT>
{
    _DataT m_data;

  public:
        using
    data_type = _DataT
    ;
    ManagedData()
    : ManagedData_Base<_ControlFieldT>()
    , m_data()
    {
    }

    ManagedData(_DataT const& _data)
    : ManagedData_Base<_ControlFieldT>(), m_data(_data)
    {
    }
        ManagedData<_DataT,_ControlFieldT>&
    data( decltype(m_data) const& _data)
    {
        this->m_data= _data;
        return *this;
    }
    //} IN properties
    //{ OUT properties
        decltype(m_data) const &
    data() const
    {
        return this->m_data;
    }
        void
    throwData() const override
    {
            throw
        this->data();
    }

        void
    setToDeletedState()  override
    {
        using data_type = decltype(m_data);
        // call destructor of the containing data
        this->m_data.~data_type();
    }

    ~ManagedData() override
    {
        // implies this->m_item.~_DataT(); TEST: ~_DataT() being called.
        setToDeletedState();
    }
        ManagedData_Base<_ControlFieldT> const*
    selfToBegin() const  override
    ;
} // ManagedData
;

//_____________________________________________________________________________________________________________________
    template<
        typename _ControlFieldT
    >
class ObservedDataBag_Base //
{
  protected:
    // constexpr static _ControlFieldT     NEXT_FREE_END{ManagedData_Base<_ControlFieldT>::UNDEFINED_SELFINDEX}    ;

        static constexpr ptrdiff_t
    UNDEFINED_OFFSET = static_cast<ptrdiff_t>(ManagedData_Base<_ControlFieldT>::UNDEFINED_SELFINDEX)
    ;
        static ptrdiff_t
    OffsetToThis
    ;
        static bool
    Is_UninitializedOffset()
    {
        return ( OffsetToThis== UNDEFINED_OFFSET );
    }
        static void
    InitializeOffset(ptrdiff_t _offset)
    {
        ASSERT(_offset<0);
        OffsetToThis= _offset;
    }

  public:
        static
        std::byte const*
    ToInstanceAddress(std::byte const* _array_begin_address)
    {
        return OffsetToThis + _array_begin_address;
    }
  protected:
        constexpr
    ObservedDataBag_Base()
    {
    }
  public:
        virtual
    ~ObservedDataBag_Base()
    {
    }
        virtual tu::state_t
    itemCheck(ManagedData_Base<_ControlFieldT> const* _item_location) const  = 0
    ;
        bool
        // returns true if item check succeeds
    contains(ManagedData_Base<_ControlFieldT> const* _location) const
    {
        auto state = STATEREPORT(this->itemCheck( _location),'E');
        return ( state==0 );
    }
        bool
        // returns true if item check succeeds
    omits(ManagedData_Base<_ControlFieldT> const* _location) const
    {
       auto state = STATEREPORT(this->itemCheck( _location),SR_EXCLUDE_ALL(2,3),'A');
       return ( state? true: false );
    }
        virtual
        _ControlFieldT
    nextFree() const  = 0
    ;
        bool
    isEmpty() const
    {
        return !nextFree();  // nextFree() points first item with index 0
    }
        virtual tu::state_t
    deleteData(ManagedData_Base<_ControlFieldT>* deinitilizing_location_) = 0
    ;
} // ObservedDataBag_Base
;
    template<
        typename _ControlFieldT
    >
    ptrdiff_t ObservedDataBag_Base<_ControlFieldT>::
/* static */ OffsetToThis{UNDEFINED_OFFSET}
;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    template<
        typename _ControlFieldT
    >
    inline
    std::byte const*
    ManagedData_Base<_ControlFieldT>::
selfToOuter() const
{
        return
    ObservedDataBag_Base<_ControlFieldT>::
        ToInstanceAddress(ByteAddress_cast(this->selfToBegin()))
    ;
}
    template<
        typename _ControlFieldT
    >
    inline
    ObservedDataBag_Base<_ControlFieldT>*
    ManagedData_Base<_ControlFieldT>::
outerBase()
{
    auto outer_address= const_cast<std::byte*>(this->selfToOuter());
    return reinterpret_cast< ObservedDataBag_Base<_ControlFieldT>* >(outer_address);
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    template<
        typename _DataT,
        typename _ControlFieldT
    >
    inline
    ManagedData_Base<_ControlFieldT> const*
    ManagedData<_DataT,_ControlFieldT>::
selfToBegin() const
{
        auto
    negative_index_to_first_item_of_array= -static_cast<ptrdiff_t>(this->selfIndex())
    ;
        auto
    p_first_item_of_array = & (this[negative_index_to_first_item_of_array])
    ;
        return
    p_first_item_of_array
    ;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    template<
        typename _DataT,
        size_t  _SizeL,
        typename _ControlFieldT
    >
class ObservedDataBag //
: public std::array<ManagedData<_DataT,_ControlFieldT>,_SizeL>
, public ObservedDataBag_Base<_ControlFieldT>
{
  public:
        // using     manageddata_type = ManagedData<_DataT,_ControlFieldT>;

        using
    data_type = ManagedData<_DataT,_ControlFieldT>::data_type
    ;
        static constexpr _ControlFieldT
    MAX_BAG_ITEMS = numeric_limits<_ControlFieldT>::max()
    ;

    static_assert( -static_cast<ptrdiff_t>(_SizeL)> numeric_limits<ptrdiff_t>::min(),
                        "Too many items to be address offset subtractable.");

    static_assert( _SizeL<= MAX_BAG_ITEMS,
                        "Too many items for too small _ControlFieldT");

  private:
    struct HelpConstructOnce
    {
            _ControlFieldT
        m_threadSliceSize
        ;
            ptrdiff_t*
        m_pOffsetToThis= &(ObservedDataBag_Base<_ControlFieldT>::OffsetToThis)
        ;
        HelpConstructOnce(ptrdiff_t _offsetToThis)
        : m_threadSliceSize(
            std::max( 1u<< 7u ,
                static_cast<_ControlFieldT>(
                    std::sqrt(
                        16.0
                        *
                        static_cast<float>(
                            _SizeL
                        )
                        /
                        static_cast<float>(
                            std::max(
                                1u,
                                std::thread::hardware_concurrency()
                            )
                        )
                    )
                )
            )
        )
        {
           ( *m_pOffsetToThis )= _offsetToThis;
        }
    };
        size_t
    m_count=0
    ;
        _ControlFieldT
    m_nextFree=0
    ;
  protected:
        using
    ObservedDataBag_Base<_ControlFieldT>::/*static*/Is_UninitializedOffset
    ;
        using
    ObservedDataBag_Base<_ControlFieldT>::/*static*/InitializeOffset
    ;
        void
    increase_count()
    {
        ++m_count;
    }
        void
    decrease_count()
    {
        --m_count;
        ASSERT(static_cast<ssize_t>(m_count)>=0); // decrementing 0 is illegal
    }
  public:
    ObservedDataBag()
    // implies construct of std::array<_DataT,_SizeL> and _SizeL * default construct of _DataT
    {
            static HelpConstructOnce
        once{ ByteAddress_cast(this)- ByteAddress_cast( this->data()) }
        ;
        INFO(VARVAL(once.m_threadSliceSize));
            _ControlFieldT
        ixend=0,
        ixben // see while loop
        ;
            // growing deque never needs to relocate/move threads
            // and jthread is self joining
            std::deque<std::jthread>
        slice_workers
        ;
            auto
        par_SliceInitWorker=
            [this](std::deque<std::jthread>* sl_, _ControlFieldT _b,_ControlFieldT _e)
            {
                sl_->emplace_back(
                    [this,_b,_e]()
                    {
                        for(_ControlFieldT index=_b; index< _e; ++index) {
                            ( *this )[ index].unchecked_selfIndex( index+ 1 );
                        }
                    }
                );
            }
        ;
        while ( ixben=ixend, ixend=_SizeL, ( ixend- ixben ) > once.m_threadSliceSize ) {
            par_SliceInitWorker(&slice_workers , ixben, ixend= ixben+ once.m_threadSliceSize);
        }
        // do small _SizeL array or remaining items
        par_SliceInitWorker(&slice_workers , ixben, _SizeL);
        INFO(VARVAL(slice_workers.size()));
    }

    ~ObservedDataBag() override
    {
    }
        decltype(m_count)
    count() const
    {
            return
        m_count
        ;
    }
        size_t
    freeCount() const
    {
            auto
        in_use_count = this->count()
        ;
        ASSERT( in_use_count<= _SizeL);  // next expression may not be (unsigned wrapped) negative
            auto
        return_value = _SizeL- this->count()
        ;
            return
        return_value
        ;
    }

        _ControlFieldT
    nextFree() const  override
    {
        return this->m_nextFree;
    }
        bool
    isFull() const
    {
        auto next_free=this->nextFree();
        ASSERT(next_free<=_SizeL); // calling isFull() for exceptional next_free values is undefined behaviour
        return ( next_free==_SizeL );
    }
        tu::state_t // -3 -2 -1 0 1 2 3
    itemCheck( ManagedData_Base<_ControlFieldT> const* _item_location) const  override
    {
        if (_item_location>= this->cend() ) {
            [[unlikely]]
            // error: out of bounds (higher address)
            return tu::state_t{-1};
        }
        //otherwise

            ManagedData_Base<_ControlFieldT> const*
        p_self_to_begin=_item_location->selfToBegin();
        ;
            ManagedData_Base<_ControlFieldT> const*
        this_begin =this->cbegin()
        ;
            auto
        org_item_refCnt=_item_location-> refCnt()
        ;
            auto
        item_refCnt=
            ( org_item_refCnt== ManagedData_Base<_ControlFieldT>::DEFINED_REFCNT0 )
            ? 0
            : org_item_refCnt
        ;
        if ( p_self_to_begin> this_begin ) {
            // within bounds but not matching, this is a certain error when item is in use
            return tu::state_t{item_refCnt
                                ? -2 // error: looks that given _item location is not aligned with a item
                                :  2 // notification: this could happen if it is a item on the internal free
                                     // ... items stack<single linked list>
                              };
        }
        //otherwise

        if ( p_self_to_begin< this_begin ) {
            return tu::state_t{item_refCnt
                                ? -3 // error: out of bounds lower address
                                :  3 // notification: this could happen if it is a item on the internal free items
                                     // ... stack<single linked list>
                              };
        }
        //otherwise

            return  // p_self_to_begin== this_begin
        tu::state_t {org_item_refCnt
                        ? 0  // Ok, item is used and with selfIndex we should point
                             //  to the first item of the array to find the owning outer data structure.
                        : 1  // notification: unexpected because m_selfIndex is used to link to the next item
                             // ... of the free list
                    };
    }
        bool
    hasReservedUnused(ManagedData_Base<_ControlFieldT> const* _item_location) const
    {
        bool return_value=( STATEREPORT(this->itemCheck(_item_location))==0 );
        return return_value;
    }
  protected:_ControlFieldT
    next_free_after_added(_ControlFieldT _new_item_index, _DataT const& _data)
    {
            auto&
        item=( *this )[_new_item_index];
        ASSERT(!item.refCnt()); // ensure item is not in use

        // pop item on top of the internal free stack<single linked list> from the stack
            auto
        next_free= item.selfIndex();
        ;
        item.selfIndex( _new_item_index);
        item.data( _data);
        item.refCnt( ManagedData_Base<_ControlFieldT>::DEFINED_REFCNT0);
        ASSERT(this->hasReservedUnused( &item));
        this->increase_count();
        return next_free;
    }
  public:ManagedData_Base<_ControlFieldT>*
    addData(_DataT const& _data) // throws exception when bag is full
    {
        if ( this->isFull() )
            [[unlikely]] tu::ThrowBreak(ObservedDataBagException<_DataT>(FUNC_MSG("Bag is full, could not store data"), &_data));

        // pop item on top of the internal free stack<single linked list>
            auto
        index= this->nextFree()
        ;
        // set data in item and remove it from the free list
        this->m_nextFree= this->next_free_after_added( index, _data);
        return &( ( *this )[ index] );
    }

        ManagedData_Base<_ControlFieldT>*
    addOrReAssignDataAt(_ControlFieldT _index, _DataT const& _data) // throws exception when bag is full
    try {
            ManagedData<_DataT,_ControlFieldT>&
        item=this->at(_index)
        ;
        // here: we are sure that _index in range 0..._SizeL-1
        size_t valid;
            auto
        valid_decrement_or_throw=[&valid,_index,&_data]()
        {
            if( !valid )  // number of counted iterations exceeds freeCount()
                tu::ThrowBreak( ObservedDataBagException<_DataT>(FUNC_MSG(
                    "ERROR: Could not find "<< VARVALS(_index) <<" on the free items list"),&_data));
            --valid;
        };
            auto
        advance_to=[this](_ControlFieldT* pNextIndex_, auto _before_index) {
            return ( *pNextIndex_ )= ( *this )[ _before_index].selfIndex();
        };
        decltype(this->nextFree()) before_index, next;
            auto // Not refCnt(), contains() or omits() but itemCheck() to get all the answers at once more efficiently.
        state = STATEREPORT(this->itemCheck( &item ),SR_EXCLUDE_0_AND(2,3),'E')
        ;
        switch ( state ) {
            case 0: // not on free list, item aleady in use, re-assign, al refering handles observe the change
                item.setToDeletedState();
                item.data(_data);
                ASSERT( this->contains( &item) );
              break;
            case 2: case 3:
                before_index= this->nextFree();
                if( before_index== _index )
                    return this->addData(_data);

                // otherwise, the search index on the free list
                next=before_index;
                valid= this->freeCount();
                while( valid_decrement_or_throw(), advance_to( &next , before_index), next!= _index ) {
                    before_index= next;
                }
                // set data in item and remove it from the free list
                ( *this )[ before_index].selfIndex( next_free_after_added( _index, _data));
              break;
            default:
                // state has an unexpected and unhandled value
                tu::ThrowBreak(
                    ObservedDataBagException<_DataT>(FUNC_MSG(
                        "ERROR: itemCheck() on _location failed, "<<VARVALS(_index)), &_data));
              break;
        } // switch ( state )
        ASSERT( this->contains(&item) );
        return &( ( *this )[ _index] );
    } // try item=this->at(_index)
    catch ( out_of_range const& _array_at_out_of_range) {
        tu::ThrowBreak(
            ObservedDataBagException<_DataT>(FUNC_MSG(
                "_index is out of range 0.._SizeL-1 . "<< _array_at_out_of_range.what()), &_data));
    } // catch _array_at_out_of_range, addOrReAssignDataAt()


        tu::state_t
    deleteData(ManagedData_Base<_ControlFieldT>* deinitilizing_location_) override
    {
        if(not this->hasReservedUnused( deinitilizing_location_) ) {
            return  tu::state_t{-1};
        }

        // otherwise and only if reference count of a used item reached 0
            ManagedData_Base<_ControlFieldT>&
        item= *deinitilizing_location_
        ;
        this->decrease_count();
        // push deleted item to top of the internal stack<single linked list>
        _ControlFieldT old_item_selfindex=item.selfIndex();
        item.unchecked_selfIndex( this->nextFree());
        this->m_nextFree = old_item_selfindex;
        item.refCnt(0);
        item.setToDeletedState();  // ManagedData object is not destroid, however the contained data should.
        ASSERT(this->omits( deinitilizing_location_));
        return tu::state_t{0};
    }
        size_t
    freeItemsCount() const
    {
        size_t free_items_count{0};
        auto next_free= this->nextFree();
        while ( next_free!= _SizeL ) {
            auto const& free_item= ( *this )[ next_free];

            // Besides and in debug mode, check that nothing referes to the free item
            ASSERT(free_item.refCnt()==0);
            ASSERT(this->omits( &free_item ));

            ++free_items_count;
            next_free= free_item.unchecked_selfIndex();
        }
        return free_items_count;
    }
        size_t
    inUseItemsCount() const
    {
        size_t in_use_items_count{0};
        for( auto const& item: ( *this ) ) {
            if ( this->contains( &item ) ) {
                ++in_use_items_count;
            }
        }
        return in_use_items_count;
    }
        size_t
    referencedItemsCount() const
    {
        size_t referenced_items_count{0};
        for( auto const& item: ( *this ) ) {
            if ( item.refCnt() ) {
                ++referenced_items_count;
            }
        }
        return referenced_items_count;
    }
} // ObservedDataBag
;
//_____________________________________________________________________________________________________________________

  //#+ findcpx
#+ oib14sub/oib14_inner1.cpp

//=====================================================================================================================
#define ODB_SIZE 100
// next is same value without thousands separator, PHP can't handle it
#define PHP_ODB_SIZE 100

using controlfield_t = uint32_t;

#!
    ObservedDataBag<long,ODB_SIZE,controlfield_t> Odb;
    WITH_SCOPE_DECORATOR_LINES;

        using
    data_bag_type = decltype(Odb)
    ;

    auto pOdb=&Odb;
    INFO(FUNC_MSG("Before, you may need to be patient ..."),
         VARVALS(Odb.inUseItemsCount(),Odb.freeItemsCount(),Odb.count()));
    try {
        BagItemHandle<controlfield_t> bih(Odb.addData(4l));
        BagItemHandle<controlfield_t> nih(Odb.addOrReAssignDataAt(static_cast<controlfield_t>(1),10001l));
        BagItemHandle<controlfield_t> mih(Odb.addOrReAssignDataAt(static_cast<controlfield_t>(99),100099l));
        INFO(VARVALS(bih.data<long>(),nih.data<long>(),mih.data<long>()));
  PHP_BEGIN
    $iterations=min(PHP_ODB_SIZE,10);
    for($i=0;$i<$iterations;++$i){PHP_HERE_BEGIN(A)
        INFO("PHP iteration i:",$i);
        BagItemHandle<controlfield_t> bih{$i}(Odb.addData({$i}l+5l));
        BagItemHandle<controlfield_t> dup{$i}(bih{$i});
        ASSERT(bih{$i}.pItem()->refCnt()==2);
        ASSERT(dup{$i}.pItem()->refCnt()==2);
    PHP_HERE_END(A)}
  PHP_END
        INFO(VARVAL(bih0.outer_cast<data_bag_type>()->count()));
        INFO(FUNC_MSG("Verification, you may need to be patient ..."),
             VARVALS(Odb.inUseItemsCount(),Odb.freeItemsCount(),Odb.count()));
        ASSERT(( Odb.inUseItemsCount()+ Odb.freeItemsCount() )== ODB_SIZE);
        auto v=bih0.data<long>();
        v+=10;
        bih0.data<long>(v);
        // retrieving the bag object address from a handle is the main goal of the ObservedDataBag design
        //    ObservedDataBag<long,ODB_SIZE>*
        ASSERT((bih0.outer_cast<ObservedDataBag,long,ODB_SIZE>()==bih1.outer_cast<ObservedDataBag,long,ODB_SIZE>()));
  PHP_BEGIN
    for($i=0;$i<$iterations;++$i){PHP_HERE_BEGIN(B)
        INFO(VARVAL(bih{$i}.data<long>()));
        ASSERT((bih{$i}.outer_cast<data_bag_type>()==pOdb));
        ASSERT((dup{$i}.outer_cast<decltype(Odb)>()==pOdb));
        INFO(VARVAL(type_name(*bih{$i}.pItem())));
    PHP_HERE_END(B)}
  PHP_END
 #if 1
        try {
            bih0.pItem()->throwData();
        }
        catch ( long const& )
        {
            INFO("catched long");
        }
        catch( ... )
        {
            INFO("catched ... ?");
        }
 #endif
    }
    catch( ObservedDataBagException<long> const& odbX) {
        INFO(FUNC_MSG("catch ObservedDataBagException"),VARVAL(odbX.what()));
        cout<< "'" << odbX.optional_data()<< "'"<< endl;
        //  INFO(VARVAL(odbX.optional_data()));
    }
    INFO(FUNC_MSG("End, you may need to be patient ..."),
         VARVALS(Odb.inUseItemsCount(),Odb.freeItemsCount(),Odb.count()));

  public:
        template<
            typename _DataT
        >
        BagItemHandle<_ControlFieldT>&
    data( _DataT const& _data)
    {
            auto
        p_managed_data=this->managedData_cast< _DataT >()
        ;
        p_managed_data->data(_data);
        return *this;
    }

        template<
            typename _DataT
        >
        _DataT const&
    data() const
    {
            BagItemHandle<_ControlFieldT>*
        rw_this = ConstRemovedFrom(this)
        ;
            ManagedData<_DataT,_ControlFieldT>*
        pManagedData = rw_this->managedData_cast<_DataT>()
        ;
            ManagedData<_DataT,_ControlFieldT> const*
        ro_mgt_data = ConstAddedFrom(pManagedData)
        ;
            return
        ro_mgt_data->data()
        ;
    }

        template<
            typename _TargetT
        >
        _TargetT*
    outer_cast()
    {
            using
        target_data_type = _TargetT::data_type
        ;
            return
        DYNAMIC_CAST_OR_THROWEXCEPTION(
            _TargetT*,
            this->managedData_cast<target_data_type>()->outerBase(),
            ObservedDataBagException<target_data_type>,
            "Failed downcast #1"
        );
    }

        template<
            template <typename,size_t,typename> typename _TargetTT,
            typename _DataT,
            size_t _SizeL
        >
        bag_target_type<_TargetTT,_DataT,_SizeL>*
    outer_cast()
    {
            return
        this->outer_cast< bag_target_type<_TargetTT,_DataT,_SizeL> >()
        ;
    }

    template<
        typename _ControlFieldT
    >
class BagItemHandle //
{
  private:
        ManagedData_Base<_ControlFieldT>*
    m_pItem
    ;
  public:
        template<
            template <typename,size_t,typename> typename _TargetTT,
            typename _DataT,
            size_t _SizeL
        >
    using bag_target_type = _TargetTT<_DataT,_SizeL,_ControlFieldT>
    ;
    BagItemHandle(ManagedData_Base<_ControlFieldT>* _pItem)
    : m_pItem(_pItem)
    {
        this->m_pItem->share();
    }
    BagItemHandle(BagItemHandle const& _other)
    : BagItemHandle( _other.m_pItem)
    {
    }
        ManagedData_Base<_ControlFieldT>*
    pItem() const
    {
        return this->m_pItem;
    }

    ~BagItemHandle()
    {
        this->m_pItem->unshare();
    }
  protected:
       template<
            typename _DataT
        >
        ManagedData<_DataT,_ControlFieldT>*
    managedData_cast()
    {
            using
        target_type = ManagedData<_DataT,_ControlFieldT>*
        ; // because of comma's in argument, cause macro parsing error(s)

        return
            DYNAMIC_CAST_OR_THROWEXCEPTION(
                target_type, this->pItem(),
                ObservedDataBagException<_DataT>, "Failed downcast #2"
            );
    }
#+ ../oib14_inner_public.cpp
} // BagItemHandle
;

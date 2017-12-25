#include "treeset.h"

// Not to compile or run as cpx test. Here is some complex code to use test cppstripws.cpp during its development.

namespace Qnode
{
using namespace util;

/*
________________________________________________________________________________________________________________________
*/
void TreeSet::byte_vector::reverse_bytes()
{
    size_t index_back= size()-1;
    size_t index_forw=0ULL;
    while ( index_forw< index_back ) {
        std::swap( (*this)[index_forw++], (*this)[index_back--]);
    }
}
/*
________________________________________________________________________________________________________________________
copy the bytes in the vector and - if neccesary - zero pad the remaining
bytes in the keyblk_
*/
int TreeSet::byte_vector::copy_to_keyblk( key_block* keyblk_) const
{
    size_t byte_cnt= size();
    for ( size_t i=0; i< keyblk_->size ; ++i ) {
        if ( i>=byte_cnt ) {
            for ( ; i< keyblk_->size; ++i ) {
                const_cast<byte_t*>(keyblk_-> adres)[i]=0;
            }
            return 1; // oversized keyblk, all key bytes in keyblk, the severity is application depended
        }
        const_cast<byte_t*>(keyblk_-> adres)[i]=(*this)[i];
    }
    // here i==keyblk_->size
    return ( keyblk_->size != byte_cnt )  ? -1  // lost bytes that where in the key  or to may bytes copied
                                        :  0  // keyblk excactly matches the key
                                        ;
}//copy_to_keyblk
/*
________________________________________________________________________________________________________________________
*/
int TreeSet::byte_vector::copy_reversed_to_keyblk( key_block* keyblk_) const
{
    size_t byte_cnt= size(), max_index= byte_cnt- 1;
    for ( size_t i=0; i< keyblk_->size ; ++i ) {
        if ( i>=byte_cnt ) {
            for ( ; i< keyblk_->size; ++i ) {
                const_cast<byte_t*>(keyblk_-> adres)[i]=0;
            }
            return 1; // oversized keyblk, all key bytes in keyblk, the severity is application depended
        }
        const_cast<byte_t*>(keyblk_-> adres)[i]=(*this)[ max_index- i ];
    }
    // here i==keyblk_->size
    return ( keyblk_->size != byte_cnt )  ? -1  // lost bytes that where in the key or to may bytes copied
                                          :  0  // keyblk excactly matches the key
                                          ;
}//copy_to_keyblk

/*
________________________________________________________________________________________________________________________
*/
int TreeSet::byte_vector::copy_to_string( std::string* stdstring_) const
{
    size_t byte_cnt= size();
    stdstring_->reserve( byte_cnt);
    stdstring_->clear();
    for ( size_t i=0; i< byte_cnt; ++i ) {
        char ch= static_cast<char>((*this)[i]);
        if ( !ch ) {
            if ( stdstring_->length()+1 != byte_cnt ) {
                return -1; // lost bytes after zero termination that where in the key or to may bytes copied
            }
            return 0; // normal zero terminated string
        }
        stdstring_->push_back( ch);
    }
    return 1; // string excactly matches the key but key did not zero terminate,
                // std::string should have done it properly anyway
}

/*
________________________________________________________________________________________________________________________
*/
int TreeSet::byte_vector::copy_reversed_to_string( std::string* stdstring_) const
{
    size_t byte_cnt= size(), max_index= byte_cnt- 1;
    stdstring_->reserve( byte_cnt);
    stdstring_->clear();
    for ( size_t i=0; i< byte_cnt; ++i ) {
        char ch= static_cast<char>((*this)[ max_index- i ]);
        if ( !ch ) {
            if ( stdstring_->length()+1 != byte_cnt ) {
                return -1; // lost bytes after zero termination that where in the key or to may bytes copied
            }
            return 0; // normal zero terminated string
        }
        stdstring_->push_back( ch);
    }
    return 1; // string excactly matches the key but key did not zero terminate,
                // std::string should have done it properly anyway
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//private:
/*
________________________________________________________________________________________________________________________
IMPORTAND:
 that this method navigates the common_locals state to the root of the treeset and the position
 information gets lost. That's why this method is private and has a leading underscore and
 is not const. It is called by TreeSet::common_locals::get_reversed_key( TreeSet::byte_vector *key_vector_) const
 on a clone of the common_locals object.
*/
void TreeSet::common_locals::_extract_reversed_key( TreeSet::byte_vector *key_vector_)
{
    byte_t key_byte(0);
    do {
#if FAST // assumes that there are no partial keys, for every key byte go up 4 parents and only the last one is checked
        // See also comments in '#if FAST' block of TreeSet::common_locals::to_parent().
        //
        // The "unconditional optimized 'to parent'" calls here are more optimized, 'child' is not assigned (it would be
        // overwritten without use anyway). And parent_idx is not determined (GET_(idxP) is used directly) since testing
        // it is optimized away.

        key_byte= child_id;
        // unconditional optimized 'to parent'
        child_id= parent->GET_(cldP);
        parent= parent->folded_child( NodeFieldBuffer::Calculate_distance( parent->GET_(idxP)), parent->GET_(sigP));

        key_byte|= ( child_id<< 2 );
        // unconditional optimized 'to parent'
        child_id= parent->GET_(cldP);
        parent= parent->folded_child( NodeFieldBuffer::Calculate_distance( parent->GET_(idxP)), parent->GET_(sigP));

        key_byte|= ( child_id<< 4 );
        // unconditional optimized 'to parent'
        child_id= parent->GET_(cldP);
        parent= parent->folded_child( NodeFieldBuffer::Calculate_distance( parent->GET_(idxP)), parent->GET_(sigP));

        key_byte|= ( child_id<< 6 );
#else
                                           // NOTE child_id can only be 0, 1 ,2 or 3 and fits in 2 bits

        key_byte= child_id;                // now have 2 bits  here assigned (0,1,..........) .

        if  ( !to_parent() ) {
            goto LB_push_partial_keybyte;  // only 1 node of 2 bits
        }
        key_byte|= ( child_id<< 2 );       // now have 4 bits  here assigned (...,2,3,......)

        if  ( !to_parent() ) {
            goto LB_push_partial_keybyte;  // only 2 nodes, 4 bits
        }
        key_byte|= ( child_id<< 4 );       // now have 6 bits  here assigned (......,4,5,...)

        if  ( !to_parent() ) {
            goto LB_push_partial_keybyte;  // only 3 nodes, 6 bits
        }
        key_byte|= ( child_id<< 6 );       // now have 8 bits  here assigned (..........,6,7)
#endif

        key_vector_-> push_back( key_byte);
    }
    while ( to_parent() );

    return;
#if !FAST
LB_push_partial_keybyte:
    key_vector_-> push_back( key_byte);
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//protected:

/*
________________________________________________________________________________________________________________________
Get a new NodeField from the allocated memory block (for heap allocation, see property capacity, the allocated memory
block changes accordingly to this property). If the NodeField can't be allocated, the memory block is reallocated
according to the grow_algorithm.

REMARK
 Due to the design and allocation strategy it is well possible that the memoryblock grows and is reallocated while
 there are still free locations in the original memory block, this is NO bug. See alloc.cpp for the details.

*/

const NodeField* TreeSet::common_locals::claim()
{
    // make a non const reference to the const parent, so only one cast is required
    NodeField*& modify_at_parent= const_cast<NodeField*&>(parent);
    TreeSet*&   modify_at_treeset= const_cast<TreeSet*&>(treeset);

    const NodeField* return_value= nullptr;

    if ( treeset->should_reallocate() ) {
        // In addition to reallocate the memory of the treeset::NodeFieldBuffer, the common_locals fields
        // must be re"initialized to reflect change of the addresses of the treeset::NodeFieldBuffer
        path_handle parent_path( *this); //store location reflected by 'this'fields in a address independed path_handle
        modify_at_treeset-> reallocate(); // IMPORTAND: reallocate() has to take place between determination of..
                                          // ..parent_path and reinitializing common_locals
#if     DEBUG
        if (  treeset-> capacity()<= /* stronger  == */TINY_QNODES_IN_TREESET ) {
            ASSERT( ! treeset->count() );       // There should be no data (count()==0) in a NodeFieldBuffer that has only room for 'TINY_QNODES_IN_TREESET' nodes
        }
#       endif // DEBUG
        new ( this) common_locals(treeset,parent_path); // re-initialize the fields to reflect prior location
    }//if


    //
    // retry, prevents this loop going on for ever in case the allocation algorithm fails (that would be a serious
    //   error in the system). The algorithm should never fail, but that can not be guaranteed and must be proven
    //    in practice. Hence it is importand to catch such cases. For sure for production code this exception is
    //     unacceptable. But there a many possiblities left unexplored to improve allocatabillity of nodes.
    //
    // With retry=1, there are no reallocations.
    // The minimum for retry is 2. If retry is 2 we can detect early cases. Ideally all reallocations succeed the
    // second time. In fact 4 is too high factor and  allocation should certainly succeed.
    // But it is small enough to prevent common systems go out of memory or the hanging in this loop.
    //
    for ( unsigned retry= 4; retry; --retry ) {
        /*|$retry%u claim retries to go|*/
        try {
            return_value= modify_at_treeset-> NodeFieldBuffer::claim( modify_at_parent, child_id);
        }
        catch( std::logic_error& le ) {
            std::cerr<< "logic error in:"<< __PRETTY_FUNCTION__<<le.what()<<std::endl;
            throw;
        }
        if ( return_value ) {
#if ALLOC_DEBUG
            ptrdiff_t root_offset= return_value- root;
            std::cerr<< " o:"<< root_offset<< std::endl;
#endif
            return return_value;
        }
        //(else) reallocate in a attempt to find new unused nodefields
        //  NOTE: this is not guaranteed to help, probably it does not
        /*|safe location from 'this'fields|*/
        path_handle parent_path( *this);
        modify_at_treeset-> reallocate();
        /*|re-initialize 'this'fields|*/
        new ( this) common_locals(treeset,parent_path);
    }//for

    Throw_Error_Break(
        std::logic_error(
            "Allocating node(s) failed due to allocation algorithm or table NodeFieldBuffer::Index2AllocationStep"));

    return return_value;
}


/*
________________________________________________________________________________________________________________________

*/
keylength_t TreeSet::common_locals::keylength(bool _count_nodes_orelse_requiredkeybytes/*=false*/)
{
    keylength_t return_value(0);
    do {
        ++return_value;
    }
    while ( to_parent() );

    if ( _count_nodes_orelse_requiredkeybytes ) {
        // the length in bit pairs of the key which is the length of the NodeField path in number of nodes
        return return_value;
    }

    // Its most likeyly that keys are a hole number of bytes
    // but to be sure - if that is not the case - that the entire key would fit in a
    // byte array of 'keylength()' length we roundup (ceil) the return_value;

    auto rest= return_value & 0x3;  // same as mod 4 but fast
    return_value>>= 2;              // divide by 4 but fast
    if ( rest ) {
        ++return_value;             // roundup (ceil)
    }
    return return_value;
}


/*
________________________________________________________________________________________________________________________
The return value is for internal use only and not part of the API interface to the using application.
The API (of the callers that pass the return value) only states that a return value < 0 indicates a error.
*/
int TreeSet::common_locals::remove( NodeField const* _root)
{
    if  ( child== _root ) {
        return -1;               // nothing left to remove, the root it self has no parent and cant be removed
    }
    if  ( child== parent ) {
        return -2;               // apparrently initializing path_handle pointed at a wrong place or
    }                           //   to root (rootOffset==0), there is no child entry to erase
    if  ( child->used() ) {
        return -3;               // if I would delete the key now, qnodes and resources remain allocated and cant be
    }                           //  reused (memory leak) to root (rootOffset==0), there is no child entry to erase
                               //   prior actions (removeAll, removeData,...) must have cleared the child before
                              //    this takes place, arriving here with something in *next is a error
#if DEBUG
    size_t allocated_before= treeset->count();
#endif
    //          MUST refer (&) the parent pointer, because the parent pointer value changes with 'to_parent()'
    //          'modify_at_parent' is in fact the same pointer as 'parent'except that is not const so modifyable
    NodeField*& modify_at_parent    = const_cast<NodeField*&>(parent);

    for(;;) {

        modify_at_parent->erase_child_entry( child_id);// erase the entry by which 'parent' accesses 'next
                                                       // (figurely: parent+"folded distance of idx by child_id"==next)
        if  ( parent== _root ) {
            break; // arrived at the local root (path_handle initialized) to which the remove action had to talke place
        }

        if  ( !to_parent() ) {
            break; // I arrived upper root of the TreeSet
        }
        if  (  child-> has_childs() ) {
            break; // despite of erasing my own child entry, there are still other child entries and 'next'
                   // is shared amoung multiple key paths, I can not remove it now
        }

        removeQnode(/*child*/);
        ++kprmv_cnt;
    }
#if DEBUG
    size_t  allocated_after= treeset->count();
    ASSERT( allocated_after== allocated_before- kprmv_cnt);
#endif
    return kprmv_cnt;
}
/*
________________________________________________________________________________________________________________________
Find the key managed in this (operation_locals) object.
Return true if found and optionally set tail (parent of the data)
*/

bool TreeSet::operation_locals::find( path_handle *tail_)
{
    // fks ( Find Key Shift) preserves the shift status
    // to be picked op by methods which use find to implement
    // their operation. This has a minor impact on find
    // it self.

    /*|$key%p, start at $parent%p $child_id%hhu $child%p|*/


#if DEBUG
    int dummy_debug_step_here=1; FAKE_USE(dummy_debug_step_here);
#endif
    for (;;) {
        keybyte= *key;
        /*|$key%p  $keybyte%hhx as hex $keybyte%c as char|*/

        child_id= ( static_to_type_cast(child_id,keybyte)>>  ( fks= 6 ) )&0x3;
        child= treeset-> child_of( parent, child_id);
        /*|$fks%d $parent%08p $child_id%hhu $child%08p|*/

        if  ( child== parent ) {
            return false;
        }

        parent= child;
        child_id= ( static_to_type_cast(child_id,keybyte)>>  ( fks= 4 ) )&0x3;
        child= treeset-> child_of( parent, child_id);
        /*|$fks%d $parent%08p $child_id%hhu $child%08p|*/
        if  ( child== parent ) {
            return false;
        }

        parent= child;
        child_id= ( static_to_type_cast(child_id,keybyte)>>  ( fks= 2 ) )&0x3;
        child= treeset-> child_of( parent, child_id);
        /*|$fks%d $parent%08p $child_id%hhu $child%08p|*/
        if  ( child== parent ) {
            return false;
        }

        parent= child;
        child_id= static_to_type_cast(child_id,keybyte)&0x3; ( fks= 0 );  // no >> required
        child= treeset-> child_of( parent, child_id);
        /*|$fks%d $parent%08p $child_id%hhu $child%08p|*/
        if  ( child== parent ) {
            return false;
        }

        ++key;
        if  ( key>= kend ) {
            /*|Found! End at $parent%p $child_id%hhu $child%p|*/
            break;
        }
        parent= child;
    }//for

    set_tail( tail_);

    // found the key
    return true;

}//find


/*
_______________________________________________________________________________________________________________________
Called by insert(), insert_new() and refer(). These callers call find() for the part of the keypath which was already
created. The operation_locals object keeps track of where find() gave up and create_remaining_keypath() continiues
there to ceate the remaining keypath.
*/
void TreeSet::operation_locals::create_remaining_keypath()
{
    NodeField*& modify_at_parent= const_cast<NodeField*&>(parent);

    /* I use a duffy device to resume in the fks (Find Shift Key) status in which the called '...find(..)' has left
    the operation locals. Note that 'find()' did fail on the state reflected by fks; The duffy device only makes a
    difference in the first cycle of the loop. Note that fks, is not maintained for the remaining part of the loop
    to end of the key. */
    /*|Enter duffy device with $fks%d $key%p, start at $parent%p $child_id%hhu $child%p|*/
    switch ( fks ) {
        case FKS_UNKNOWN:
      for (;;) {
            keybyte = *key;
            /*|$keybyte%hhd $keybyte%c|*/

            child_id= ( static_to_type_cast(child_id,keybyte)>> 6 )&0x3;
        case 6:
            child    = claim();
            /*|$parent%p $child_id%hhu claimed $child%p|*/

            modify_at_parent= const_cast<NodeField*>(child);
            child_id= ( static_to_type_cast(child_id,keybyte)>> 4 )&0x3;
        case 4:
            child    = claim();
            /*|$parent%p $child_id%hhu claimed $child%p|*/

            modify_at_parent= const_cast<NodeField*>(child);
            child_id= ( static_to_type_cast(child_id,keybyte)>> 2 )&0x3;
        case 2:
            child    = claim();
            /*|$parent%p $child_id%hhu claimed $child%p|*/

            modify_at_parent= const_cast<NodeField*>(child);
            child_id= static_to_type_cast(child_id,keybyte)&0x3;
        case 0:
            child    = claim();
            /*|$parent%p $child_id%hhu claimed $child%p|*/

            ++key;
            if  ( key>= kend ) {
                /*|End of $key%p|*/
                return;
            }
            modify_at_parent= const_cast<NodeField*>(child);
      }//for (in switch)
        break;
        default:
        ASSERT(false);  // incorrect fks
    }//switch
}
/*
________________________________________________________________________________________________________________________
TODO: To be reviewed again, comments (regarding to _data_==nullptr or !_data_ ) are obselete  (see ****)

return value
     0 indicate that the key did not exist as a path in the TreeSet there is no resource to be freed
       because there was no data (jet) stored. The key is inserted into the tree set and given data pointer
       is inserted (hence the data content is attached to the key).
       This is the (most) desired and common outcome for the insert operation.

   > 0 returned means that pointer/resource-id *_data_ should (possibly) be cleaned up because the
       new inserted value replaces/overwrites it.

   -1..-1000 special (non exception) controlled situations and because return_value<0,
        the  returned data should not be cleaned up!!!
    -1..-10 cases where the entire key was found
    -11..20 cases where the key (at least partly) needed to be created

**** -2 found key, but there was no (input) data given, any existing data remains untouched, did the same as find()
    -11 keypath is not created because there was no _data_ provided

    -11 new key was created partly but no (input) data was given, hence there is no data associated to the created key
       access via the last 2 bits of the last key byte will fail, but all the QF_qnode key nodes exist.

< -1000  internal software errors, only for development purposes, must be impossible to occur in a released version
  -1001 coding error in insert(), no return_value has been assigned and the effect of the insert operation is unknown.


 example:
    // insert handler
    auto data_pointer= &resource;
    int insert_termination_state= ts->insert( &ph4key, data_pointer);

    switch ( insert_termination_state ) {

        case 1: // swapped resource with content in ts
            delete data_pointer;
            break;

        case 0: // Ok, put in new data, nothing came out
            break;

        case -1: case -2: case -11: case -12:
            throw std::runtime_exception(
                       "insert() has not the desired effect. Could not associate data to the key"
                  );

        default:
            assert(false);  // Bad...!  the developer/maintainer ( I guess thats me) of insert() was too sloppy  ;-(
    }

    Note:
        The reason that you can insert while _data_==nullptr is NOT to be used actively, it is however
        good for robustness. The proper operation of insert is made independend of the passed pointer
        and will not misbehave in case a nullptr slips through because it was (in some case) not checked
        on nullptr.
        Unless you (or... rather me) excactly know what you do, DO NOT PASS nullptr in the _data_ argument.

*/

#if 0
/////////////////////////////// OBSOLETE /////////////////////////////////////////
int TreeSet::operation_locals::insert(path_handle* tail_,  genericdata_t* _data_  )
{
/*
 Possible change:  if the data pointer is nullptr, then remove the key that was to be inserted
                   instead of creating it.

                   rationale 1:
                     Simple and common way to erase a item from a collection is by setting the reference
                     of it to nullptr (NULL,NIL,0... e.g). Its also a simple method of controlling the
                     (kind of 'chained') action where on basis of the data pointer a entry is either inserted
                     the treeset or removed from it.

                   rationale 2:
                     A partly inserted key path creation (the current solution) can polute the treeset.
                     And without manual action, those partial keys are hard to remove. If the key (of cause
                     as far nodes are not shared with other keys) is removed instead, no partial keys are
                     created in the first place and the treeset stays clean.

                   rationale 3:
                     A well defined behaviour of insert() when _data_==nullptr
 */
    int return_value= -1001;  // this value MUST be overwritten to designate the termination state of this method.
    //ASSERT(_data_);    relaxed _data value, can also create a path to a node but leave it the way it was created

    fks= FKS_UNKNOWN;

    // by design, child can't be nullptr, child = parent '+ delta' which in the [base...end) range
    // but we play for being paranoid in the debug mode for free so
#if FAST
    // 'parent' pointer is (possibly) not related to 'child' and should NOT be used in 'insert()'
    ASSERT( child);
#else
    ASSERT( parent && child);
#endif

    NodeField*& modify_at_child= const_cast<NodeField*&>(child);

    if  ( find() ) {
        if ( _data_ ) {
            //ASSERT(!(*_data_ &0x02ul));// bit 1 indicates that _data is of a reserved node (QF_node or QF_internal)
                                         // type and not a data node type, see field.h:enum struct Field_type
                                         // insert() should not be (ab)used to a insert node of a reserved type.

            //  check for equal pointers/resource id's, is the same object refered
            if  ( *_data_ == genericdata_t(*modify_at_child) ) {
                return_value= -1; // key found, data is the same object, caller should NOT cleanup
            }
            else {
                // The existing original is given back to the caller for deallocation, free resources e.g.
                NodeField::SwapWithGeneric( modify_at_child, _data_ );

                return_value= 1;
            }
        }
        else {
            // TODO internal remove ( that is by making use of the already performed keypath navigation by
            // operation_locals::find

            return_value= -2; // found key, but there was no (input) data given the
                              // content of _data_ (which is nullptr) is not accessed
        }
    }// if find
    else {
        if ( _data_ ) {
            // only create a key path and associate the data if there is data
            create_remaining_keypath();
            //ASSERT( !( *_data_ &0x02ul ) );  // see comments above at the first ASSERT(..*_data..)
            new (modify_at_child) NodeField(*_data_);
            // indicate that the key did not exist as a path in the TreeSet there is no resource to be freed
            return_value=0;
        }//if _data_
        else {
            return_value= -11;  // keypath is not created because there was no _data_ provided
        }


        fks= FKS_UNKNOWN; // unset after use so the used value can not be used again by mistake

    }//else find

    set_tail( tail_);

    return return_value;
}//insert()
#endif
/* NOTE
 * The insert() below will replace the insert() above.
 *
 * IMPORTAND: return values have changed!!!
 */

int TreeSet::operation_locals::insert( path_handle* tail_,  NodeField* _data_, bool _allowUpdate  )
{
    int return_value= -1003;                               // uninitialized return_value
    fks= FKS_UNKNOWN;                                      // has to be defined by find

    NodeField*& modify_at_child= const_cast<NodeField*&>(child);

    if ( find() ) {                              // if found key then b1 of the return_value is set and return_value > 0
        fks= FKS_UNKNOWN;
#if !FAST
        bool child_is_used= child->used();
        ASSERT( child_is_used);                            // abort: A key may only point node fields that are in use
        if ( !child_is_used ) {
            return -13;                                    // release mode inconsistancy: key to unused child
        }
#endif
        if ( _allowUpdate ) {
            _allowUpdate= _data_ && ( ( *_data_ )!= ( *child ) );
        }
        if (_allowUpdate) {                              // value or resource localion update
            std::swap( *modify_at_child , *_data_ ); // outputs old value to eventually release resources
            return_value= 2;                         // key found, exchange existing values on existing keypath
        }
        else {
            return_value= 3;                          // key found but value is not updated, *_data_ is unchanged
        }                                             // because 1 _allowUpdate false 2 no data valid pointer
    }// if find                                       //         3 data to insert and data inside are the same
    else {
        create_remaining_keypath();
        fks= FKS_UNKNOWN;
#if !FAST
        bool virgin_child= child->is_virgin();
        ASSERT( virgin_child );                            // abort: avoid reinitialize of already connected child node
        if ( !virgin_child ) {
            return -12;       // release mode inconsistancy: child state after create_remaining_keypath() is not correct
        }
#endif
        if (_data_) {
            new (modify_at_child) NodeField(*_data_);
            return_value = 0;                              // new key and (*child) value
        }
        else {
            return_value = 1;                              // a key path has been defined with a empty qnode at his end
        }
    }//else find

    set_tail( tail_);

    /*|$return_value%d|*/
    return return_value;
}//insert()

/*
________________________________________________________________________________________________________________________

*/
bool TreeSet::operation_locals::refer( NodeField** refered_ptr_ )
{
    ASSERT( refered_ptr_ ); // A address to write the adress of the associated genericdata_t is MANDATORY

    fks= FKS_UNKNOWN;

    // by design, child can't be nullptr, child = parent '+ delta' which in the [base...end) range
    // but we play for being paranoid in the debug mode for free so
    ASSERT(parent); ASSERT(child);


    bool return_value= find();

    if  ( !return_value ) {
        create_remaining_keypath();
    }//if !return_value

    *refered_ptr_= const_cast<NodeField*>(child);
#if !FAST
    fks= FKS_UNKNOWN; // unset after use so the used value can not be used again by mistake
#endif
    return return_value;
}
/*
________________________________________________________________________________________________________________________

                                                !!!    S T O P   !!!

 DO NOT USE 'insert_new()' it is only intended to develop and test 'find()' and can only be called excactly once
 from one root when there are no other keys prefere use of 'insert()'.
 It is still in use for test purposes.
*/
void TreeSet::operation_locals::insert_new(path_handle* tail_,  genericdata_t const& _data) //!!! _data is ro reference
{
    create_remaining_keypath();

    new ( const_cast<NodeField*>(child) ) NodeField(_data);
    set_tail( tail_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//private:

/*______________________________________________________________________________________________________________________

Where as extract_key() retrieves the entire path, extract_relative_key() only retrieves the part up to the
initiation path for the iteration. It is safer to use and has locallity.

IMPORTAND
      extarct(_relative)_key changes the internal state,
      in the context of the caller is recommended to be executed
      on a copy of the object so the original state is preserved.
*/
void TreeSet::iteration_locals::_extract_relative_reversed_key( std::vector<byte_t>* reversed_keybytes_)
{
    byte_t key_byte(0);
    do {
        key_byte= child_id;

        if  ( !( to_parent() && ( ++level< 0 ) ) ) {
            goto LB_push_partial_keybyte;
        }
        key_byte|= ( child_id<< 2 );

        if  ( !( to_parent()&& ( ++level< 0 ) ) ) {
            goto LB_push_partial_keybyte;
        }
        key_byte|= ( child_id<< 4 );

        if  ( !( to_parent()&& ( ++level< 0 ) ) ) {
            goto LB_push_partial_keybyte;
        }
        key_byte|= ( child_id<< 6 );

        reversed_keybytes_-> push_back( key_byte);
    }
    while ( to_parent()&& ( ++level< 0 ) );

    return;

LB_push_partial_keybyte:
    reversed_keybytes_-> push_back( key_byte);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//public:

/*
________________________________________________________________________________________________________________________

This demonstrator method reviels a remarkable property of qnodes. Normally a tree structure is descended recursively or
you need a stack to descend through the structure and return to the higher level at complition of the iteration at
parent level. But because each qnode it self has the required parent information, it uses that to resume on the
higher level. This is the future of iterating through a TreeSet, which is the basis of all kinds of table scan
operations.


 lambda's implementation in near future
 _for_each_path_begin    called at start level, before anything is done
 _iterated               called for each allocated node from the start qnode
 _descended              called each (from the start qnode) time a qnode has childs and descended to it
 _tailfunction           called each (from the start qnode) time a qnode field is no qnode and contains other data
 _ascended               called each (from the start qnode) the valid child nodes are iterated and we ascended back to
                         the parent level
 _for_each_path_end      called at start level after all is done)

 in remove and for_each_path.

*/
            /***********************
             *                     *
             * demonstrator method *
             *                     *
             ***********************/

bool TreeSet::iteration_locals::for_each_path()
{
#if 0  // ismpl work arround
    auto no_qnode=[](Field_type _qft,const genericdata_t* _subject)->void
    {
        // Here the TreeSet has reached a key path from where the key and the belonging data are accessable
        // trough '..extract_key(..)' and '..extract_data(..)'.. or removable
        INFO("(a)      DATA: ",VARVAL(_qft),std::hex,VARVAL(*_subject),std::dec,'\n');
    };

    auto per_child=[this,no_qnode]()->int
    {
        if ( parent->unused_child(child_id) ) {
            return -1;
        }
        child= treeset->child_of(  parent, child_id);

        // lambda: _iterated

        //FieldExtract(Field_type,qft,next,type);
        ;
        switch ( auto qft= child-> GET_(type).as_enum ) {
          case Field_type::QF_qnode:
            parent= child;
            //mon.observe( parent).printFieldsTo( &std::cout, "(a) parent");
            child_id= 0;

            // lambda: _descended
            return 1;

          default:
            // Here itr is at the tail of a path, path key and path data can be retrieved from here



            // lambda: _tailfunction

            no_qnode( qft, static_cast<const genericdata_t*>(static_cast<const void*>(child)) );
            return 0; // do not further descent
        }
    };
#endif
    // lambda: _for_each_path_begin

    if  ( parent!=root ) {
        switch ( per_child() ) {
          case 0:
            return true;

          case -1:
            INFO("Error: appointed qnode failed, child is not defined",VARVAL(parent),VARCHRNUM(child_id));
            return false;

          case 1:
            // proceeed... see below
            break;
          default:
            assert( false);
        }//switch
    }
    // proceeed with child tree, start level must be 0

    --level;
    for (;;) {
        switch  ( per_child() ) {
          case 1:
            --level;
            INFO("decent to child ",VARVAL(level),VARCHRNUM(child_id),'\n');
            continue;
        }

        INFO("iterate ",VARVAL(level),VARCHRNUM(child_id),'\n');
        while ( ++child_id>=4 ) {
            if (! ++level ) {

                // lambda: _for_each_path_end

                INFO("exceeded start level ",VARVAL(level),VARCHRNUM(child_id),'\n');
                return true;
            }
            ASSERT_ALWAYS_DO(to_parent());

            // lambda: _ascended

            INFO("ascent to parent ",VARVAL(level),VARCHRNUM(child_id),'\n');
        }
    }
}// bool TreeSet::iteration_locals::for_each_path()



/*
________________________________________________________________________________________________________________________

*/
void Indent( int _repeat,const char *_fill="|")
{
    if ( _repeat< 0 ) {
        _repeat= -_repeat;
    }
    for ( auto r= _repeat; r; --r ) {
        std::cout<< _fill;
    }
}


/*
________________________________________________________________________________________________________________________

*/
            /***********************
             *                     *
             * demonstrator method *
             *                     *
             ***********************/

void TreeSet::iteration_locals::visit_nodes()
{
    // idtr is a shortcut for: Imaginary Distance To Root
    uint16_t  idx= 0;
    bool      sig= false;
    uint32_t  distance;

    std::string key;
    NodeField data;

    // If treeset::start has been set or directly used from the common_locals constructor
    // then parent is set to represent that location which requires some special
    // handling here.
    // Note that there are some checks here but errors are ignored. In DEBUG mode
    // these errors should already have triggered ASSERT failure. In !DEBUG mode
    // simply do what is best. And a data node has mo subnodes to visit, so ... leave.
    //
    // see also: TreeSet::common_locals::common_locals( const TreeSet *_treeset_)
    //           and subsequent constructor calls it issues.
    //

    if ( parent!= root ) {
        if ( parent-> GET_(type).as_enum!= Field_type::QF_qnode ) {
            // expected a qnode but found data, preceeding operations
            // would be invalid
            return;
        }
        // iterate over a tree set erea (set by pathhandle start), not the hole tree
        if ( parent->unused_child( child_id) ) {
            // nothing here
            return;
        }
        if ( child-> GET_(type).as_enum!= Field_type::QF_qnode ) {
            // expected a qnode but found data, preceeding operations
            // would be invalid
            return;
        }
        //else, start position (visit nodes of a part of the tree)
        // was given by root offset and a child id. So the calculated
        //  is the root of the nodes to be visited
        parent= child;
    }

    const NodeField* parent_end= parent;
    int32_t idtr_parent= 0, idtr_child=0;  // actually parent.rootOffset


    Indent(level), GloballyObservedQnode.printHeaderTo( &std::cout);
    --level;
    for(;;) { // while ( parent!= parent_end );
        Indent(level), GloballyObservedQnode.observe(parent).printFieldsTo( &std::cout, "initiating parent");
        Indent(level), INFO("parently at :",VARVAL(level),",",VARCHRNUM(child_id),'\n');
        if ( parent-> used_child( child_id) ) {
            // get child of parent by child_id
            parent-> get_idx_sig_from_Child( &idx, &sig, child_id);
            distance= NodeFieldBuffer::Calculate_distance( idx);
            if ( sig ) {
                //track imaginary child
                idtr_child= idtr_parent- distance;
                //calc child
                child= parent- distance;
            }
            else {
                //track imaginary child
                idtr_child= idtr_parent+ distance;
                //calc child
                child= parent+ distance;
            }
            ASSERT(treeset->in_range(child));
               /***************************************.......*/
              /* HERE: IS THE VISIT OF EACH VALID CHILD NODE  */
             /*********************************...............*/
            ptrdiff_t childOffset(child-root);

            Indent(level), GloballyObservedQnode.observe(child).printFieldsTo( &std::cout, "visitted child");
            Indent(level), INFO("visit on",VARVAL(level),"of",VARCHRNUM(child_id),PTRVAL(child)
                ,VARVAL(NodeField::TypeToName(child->GET_(type))),VARVAL(childOffset),VARVAL(idtr_child));
                                                               /*...................***************************/
                                                               /*  END OF VISIT OF EACH VALID CHILD NODE     */
                                                               /*......**************************************/
#if 1  // as long we use no folding
            ASSERT(idtr_child==(child-root));
#endif
            if ( child-> GET_(type).as_enum== Field_type::QF_qnode ) {
                // go to child of that childId
                idtr_parent= idtr_child;
                parent= child;
                child_id= 0;
                --level;
                Indent(level), GloballyObservedQnode.printHeaderTo( &std::cout);
                Indent(level), INFO("in qnode, decent to childeren on",VARVAL(level),'\n');
                continue;
            }
               /***************************************.......*/
              /* HERE: IS THE VISIT OF EACH VALID NON QNODE   */
             /*********************************...............*/
            // to verify that it works
            ASSERT_ALWAYS_DO(extract_key( &key)>= 0); ASSERT_ALWAYS_DO(extract_data_bits( &data));
            Indent(level), INFO("<QF_data> stored info:",VARVAL(key),VARVAL(data.GET_(data_bits)));
            std::cout<<"___________________________________________\n\n\n";
                                                               /*...................***************************/
                                                               /*  END OF VISIT OF EACH VALID NON QNODE      */
                                                               /*......**************************************/
        }// if used child
        else {
            Indent(level), INFO("No child at:",VARCHRNUM(child_id));
        }


        while ( ++child_id & ~3 ) {
            ++level;
            if ( parent== parent_end ) {
                ASSERT(!level);
                // finished
                return;
            }
            // go to parent of parent
            idtr_child= idtr_parent;
            child= parent;
            distance= NodeFieldBuffer::Calculate_distance( parent->GET_(idxP));
            if ( parent->GET_(sigP) ) {
                // track imaginairy parent
                idtr_parent= idtr_child- distance;
                //calc parent
                parent= child- distance;
            }
            else {
                // track imaginairy parent
                idtr_parent= idtr_child+ distance;
                //calc parent
                parent= child+ distance;
            }
            child_id= child->GET_(cldP);

            ASSERT(treeset->in_range(parent));

               /***************************************.......*/
              /* HERE: IS THE VISIT OF EACH VALID QNODE       */
             /*********************************...............*/
            ptrdiff_t parentOffset= parent- root;
            Indent(level), INFO("ascended to parent ",VARVAL(level),VARVAL(parentOffset),VARVAL(idtr_parent),VARCHRNUM(child_id),'\n');
            Indent(level), GloballyObservedQnode.printHeaderTo( &std::cout);
#if 1  // as long we use no folding
            ASSERT(idtr_parent==parentOffset);
#endif
                                                               /*...................***************************/
                                                               /*  END OF VISIT OF EACH VALID QNODE          */
                                                               /*......**************************************/
        }//while(++child_id & ~3)
    }//loop
}//visit_nodes()



/*
________________________________________________________________________________________________________________________

*/

int TreeSet::iteration_locals::final_child()
{
    if ( parent-> unused_child( child_id) ) {
          /*|do not further descend, there nothing here|*/
        return -4;
    }
    child= treeset->child_of(  parent, child_id);

    if  ( child-> GET_(type).as_enum== Field_type::QF_qnode ) {

        parent= child;
        child_id= 0;
        /*|found qnode that has 0..4 child qnodes or data nodes|*/
        return 0;
    }

    /*|do not further descend, there no childs to descent to|*/
    ++rmvdata_cnt;
    removeData();
    return 1;
}//final_child

static void no_qnode(Field_type _qft,const genericdata_t* _subject)
{
    // Here the TreeSet has reached a key path from where the key and the belonging data are accessable
    // trough '..extract_key(..)' and '..extract_data(..)'.. or removable
    INFO("(a)      DATA: ",VARVAL(_qft),std::hex,VARVAL(*_subject),std::dec,'\n');
};


// ismpl work arround called by TreeSet::iteration_locals::for_each_path()
int TreeSet::iteration_locals::per_child()
{
    if ( parent->unused_child(child_id) ) {
        return -1;
    }
    child= treeset->child_of(  parent, child_id);

    // lambda: _iterated

    //FieldExtract(Field_type,qft,next,type);
    ;
    switch ( auto qft= child-> GET_(type).as_enum ) {
      case Field_type::QF_qnode:
        parent= child;
        //mon.observe( parent).printFieldsTo( &std::cout, "(a) parent");
        child_id= 0;

        // lambda: _descended
        return 1;

      default:
        // Here itr is at the tail of a path, path key and path data can be retrieved from here



        // lambda: _tailfunction

        no_qnode( qft, static_cast<const genericdata_t*>(static_cast<const void*>(child)) );
        return 0; // do not further descent
    }
}

// removeAll that is below 'parent'
int TreeSet::iteration_locals::removeAll()
{
  // FIXME MT2014-0617-0851
  // Not all nested child nodes are removed. Either fix the bug or rewrite remove
  // combined assignment: call in help of SeqIntra source code instrumentation
  // Analyzes: path_handle start initializes on child id 0 of parent, only those 4 entries are removed.
  //           but this test could be wrong, studu iteration_operation_locals::remove

    level= 0;
    /*|$root%p $parent%p $child_id%hhu|*/
    /*|$level%d $rmvdata_cnt%d $rmvnode_cnt%d|*/

    //if  ( parent!=root ) {
    if  ( final_child() ) {
          /*|No child nodes|*/
        return rmvdata_cnt;
    }
    //}

    // HERE: final_child() returned 0    child was not final and I descent level(s) deeper
    --level;
    // HERE: level must be -1
    for (;;) {
        /*|probing $child_id%hhu on $level%d|*/
        if  ( !final_child() ) {
            --level;
            /*|$level%d decreased|*/
            continue;
        }
        /*|advancing to $child_id%hhu+1  on $level%d|*/
        while ( ++child_id>= 4 ) {
            ++level;
            /*|all $child_id%hhu 's probed leaving qnode, $level%d increased|*/
            if ( !level ) {
                /*|Clear the upper 'parent' node its parent relation stays, but it childs should be all cleared.|*/
                NodeField*& modify_at_parent = const_cast<NodeField*&>(parent);
                modify_at_parent->clear_all_child_fields();
                int return_value= rmvdata_cnt+rmvnode_cnt;
                /*|$level%d $rmvdata_cnt%d $rmvnode_cnt%d $return_value%d|*/
                return return_value;
            }
            // to_parent() affects child_id
            if ( !to_parent() || ( level> 0 ) ) {
                // endless loop prevention due to inconsistancies
                Throw_Error_Break(std::runtime_error(
                  "Qnode::TreeSet::iteration_locals::removeAll(): internal inconsistancy at root of TreeSet"));
            }
            /*|clear the node that was - before moving  one level up to parent - the 'parent'|*/
            // This will also allow the allocator to reuse this cleared qnode again.
            ++rmvnode_cnt;
            removeQnode();
        }
    }
}
/*
________________________________________________________________________________________________________________________
*/
TreeSet::genericdata_iterator::genericdata_iterator(TreeSet*_treeset)
: common_locals(_treeset), iteration_locals(_treeset)
{
    if ( parent!= root ) {
        // iterate over a tree set erea (set by pathhandle start), not the hole tree
        if ( parent->unused_child( child_id) ) {
            // nothing here
            return;
        }
        child= treeset-> child_of(  parent, child_id);
        if ( child-> GET_(type).as_enum!= Field_type::QF_qnode ) {
            // found data
            return;
        }
        //else
        parent= child;
    }
}

/*
________________________________________________________________________________________________________________________

*/


/*************************************************************************************************************
 *  The original code should be left here for documenting purposes and debugging in case we run into trouble *
 *************************************************************************************************************
void TreeSet::genericdata_iterator::iterate ()
{
    for (;;) {
        while ( advance_exceeds_boundary() ) {
            if ( !to_parent() ) {
                // at root level, no required test, but more robust
                level= 0;
                return;
            }
            if ( ! ++level ) {
                return;
            }
        }//while advance_exceeds_boundary
        child= treeset->child_of( parent, child_id);
        if ( child!= parent ) {
            // parent is a QF_qnode with a child by child_id
            auto qft= child-> GET_(type).as_enum;
            if ( qft== Field_type::QF_qnode ) {
                // The child is also a QF_qnode, going one level deeper into the TreeSet
                --level;
                parent= child;
                child_id= cid_start;
                continue;
            }
            //else found (internal) data
            return;
        }//if child!= parent
    }//for
}
  ************************************************************************************************************
*/

/*
________________________________________________________________________________________________________________________

*/
TreeSet::iteration_operation_locals::iteration_operation_locals(TreeSet* _treeset_, key_block const& _keyblk)
: common_locals(_treeset_)
, operation_locals(_treeset_,_keyblk)
, iteration_locals(_treeset_)
{
    start_root= parent;
}

/*
________________________________________________________________________________________________________________________

*/
int TreeSet::iteration_operation_locals::remove()
{
    if  ( !find() ) {
        return 0;
    }
    int ra_state= removeAll();
    if  ( ra_state< 0 ) {
        return ra_state; // range -4..-4   (something wrong with finding the node?)
    }
    int rk_state= common_locals::remove(  start_root);

    if  ( rk_state< 0 ) {
        // there was a error because the value is below 0 otherwhise it would indicate how many qnodes where removed.
        return rk_state; // range -1..-3
    }
    INFO("removecounters and states : ",
         VARVAL(rmvdata_cnt),
         VARVAL(rmvnode_cnt),
         VARVAL(kprmv_cnt),
         VARVAL(ra_state),
         VARVAL(rk_state));

    int return_value= ra_state+rk_state;
    return return_value;  // sum of all data nodes sub key nodes and key path nodes that where removed
}

/*
________________________________________________________________________________________________________________________

*/
// ismpl work arround
void TreeSet::debug_locals::print_keybyte(char _kb)
{
    unsigned char ukb= static_cast<unsigned char>(_kb);
    unsigned ukbhexnum= static_cast<unsigned>(ukb);
    *pos<< std::hex<< "0x"<< ukbhexnum<< std::dec;
    if ( ukbhexnum>32 && ukbhexnum<128 ) {  // fixed: bad printing when kb is a control char or non asci
      // also show visible character
      *pos<< " '"<< _kb<< "'";
    }
}

void TreeSet::debug_locals::debug_dump(   NodeField const* _parent, const uint8_t& _child_id, int const& _level)
{                                                  // ^^^^pointer, but truely input, its about the pointer value
                                                   //              and not its pointee
    static char indent_strbase[512];
    static char *indent_str=indent_strbase+  2;
    auto cnt_childs=  _parent-> number_of_childs();
    char child_cht_char= static_cast<char>( '0'+ cnt_childs);
    indent_str[_level]= child_cht_char;
    indent_str[_level+1]='\0';


    const NodeField *child= treeset-> child_of( _parent, _child_id);
    size_t idx= static_cast<size_t>(_level)>> 2;
    auto mod= _level& 0x3;


  // print a node per line, format:
  // level { parent_ptr [ child_id ] -> child_ptr } child_cnt[parent_root_offset] { child_id / child_parent_distance } [child_root_offset] : qnode type
  // when key is completed, also print the keybytes and the generic data it refers to.
  // TODO: make a nice header to describe the printed fields.

  //////////////Section: print child data (of child pointed by parent through index on its child id)
    Field_type qft;
    qft=child-> GET_(type).as_enum;

    *pos<<std::setw(3)<<_level<<"  { "<<_parent<<" ["<<static_cast<unsigned>(_child_id)<<"] "<<"-> "<<child<<" }  | ";


    //for( int u=0; u< _level; ++u) *pos<<" ";
    *pos<< indent_str;


    // pointers are less meaningfull and harder to read then the offset
    // (in qnodes of these pointers from the center of the buffer (the root)
    ptrdiff_t parent_root_offset= _parent- root
    ,         child_parent_distance= child- _parent   // distance from child to root
    ,         child_root_offset= child- root
    ;

    unsigned const swidth=4;

    //*pos<< " "<< child_cht_char;
    *pos<< "["<< std::setw(swidth)<< parent_root_offset<< "]-{ ";
    *pos<< static_cast<unsigned>(_child_id)<<" /"<< std::setw(swidth)<< child_parent_distance;
    *pos<< "}-["<< std::setw(swidth)<< child_root_offset<< "]:"<< qft;

#if 0 // ismpl work arround
    auto print_keybyte = [&](char _kb)->void
    {
        unsigned char ukb= static_cast<unsigned char>(_kb);
        unsigned ukbhexnum= static_cast<unsigned>(ukb);
        *pos<< std::hex<< "0x"<< ukbhexnum<< std::dec;
        if ( ukbhexnum>32 && ukbhexnum<128 ) {  // fixed: bad printing when kb is a control char or non asci
          // also show visible character
          *pos<< " '"<< _kb<< "'";
        }
    };
#endif
    if  ( _level< 0 ) {
        // root qnode end marking
        *pos<< std::endl<< "---------------------------------------"<< std::endl;
    }
    else {
        // data tree printing

        static const byte_t shifts[4]={ static_cast<byte_t>(6)
                                      , static_cast<byte_t>(4)
                                      , static_cast<byte_t>(2)
                                      , static_cast<byte_t>(0)
                                      };
        static const byte_t  masks[4]={ static_cast<byte_t>(0xC0)
                                      , static_cast<byte_t>(0x30)
                                      , static_cast<byte_t>(0x0C)
                                      , static_cast<byte_t>(0x03)
                                      };

        //byte_t new_pair= _child_id<< shifts[ mod];;
        //keybyte|= new_pair;


        if  ( idx>= key.size() ) {
            key.push_back( static_cast<byte_t>(0));
        }

        // set pair bits of keybyte without effecting the others
        (key[idx] &= ~(masks[ mod]))|= (masks[ mod]& (_child_id<< shifts[ mod]));

        if ( mod== 3 ) {
           *pos<< std::endl;
           print_keybyte( key[ idx]);
        }
        *pos<< std::endl;
    }

    switch ( qft ) {

      case Field_type::QF_data_pointer:
      case Field_type::QF_data_bits:
      case Field_type::QF_internal:
#if 1
        {// genric representations for other types ( type in (0,1,3) )
            NodeField qndata= *child;
            genericdata_t gdata= static_cast<genericdata_t>(qndata);
            *pos<<" genericdata>>2:"<< (gdata>>2) <<  std::endl;
            *pos<<"key bytes: ";
            decltype(idx) cnt=0;
            for ( auto kb : key ) {
                print_keybyte( kb);
                if ( ++cnt>idx) {
                    break;
                }
            }
            *pos<< std::endl;
        }
#endif
        break;

      case Field_type::QF_qnode:
        {//////////////Section: Child becomes next parent
        auto child_level= _level+ 1;
        if  ( child-> GET_(idxA) ) {
            debug_dump( child, 0, child_level);
        }
        if  ( child-> GET_(idxB) ) {
            debug_dump( child, 1, child_level);
        }
        if  ( child-> GET_(idxC) ) {
            debug_dump( child, 2, child_level);
        }
        if  ( child-> GET_(idxD) ) {
            debug_dump( child, 3, child_level);
        }
        }//////////////End Section: Child becomes next parent

        break;
      default:
        ASSERT( false);// impossible! but let's catch it, case unhandled QF_type
    }
  //////////////End Section: print child data

}

/*
________________________________________________________________________________________________________________________

 The head is set if the path_handle:_head does not point to the root and it points to an allocated qnode field
 otherwhise '..set_head(..)' has no effect. Returns true if the head is set, or false if '..set_head(..)' did
 not have any effect.

 *
 * TODO: test case: find, insert and remove from subtree
 *       test case: attempt to set to invalid root
*/
bool TreeSet::set_head(  const path_handle& _head)
{

    if  ( !_head.rootOffset ) {
        return false;
    }

    const NodeField *maybe_parent= root()+ _head.rootOffset;
    if  ( maybe_parent->GET_(type).as_enum!= Field_type::QF_qnode ) {
        return false;
    }

    start= _head;
    return true;
}

/*
________________________________________________________________________________________________________________________
*/

void debug_dump_Key_To( std::ostream* pos_, const std::vector<byte_t> _keyvec, unsigned _flags)
{
    ASSERT(pos_);
/*
_flags gives some low level formatting options. Bit in _flag are used as switches. Every nibble is for a independed
representation type. I favor flexibility over performance here, since it is IO, the effects can be neglected anyway


nummeric
nible0  n0b0 : show each key byte as a separate number
        n0b1 : if set show each byte hexadecimal, otherwise as decimal
        n0b2 : reserved
        n0b3 : reserved
DNA
nible1  n1b0 : show key as DNA strain
        n1b1 : sepate each byte by a dot, nothing otherwise
        n0b2 : Uppercase
        n0b3 : Capital
string
nible2  n2b0 print key as if it would be a zero terminated string

common
nible3  n3b0 : extra new lines
        n3b1 : labels
 */

    if ( _flags & 0x1000 ) *pos_ << std::endl;

        // print key as string
    if ( _flags & 0x0100 ) {
        if ( _flags& 0x2000 ) *pos_<<"key string: "<<'"';
        for ( auto kb: _keyvec ) {
            if ( !kb ) break;
            *pos_<< kb;
        }
        *pos_ <<'"'<< std::endl;
    }
    if ( _flags & 0x1000 ) *pos_ << std::endl;

    //print key as DNA strain (DNA got nothing to do with it, except that a,b,c,d are used to indicate the child fields)
    if  ( _flags & 0x10 ) {
        if ( _flags& 0x2000 ) *pos_<<"key pair-vector: ";
        for ( auto kb: _keyvec ) {

            static const char DNA[4]={'A','B','C','D'};
            static const char dna[4]={'a','b','c','d'};

            unsigned cid;

            cid= ( kb>> 6 )& 0x3;
            *pos_<< ( _flags&0xC0 ? DNA[ cid] : dna[ cid] );

            cid= ( kb>> 4 )& 0x3;
            *pos_<< ( _flags&0x40 ? DNA[ cid] : dna[ cid] );

            cid= ( kb>> 2 )& 0x3;
            *pos_<< ( _flags&0x40 ? DNA[ cid] : dna[ cid] );

            cid= ( kb     )& 0x3;
            *pos_<< ( _flags&0x40 ? DNA[ cid] : dna[ cid] );

            if  ( _flags & 0x20 ) *pos_<<'.';  // sepatrate bytes by a doty to enhance readabillity

        }
        *pos_ << std::endl;
    }
    if ( _flags & 0x1000 ) *pos_ << std::endl;

    if ( _flags & 0x1 ) {
        // TODO move to TreeSet as debug_dump key bytes
        //      (or as quartary numbers (byte= aaaa..dddd) which visualizes the key contruction in each qnode)
        //      example "ABC" => 41.42.43.00 =>0010.0001 0010.0010 0010.0011 0000.0000 => acab.acac.acad.aaaa
        if ( _flags& 0x2000 ) *pos_<<"key bytes: ";
        for ( auto kb: _keyvec ) {

            unsigned char ukb        = static_cast<unsigned char>(kb);
            unsigned      ukbhexnum  = static_cast<unsigned>(ukb);

            if   ( _flags& 0x2 ) *pos_ << std::hex  << std::setw(2);
                                 *pos_ << ukbhexnum ;
            if   ( _flags& 0x2 ) *pos_ << std::dec  ;
                                 *pos_ << ' '       ;
        }
        *pos_ << std::endl;
    }
    if ( _flags & 0x1000 ) *pos_ << std::endl;

    // print

}

/*
________________________________________________________________________________________________________________________

*/



}//namespace Qnode



#ifndef SKIPLIST_MULTIMAP_RO_H_HEADER
#define SKIPLIST_MULTIMAP_RO_H_HEADER

#include <algorithm>
#include <functional>
#include <istream>
#include <ostream>
#include <memory>
#include <cstddef>
#include <cassert>
#include <cstring>
#include "skiplist_traits.h"
#include "skiplist_multimap.h"

#define MAX_DATA_COUNT_PER_KEY 512

#ifdef SL_DEBUG

#include <iostream>
#include <iomanip>

/// Print out debug information to std::cout if SL_DEBUG is defined.
#define SL_PRINT(x)          do { (std::cout << x << std::endl); } while (0)

/// Assertion only if BTREE_DEBUG is defined. This is not used in verify().
#define SL_ASSERT(x)         do { assert(x); } while (0)

#else

/// Print out debug information to std::cout if SL_DEBUG is defined.
#define SL_PRINT(x)          do { } while (0)

/// Assertion only if BTREE_DEBUG is defined. This is not used in verify().
#define SL_ASSERT(x)         do { } while (0)

#endif

namespace cmu {

// forward declaration of friend class
template <typename _Key, typename _Data,
          typename _Compare,
          typename _Traits,
          typename _Alloc>
class skiplist_multimap_compact;

template <typename _Key, typename _Data,
          typename _Compare,
          typename _Traits,
          typename _Alloc>
class skiplist_multimap;

template <typename _Key, typename _Data,
          typename _Compare = std::less<_Key>,
          typename _Traits = skiplist_default_map_traits<_Key, _Data>,
          typename _Alloc = std::allocator<std::pair<_Key, _Data> > >
class skiplist_multimap_ro
{
#define SL_FRIENDS friend class skiplist_multimap_compact<_Key, _Data, _Compare, _Traits, _Alloc>;
public:
    typedef _Key key_type;
    typedef _Data data_type;
    typedef _Compare key_compare;
    typedef _Traits traits;
    typedef _Alloc allocator_type;
    SL_FRIENDS

public:
    typedef skiplist_multimap_ro<key_type, data_type, key_compare, traits,
                         allocator_type> self_type;
    typedef skiplist_multimap<key_type, data_type, key_compare, traits,
                         allocator_type> writable_type;
    typedef std::pair<key_type, data_type> value_type;
    typedef std::pair<key_type, data_type> pair_type;
    typedef size_t size_type;

public:
    static const short i_order = traits::order;
    static const short i_half_order = (i_order % 2 == 0) ? (i_order / 2) : (i_order / 2 + 1);

    static const short l_order = traits::leaf_order;
    static const short l_half_order = (l_order % 2 == 0) ? (l_order / 2) : (l_order / 2 + 1);

private:
    struct node {
        short is_leaf;
        short count;
    };

    struct inner_node: public node {
        typedef typename _Alloc::template rebind<inner_node>::other alloc_type;

        key_type   key[i_order];
        node       *down[i_order];
        inner_node *right;
    };

    struct leaf_node: public node {
        typedef typename _Alloc::template rebind<leaf_node>::other alloc_type;

        leaf_node *left;
        leaf_node *right;
        key_type  key[l_order];
        unsigned short data_count[l_order];
        data_type *data_array[l_order];
    };

    typedef typename _Alloc::template rebind<data_type>::other data_alloc_type;

public:
    class iterator;
    class const_iterator;
    class reverse_iterator;
    class const_reverse_iterator;

    class iterator {
    public:
        typedef typename skiplist_multimap_ro::key_type key_type;
        typedef typename skiplist_multimap_ro::data_type data_type;
        typedef typename skiplist_multimap_ro::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        typename skiplist_multimap_ro::leaf_node *currnode;
        short currindex;
        unsigned short currdataindex;

        friend class reverse_iterator;
        friend class const_iterator;
        friend class const_reverse_iterator;
        friend class skiplist_multimap_ro<key_type, data_type, key_compare, traits,
                                  allocator_type>;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline iterator()
            : currnode(NULL), currindex(0), currdataindex(0)
        { }

        inline iterator(typename skiplist_multimap_ro::leaf_node *n, short index, unsigned short dataindex)
            : currnode(n), currindex(index), currdataindex(dataindex)
        { }

        inline iterator(const reverse_iterator& it)
            : currnode(it.currnode), currindex(it.currindex), currdataindex(it.currdataindex)
        { }

        inline reference operator * () const
        {
            temp_value = value_type(key(), data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            temp_value = value_type(key(), data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            return currnode->key[currindex];
        }

        inline data_type & data() const
        {
            return currnode->data_array[currindex][currdataindex];
        }

        inline iterator& operator ++ ()
        {
            if (currnode->right != NULL) {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex == currnode->count - 1) {
                            if (currnode->right->count != 1 || currnode->right->right != NULL)
                            {
                                currnode = currnode->right;
                                currindex = 0;
                                currdataindex = 0;
                            }
                        }
                        else {
                            ++currindex;
                            currdataindex = 0;
                        }
                    }
                }
            }
            else {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex < currnode->count - 2) {
                            currdataindex = 0;
                            ++currindex;
                        }
                    }
                }
            }
            return *this;
        }

        inline iterator operator ++ (int)
        {
            iterator tmp = *this;

            if (currnode->right != NULL) {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex == currnode->count - 1) {
                            if (currnode->right->count != 1 || currnode->right->right != NULL)
                            {
                                currnode = currnode->right;
                                currindex = 0;
                                currdataindex = 0;
                            }
                        }
                        else {
                            ++currindex;
                            currdataindex = 0;
                        }
                    }
                }
            }
            else {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex < currnode->count - 2) {
                            currdataindex = 0;
                            ++currindex;
                        }
                    }
                }
            }
            return tmp;
        }

        inline iterator& operator -- ()
        {
            if (currdataindex > 0) {
                --currdataindex;
            }
            else if (currindex > 0) {
                --currindex;
                currdataindex = currnode->data_count[currindex] - 1;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
                currdataindex = currnode->data_count[currindex] - 1;
            }
            return *this;
        }

        inline iterator operator -- (int)
        {
            iterator tmp = *this;

            if (currdataindex > 0) {
                --currdataindex;
            }
            else if (currindex > 0) {
                --currindex;
                currdataindex = currnode->data_count[currindex] - 1;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
                currdataindex = currnode->data_count[currindex] - 1;
            }
            return tmp;
        }

        inline bool operator == (const iterator &x) const
        {
            return (x.currnode == currnode && x.currindex == currindex && x.currdataindex == currdataindex);
        }

        inline bool operator != (const iterator &x) const
        {
            return (x.currnode != currnode || x.currindex != currindex || x.currdataindex != currdataindex);
        }

        inline bool is_invalid() const
        {
            return currnode == NULL;
        }

        inline bool is_end() const
        {
            return currdataindex == currnode->data_count[currindex];
        }

        inline bool is_lazy_deleted() const
        {
            return data() == (data_type)0;
        }

        inline void lazy_delete()
        {
            data() = (data_type)0;
        }
    };

    class const_iterator {
    public:
        typedef typename skiplist_multimap_ro::key_type key_type;
        typedef typename skiplist_multimap_ro::data_type data_type;
        typedef typename skiplist_multimap_ro::value_type value_type;
        typedef const value_type& reference;
        typedef const value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        const typename skiplist_multimap_ro::leaf_node *currnode;
        short currindex;
        unsigned short currdataindex;

        friend class const_reverse_iterator;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline const_iterator()
            : currnode(NULL), currindex(0), currdataindex(0)
        { }

        inline const_iterator(const typename skiplist_multimap_ro::leaf_node *n, short index, unsigned short dataindex)
            : currnode(n), currindex(index), currdataindex(dataindex)
        { }

        inline const_iterator(const iterator& it)
            : currnode(it.currnode), currindex(it.currindex), currdataindex(it.currdataindex)
        { }

        inline const_iterator(const reverse_iterator& it)
            : currnode(it.currnode), currindex(it.currindex), currdataindex(it.currdataindex)
        { }

        inline const_iterator(const const_reverse_iterator& it)
            : currnode(it.currnode), currindex(it.currindex), currdataindex(it.currdataindex)
        { }

        inline reference operator * () const
        {
            temp_value = value_type(key(), data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            temp_value = value_type(key(), data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            return currnode->key[currindex];
        }

        inline const data_type & data() const
        {
            return currnode->data_array[currindex][currdataindex];
        }

        inline const_iterator& operator ++ ()
        {
            if (currnode->right != NULL) {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex == currnode->count - 1) {
                            if (currnode->right->count != 1 || currnode->right->right != NULL)
                            {
                                currnode = currnode->right;
                                currindex = 0;
                                currdataindex = 0;
                            }
                        }
                        else {
                            ++currindex;
                            currdataindex = 0;
                        }
                    }
                }
            }
            else {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex < currnode->count - 2) {
                            currdataindex = 0;
                            ++currindex;
                        }
                    }
                }
            }
            return *this;
        }

        inline const_iterator operator ++ (int)
        {
            const_iterator tmp = *this;

            if (currnode->right != NULL) {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex == currnode->count - 1) {
                            if (currnode->right->count != 1 || currnode->right->right != NULL)
                            {
                                currnode = currnode->right;
                                currindex = 0;
                                currdataindex = 0;
                            }
                        }
                        else {
                            ++currindex;
                            currdataindex = 0;
                        }
                    }
                }
            }
            else {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex < currnode->count - 2) {
                            currdataindex = 0;
                            ++currindex;
                        }
                    }
                }
            }
            return tmp;
        }

        inline const_iterator& operator -- ()
        {
            if (currdataindex > 0) {
                --currdataindex;
            }
            else if (currindex > 0) {
                --currindex;
                currdataindex = currnode->data_count[currindex] - 1;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
                currdataindex = currnode->data_count[currindex] - 1;
            }
            return *this;
        }

        inline const_iterator operator -- (int)
        {
            const_iterator tmp = *this;

            if (currdataindex > 0) {
                --currdataindex;
            }
            else if (currindex > 0) {
                --currindex;
                currdataindex = currnode->data_count[currindex] - 1;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
                currdataindex = currnode->data_count[currindex] - 1;
            }
            return tmp;
        }

        inline bool operator == (const const_iterator &x) const
        {
            return (x.currnode == currnode && x.currindex == currindex && x.currdataindex == currdataindex);
        }

        inline bool operator != (const const_iterator &x) const
        {
            return (x.currnode != currnode || x.currindex != currindex || x.currdataindex != currdataindex);
        }

        inline bool is_invalid() const
        {
            return currnode == NULL;
        }

        inline bool is_end() const
        {
            return currdataindex == currnode->data_count[currindex];
        }

        inline bool is_lazy_deleted() const
        {
            return data() == (data_type)0;
        }
    };

    class reverse_iterator {
    public:
        typedef typename skiplist_multimap_ro::key_type key_type;
        typedef typename skiplist_multimap_ro::data_type data_type;
        typedef typename skiplist_multimap_ro::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        typename skiplist_multimap_ro::leaf_node *currnode;
        short currindex;
        unsigned short currdataindex;

        friend class iterator;
        friend class const_iterator;
        friend class const_reverse_iterator;
        friend class skiplist_multimap_ro<key_type, data_type, key_compare, traits,
                                  allocator_type>;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline reverse_iterator()
            : currnode(NULL), currindex(0), currdataindex(0)
        { }

        inline reverse_iterator(typename skiplist_multimap_ro::leaf_node *n, short index, unsigned short dataindex)
            : currnode(n), currindex(index), currdataindex(dataindex)
        { }

        inline reverse_iterator(const iterator& it)
            : currnode(it.currnode), currindex(it.currindex), currdataindex(it.currdataindex)
        { }

        inline reference operator * () const
        {
            SL_ASSERT(currdataindex > 0);
            temp_value = value_type(key(), data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            SL_ASSERT(currdataindex > 0);
            temp_value = value_type(key(), data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            SL_ASSERT(currdataindex > 0);
            return currnode->key[currindex];
        }

        inline data_type & data() const
        {
            SL_ASSERT(currdataindex > 0);
            return currnode->data_array[currindex][currdataindex - 1];
        }

        inline reverse_iterator& operator ++ ()
        {
            if (currdataindex > 1) {
                --currdataindex;
            }
            else if (currindex > 0) {
                --currindex;
                currdataindex = currnode->data_count[currindex];
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
                currdataindex = currnode->data_count[currindex];
            }
            else {
                currdataindex = 0;
            }
            return *this;
        }

        inline reverse_iterator operator ++ (int)
        {
            reverse_iterator tmp = *this;

            if (currdataindex > 1) {
                --currdataindex;
            }
            else if (currindex > 0) {
                --currindex;
                currdataindex = currnode->data_count[currindex];
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
                currdataindex = currnode->data_count[currindex];
            }
            else {
                currdataindex = 0;
            }
            return tmp;
        }

        inline reverse_iterator& operator -- ()
        {
            if (currnode->right != NULL) {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                }
                else {
                    if (currindex == currnode->count - 1) {
                        if (currnode->right->count != 1 || currnode->right->right != NULL)
                        {
                            currnode = currnode->right;
                            currindex = 0;
                            currdataindex = 1;
                        }
                    }
                    else {
                        ++currindex;
                        currdataindex = 1;
                    }
                }
            }
            else {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex < currnode->count - 2) {
                            currdataindex = 0;
                            ++currindex;
                        }
                    }
                }
            }
            return *this;
        }

        inline reverse_iterator operator -- (int)
        {
            reverse_iterator tmp = *this;

            if (currnode->right != NULL) {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                }
                else {
                    if (currindex == currnode->count - 1) {
                        if (currnode->right->count != 1 || currnode->right->right != NULL)
                        {
                            currnode = currnode->right;
                            currindex = 0;
                            currdataindex = 1;
                        }
                    }
                    else {
                        ++currindex;
                        currdataindex = 1;
                    }
                }
            }
            else {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex < currnode->count - 2) {
                            currdataindex = 0;
                            ++currindex;
                        }
                    }
                }
            }
            return tmp;
        }

        inline bool operator == (const reverse_iterator &x) const
        {
            return (x.currnode == currnode && x.currindex == currindex && x.currdataindex == currdataindex);
        }

        inline bool operator != (const reverse_iterator &x) const
        {
            return (x.currnode != currnode || x.currindex != currindex || x.currdataindex != currdataindex);
        }

        inline bool is_invalid() const
        {
            return currnode == NULL;
        }

        inline bool is_end() const
        {
            return currdataindex == 0;
        }

        inline bool is_lazy_deleted() const
        {
            return data() == (data_type)0;
        }

        inline void lazy_delete()
        {
            data() = (data_type)0;
        }
    };

    class const_reverse_iterator {
    public:
        typedef typename skiplist_multimap_ro::key_type key_type;
        typedef typename skiplist_multimap_ro::data_type data_type;
        typedef typename skiplist_multimap_ro::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        const typename skiplist_multimap_ro::leaf_node *currnode;
        short currindex;
        unsigned short currdataindex;

        friend class const_iterator;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline const_reverse_iterator()
            : currnode(NULL), currindex(0), currdataindex(0)
        { }

        inline const_reverse_iterator(typename skiplist_multimap_ro::leaf_node *n, short index, unsigned short dataindex)
            : currnode(n), currindex(index), currdataindex(dataindex)
        { }

        inline const_reverse_iterator(const iterator& it)
            : currnode(it.currnode), currindex(it.currindex), currdataindex(it.currdataindex)
        { }

        inline const_reverse_iterator(const const_iterator& it)
            : currnode(it.currnode), currindex(it.currindex), currdataindex(it.currdataindex)
        { }

        inline const_reverse_iterator(const reverse_iterator& it)
            : currnode(it.currnode), currindex(it.currindex), currdataindex(it.currdataindex)
        { }

        inline reference operator * () const
        {
            SL_ASSERT(currdataindex > 0);
            temp_value = value_type(key(), data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            SL_ASSERT(currdataindex > 0);
            temp_value = value_type(key(), data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            SL_ASSERT(currdataindex > 0);
            return currnode->key[currindex];
        }

        inline const data_type & data() const
        {
            SL_ASSERT(currdataindex > 0);
            return currnode->data_array[currindex][currdataindex - 1];
        }

        inline const_reverse_iterator& operator ++ ()
        {
            if (currdataindex > 1) {
                --currdataindex;
            }
            else if (currindex > 0) {
                --currindex;
                currdataindex = currnode->data_count[currindex];
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
                currdataindex = currnode->data_count[currindex];
            }
            else {
                currdataindex = 0;
            }
            return *this;
        }

        inline const_reverse_iterator operator ++ (int)
        {
            const_reverse_iterator tmp = *this;

            if (currdataindex > 1) {
                --currdataindex;
            }
            else if (currindex > 0) {
                --currindex;
                currdataindex = currnode->data_count[currindex];
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
                currdataindex = currnode->data_count[currindex];
            }
            else {
                currdataindex = 0;
            }
            return tmp;
        }

        inline const_reverse_iterator& operator -- ()
        {
            if (currnode->right != NULL) {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                }
                else {
                    if (currindex == currnode->count - 1) {
                        if (currnode->right->count != 1 || currnode->right->right != NULL)
                        {
                            currnode = currnode->right;
                            currindex = 0;
                            currdataindex = 1;
                        }
                    }
                    else {
                        ++currindex;
                        currdataindex = 1;
                    }
                }
            }
            else {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex < currnode->count - 2) {
                            currdataindex = 0;
                            ++currindex;
                        }
                    }
                }
            }
            return *this;
        }

        inline const_reverse_iterator operator -- (int)
        {
            const_reverse_iterator tmp = *this;

            if (currnode->right != NULL) {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                }
                else {
                    if (currindex == currnode->count - 1) {
                        if (currnode->right->count != 1 || currnode->right->right != NULL)
                        {
                            currnode = currnode->right;
                            currindex = 0;
                            currdataindex = 1;
                        }
                    }
                    else {
                        ++currindex;
                        currdataindex = 1;
                    }
                }
            }
            else {
                if (currdataindex < currnode->data_count[currindex]) {
                    ++currdataindex;
                    if (currdataindex == currnode->data_count[currindex]) {
                        if (currindex < currnode->count - 2) {
                            currdataindex = 0;
                            ++currindex;
                        }
                    }
                }
            }
            return tmp;
        }

        inline bool operator == (const const_reverse_iterator &x) const
        {
            return (x.currnode == currnode && x.currindex == currindex && x.currdataindex == currdataindex);
        }

        inline bool operator != (const const_reverse_iterator &x) const
        {
            return (x.currnode != currnode || x.currindex != currindex || x.currdataindex != currdataindex);
        }

        inline bool is_invalid() const
        {
            return currnode == NULL;
        }

        inline bool is_end() const
        {
            return currdataindex == 0;
        }

        inline bool is_lazy_deleted() const
        {
            return data() == (data_type)0;
        }
    };

private:
    node *m_head;

    leaf_node *m_head_leaf;

    leaf_node *m_tail_leaf;

    size_type m_size;

    size_type m_level;

    size_type m_inner_count;

    size_type m_leaf_count;

    key_compare m_key_less;

    allocator_type m_allocator;

    typename inner_node::alloc_type m_inner_allocator;

    typename leaf_node::alloc_type m_leaf_allocator;

    data_alloc_type m_data_allocator;

public:
    explicit inline skiplist_multimap_ro(const allocator_type& alloc = allocator_type())
        : m_allocator(alloc)
    {
        m_inner_allocator = m_allocator;
        m_leaf_allocator = m_allocator;
        m_data_allocator = m_allocator;
        m_size = m_level = 0;
        m_inner_count = m_leaf_count = 0;

        m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
        leaf_node *l_head = static_cast<leaf_node *>(m_head);
        l_head->count = 1; // placeholder for virtual max key
        l_head->key[0] = key_type(); // placeholder for virtual max key
        l_head->data_count[0] = 0;
        l_head->data_array[0] = NULL;
    }

    explicit inline skiplist_multimap_ro(const key_compare& kcf,
                                 const allocator_type& alloc = allocator_type())
        : m_key_less(kcf), m_allocator(alloc)
    {
        m_inner_allocator = m_allocator;
        m_leaf_allocator = m_allocator;
        m_data_allocator = m_allocator;
        m_size = m_level = 0;
        m_inner_count = m_leaf_count = 0;

        m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
        leaf_node *l_head = static_cast<leaf_node *>(m_head);
        l_head->count = 1; // placeholder for virtual max key
        l_head->key[0] = key_type(); // placeholder for virtual max key
        l_head->data_count[0] = 0;
        l_head->data_array[0] = NULL;
    }

    inline ~skiplist_multimap_ro()
    {
        clear_all();
    }

    void swap(self_type &from)
    {
        std::swap(m_head, from.m_head);
        std::swap(m_head_leaf, from.m_head_leaf);
        std::swap(m_tail_leaf, from.m_tail_leaf);
        std::swap(m_size, from.m_size);
        std::swap(m_level, from.m_level);
        std::swap(m_inner_count, from.m_inner_count);
        std::swap(m_leaf_count, from.m_leaf_count);
        std::swap(m_key_less, from.m_key_less);
        std::swap(m_allocator, from.m_allocator);
    }

    class value_compare
    {
    protected:
        /// Key comparison function from the template parameter
        key_compare key_comp;

        /// Constructor called from btree::value_comp()
        inline explicit value_compare(key_compare kc)
            : key_comp(kc)
        { }

        /// Friendly to the skip list class so it may call the constructor
        friend class skiplist_multimap_ro<key_type, data_type, key_compare, traits,
                                  allocator_type>;

    public:
        /// Function call "less"-operator resulting in true if x < y.
        inline bool operator () (const value_type& x, const value_type& y) const
        {
            return key_comp(x.first, y.first);
        }
    };

    inline key_compare key_comp() const
    {
        return m_key_less;
    }

    allocator_type get_allocator() const
    {
        return m_allocator;
    }

private:
    // *** Convenient Key Comparison Functions Generated From key_less

    /// True if a < b ? "constructed" from m_key_less()
    inline bool key_less(const key_type& a, const key_type b) const
    {
        return m_key_less(a, b);
    }

    /// True if a <= b ? constructed from key_less()
    inline bool key_lessequal(const key_type& a, const key_type b) const
    {
        return !m_key_less(b, a);
    }

    /// True if a > b ? constructed from key_less()
    inline bool key_greater(const key_type& a, const key_type& b) const
    {
        return m_key_less(b, a);
    }

    /// True if a >= b ? constructed from key_less()
    inline bool key_greaterequal(const key_type& a, const key_type b) const
    {
        return !m_key_less(a, b);
    }

    /// True if a == b ? constructed from key_less(). This requires the <
    /// relation to be a total order, otherwise the B+ tree cannot be sorted.
    inline bool key_equal(const key_type& a, const key_type& b) const
    {
        return !m_key_less(a, b) && !m_key_less(b, a);
    }

private:

    inline leaf_node *allocate_leaf()
    {
        leaf_node *n = new (m_leaf_allocator.allocate(1)) leaf_node();
        n->is_leaf = 1;
        n->count = 0;
        n->left = NULL;
        n->right = NULL;
        m_leaf_count++;
        return n;
    }

    inline inner_node *allocate_inner()
    {
        inner_node *n = new (m_inner_allocator.allocate(1)) inner_node();
        n->is_leaf = 0;
        n->count = 0;
        n->right = NULL;
        m_inner_count++;
        return n;
    }

    inline data_type *allocate_data_array(unsigned short count)
    {
        data_type *a = new (m_data_allocator.allocate(count)) data_type();
        return a;
    }

    inline void free_node(node *n)
    {
        if (n->is_leaf) {
            leaf_node* ln = static_cast<leaf_node*>(n);
            short ln_count = ln->count;
            if (NULL == ln->right) {
                --ln_count;
            }
            for (short i = 0; i < ln_count; i++) {
                if (NULL != ln->data_array[i]) {
                    m_data_allocator.destroy(ln->data_array[i]);
                    m_data_allocator.deallocate(ln->data_array[i], ln->data_count[i]);
                }
            }
            m_leaf_allocator.destroy(ln);
            m_leaf_allocator.deallocate(ln, 1);
            m_leaf_count--;
        }
        else {
            inner_node* in = static_cast<inner_node*>(n);
            m_inner_allocator.destroy(in);
            m_inner_allocator.deallocate(in, 1);
            m_inner_count--;
        }
    }

    inline void free_data_array(data_type *&a, unsigned short count)
    {
        m_data_allocator.destroy(a);
        m_data_allocator.deallocate(a, count);
        a = NULL;
    }

    // free all allocated memory, only used in destructor
    void clear_all()
    {
        if (m_head == NULL) {
            return;
        }
        node *head = m_head, *next;
        while (!head->is_leaf) {
            inner_node *in = static_cast<inner_node*>(head);
            next = in->down[0];

            while (in != NULL) {
                inner_node *tmp = in->right;
                free_node(in);
                in = tmp;
            }

            head = next;
        }

        leaf_node *ln = static_cast<leaf_node *>(head);
        while (ln != NULL) {
            leaf_node *tmp = ln->right;
            free_node(ln);
            ln = tmp;
        }
        m_head = m_head_leaf = m_tail_leaf = NULL;
        m_size = m_level = 0;
    }

    // helper function for merging, m_head will point to head leaf after
    void clear_inner()
    {
        if (m_head == NULL) {
            return;
        }
        node *head = m_head, *next;
        while (!head->is_leaf) {
            inner_node *in = static_cast<inner_node*>(head);
            next = in->down[0];

            while (in != NULL) {
                inner_node *tmp = in->right;
                free_node(in);
                in = tmp;
            }

            head = next;
        }

        m_head = head;
        m_level = 0;
        m_inner_count = 0;
    }

public:
    // clear all nodes except the starting leaf node for empty skip list
    void clear()
    {
        if (m_head != NULL) {
            node *head = m_head, *next;
            while (!head->is_leaf) {
                inner_node *in = static_cast<inner_node*>(head);
                next = in->down[0];

                while (in != NULL) {
                    inner_node *tmp = in->right;
                    free_node(in);
                    in = tmp;
                }

                head = next;
            }

            leaf_node *ln = static_cast<leaf_node *>(head);
            while (ln != NULL) {
                leaf_node *tmp = ln->right;
                free_node(ln);
                ln = tmp;
            }
        }

        m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
        leaf_node *l_head = static_cast<leaf_node *>(m_head);
        l_head->count = 1; // placeholder for virtual max key
        l_head->key[0] = key_type(); // placeholder for virtual max key
        l_head->data_count[0] = 0;
        l_head->data_array[0] = NULL;

        m_size = m_level = 0;
        m_inner_count = 0;
        m_leaf_count = 1;
    }

public:
    inline iterator begin()
    {
        return iterator(m_head_leaf, 0, 0);
    }

    inline iterator end()
    {
        if (m_tail_leaf->count == 1 && m_tail_leaf->left != NULL) {
            return iterator(m_tail_leaf->left,
                            m_tail_leaf->left->count - 1,
                            m_tail_leaf->left->data_count[m_tail_leaf->left->count - 1]);
        }
        return iterator(m_tail_leaf,
                        m_tail_leaf->count - 2,
                        m_tail_leaf->data_count[m_tail_leaf->count - 2]);
    }

    inline const_iterator begin() const
    {
        return const_iterator(m_head_leaf, 0, 0);
    }

    inline const_iterator end() const
    {
        if (m_tail_leaf->count == 1 && m_tail_leaf->left != NULL) {
            return const_iterator(m_tail_leaf->left,
                                  m_tail_leaf->left->count - 1,
                                  m_tail_leaf->left->data_count[m_tail_leaf->left->count - 1]);
        }
        return const_iterator(m_tail_leaf,
                              m_tail_leaf->count - 2,
                              m_tail_leaf->data_count[m_tail_leaf->count - 2]);
    }

    inline reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    inline reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    inline const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    inline const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

public:
    // *** Access Functions to the basic stats

    inline size_type size() const
    {
        return m_size;
    }

    inline bool empty() const
    {
        return (size() == size_type(0));
    }

    inline size_type max_size() const
    {
        return size_type(-1);
    }

    inline size_type level() const
    {
        return m_level;
    }

    inline size_type inner_count() const
    {
        return m_inner_count;
    }

    inline size_type leaf_count() const
    {
        return m_leaf_count;
    }

public:
    // *** Standard Access Functions

    bool exists(const key_type& key) const
    {
        if (m_head_leaf->count > 1 && key_less(key, m_head_leaf->key[0])) {
            return false;
        }
        if (m_tail_leaf->count > 1) {
            if (key_greater(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return false;
            }
        }
        else if (m_tail_leaf->left != NULL) {
            leaf_node *prev_leaf = m_tail_leaf->left;
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return false;
            }
        }

        const node *n = m_head;
        short i;

        while (!n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }
            n = in->down[i];
        }

        const leaf_node *ln = static_cast<const leaf_node *>(n);
        for (i = 0; key_greater(key, ln->key[i]); i++);

        bool exists = key_equal(key, ln->key[i]);
        if (!exists) {
            return false;
        }
        else {
            for (unsigned short dataindex = 0; dataindex < ln->data_count[i]; dataindex++) {
                if (ln->data_array[i][dataindex] != (data_type)0) {
                    return true;
                }
            }
            return false;
        }
    }

    iterator find(const key_type& key)
    {
        if (m_head_leaf->count > 1 && key_less(key, m_head_leaf->key[0])) {
            return end();
        }
        if (m_tail_leaf->count > 1) {
            if (key_greater(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_tail_leaf->left != NULL) {
            leaf_node *prev_leaf = m_tail_leaf->left;
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        node *n = m_head;
        short i;

        while (!n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }
            n = in->down[i];
        }

        leaf_node *ln = static_cast<leaf_node *>(n);
        for (i = 0; key_greater(key, ln->key[i]); i++);

        bool exists = key_equal(key, ln->key[i]);
        if (!exists) {
            return end();
        }
        else {
            for (unsigned short dataindex = 0; dataindex < ln->data_count[i]; dataindex++) {
                if (ln->data_array[i][dataindex] != (data_type)0) {
                    return iterator(ln, i, dataindex);
                }
            }
            return end();
        }
    }

    const_iterator find(const key_type& key) const
    {
        if (m_head_leaf->count > 1 && key_less(key, m_head_leaf->key[0])) {
            return end();
        }
        if (m_tail_leaf->count > 1) {
            if (key_greater(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_tail_leaf->left != NULL) {
            leaf_node *prev_leaf = m_tail_leaf->left;
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        const node *n = m_head;
        short i;

        while (!n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }
            n = in->down[i];
        }

        const leaf_node *ln = static_cast<const leaf_node *>(n);
        for (i = 0; key_greater(key, ln->key[i]); i++);

        bool exists = key_equal(key, ln->key[i]);
        if (!exists) {
            return end();
        }
        else {
            for (unsigned short dataindex = 0; dataindex < ln->data_count[i]; dataindex++) {
                if (ln->data_array[i][dataindex] != (data_type)0) {
                    return const_iterator(ln, i, dataindex);
                }
            }
            return end();
        }
    }

    size_type count(const key_type& key) const
    {
        if (m_head_leaf->count > 1 && key_less(key, m_head_leaf->key[0])) {
            return 0;
        }
        if (m_tail_leaf->count > 1) {
            if (key_greater(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return 0;
            }
        }
        else if (m_tail_leaf->left != NULL) {
            leaf_node *prev_leaf = m_tail_leaf->left;
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return 0;
            }
        }

        const node *n = m_head;
        short i;

        while (!n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }
            n = in->down[i];
        }

        const leaf_node *ln = static_cast<const leaf_node *>(n);
        for (i = 0; key_greater(key, ln->key[i]); i++);

        bool exists = key_equal(key, ln->key[i]);
        if (!exists) {
            return 0;
        }
        else {
            unsigned short datacount = 0;
            for (unsigned short dataindex = 0; dataindex < ln->data_count[i]; dataindex++) {
                if (ln->data_array[i][dataindex] != (data_type)0) {
                    datacount++;
                }
            }
            return static_cast<size_type>(datacount);
        }
    }

    iterator lower_bound(const key_type& key)
    {
        if (m_tail_leaf->count > 1) {
            if (key_greater(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_tail_leaf->left != NULL) {
            leaf_node *prev_leaf = m_tail_leaf->left;
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        node *n = m_head;
        short i;

        while (!n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }
            n = in->down[i];
        }

        leaf_node *ln = static_cast<leaf_node *>(n);
        for (i = 0; key_greater(key, ln->key[i]); i++);

        iterator res = iterator(ln, i, 0);
        while (!res.is_end() && res.is_lazy_deleted()) {
            ++res;
        }
        return res;
    }

    const_iterator lower_bound(const key_type& key) const
    {
        if (m_tail_leaf->count > 1) {
            if (key_greater(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_tail_leaf->left != NULL) {
            leaf_node *prev_leaf = m_tail_leaf->left;
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        const node *n = m_head;
        short i;

        while (!n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }
            n = in->down[i];
        }

        const leaf_node *ln = static_cast<const leaf_node *>(n);
        for (i = 0; key_greater(key, ln->key[i]); i++);

        iterator res = const_iterator(ln, i, 0);
        while (!res.is_end() && res.is_lazy_deleted()) {
            ++res;
        }
        return res;
    }

    iterator upper_bound(const key_type& key)
    {
        if (m_tail_leaf->count > 1) {
            if (key_greaterequal(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_tail_leaf->left != NULL) {
            leaf_node *prev_leaf = m_tail_leaf->left;
            if (key_greaterequal(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        node *n = m_head;
        short i;

        while (!n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_less(key, in->key[i])) {
                    break;
                }
            }
            n = in->down[i];
        }

        leaf_node *ln = static_cast<leaf_node *>(n);
        for (i = 0; key_greaterequal(key, ln->key[i]); i++);

        iterator res = iterator(ln, i, 0);
        while (!res.is_end() && res.is_lazy_deleted()) {
            ++res;
        }
        return res;
    }

    const_iterator upper_bound(const key_type& key) const
    {
        if (m_tail_leaf->count > 1) {
            if (key_greaterequal(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_tail_leaf->left != NULL) {
            leaf_node *prev_leaf = m_tail_leaf->left;
            if (key_greaterequal(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        const node *n = m_head;
        short i;

        while (!n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_less(key, in->key[i])) {
                    break;
                }
            }
            n = in->down[i];
        }

        const leaf_node *ln = static_cast<const leaf_node *>(n);
        for (i = 0; key_greaterequal(key, ln->key[i]); i++);

        iterator res = const_iterator(ln, i, 0);
        while (!res.is_end() && res.is_lazy_deleted()) {
            ++res;
        }
        return res;
    }

    inline std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        return std::pair<iterator, iterator>(lower_bound(key), upper_bound(key));
    }

    inline std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        return std::pair<const_iterator, const_iterator>(lower_bound(key), upper_bound(key));
    }

public:
    // *** Skiplist Object Comparison Functions

    inline bool operator == (const self_type& other) const
    {
        return (size() == other.size()) && std::equal(begin(), end(), other.begin());
    }

    inline bool operator != (const self_type& other) const
    {
        return !(*this == other);
    }

//public:
    /// *** Fast Copy: Assign Operator and Copy Constructors

    /*
    inline self_type& operator = (const self_type& other) {

    }

    inline skiplist_multimap(const skiplist_multimap& other) {

    }
    */

private:
    bool is_valid_iterator(iterator iter)
    {
        if (NULL == iter.currnode ||
            (iter.currnode)->is_leaf != 1 ||
            iter.currindex < 0 ||
            iter.currindex >= (iter.currnode)->count ||
            iter.currdataindex < 0 ||
            iter.currdataindex >= (iter.currnode)->data_count[iter.currindex])
        {
            return false;
        }
        return true;
    }

    bool is_valid_reverse_iterator(reverse_iterator iter)
    {
        if (NULL == iter.currnode ||
            (iter.currnode)->is_leaf != 1 ||
            iter.currindex <= 0 ||
            iter.currindex > (iter.currnode)->count ||
            iter.currdataindex <= 0 ||
            iter.currdataindex > (iter.currnode)->data_count[iter.currindex])
        {
            return false;
        }
        return true;
    }

public:
    bool lazy_erase_one(const key_type& key)
    {
        iterator it = find(key);
        if (!it.is_end()) {
            it.lazy_delete();
            --m_size;
            return true;
        }
        return false;
    }

    bool lazy_erase(const key_type& key)
    {
        size_type c = 0;

        while (lazy_erase_one(key)) {
            c++;
        }

        return c;
    }

    void lazy_erase(iterator iter)
    {
        if (is_valid_iterator(iter) && !iter.is_lazy_deleted()) {
            iter.lazy_delete();
            --m_size;
        }
    }

    void lazy_erase(reverse_iterator iter)
    {
        if (is_valid_reverse_iterator(iter) && !iter.is_lazy_deleted()) {
            iter.lazy_delete();
            --m_size;
        }
    }

//private:
public:
    // for static stage skiplist, it is the only way to rebuild it
    // merge a normal skip list and rebuild a compact skip list
    void merge(writable_type& from)
    {
        if (from.m_size == 0) {
            return;
        }

        clear_inner();
        from.clear_inner();

        typename writable_type::leaf_node *dyna_ln = from.m_head_leaf;
        leaf_node *static_ln = m_head_leaf;
        size_t node_count = 1;

        data_type data_buf[MAX_DATA_COUNT_PER_KEY];
        unsigned short data_count = 0;

        // rebuild leaf nodes
        // case 1: self is empty
        if (m_size == 0) {
            short count = 0;
            static_ln->count = 0;

            key_type cur_key = dyna_ln->key[0];

            while (dyna_ln != NULL) {
                short dyna_count = dyna_ln->count;
                if (NULL == dyna_ln->right) {
                    --dyna_count;
                }
                for (short i = 0; i < dyna_count; i++) {
                    if (count == l_order) {
                        leaf_node *new_static_ln = allocate_leaf();
                        static_ln->right = new_static_ln;
                        new_static_ln->left = static_ln;
                        static_ln->count = count;
                        static_ln = new_static_ln;
                        count = 0;
                        node_count++;
                    }

                    if (!key_equal(cur_key, dyna_ln->key[i])) {
                        static_ln->key[count] = cur_key;
                        static_ln->data_array[count] = allocate_data_array(data_count);
                        memcpy(static_ln->data_array[count], data_buf, data_count * sizeof(data_type));
                        static_ln->data_count[count] = data_count;
                        count++;

                        cur_key = dyna_ln->key[i];
                        data_count = 0;
                    }

                    data_buf[data_count++] = dyna_ln->data[i];
                    m_size++;
                }
                static_ln->count = count;

                typename writable_type::leaf_node *next_dyna_ln = dyna_ln->right;
                if (next_dyna_ln != NULL) {
                    from.free_node(dyna_ln);
                }
                dyna_ln = next_dyna_ln;
            }

            // add the last key, possibly allocate one more leaf node
            if (count == l_order) {
                leaf_node *new_static_ln = allocate_leaf();
                static_ln->right = new_static_ln;
                new_static_ln->left = static_ln;
                static_ln->count = count;
                static_ln = new_static_ln;
                count = 0;
                node_count++;
            }

            // allocate data array for the last key
            static_ln->key[count] = cur_key;
            static_ln->data_array[count] = allocate_data_array(data_count);
            memcpy(static_ln->data_array[count], data_buf, data_count * sizeof(data_type));
            static_ln->data_count[count] = data_count;
            count++;

            // add the virtual max key, possibly allocate one more leaf node
            if (count == l_order) {
                leaf_node *new_static_ln = allocate_leaf();
                static_ln->right = new_static_ln;
                new_static_ln->left = static_ln;
                static_ln->count = count;
                static_ln = new_static_ln;
                count = 0;
                node_count++;
            }

            // record the virtual max key
            count++;
            static_ln->count = count;

            static_ln->right = NULL;
            m_tail_leaf = static_ln;
            from.m_head = from.m_head_leaf = from.m_tail_leaf;
            from.m_tail_leaf->count = 1;
            from.m_tail_leaf->left = NULL;
            from.m_tail_leaf->right = NULL;
            from.m_size = 0;
        }
        else {
            short dyna_index = 0;
            short static_index = 0;
            short new_index = 0;
            m_size = 0;

            leaf_node *new_static_ln = allocate_leaf();
            m_head = m_head_leaf = new_static_ln;

            key_type cur_key = key_lessequal(dyna_ln->key[0], static_ln->key[0]) ?
                               dyna_ln->key[0] : static_ln->key[0];

            while (dyna_ln != NULL && static_ln != NULL) {
                short dyna_count = dyna_ln->count;
                if (dyna_ln->right == NULL) {
                    dyna_count--;
                }
                short static_count = static_ln->count;
                if (static_ln->right == NULL) {
                    static_count--;
                }
                while (dyna_index < dyna_count && static_index < static_count) {
                    if (new_index == l_order) {
                        leaf_node *next_new_static_ln = allocate_leaf();
                        new_static_ln->right = next_new_static_ln;
                        next_new_static_ln->left = new_static_ln;
                        new_static_ln->count = new_index;
                        new_static_ln = next_new_static_ln;
                        new_index = 0;
                        node_count++;
                    }

                    if (key_lessequal(dyna_ln->key[dyna_index], static_ln->key[static_index])) {
                        if (!key_equal(cur_key, dyna_ln->key[dyna_index])) {
                            new_static_ln->key[new_index] = cur_key;
                            new_static_ln->data_array[new_index] = allocate_data_array(data_count);
                            memcpy(new_static_ln->data_array[new_index], data_buf, data_count * sizeof(data_type));
                            new_static_ln->data_count[new_index] = data_count;

                            new_index++;
                            cur_key = dyna_ln->key[dyna_index];
                            data_count = 0;
                        }

                        data_buf[data_count++] = dyna_ln->data[dyna_index];
                        dyna_index++;
                        m_size++;
                    }
                    else {
                        if (!key_equal(cur_key, static_ln->key[static_index])) {
                            new_static_ln->key[new_index] = cur_key;
                            new_static_ln->data_array[new_index] = allocate_data_array(data_count);
                            memcpy(new_static_ln->data_array[new_index], data_buf, data_count * sizeof(data_type));
                            new_static_ln->data_count[new_index] = data_count;

                            new_index++;
                            cur_key = static_ln->key[static_index];
                            data_count = 0;
                        }

                        unsigned short valid_count = 0;
                        for (unsigned short i = 0; i < static_ln->data_count[static_index]; i++) {
                            if (static_ln->data_array[static_index][i] != (data_type)0) {
                                valid_count++;
                                data_buf[data_count++] = static_ln->data_array[static_index][i];
                            }
                        }
                        free_data_array(static_ln->data_array[static_index], static_ln->data_count[static_index]);

                        static_index++;
                        m_size += valid_count;
                    }
                }
                new_static_ln->count = new_index;
                if (dyna_index == dyna_count) {
                    typename writable_type::leaf_node *next_dyna_ln = dyna_ln->right;
                    if (next_dyna_ln != NULL) {
                        from.free_node(dyna_ln);
                    }
                    dyna_ln = next_dyna_ln;
                    dyna_index = 0;
                }
                else if (static_index == static_count) {
                    leaf_node *next_static_ln = static_ln->right;
                    free_node(static_ln);
                    static_ln = next_static_ln;
                    static_index = 0;
                }
            }

            while (dyna_ln != NULL) {
                short dyna_count = dyna_ln->count;
                if (NULL == dyna_ln->right) {
                    dyna_count--;
                }
                while (dyna_index < dyna_count) {
                    if (new_index == l_order) {
                        leaf_node *next_new_static_ln = allocate_leaf();
                        new_static_ln->right = next_new_static_ln;
                        next_new_static_ln->left = new_static_ln;
                        new_static_ln->count = new_index;
                        new_static_ln = next_new_static_ln;
                        new_index = 0;
                        node_count++;
                    }

                    if (!key_equal(cur_key, dyna_ln->key[dyna_index])) {
                        new_static_ln->key[new_index] = cur_key;
                        new_static_ln->data_array[new_index] = allocate_data_array(data_count);
                        memcpy(new_static_ln->data_array[new_index], data_buf, data_count * sizeof(data_type));
                        new_static_ln->data_count[new_index] = data_count;

                        new_index++;
                        cur_key = dyna_ln->key[dyna_index];
                        data_count = 0;
                    }

                    data_buf[data_count++] = dyna_ln->data[dyna_index];
                    dyna_index++;
                    m_size++;
                }
                new_static_ln->count = new_index;

                typename writable_type::leaf_node *next_dyna_ln = dyna_ln->right;
                if (next_dyna_ln != NULL) {
                    from.free_node(dyna_ln);
                }
                dyna_ln = next_dyna_ln;
                dyna_index = 0;
            }

            while (static_ln != NULL) {
                short static_count = static_ln->count;
                if (NULL == static_ln->right) {
                    static_count--;
                }
                while (static_index < static_count) {
                    if (new_index == l_order) {
                        leaf_node *next_new_static_ln = allocate_leaf();
                        new_static_ln->right = next_new_static_ln;
                        next_new_static_ln->left = new_static_ln;
                        new_static_ln->count = new_index;
                        new_static_ln = next_new_static_ln;
                        new_index = 0;
                        node_count++;
                    }

                    if (!key_equal(cur_key, static_ln->key[static_index])) {
                        new_static_ln->key[new_index] = cur_key;
                        new_static_ln->data_array[new_index] = allocate_data_array(data_count);
                        memcpy(new_static_ln->data_array[new_index], data_buf, data_count * sizeof(data_type));
                        new_static_ln->data_count[new_index] = data_count;

                        new_index++;
                        cur_key = static_ln->key[static_index];
                        data_count = 0;
                    }

                    unsigned short valid_count = 0;
                    for (unsigned short i = 0; i < static_ln->data_count[static_index]; i++) {
                        if (static_ln->data_array[static_index][i] != (data_type)0) {
                            valid_count++;
                            data_buf[data_count++] = static_ln->data_array[static_index][i];
                        }
                    }
                    free_data_array(static_ln->data_array[static_index], static_ln->data_count[static_index]);

                    static_index++;
                    m_size += valid_count;
                }
                new_static_ln->count = new_index;

                leaf_node *next_static_ln = static_ln->right;
                free_node(static_ln);
                static_ln = next_static_ln;
                static_index = 0;
            }

            // add the last key, possibly allocate one more leaf node
            if (new_index == l_order) {
                leaf_node *next_new_static_ln = allocate_leaf();
                new_static_ln->right = next_new_static_ln;
                next_new_static_ln->left = new_static_ln;
                new_static_ln->count = new_index;
                new_static_ln = next_new_static_ln;
                new_index = 0;
                node_count++;
            }

            // allocate data array for the last key
            new_static_ln->key[new_index] = cur_key;
            new_static_ln->data_array[new_index] = allocate_data_array(data_count);
            memcpy(new_static_ln->data_array[new_index], data_buf, data_count * sizeof(data_type));
            new_static_ln->data_count[new_index] = data_count;
            new_index++;

            // add the virtual max key, possibly allocate one more leaf node
            if (new_index == l_order) {
                leaf_node *next_new_static_ln = allocate_leaf();
                new_static_ln->right = next_new_static_ln;
                next_new_static_ln->left = new_static_ln;
                new_static_ln->count = new_index;
                new_static_ln = next_new_static_ln;
                new_index = 0;
                node_count++;
            }

            // record the virtual max key
            new_index++;
            new_static_ln->count = new_index;

            new_static_ln->right = NULL;
            m_tail_leaf = new_static_ln;
            m_leaf_count = node_count;
            from.m_head = from.m_head_leaf = from.m_tail_leaf;
            from.m_tail_leaf->count = 1;
            from.m_tail_leaf->left = NULL;
            from.m_tail_leaf->right = NULL;
            from.m_size = 0;
        }

        // build inner nodes
        m_level = 0;
        if (node_count > 1) {
            inner_node *head_inner;
            static_ln = m_head_leaf;
            inner_node *static_in = allocate_inner();
            head_inner = static_in;
            node_count = 1;
            short inner_index = 0;
            while (static_ln != NULL) {
                if (inner_index == i_order) {
                    inner_node *next_static_in = allocate_inner();
                    static_in->right = next_static_in;
                    static_in->count = inner_index;
                    static_in = next_static_in;
                    node_count++;
                    inner_index = 0;
                }
                static_in->key[inner_index] = static_ln->key[static_ln->count - 1];
                static_in->down[inner_index] = static_ln;
                inner_index++;
                static_ln = static_ln->right;
            }
            static_in->right = NULL;
            static_in->count = inner_index;
            m_level++;

            while (node_count > 1) {
                inner_node *static_in = head_inner;
                inner_node *upper_static_in = allocate_inner();
                head_inner = upper_static_in;
                node_count = 1;
                short inner_index = 0;
                while (static_in != NULL) {
                    if (inner_index == i_order) {
                        inner_node *next_upper_static_in = allocate_inner();
                        upper_static_in->right = next_upper_static_in;
                        upper_static_in->count = inner_index;
                        upper_static_in = next_upper_static_in;
                        node_count++;
                        inner_index = 0;
                    }
                    upper_static_in->key[inner_index] = static_in->key[static_in->count - 1];
                    upper_static_in->down[inner_index] = static_in;
                    inner_index++;
                    static_in = static_in->right;
                }
                upper_static_in->right = NULL;
                upper_static_in->count = inner_index;
                m_level++;
            }

            m_head = head_inner;
        }
    }

#ifdef SL_DEBUG

public:
    // *** Debug Printing

    void print(std::ostream& os) const
    {
        os << "Level: " << m_level << std::endl;
        os << "Size: " << m_size << std::endl;
        os << "Inner count: " << m_inner_count << std::endl;
        os << "Leaf count: " << m_leaf_count << std::endl;
        os << "Inner order: " << i_order << std::endl;
        os << "Leaf order: " << l_order << std::endl;

        node *level_head = m_head;
        if (level_head == NULL) {
            return;
        }

        while (!level_head->is_leaf) {
            inner_node *tmp = static_cast<inner_node *>(level_head);
            while (tmp != NULL) {
                os << "[";
                for (short i = 0; i < tmp->count - 1; i++) {
                    os << std::setw(6) << tmp->key[i];
                }
                if (tmp->right != NULL) {
                    os << std::setw(6) << tmp->key[tmp->count - 1];
                }
                else {
                    os << std::setw(6) << "Max";
                }
                os << "]";
                tmp = tmp->right;
            }
            os << std::endl;
            inner_node *ihead = static_cast<inner_node *>(level_head);
            level_head = ihead->down[0];
        }

        leaf_node *tmp = static_cast<leaf_node *>(level_head);
        while (tmp != NULL) {
            os << "[";
            for (short i = 0; i < tmp->count - 1; i++) {
                os << std::setw(6) << tmp->key[i];
                os << "{" << tmp->data_array[i][0];
                for (unsigned short j = 1; j < tmp->data_count[i]; j++) {
                    os << "," << tmp->data_array[i][j];
                }
                os << "}";
            }
            if (tmp->right != NULL) {
                os << std::setw(6) << tmp->key[tmp->count - 1];
                os << "{" << tmp->data_array[tmp->count - 1][0];
                for (unsigned short j = 1; j < tmp->data_count[tmp->count - 1]; j++) {
                    os << "," << tmp->data_array[tmp->count - 1][j];
                }
                os << "}";
            }
            else {
                os << std::setw(6) << "Max";
            }
            os << "]";
            tmp = tmp->right;
        }
        os << std::endl << std::endl;
    }
#endif
};

}

#endif

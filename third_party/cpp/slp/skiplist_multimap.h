#ifndef SKIPLIST_MULTIMAP_H_HEADER
#define SKIPLIST_MULTIMAP_H_HEADER

#include <algorithm>
#include <functional>
#include <istream>
#include <ostream>
#include <memory>
#include <cstddef>
#include <cassert>
#include "skiplist_traits.h"

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
class skiplist_multimap_ro;

template <typename _Key, typename _Data,
          typename _Compare = std::less<_Key>,
          typename _Traits = skiplist_default_map_traits<_Key, _Data>,
          typename _Alloc = std::allocator<std::pair<_Key, _Data> > >
class skiplist_multimap
{
#define SL_FRIENDS friend class skiplist_multimap_compact<_Key, _Data, _Compare, _Traits, _Alloc>;
public:
    typedef _Key key_type;
    typedef _Data data_type;
    typedef _Compare key_compare;
    typedef _Traits traits;
    typedef _Alloc allocator_type;
    friend class skiplist_multimap_ro<key_type, data_type, key_compare, traits,
                                      allocator_type>;
    SL_FRIENDS

public:
    typedef skiplist_multimap<key_type, data_type, key_compare, traits,
                         allocator_type> self_type;
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
        data_type data[l_order];
    };

public:
    class iterator;
    class const_iterator;
    class reverse_iterator;
    class const_reverse_iterator;

    class iterator {
    public:
        typedef typename skiplist_multimap::key_type key_type;
        typedef typename skiplist_multimap::data_type data_type;
        typedef typename skiplist_multimap::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        typename skiplist_multimap::leaf_node *currnode;
        short currindex;

        friend class reverse_iterator;
        friend class const_iterator;
        friend class const_reverse_iterator;
        friend class skiplist_multimap<key_type, data_type, key_compare, traits,
                                  allocator_type>;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline iterator()
            : currnode(NULL), currindex(0)
        { }

        inline iterator(typename skiplist_multimap::leaf_node *n, short index)
            : currnode(n), currindex(index)
        { }

        inline iterator(const reverse_iterator& it)
            : currnode(it.currnode), currindex(it.currindex)
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
            return currnode->data[currindex];
        }

        inline iterator& operator ++ ()
        {
            if (currnode->right != NULL) {
                if (currindex < currnode->count) {
                    ++currindex;
                    if (currindex == currnode->count &&
                        (currnode->right->count != 1 || currnode->right->right != NULL))
                    {
                        currnode = currnode->right;
                        currindex = 0;
                    }
                }
            }
            else if (currindex < currnode->count - 1) {
                ++currindex;
            }
            return *this;
        }

        inline iterator operator ++ (int)
        {
            iterator tmp = *this;

            if (currnode->right != NULL) {
                if (currindex < currnode->count) {
                    ++currindex;
                    if (currindex == currnode->count &&
                        (currnode->right->count != 1 || currnode->right->right != NULL))
                    {
                        currnode = currnode->right;
                        currindex = 0;
                    }
                }
            }
            else if (currindex < currnode->count - 1) {
                ++currindex;
            }
            return tmp;
        }

        inline iterator& operator -- ()
        {
            if (currindex > 0) {
                --currindex;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
            }
            return *this;
        }

        inline iterator operator -- (int)
        {
            iterator tmp = *this;

            if (currindex > 0) {
                --currindex;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
            }
            return tmp;
        }

        inline bool operator == (const iterator &x) const
        {
            return (x.currnode == currnode && x.currindex == currindex);
        }

        inline bool operator != (const iterator &x) const
        {
            return (x.currnode != currnode || x.currindex != currindex);
        }

        inline bool is_invalid() const
        {
            return currnode == NULL;
        }

        inline bool is_end() const
        {
            short end_index = currnode->count;
            if (currnode->right == NULL) {
                --end_index;
            }
            return (currindex == end_index);
        }
    };

    class const_iterator {
    public:
        typedef typename skiplist_multimap::key_type key_type;
        typedef typename skiplist_multimap::data_type data_type;
        typedef typename skiplist_multimap::value_type value_type;
        typedef const value_type& reference;
        typedef const value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        const typename skiplist_multimap::leaf_node *currnode;
        short currindex;

        friend class const_reverse_iterator;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline const_iterator()
            : currnode(NULL), currindex(0)
        { }

        inline const_iterator(const typename skiplist_multimap::leaf_node *n, short index)
            : currnode(n), currindex(index)
        { }

        inline const_iterator(const iterator& it)
            : currnode(it.currnode), currindex(it.currindex)
        { }

        inline const_iterator(const reverse_iterator& it)
            : currnode(it.currnode), currindex(it.currindex)
        { }

        inline const_iterator(const const_reverse_iterator& it)
            : currnode(it.currnode), currindex(it.currindex)
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
            return currnode->data[currindex];
        }

        inline const_iterator& operator ++ ()
        {
            if (currnode->right != NULL) {
                if (currindex < currnode->count) {
                    ++currindex;
                    if (currindex == currnode->count &&
                        (currnode->right->count != 1 || currnode->right->right != NULL))
                    {
                        currnode = currnode->right;
                        currindex = 0;
                    }
                }
            }
            else if (currindex < currnode->count - 1) {
                ++currindex;
            }
            return *this;
        }

        inline const_iterator operator ++ (int)
        {
            const_iterator tmp = *this;

            if (currnode->right != NULL) {
                if (currindex < currnode->count) {
                    ++currindex;
                    if (currindex == currnode->count &&
                        (currnode->right->count != 1 || currnode->right->right != NULL))
                    {
                        currnode = currnode->right;
                        currindex = 0;
                    }
                }
            }
            else if (currindex < currnode->count - 1) {
                ++currindex;
            }
            return tmp;
        }

        inline const_iterator& operator -- ()
        {
            if (currindex > 0) {
                --currindex;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
            }

            return *this;
        }

        inline const_iterator operator -- (int)
        {
            const_iterator tmp = *this;

            if (currindex > 0) {
                --currindex;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count - 1;
            }

            return tmp;
        }

        inline bool operator == (const const_iterator &x) const
        {
            return (x.currnode == currnode && x.currindex == currindex);
        }

        inline bool operator != (const const_iterator &x) const
        {
            return (x.currnode != currnode || x.currindex != currindex);
        }

        inline bool is_invalid() const
        {
            return currnode == NULL;
        }

        inline bool is_end() const
        {
            short end_index = currnode->count;
            if (currnode->right == NULL) {
                --end_index;
            }
            return (currindex == end_index);
        }
    };

    class reverse_iterator {
    public:
        typedef typename skiplist_multimap::key_type key_type;
        typedef typename skiplist_multimap::data_type data_type;
        typedef typename skiplist_multimap::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        typename skiplist_multimap::leaf_node *currnode;
        short currindex;

        friend class iterator;
        friend class const_iterator;
        friend class const_reverse_iterator;
        friend class skiplist_multimap<key_type, data_type, key_compare, traits,
                                  allocator_type>;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline reverse_iterator()
            : currnode(NULL), currindex(0)
        { }

        inline reverse_iterator(typename skiplist_multimap::leaf_node *n, short index)
            : currnode(n), currindex(index)
        { }

        inline reverse_iterator(const iterator& it)
            : currnode(it.currnode), currindex(it.currindex)
        { }

        inline reference operator * () const
        {
            SL_ASSERT(currindex > 0);
            temp_value = value_type(key(), data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            SL_ASSERT(currindex > 0);
            temp_value = value_type(key(), data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            SL_ASSERT(currindex > 0);
            return currnode->key[currindex - 1];
        }

        inline data_type & data() const
        {
            SL_ASSERT(currindex > 0);
            return currnode->data[currindex - 1];
        }

        inline reverse_iterator& operator ++ ()
        {
            if (currindex > 1) {
                --currindex;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count;
            }
            else {
                currindex = 0;
            }
            return *this;
        }

        inline reverse_iterator operator ++ (int)
        {
            reverse_iterator tmp = *this;

            if (currindex > 1) {
                --currindex;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count;
            }
            else {
                currindex = 0;
            }
            return tmp;
        }

        inline reverse_iterator& operator -- ()
        {
            if (currnode->right != NULL) {
                if (currindex < currnode->count) {
                    ++currindex;
                }
                else if (currnode->right->count != 1 ||
                         currnode->right->right != NULL)
                {
                    currnode = currnode->right;
                    currindex = 1;
                }
            }
            else if (currindex < currnode->count - 1) {
                ++currindex;
            }
            return *this;
        }

        inline reverse_iterator operator -- (int)
        {
            reverse_iterator tmp = *this;

            if (currnode->right != NULL) {
                if (currindex < currnode->count) {
                    ++currindex;
                }
                else if (currnode->right->count != 1 ||
                         currnode->right->right != NULL)
                {
                    currnode = currnode->right;
                    currindex = 1;
                }
            }
            else if (currindex < currnode->count - 1) {
                ++currindex;
            }
            return tmp;
        }

        inline bool operator == (const reverse_iterator &x) const
        {
            return (x.currnode == currnode && x.currindex == currindex);
        }

        inline bool operator != (const reverse_iterator &x) const
        {
            return (x.currnode != currnode || x.currindex != currindex);
        }

        inline bool is_invalid() const
        {
            return currnode == NULL;
        }

        inline bool is_end() const
        {
            return currindex == 0;
        }
    };

    class const_reverse_iterator {
    public:
        typedef typename skiplist_multimap::key_type key_type;
        typedef typename skiplist_multimap::data_type data_type;
        typedef typename skiplist_multimap::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        const typename skiplist_multimap::leaf_node *currnode;
        short currindex;

        friend class const_iterator;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline const_reverse_iterator()
            : currnode(NULL), currindex(0)
        { }

        inline const_reverse_iterator(typename skiplist_multimap::leaf_node *n, short index)
            : currnode(n), currindex(index)
        { }

        inline const_reverse_iterator(const iterator& it)
            : currnode(it.currnode), currindex(it.currindex)
        { }

        inline const_reverse_iterator(const const_iterator& it)
            : currnode(it.currnode), currindex(it.currindex)
        { }

        inline const_reverse_iterator(const reverse_iterator& it)
            : currnode(it.currnode), currindex(it.currindex)
        { }

        inline reference operator * () const
        {
            SL_ASSERT(currindex > 0);
            temp_value = value_type(key(), data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            SL_ASSERT(currindex > 0);
            temp_value = value_type(key(), data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            SL_ASSERT(currindex > 0);
            return currnode->key[currindex - 1];
        }

        inline const data_type & data() const
        {
            SL_ASSERT(currindex > 0);
            return currnode->data[currindex - 1];
        }

        inline const_reverse_iterator& operator ++ ()
        {
            if (currindex > 1) {
                --currindex;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count;
            }
            else {
                currindex = 0;
            }
            return *this;
        }

        inline const_reverse_iterator operator ++ (int)
        {
            const_reverse_iterator tmp = *this;

            if (currindex > 1) {
                --currindex;
            }
            else if (currnode->left != NULL) {
                currnode = currnode->left;
                currindex = currnode->count;
            }
            else {
                currindex = 0;
            }
            return tmp;
        }

        inline const_reverse_iterator& operator -- ()
        {
            if (currnode->right != NULL) {
                if (currindex < currnode->count) {
                    ++currindex;
                }
                else if (currnode->right->count != 1 ||
                         currnode->right->right != NULL)
                {
                    currnode = currnode->right;
                    currindex = 1;
                }
            }
            else if (currindex < currnode->count - 1) {
                ++currindex;
            }
            return *this;
        }

        inline const_reverse_iterator operator -- (int)
        {
            const_reverse_iterator tmp = *this;

            if (currnode->right != NULL) {
                if (currindex < currnode->count) {
                    ++currindex;
                }
                else if (currnode->right->count != 1 ||
                         currnode->right->right != NULL)
                {
                    currnode = currnode->right;
                    currindex = 1;
                }
            }
            else if (currindex < currnode->count - 1) {
                ++currindex;
            }
            return tmp;
        }

        inline bool operator == (const const_reverse_iterator &x) const
        {
            return (x.currnode == currnode && x.currindex == currindex);
        }

        inline bool operator != (const const_reverse_iterator &x) const
        {
            return (x.currnode != currnode || x.currindex != currindex);
        }

        inline bool is_invalid() const
        {
            return currnode == NULL;
        }

        inline bool is_end() const
        {
            return currindex == 0;
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

public:
    explicit inline skiplist_multimap(const allocator_type& alloc = allocator_type())
        : m_allocator(alloc)
    {
        m_inner_allocator = m_allocator;
        m_leaf_allocator = m_allocator;
        m_size = m_level = 0;
        m_inner_count = m_leaf_count = 0;

        m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
        leaf_node *l_head = static_cast<leaf_node *>(m_head);
        l_head->count = 1; // placeholder for virtual max key
        l_head->key[0] = key_type(); // placeholder for virtual max key
        l_head->data[0] = data_type();
    }

    explicit inline skiplist_multimap(const key_compare& kcf,
                                 const allocator_type& alloc = allocator_type())
        : m_key_less(kcf), m_allocator(alloc)
    {
        m_inner_allocator = m_allocator;
        m_leaf_allocator = m_allocator;
        m_size = m_level = 0;
        m_inner_count = m_leaf_count = 0;

        m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
        leaf_node *l_head = static_cast<leaf_node *>(m_head);
        l_head->count = 1; // placeholder for virtual max key
        l_head->key[0] = key_type(); // placeholder for virtual max key
        l_head->data[0] = data_type();
    }

    inline ~skiplist_multimap()
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
        friend class skiplist_multimap<key_type, data_type, key_compare, traits,
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

    inline void free_node(node *n)
    {
        if (n->is_leaf) {
            leaf_node* ln = static_cast<leaf_node*>(n);
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
            leaf_node *first_leaf = ln;
            ln = ln->right;
            while (ln != NULL) {
                leaf_node *tmp = ln->right;
                free_node(ln);
                ln = tmp;
            }

            // reset the only leaf node left to be the root node
            first_leaf->count = 1;
            first_leaf->key[0] = key_type();
            first_leaf->data[0] = data_type();
            first_leaf->left = first_leaf->right = NULL;
            m_head = m_head_leaf = m_tail_leaf = first_leaf;
        }
        else {
            m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
            leaf_node *l_head = static_cast<leaf_node *>(m_head);
            l_head->count = 1; // placeholder for virtual max key
            l_head->key[0] = key_type(); // placeholder for virtual max key
            l_head->data[0] = data_type();
        }

        m_size = m_level = 0;
        m_inner_count = 0;
        m_leaf_count = 1;
    }

public:
    inline iterator begin()
    {
        return iterator(m_head_leaf, 0);
    }

    inline iterator end()
    {
        if (m_tail_leaf->count == 1 && m_tail_leaf->left != NULL) {
            return iterator(m_tail_leaf->left, m_tail_leaf->left->count);
        }
        return iterator(m_tail_leaf, m_tail_leaf->count - 1);
    }

    inline const_iterator begin() const
    {
        return const_iterator(m_head_leaf, 0);
    }

    inline const_iterator end() const
    {
        if (m_tail_leaf->count == 1 && m_tail_leaf->left != NULL) {
            return const_iterator(m_tail_leaf->left, m_tail_leaf->left->count);
        }
        return const_iterator(m_tail_leaf, m_tail_leaf->count - 1);
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
        if (m_size == 0) {
            return false;
        }
        if (key_less(key, m_head_leaf->key[0])) {
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

        return key_equal(key, ln->key[i]);
    }

    iterator find(const key_type& key)
    {
        if (m_size == 0) {
            return end();
        }
        if (key_less(key, m_head_leaf->key[0])) {
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

        return key_equal(key, ln->key[i]) ? iterator(ln, i) : end();
    }

    const_iterator find(const key_type& key) const
    {
        if (m_size == 0) {
            return end();
        }
        if (key_less(key, m_head_leaf->key[0])) {
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

        return key_equal(key, ln->key[i]) ? const_iterator(ln, i) : end();
    }

    size_type count(const key_type& key) const
    {
        if (m_size == 0) {
            return 0;
        }
        if (key_less(key, m_head_leaf->key[0])) {
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

        return key_equal(key, ln->key[i]) ? 1 : 0;
    }

    iterator lower_bound(const key_type& key)
    {
        if (m_size == 0) {
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

        return iterator(ln, i);
    }

    const_iterator lower_bound(const key_type& key) const
    {
        if (m_size == 0) {
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

        return const_iterator(ln, i);
    }

    iterator upper_bound(const key_type& key)
    {
        if (m_size == 0) {
            return end();
        }
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

        return iterator(ln, i);
    }

    const_iterator upper_bound(const key_type& key) const
    {
        if (m_size == 0) {
            return end();
        }
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

        return const_iterator(ln, i);
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

public:
    /// *** Fast Copy: Assign Operator and Copy Constructors

    /*
    inline self_type& operator = (const self_type& other) {

    }

    inline skiplist_multimap(const skiplist_multimap& other) {

    }
    */

public:
    // *** Public Insertion Functions

    inline std::pair<iterator, bool> insert(const pair_type& x)
    {
        return insert_common(x.first, x.second);
    }

    inline std::pair<iterator, bool> insert(const key_type& key, const data_type& data)
    {
        return insert_common(key, data);
    }

private:
    inline void inner_shift_right(inner_node *n, short i)
    {
        for (short j = n->count - 1; j >= i; j--) {
            n->key[j+1] = n->key[j];
            n->down[j+1] = n->down[j];
        }

        n->count++;
    }

    inline void inner_shift_left(inner_node *n, short i)
    {
        for (; i < n->count - 1; i++) {
            n->key[i] = n->key[i+1];
            n->down[i] = n->down[i+1];
        }

        n->count--;
    }

    inline void leaf_shift_right(leaf_node *n, short i)
    {
        for (short j = n->count - 1; j >= i; j--) {
            n->key[j+1] = n->key[j];
            n->data[j+1] = n->data[j];
        }

        n->count++;
    }

    inline void leaf_shift_left(leaf_node *n, short i)
    {
        for (; i < n->count - 1; i++) {
            n->key[i] = n->key[i+1];
            n->data[i] = n->data[i+1];
        }

        n->count--;
    }

    inline node *split_node(node *node)
    {
        if (node->is_leaf) {
            leaf_node *l = static_cast<leaf_node *>(node);
            leaf_node *r = allocate_leaf();

            l->count = l_half_order;
            r->count = l_order - l_half_order;
            r->right = l->right;
            l->right = r;
            r->left = l;
            // update m_tail_leaf if needed
            if (r->right == NULL) {
                m_tail_leaf = r;
            }

            for (short i = l_order - 1; i >= l_half_order; i--) {
                r->key[i - l_half_order] = l->key[i];
                r->data[i - l_half_order] = l->data[i];
            }

            return r;
        }
        else {
            inner_node *l = static_cast<inner_node *>(node);
            inner_node *r = allocate_inner();

            l->count = i_half_order;
            r->count = i_order - i_half_order;
            r->right = l->right;
            l->right = r;

            for (short i = i_order - 1; i >= i_half_order; i--) {
                r->key[i - i_half_order] = l->key[i];
                r->down[i - i_half_order] = l->down[i];
            }

            return r;
        }
    }

    inline void concat_node(node *left, node *right)
    {
        if (left->is_leaf) {
            leaf_node *l = static_cast<leaf_node *>(left);
            leaf_node *r = static_cast<leaf_node *>(right);
            short j = l->count;
            for (short i = 0; i < r->count; i++, j++) {
                l->key[j] = r->key[i];
                l->data[j] = r->data[i];
            }

            l->count += r->count;
            l->right = r->right;
            if (l->right == NULL) {
                m_tail_leaf = l;
            }
            else {
                leaf_node *rr = static_cast<leaf_node *>(r->right);
                rr->left = l;
            }

            free_node(r);
        }
        else {
            inner_node *l = static_cast<inner_node *>(left);
            inner_node *r = static_cast<inner_node *>(right);
            short j = l->count;
            for (short i = 0; i < r->count; i++, j++) {
                l->key[j] = r->key[i];
                l->down[j] = r->down[i];
            }

            l->count += r->count;
            l->right = r->right;

            free_node(r);
        }
    }

    inline void redistribute_right_left(node *left, node *right)
    {
        short lc = left->count;
        short rc = right->count;
        short move = rc - ((lc + rc) >> 1);

        if (left->is_leaf) {
            leaf_node *l = static_cast<leaf_node *>(left);
            leaf_node *r = static_cast<leaf_node *>(right);
            short i, j;

            for (i = 0, j = lc; i < move; i++, j++) {
                l->key[j] = r->key[i];
                l->data[j] = r->data[i];
            }

            for (j = 0; i < rc; i += 1, j += 1) {
                r->key[j] = r->key[i];
                r->data[j] = r->data[i];
            }
        }
        else {
            inner_node *l = static_cast<inner_node *>(left);
            inner_node *r = static_cast<inner_node *>(right);
            short i, j;

            for (i = 0, j = lc; i < move; i++, j++) {
                l->key[j] = r->key[i];
                l->down[j] = r->down[i];
            }

            for (j = 0; i < rc; i++, j++) {
                r->key[j] = r->key[i];
                r->down[j] = r->down[i];
            }
        }

        right->count = rc - move;
        left->count = lc + move;
    }

    inline void redistribute_left_right(node *left, node *right)
    {
        short lc = left->count;
        short rc = right->count;
        short move = lc - ((lc + rc) >> 1);

        if (left->is_leaf) {
            leaf_node *l = static_cast<leaf_node *>(left);
            leaf_node *r = static_cast<leaf_node *>(right);
            short i, j;

            for (i = rc-1, j = i+move; i >= 0; i--, j--) {
                r->key[j] = r->key[i];
                r->data[j] = r->data[i];
            }

            for (i = lc - move, j = 0; i < lc; i++, j++) {
                r->key[j] = l->key[i];
                r->data[j] = l->data[i];
            }
        }
        else {
            inner_node *l = static_cast<inner_node *>(left);
            inner_node *r = static_cast<inner_node *>(right);
            short i, j;

            for (i = rc-1, j = i+move; i >= 0; i--, j--) {
                r->key[j] = r->key[i];
                r->down[j] = r->down[i];
            }

            for (i = lc - move, j = 0; i < lc; i++, j++) {
                r->key[j] = l->key[i];
                r->down[j] = l->down[i];
            }
        }

        right->count = rc - move;
        left->count = lc + move;
    }

    std::pair<iterator, bool> insert_common(const key_type& key, const data_type& data)
    {
        node *n = m_head;
        short i;

        while (!n->is_leaf) {
            inner_node *in = static_cast<inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }

            node *child = in->down[i];
            if (child->count == ((child->is_leaf) ? l_order : i_order)) {
                node *new_child = split_node(child);
                inner_shift_right(in, i);
                if (child->is_leaf) {
                    in->key[i] = (static_cast<leaf_node *>(child))->key[l_half_order-1];
                }
                else {
                    in->key[i] = (static_cast<inner_node *>(child))->key[i_half_order-1];
                }
                in->down[i] = child;
                in->down[i+1] = new_child;

                if (key_greater(key, in->key[i])) {
                    child = new_child;
                }
            }

            n = child;
        }

        // leaf node
        leaf_node *ln = static_cast<leaf_node *>(n);
        if (ln->right == NULL) {
            short count = ln->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, ln->key[i])) {
                    break;
                }
            }
        }
        else {
            for (i = 0; key_greater(key, ln->key[i]); i++);
        }

        // add the new key and data
        leaf_shift_right(ln, i);
        m_size++;
        ln->key[i] = key;
        ln->data[i] = data;

        // check if m_head needs to be split
        if (m_head->count == ((m_head->is_leaf) ? l_order : i_order)) {
            node *child = m_head;
            node *new_child = split_node(child);

            m_head = allocate_inner();
            inner_node *head = static_cast<inner_node *>(m_head);
            m_level++;
            head->count = 2;

            if (child->is_leaf) {
                head->key[0] = (static_cast<leaf_node *>(child))->key[l_half_order - 1];
                head->key[1] = (static_cast<leaf_node *>(new_child))->key[l_order - l_half_order - 1];
            }
            else {
                head->key[0] = (static_cast<inner_node *>(child))->key[i_half_order - 1];
                head->key[1] = (static_cast<inner_node *>(new_child))->key[i_order - i_half_order - 1];
            }
            head->down[0] = child;
            head->down[1] = new_child;
        }

        return std::pair<iterator, bool>(iterator(ln, i), true);
    }

    bool is_valid_iterator(iterator iter)
    {
        if (NULL == iter.currnode ||
            (iter.currnode)->is_leaf != 1 ||
            iter == end() ||
            iter.currindex < 0 ||
            iter.currindex >= (iter.currnode)->count) {
            return false;
        }
        return true;
    }

    bool is_valid_reverse_iterator(reverse_iterator iter)
    {
        if (NULL == iter.currnode ||
            (iter.currnode)->is_leaf != 1 ||
            iter == rend() ||
            iter.currindex <= 0 ||
            iter.currindex > (iter.currnode)->count) {
            return false;
        }
        return true;
    }

public:
    // *** Public Erase Functions

    bool erase_one(const key_type& key)
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

        node *n = m_head;
        short i;

        while (!n->is_leaf) {
            inner_node *in = static_cast<inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }

            node *child = in->down[i];
            if (child->count < ((child->is_leaf) ? l_half_order : i_half_order)) {
                if (i == 0) {
                    node *rchild = in->down[i+1];
                    if (rchild->count <= ((child->is_leaf) ? l_half_order : i_half_order)) {
                        inner_shift_left(in, i);
                        concat_node(child, rchild);
                        in->down[i] = child;
                    }
                    else {
                        redistribute_right_left(child, rchild);
                        if (child->is_leaf) {
                            in->key[i] = (static_cast<leaf_node *>(child))->key[child->count - 1];
                        }
                        else {
                            in->key[i] = (static_cast<inner_node *>(child))->key[child->count - 1];
                        }
                    }
                }
                else {
                    node *lchild = in->down[i-1];
                    if (lchild->count <= ((child->is_leaf) ? l_half_order : i_half_order)) {
                        inner_shift_left(in, i-1);
                        concat_node(lchild, child);
                        in->down[i-1] = lchild;
                        child = lchild;
                    }
                    else {
                        redistribute_left_right(lchild, child);
                        if (lchild->is_leaf) {
                            in->key[i-1] = (static_cast<leaf_node *>(lchild))->key[lchild->count - 1];
                        }
                        else {
                            in->key[i-1] = (static_cast<inner_node *>(lchild))->key[lchild->count - 1];
                        }
                    }
                }
            }

            n = child;
        }

        leaf_node *ln = static_cast<leaf_node *>(n);
        if (ln->right == NULL) {
            short count = ln->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, ln->key[i])) {
                    break;
                }
            }
            if (i == count || !key_equal(key, ln->key[i])) {
                return false;
            }
        }
        else {
            for (i = 0; key_greater(key, ln->key[i]); i++);

            if (!key_equal(key, ln->key[i])) {
                return false;
            }
        }

        // found key
        leaf_shift_left(ln, i);
        m_size--;

        if (m_head->count == 1 && m_head->is_leaf == 0) {
            inner_node *ihead = static_cast<inner_node *>(m_head);
            m_head = ihead->down[0];
            free_node(ihead);
            m_level--;
        }

        return true;
    }

    size_type erase(const key_type& key)
    {
        size_type c = 0;

        while (erase_one(key)) {
            c++;
        }

        return c;
    }

    void erase(iterator iter)
    {
        if (is_valid_iterator(iter)) {
            // TODO need a erase_iter method
        }
    }

    void erase(reverse_iterator iter)
    {
        if (is_valid_reverse_iterator(iter)) {
            // TODO need a erase_iter method
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
        os << std::endl << std::endl;
    }
#endif
};

}

#endif

#ifndef SKIPLIST_MAP_RO_H_HEADER
#define SKIPLIST_MAP_RO_H_HEADER

#include <algorithm>
#include <functional>
#include <istream>
#include <ostream>
#include <memory>
#include <cstddef>
#include <cassert>
#include <vector>

#ifdef SL_DEBUG

#include <iostream>
#include <iomanip>

#ifndef SL_PRINT
/// Print out debug information to std::cout if SL_DEBUG is defined.
#define SL_PRINT(x)          do { (std::cout << x << std::endl); } while (0)
#endif

#ifndef SL_ASSERT
/// Assertion only if BTREE_DEBUG is defined. This is not used in verify().
#define SL_ASSERT(x)         do { assert(x); } while (0)
#endif

#else

#ifndef SL_PRINT
/// Print out debug information to std::cout if SL_DEBUG is defined.
#define SL_PRINT(x)          do { } while (0)
#endif

#ifndef SL_ASSERT
/// Assertion only if BTREE_DEBUG is defined. This is not used in verify().
#define SL_ASSERT(x)         do { } while (0)
#endif

#endif

#define SL_MAX(a, b)         ((a) < (b) ? (b) : (a))

namespace cmu {

template <typename _Key, typename _Data>
class skiplist_ro_default_map_traits
{
public:
    static const int pagesize = 512;
    static const short order = SL_MAX(8, pagesize / (sizeof(_Key) + sizeof(void *)));
    static const short leaf_order = SL_MAX(8, pagesize / (sizeof(_Key) + sizeof(_Data)));
};

// forward declaration of friend class
template <typename _Key, typename _Data,
          typename _Compare,
          typename _Traits,
          bool _Duplicates,
          typename _Alloc>
class skiplist_map_compact;

template <typename _Key, typename _Data,
          typename _Compare,
          typename _Traits,
          bool _Duplicates,
          typename _Alloc>
class skiplist_map;

template <typename _Key, typename _Data,
          typename _Compare = std::less<_Key>,
          typename _Traits = skiplist_ro_default_map_traits<_Key, _Data>,
          bool _Duplicates = false,
          typename _Alloc = std::allocator<std::pair<_Key, _Data>>>
class skiplist_map_ro
{
#define SL_FRIENDS friend class skiplist_map_compact<_Key, _Data, _Compare, _Traits, _Duplicates, _Alloc>;
public:
    typedef _Key key_type;
    typedef _Data data_type;
    typedef _Compare key_compare;
    typedef _Traits traits;
    static const bool allow_duplicates = _Duplicates;
    typedef _Alloc allocator_type;
    friend class skiplist_map<key_type, data_type, key_compare, traits,
                              allow_duplicates, allocator_type>;
    SL_FRIENDS

public:
    typedef skiplist_map_ro<key_type, data_type, key_compare, traits,
                            allow_duplicates, allocator_type> self_type;
    typedef skiplist_map<key_type, data_type, key_compare, traits,
                         allow_duplicates, allocator_type> writable_type;
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

        key_type  key[l_order];
        data_type data[l_order];
    };

    typedef typename _Alloc::template rebind<leaf_node *>::other leaf_pointer_alloc_type;

public:
    class iterator;
    class const_iterator;
    class reverse_iterator;
    class const_reverse_iterator;

    class iterator {
    public:
        typedef typename skiplist_map_ro::key_type key_type;
        typedef typename skiplist_map_ro::data_type data_type;
        typedef typename skiplist_map_ro::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        typename skiplist_map_ro::leaf_node **nodearray;
        int currnodeindex;
        int maxnodeindex;
        short currindex;

        friend class reverse_iterator;
        friend class const_iterator;
        friend class const_reverse_iterator;
        friend class skiplist_map_ro<key_type, data_type, key_compare, traits,
                                     allow_duplicates, allocator_type>;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline iterator()
            : nodearray(NULL), currnodeindex(0), maxnodeindex(0), currindex(0)
        { }

        inline iterator(typename skiplist_map_ro::leaf_node **nodearray,
                        int nodeindex, int maxnodeindex, short index)
            : nodearray(nodearray), currnodeindex(nodeindex), maxnodeindex(maxnodeindex), currindex(index)
        { }

        inline iterator(const reverse_iterator& it)
            : nodearray(it.nodearray), currnodeindex(it.currnodeindex), maxnodeindex(it.maxnodeindex), currindex(it.currindex)
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
            return nodearray[currnodeindex]->key[currindex];
        }

        inline data_type & data() const
        {
            return nodearray[currnodeindex]->data[currindex];
        }

        inline iterator& operator ++ ()
        {
            if (currnodeindex < maxnodeindex) {
                ++currindex;
                if (currindex == nodearray[currnodeindex]->count) {
                    ++currnodeindex;
                    currindex = 0;
                }
            }
            else if (currindex < nodearray[currnodeindex]->count - 1) {
                ++currindex;
            }
            return *this;
        }

        inline iterator operator ++ (int)
        {
            iterator tmp = *this;

            if (currnodeindex < maxnodeindex) {
                ++currindex;
                if (currindex == nodearray[currnodeindex]->count) {
                    ++currnodeindex;
                    currindex = 0;
                }
            }
            else if (currindex < nodearray[currnodeindex]->count - 1) {
                ++currindex;
            }
            return tmp;
        }

        inline iterator& operator -- ()
        {
            if (currindex > 0) {
                --currindex;
            }
            else if (currnodeindex > 0) {
                --currnodeindex;
                currindex = nodearray[currnodeindex]->count - 1;
            }
            return *this;
        }

        inline iterator operator -- (int)
        {
            iterator tmp = *this;

            if (currindex > 0) {
                --currindex;
            }
            else if (currnodeindex > 0) {
                --currnodeindex;
                currindex = nodearray[currnodeindex]->count - 1;
            }
            return tmp;
        }

        inline bool operator == (const iterator &x) const
        {
            return (x.nodearray == nodearray && x.maxnodeindex == maxnodeindex &&
                    x.currnodeindex == currnodeindex && x.currindex == currindex);
        }

        inline bool operator != (const iterator &x) const
        {
            return (x.nodearray != nodearray || x.maxnodeindex != maxnodeindex ||
                    x.currnodeindex != currnodeindex || x.currindex != currindex);
        }
    };

    class const_iterator {
    public:
        typedef typename skiplist_map_ro::key_type key_type;
        typedef typename skiplist_map_ro::data_type data_type;
        typedef typename skiplist_map_ro::value_type value_type;
        typedef const value_type& reference;
        typedef const value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        const typename skiplist_map_ro::leaf_node **nodearray;
        int currnodeindex;
        int maxnodeindex;
        short currindex;

        friend class const_reverse_iterator;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline const_iterator()
            : nodearray(NULL), currnodeindex(0), maxnodeindex(0), currindex(0)
        { }

        inline const_iterator(typename skiplist_map_ro::leaf_node **nodearray,
                              int nodeindex, int maxnodeindex, short index)
            : nodearray(nodearray), currnodeindex(nodeindex), maxnodeindex(maxnodeindex), currindex(index)
        { }

        inline const_iterator(const iterator& it)
            : nodearray(it.nodearray), currnodeindex(it.currnodeindex), maxnodeindex(it.maxnodeindex), currindex(it.currindex)
        { }

        inline const_iterator(const reverse_iterator& it)
            : nodearray(it.nodearray), currnodeindex(it.currnodeindex), maxnodeindex(it.maxnodeindex), currindex(it.currindex)
        { }

        inline const_iterator(const const_reverse_iterator& it)
            : nodearray(it.nodearray), currnodeindex(it.currnodeindex), maxnodeindex(it.maxnodeindex), currindex(it.currindex)
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
            return nodearray[currnodeindex]->key[currindex];
        }

        inline const data_type & data() const
        {
            return nodearray[currnodeindex]->data[currindex];
        }

        inline const_iterator& operator ++ ()
        {
            if (currnodeindex < maxnodeindex) {
                ++currindex;
                if (currindex == nodearray[currnodeindex]->count) {
                    ++currnodeindex;
                    currindex = 0;
                }
            }
            else if (currindex < nodearray[currnodeindex]->count - 1) {
                ++currindex;
            }
            return *this;
        }

        inline const_iterator operator ++ (int)
        {
            const_iterator tmp = *this;

            if (currnodeindex < maxnodeindex) {
                ++currindex;
                if (currindex == nodearray[currnodeindex]->count) {
                    ++currnodeindex;
                    currindex = 0;
                }
            }
            else if (currindex < nodearray[currnodeindex]->count - 1) {
                ++currindex;
            }
            return tmp;
        }

        inline const_iterator& operator -- ()
        {
            if (currindex > 0) {
                --currindex;
            }
            else if (currnodeindex > 0) {
                --currnodeindex;
                currindex = nodearray[currnodeindex]->count - 1;
            }

            return *this;
        }

        inline const_iterator operator -- (int)
        {
            const_iterator tmp = *this;

            if (currindex > 0) {
                --currindex;
            }
            else if (currnodeindex > 0) {
                --currnodeindex;
                currindex = nodearray[currnodeindex]->count - 1;
            }

            return tmp;
        }

        inline bool operator == (const const_iterator &x) const
        {
            return (x.nodearray == nodearray && x.maxnodeindex == maxnodeindex &&
                    x.currnodeindex == currnodeindex && x.currindex == currindex);
        }

        inline bool operator != (const const_iterator &x) const
        {
            return (x.nodearray != nodearray || x.maxnodeindex != maxnodeindex ||
                    x.currnodeindex != currnodeindex || x.currindex != currindex);
        }
    };

    class reverse_iterator {
    public:
        typedef typename skiplist_map_ro::key_type key_type;
        typedef typename skiplist_map_ro::data_type data_type;
        typedef typename skiplist_map_ro::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        typename skiplist_map_ro::leaf_node **nodearray;
        int currnodeindex;
        int maxnodeindex;
        short currindex;

        friend class iterator;
        friend class const_iterator;
        friend class const_reverse_iterator;
        friend class skiplist_map_ro<key_type, data_type, key_compare, traits,
                                     allow_duplicates, allocator_type>;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline reverse_iterator()
            : nodearray(NULL), currnodeindex(0), maxnodeindex(0), currindex(0)
        { }

        inline reverse_iterator(typename skiplist_map_ro::leaf_node **nodearray,
                                int nodeindex, int maxnodeindex, short index)
            : nodearray(nodearray), currnodeindex(nodeindex), maxnodeindex(maxnodeindex), currindex(index)
        { }

        inline reverse_iterator(const iterator& it)
            : nodearray(it.nodearray), currnodeindex(it.currnodeindex), maxnodeindex(it.maxnodeindex), currindex(it.currindex)
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
            return nodearray[currnodeindex]->key[currindex - 1];
        }

        inline data_type & data() const
        {
            SL_ASSERT(currindex > 0);
            return nodearray[currnodeindex]->data[currindex - 1];
        }

        inline reverse_iterator& operator ++ ()
        {
            if (currindex > 1) {
                --currindex;
            }
            else if (currnodeindex > 0) {
                --currnodeindex;
                currindex = nodearray[currnodeindex]->count;
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
            else if (currnodeindex > 0) {
                --currnodeindex;
                currindex = nodearray[currnodeindex]->count;
            }
            else {
                currindex = 0;
            }
            return tmp;
        }

        inline reverse_iterator& operator -- ()
        {
            if (currnodeindex < maxnodeindex) {
                ++currindex;
                if (currindex == nodearray[currnodeindex]->count + 1) {
                    ++currnodeindex;
                    currindex = 1;
                }
            }
            else if (currindex < nodearray[currnodeindex]->count) {
                ++currindex;
            }
            return *this;
        }

        inline reverse_iterator operator -- (int)
        {
            reverse_iterator tmp = *this;

            if (currnodeindex < maxnodeindex) {
                ++currindex;
                if (currindex == nodearray[currnodeindex]->count + 1) {
                    ++currnodeindex;
                    currindex = 1;
                }
            }
            else if (currindex < nodearray[currnodeindex]->count) {
                ++currindex;
            }
            return tmp;
        }

        inline bool operator == (const reverse_iterator &x) const
        {
            return (x.nodearray == nodearray && x.maxnodeindex == maxnodeindex &&
                    x.currnodeindex == currnodeindex && x.currindex == currindex);
        }

        inline bool operator != (const reverse_iterator &x) const
        {
            return (x.nodearray != nodearray || x.maxnodeindex != maxnodeindex ||
                    x.currnodeindex != currnodeindex || x.currindex != currindex);
        }
    };

    class const_reverse_iterator {
    public:
        typedef typename skiplist_map_ro::key_type key_type;
        typedef typename skiplist_map_ro::data_type data_type;
        typedef typename skiplist_map_ro::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        const typename skiplist_map_ro::leaf_node **nodearray;
        int currnodeindex;
        int maxnodeindex;
        short currindex;

        friend class const_iterator;

        mutable value_type temp_value;
        SL_FRIENDS

    public:
        inline const_reverse_iterator()
            : nodearray(NULL), currnodeindex(0), maxnodeindex(0), currindex(0)
        { }

        inline const_reverse_iterator(typename skiplist_map_ro::leaf_node **nodearray,
                                      int nodeindex, int maxnodeindex, short index)
            : nodearray(nodearray), currnodeindex(nodeindex), maxnodeindex(maxnodeindex), currindex(index)
        { }

        inline const_reverse_iterator(const iterator& it)
            : nodearray(it.nodearray), currnodeindex(it.currnodeindex), maxnodeindex(it.maxnodeindex), currindex(it.currindex)
        { }

        inline const_reverse_iterator(const const_iterator& it)
            : nodearray(it.nodearray), currnodeindex(it.currnodeindex), maxnodeindex(it.maxnodeindex), currindex(it.currindex)
        { }

        inline const_reverse_iterator(const reverse_iterator& it)
            : nodearray(it.nodearray), currnodeindex(it.currnodeindex), maxnodeindex(it.maxnodeindex), currindex(it.currindex)
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
            return nodearray[currnodeindex]->key[currindex - 1];
        }

        inline const data_type & data() const
        {
            SL_ASSERT(currindex > 0);
            return nodearray[currnodeindex]->data[currindex - 1];
        }

        inline const_reverse_iterator& operator ++ ()
        {
            if (currindex > 1) {
                --currindex;
            }
            else if (currnodeindex > 0) {
                --currnodeindex;
                currindex = nodearray[currnodeindex]->count;
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
            else if (currnodeindex > 0) {
                --currnodeindex;
                currindex = nodearray[currnodeindex]->count;
            }
            else {
                currindex = 0;
            }
            return tmp;
        }

        inline const_reverse_iterator& operator -- ()
        {
            if (currnodeindex < maxnodeindex) {
                ++currindex;
                if (currindex == nodearray[currnodeindex]->count + 1) {
                    ++currnodeindex;
                    currindex = 1;
                }
            }
            else if (currindex < nodearray[currnodeindex]->count) {
                ++currindex;
            }
            return *this;
        }

        inline const_reverse_iterator operator -- (int)
        {
            const_reverse_iterator tmp = *this;

            if (currnodeindex < maxnodeindex) {
                ++currindex;
                if (currindex == nodearray[currnodeindex]->count + 1) {
                    ++currnodeindex;
                    currindex = 1;
                }
            }
            else if (currindex < nodearray[currnodeindex]->count) {
                ++currindex;
            }
            return tmp;
        }

        inline bool operator == (const const_reverse_iterator &x) const
        {
            return (x.nodearray == nodearray && x.maxnodeindex == maxnodeindex &&
                    x.currnodeindex == currnodeindex && x.currindex == currindex);
        }

        inline bool operator != (const const_reverse_iterator &x) const
        {
            return (x.nodearray != nodearray || x.maxnodeindex != maxnodeindex ||
                    x.currnodeindex != currnodeindex || x.currindex != currindex);
        }
    };

private:
    node *m_head;

    leaf_node **m_leaf_array;

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

    typename leaf_pointer_alloc_type m_leaf_pointer_allocator;

public:
    explicit inline skiplist_map_ro(const allocator_type& alloc = allocator_type())
    {
        m_allocator = alloc;
        m_inner_allocator = m_allocator;
        m_leaf_allocator = m_allocator;
        m_leaf_pointer_allocator = m_allocator;
        m_size = m_level = 0;
        m_inner_count = m_leaf_count = 0;

        m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
        leaf_node *l_head = static_cast<leaf_node *>(m_head);
        l_head->count = 1; // placeholder for virtual max key
        l_head->key[0] = key_type(); // placeholder for virtual max key
        l_head->data[0] = data_type();

        m_leaf_array = allocate_leaf_array(1);
        m_leaf_array[0] = m_head_leaf;
    }

    explicit inline skiplist_map_ro(const key_compare& kcf,
                                    const allocator_type& alloc = allocator_type())
    {
        m_key_less = kcf;
        m_allocator = alloc;
        m_inner_allocator = m_allocator;
        m_leaf_allocator = m_allocator;
        m_leaf_pointer_allocator = m_allocator;
        m_size = m_level = 0;
        m_inner_count = m_leaf_count = 0;

        m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
        leaf_node *l_head = static_cast<leaf_node *>(m_head);
        l_head->count = 1; // placeholder for virtual max key
        l_head->key[0] = key_type(); // placeholder for virtual max key
        l_head->data[0] = data_type();

        m_leaf_array = allocate_leaf_array(1);
        m_leaf_array[0] = m_head_leaf;
    }

    inline ~skiplist_map_ro()
    {
        clear_all();
    }

    void swap(self_type &from)
    {
        std::swap(m_head, from.m_head);
        std::swap(m_leaf_array, from.m_leaf_array);
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
        friend class skiplist_map_ro<key_type, data_type, key_compare, traits,
                                     allow_duplicates, allocator_type>;

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

    inline leaf_node **allocate_leaf_array(size_type array_len)
    {
        leaf_node **array = new (m_leaf_pointer_allocator.allocate(array_len)) (leaf_node *);
        return array;
    }

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
        if (1 == n->is_leaf) {
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

    inline void free_leaf_array(leaf_node **array, size_type array_len)
    {
        m_leaf_pointer_allocator.destroy(array);
        m_leaf_pointer_allocator.deallocate(array, array_len);
    }

    // free all allocated memory, only used in destructor
    void clear_all()
    {
        if (m_head == NULL) {
            return;
        }
        node *head = m_head, *next;
        while (1 != head->is_leaf) {
            inner_node *in = static_cast<inner_node*>(head);
            next = in->down[0];

            while (in != NULL) {
                inner_node *tmp = in->right;
                free_node(in);
                in = tmp;
            }

            if (-1 == head->is_leaf) {
                break;
            }
            head = next;
        }

        size_type leaf_count = m_leaf_count;
        for (size_type i = 0; i < m_leaf_count; i++) {
            free_node(m_leaf_array[i]);
        }
        free_leaf_array(m_leaf_array, leaf_count);

        m_leaf_array = NULL;
        m_head = m_head_leaf = m_tail_leaf = NULL;
        m_size = m_level = 0;
        m_inner_count = m_leaf_count = 0;
    }

    // helper function for merging, m_head will point to head leaf after
    void clear_inner()
    {
        if (m_head == NULL) {
            return;
        }
        node *head = m_head, *next;
        while (1 != head->is_leaf) {
            inner_node *in = static_cast<inner_node*>(head);
            next = in->down[0];

            while (in != NULL) {
                inner_node *tmp = in->right;
                free_node(in);
                in = tmp;
            }

            if (-1 == head->is_leaf) {
                break;
            }
            head = next;
        }

        m_head = m_head_leaf;
        m_level = 0;
        m_inner_count = 0;
    }

public:
    // clear all nodes except the starting leaf node for empty skip list
    void clear()
    {
        clear_all();

        m_head = m_head_leaf = m_tail_leaf = allocate_leaf();
        leaf_node *l_head = static_cast<leaf_node *>(m_head);
        l_head->count = 1; // placeholder for virtual max key
        l_head->key[0] = key_type(); // placeholder for virtual max key
        l_head->data[0] = data_type();

        m_leaf_array = allocate_leaf_array(1);
        m_leaf_array[0] = m_head_leaf;
    }

public:
    inline iterator begin()
    {
        return iterator(m_leaf_array, 0, m_leaf_count - 1, 0);
    }

    inline iterator end()
    {
        return iterator(m_leaf_array, m_leaf_count - 1, m_leaf_count - 1, m_tail_leaf->count - 1);
    }

    inline const_iterator begin() const
    {
        return const_iterator(m_leaf_array, 0, m_leaf_count - 1, 0);
    }

    inline const_iterator end() const
    {
        return const_iterator(m_leaf_array, m_leaf_count - 1, m_leaf_count - 1, m_tail_leaf->count - 1);
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
        else if (m_leaf_count > 1) {
            leaf_node *prev_leaf = m_leaf_array[m_leaf_count - 2];
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return false;
            }
        }

        const node *n = m_head;
        short i;
        int leaf_index;

        while (1 != n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }

            if (-1 == in->is_leaf) {
                leaf_index = static_cast<int>(in->down[i]);
                break;
            }
            n = in->down[i];
        }


        const leaf_node *ln = m_leaf_array[leaf_index];
        for (i = 0; key_greater(key, ln->key[i]); i++);

        return key_equal(key, ln->key[i]);
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
        else if (m_leaf_count > 1) {
            leaf_node *prev_leaf = m_leaf_array[m_leaf_count - 2];
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        node *n = m_head;
        short i;
        int leaf_index;

        while (1 != n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }

            if (-1 == in->is_leaf) {
                leaf_index = static_cast<int>(in->down[i]);
                break;
            }
            n = in->down[i];
        }

        leaf_node *ln = m_leaf_array[leaf_index];
        for (i = 0; key_greater(key, ln->key[i]); i++);

        return key_equal(key, ln->key[i]) ?
               iterator(m_leaf_array, leaf_index, m_leaf_count - 1, i) : end();
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
        else if (m_leaf_count > 1) {
            leaf_node *prev_leaf = m_leaf_array[m_leaf_count - 2];
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        const node *n = m_head;
        short i;
        int leaf_index;

        while (1 != n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }

            if (-1 == in->is_leaf) {
                leaf_index = static_cast<int>(in->down[i]);
                break;
            }
            n = in->down[i];
        }

        const leaf_node *ln = m_leaf_array[leaf_index];
        for (i = 0; key_greater(key, ln->key[i]); i++);

        return key_equal(key, ln->key[i]) ?
               const_iterator(m_leaf_array, leaf_index, m_leaf_count - 1, i) : end();
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
        else if (m_leaf_count > 1) {
            leaf_node *prev_leaf = m_leaf_array[m_leaf_count - 2];
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return 0;
            }
        }

        const node *n = m_head;
        short i;
        int leaf_index;

        while (1 != n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }

            if (-1 == in->is_leaf) {
                leaf_index = static_cast<int>(in->down[i]);
                break;
            }
            n = in->down[i];
        }

        const leaf_node *ln = m_leaf_array[leaf_index];
        for (i = 0; key_greater(key, ln->key[i]); i++);

        return key_equal(key, ln->key[i]) ? 1 : 0;
    }

    iterator lower_bound(const key_type& key)
    {
        if (m_tail_leaf->count > 1) {
            if (key_greater(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_leaf_count > 1) {
            leaf_node *prev_leaf = m_leaf_array[m_leaf_count - 2];
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        node *n = m_head;
        short i;
        int leaf_index;

        while (1 != n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }

            if (-1 == in->is_leaf) {
                leaf_index = static_cast<int>(in->down[i]);
                break;
            }
            n = in->down[i];
        }

        leaf_node *ln = m_leaf_array[leaf_index];
        for (i = 0; key_greater(key, ln->key[i]); i++);

        return iterator(m_leaf_array, leaf_index, m_leaf_count - 1, i);
    }

    const_iterator lower_bound(const key_type& key) const
    {
        if (m_tail_leaf->count > 1) {
            if (key_greater(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_leaf_count > 1) {
            leaf_node *prev_leaf = m_leaf_array[m_leaf_count - 2];
            if (key_greater(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        const node *n = m_head;
        short i;
        int leaf_index;

        while (1 != n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_lessequal(key, in->key[i])) {
                    break;
                }
            }

            if (-1 == in->is_leaf) {
                leaf_index = static_cast<int>(in->down[i]);
                break;
            }
            n = in->down[i];
        }

        const leaf_node *ln = m_leaf_array[leaf_index];
        for (i = 0; key_greater(key, ln->key[i]); i++);

        return const_iterator(m_leaf_array, leaf_index, m_leaf_count - 1, i);
    }

    iterator upper_bound(const key_type& key)
    {
        if (m_tail_leaf->count > 1) {
            if (key_greaterequal(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_leaf_count > 1) {
            leaf_node *prev_leaf = m_leaf_array[m_leaf_count - 2];
            if (key_greaterequal(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        node *n = m_head;
        short i;
        int leaf_index;

        while (1 != n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_less(key, in->key[i])) {
                    break;
                }
            }

            if (-1 == in->is_leaf) {
                leaf_index = static_cast<int>(in->down[i]);
                break;
            }
            n = in->down[i];
        }

        leaf_node *ln = m_leaf_array[leaf_index];
        for (i = 0; key_greaterequal(key, ln->key[i]); i++);

        return iterator(m_leaf_array, leaf_index, m_leaf_count - 1, i);
    }

    const_iterator upper_bound(const key_type& key) const
    {
        if (m_tail_leaf->count > 1) {
            if (key_greaterequal(key, m_tail_leaf->key[m_tail_leaf->count - 2])) {
                return end();
            }
        }
        else if (m_leaf_count > 1) {
            leaf_node *prev_leaf = m_leaf_array[m_leaf_count - 2];
            if (key_greaterequal(key, prev_leaf->key[prev_leaf->count - 1])) {
                return end();
            }
        }

        const node *n = m_head;
        short i;
        int leaf_index;

        while (1 != n->is_leaf) {
            const inner_node *in = static_cast<const inner_node *>(n);
            short count = in->count - 1;
            for (i = 0; i < count; i++) {
                if (key_less(key, in->key[i])) {
                    break;
                }
            }

            if (-1 == in->is_leaf) {
                leaf_index = static_cast<int>(in->down[i]);
                break;
            }
            n = in->down[i];
        }

        const leaf_node *ln = m_leaf_array[leaf_index];
        for (i = 0; key_greaterequal(key, ln->key[i]); i++);

        return const_iterator(m_leaf_array, leaf_index, m_leaf_count - 1, i);
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

    inline skiplist_map_ro(const skiplist_map_ro& other) {

    }
    */

private:
    bool is_valid_iterator(iterator iter)
    {
        leaf_node *currnode = iter.nodearray[iter.currnodeindex];
        if (NULL == currnode ||
            currnode->is_leaf != 1 ||
            iter == end() ||
            iter.currindex < 0 ||
            iter.currindex >= currnode->count) {
            return false;
        }
        return true;
    }

    bool is_valid_reverse_iterator(reverse_iterator iter)
    {
        leaf_node *currnode = iter.nodearray[iter.currnodeindex];
        if (NULL == currnode ||
            currnode->is_leaf != 1 ||
            iter == rend() ||
            iter.currindex <= 0 ||
            iter.currindex > currnode->count) {
            return false;
        }
        return true;
    }

// TODO private:
public:
    // for static stage skiplist, it is the only way to rebuild it
    // merge a writable skip list and rebuild a compact skip list
    void merge(writable_type& from)
    {
        if (from.m_size == 0) {
            return;
        }

        clear_inner();
        from.clear_inner();

        typename writable_type::leaf_node *dyna_ln = from.m_head_leaf;
        leaf_node *static_ln;
        size_t node_count = 1;
        size_type new_leaf_count;
        size_type old_leaf_count = m_leaf_count;

        // rebuild leaf nodes
        // case 1: self is empty
        // TODO reuse head leaf
        if (m_size == 0) {
            free_leaf_array(m_leaf_array, old_leaf_count);
            new_leaf_count = from.m_leaf_count;
            leaf_node **new_leaf_array = allocate_leaf_array(new_leaf_count);
            m_leaf_count = 0;

            short count = 0;
            int new_leaf_index = 0;
            static_ln = allocate_leaf();
            static_ln->count = 0;
            new_leaf_array[new_leaf_index++] = static_ln;
            while (dyna_ln != NULL) {
                for (short i = 0; i < dyna_ln->count; i++) {
                    if (count == l_order) {
                        leaf_node *new_static_ln = allocate_leaf();
                        new_leaf_array[new_leaf_index++] = new_static_ln;
                        static_ln->count = count;
                        static_ln = new_static_ln;
                        count = 0;
                        node_count++;
                    }
                    static_ln->key[count] = dyna_ln->key[i];
                    static_ln->data[count] = dyna_ln->data[i];
                    count++;
                    m_size++;
                }
                static_ln->count = count;

                typename writable_type::leaf_node *next_dyna_ln = dyna_ln->right;
                if (next_dyna_ln != NULL) {
                    from.free_node(dyna_ln);
                }
                dyna_ln = next_dyna_ln;
            }

            m_leaf_array = new_leaf_array;
            m_size--; // do not count virtual max key
            m_head = m_head_leaf = new_leaf_array[0];
            m_tail_leaf = static_ln;
            from.m_head = from.m_head_leaf = from.m_tail_leaf;
            from.m_tail_leaf->count = 1;
            from.m_tail_leaf->left = NULL;
            from.m_tail_leaf->right = NULL;
            from.m_size = 0;
        }
        else {
            // TODO use vector and copy content to new_leaf_array at last
            std::vector<leaf_node *> new_leaf_vector;
            short dyna_index = 0;
            short static_index = 0;
            short new_index = 0;
            int new_leaf_index = 0;
            int old_leaf_index = 0;
            m_size = 0;

            leaf_node *new_static_ln = allocate_leaf();
            new_leaf_vector.push_back(new_static_ln);
            static_ln = m_leaf_array[old_leaf_index];

            while (dyna_ln != NULL && old_leaf_index < old_leaf_count) {
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
                        new_leaf_vector.push_back(next_new_static_ln);
                        new_static_ln->count = new_index;
                        new_static_ln = next_new_static_ln;
                        new_index = 0;
                        node_count++;
                    }
                    if (static_ln->data[static_index] == (data_type)0 ||
                        key_equal(dyna_ln->key[dyna_index], static_ln->key[static_index]))
                    {
                        static_index++;
                    }
                    else if (key_less(dyna_ln->key[dyna_index], static_ln->key[static_index])) {
                        new_static_ln->key[new_index] = dyna_ln->key[dyna_index];
                        new_static_ln->data[new_index] = dyna_ln->data[dyna_index];
                        new_index++;
                        dyna_index++;
                        m_size++;
                    }
                    else {
                        new_static_ln->key[new_index] = static_ln->key[static_index];
                        new_static_ln->data[new_index] = static_ln->data[static_index];
                        new_index++;
                        static_index++;
                        m_size++;
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
                    free_node(static_ln);
                    old_leaf_index++;
                    if (old_leaf_index < old_leaf_count) {
                        static_ln = m_leaf_array[old_leaf_index];
                    }
                    static_index = 0;
                }
            }

            while (dyna_ln != NULL) {
                short dyna_count = dyna_ln->count;
                while (dyna_index < dyna_count) {
                    if (new_index == l_order) {
                        leaf_node *next_new_static_ln = allocate_leaf();
                        new_leaf_vector.push_back(next_new_static_ln);
                        new_static_ln->count = new_index;
                        new_static_ln = next_new_static_ln;
                        new_index = 0;
                        node_count++;
                    }
                    new_static_ln->key[new_index] = dyna_ln->key[dyna_index];
                    new_static_ln->data[new_index] = dyna_ln->data[dyna_index];
                    new_index++;
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

            while (old_leaf_index < old_leaf_count) {
                short static_count = static_ln->count;
                while (static_index < static_count) {
                    if (new_index == l_order) {
                        leaf_node *next_new_static_ln = allocate_leaf();
                        new_leaf_vector.push_back(next_new_static_ln);
                        new_static_ln->count = new_index;
                        new_static_ln = next_new_static_ln;
                        new_index = 0;
                        node_count++;
                    }
                    if (static_ln->data[static_index] == (data_type)0 &&
                        ((static_index != static_count - 1) || (old_leaf_index != old_leaf_count - 1)))
                    {
                        static_index++;
                    }
                    else {
                        new_static_ln->key[new_index] = static_ln->key[static_index];
                        new_static_ln->data[new_index] = static_ln->data[static_index];
                        new_index++;
                        static_index++;
                        m_size++;
                    }
                }
                new_static_ln->count = new_index;

                free_node(static_ln);
                old_leaf_index++;
                if (old_leaf_index < old_leaf_count) {
                    static_ln = m_leaf_array[old_leaf_index];
                }
                static_index = 0;
            }

            free_leaf_array(m_leaf_array, old_leaf_count);
            m_leaf_array = new_leaf_array;
            m_size--;
            m_head = m_head_leaf = new_leaf_array[0];
            m_tail_leaf = new_static_ln;
            m_leaf_count = new_leaf_count;
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

        while (1 != level_head->is_leaf) {
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

#ifndef SKIPLIST_MAP_COMPACT_H_HEADER
#define SKIPLIST_MAP_COMPACT_H_HEADER

#include <algorithm>
#include <functional>
#include <istream>
#include <ostream>
#include <memory>
#include <cstddef>
#include <cassert>
#include "bloomfilter.h"
#include "skiplist_map.h"

#define SL_MERGE 1
#define SL_MERGE_THRESHOLD 100
#define SL_MERGE_RATIO 10

#define USE_BLOOM_FILTER 1
#define LITTLEENDIAN 1
#define BITS_PER_KEY 8
#define K 2

namespace cmu {

template <typename _Key, typename _Data,
          typename _Compare = std::less<_Key>,
          typename _Traits = skiplist_default_map_traits<_Key, _Data>,
          typename _Alloc = std::allocator<std::pair<_Key, _Data> > >
class skiplist_map_compact
{
public:
    typedef _Key key_type;
    typedef _Data data_type;
    typedef _Compare key_compare;
    typedef _Traits traits;
    typedef _Alloc allocator_type;
    typedef std::pair<key_type, data_type> value_type;
    typedef std::pair<key_type, data_type> pair_type;
    typedef size_t size_type;
    typedef skiplist_map_compact<key_type, data_type, key_compare, traits,
                                 allocator_type> self_type;

private:
    typedef skiplist_map<key_type, data_type, key_compare, traits,
                         allocator_type> sl_type;

public:
    class iterator;
    class const_iterator;
    class reverse_iterator;
    class const_reverse_iterator;

    class iterator {
    public:
        typedef typename sl_type::key_type key_type;
        typedef typename sl_type::data_type data_type;
        typedef typename sl_type::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        bool in_dyna;
        typename sl_type::iterator d_iter;
        typename sl_type::iterator s_iter;
        key_compare key_less;

        friend class const_iterator;
        friend class skiplist_map_compact<key_type, data_type, key_compare, traits,
                                          allocator_type>;

        mutable value_type temp_value;

        void move_forward()
        {
            //SL_PRINT((in_dyna ? "d" : "s") << " " << d_iter.key() << " " << s_iter.key());
            if (in_dyna) {
                ++d_iter;
            }
            else {
                do {
                    ++s_iter;
                } while (!s_iter.is_end() && s_iter.is_lazy_deleted());
            }
            if (!d_iter.is_end() && !s_iter.is_end() &&
                !key_less(d_iter.key(), s_iter.key()) &&
                !key_less(s_iter.key(), d_iter.key()))
            {
                do {
                    ++s_iter;
                } while (!s_iter.is_end() && s_iter.is_lazy_deleted());
            }

            in_dyna = s_iter.is_end();
            if (!in_dyna && !d_iter.is_end()) {
                in_dyna = !key_less(s_iter.key(), d_iter.key());
            }
        }

    public:
        inline iterator(bool in_dyna, typename sl_type::iterator d_iter, typename sl_type::iterator s_iter, key_compare key_less)
            : in_dyna(in_dyna), d_iter(d_iter), s_iter(s_iter), key_less(key_less)
        { }

        inline iterator()
            : in_dyna(true)
        { }

        inline iterator(const key_compare& kcf)
            : in_dyna(true), key_less(kcf)
        { }

        inline reference operator * () const
        {
            temp_value = in_dyna ? value_type(d_iter.key(), d_iter.data())
                                 : value_type(s_iter.key(), s_iter.data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            temp_value = in_dyna ? value_type(d_iter.key(), d_iter.data())
                                 : value_type(s_iter.key(), s_iter.data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            return in_dyna ? d_iter.key() : s_iter.key();
        }

        inline data_type & data() const
        {
            return in_dyna ? d_iter.data() : s_iter.data();
        }

        inline iterator& operator ++ ()
        {
            move_forward();
            return *this;
        }

        inline iterator operator ++ (int)
        {
            iterator tmp = *this;

            move_forward();
            return tmp;
        }

        inline iterator& operator -- ()
        {
            // TODO
            return *this;
        }

        inline iterator operator -- (int)
        {
            iterator tmp = *this;

            // TODO
            return tmp;
        }

        inline bool operator == (const iterator &x) const
        {
            return (x.in_dyna == in_dyna && x.d_iter == d_iter && x.s_iter == s_iter);
        }

        inline bool operator != (const iterator &x) const
        {
            return (x.in_dyna != in_dyna || x.d_iter != d_iter || x.s_iter != s_iter);
        }

        inline bool is_incomplete()
        {
            return d_iter.is_invalid() || s_iter.is_invalid();
        }
    };

    class const_iterator {
    public:
        typedef typename sl_type::key_type key_type;
        typedef typename sl_type::data_type data_type;
        typedef typename sl_type::value_type value_type;
        typedef const value_type& reference;
        typedef const value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        bool in_dyna;
        typename sl_type::const_iterator d_iter;
        typename sl_type::const_iterator s_iter;
        key_compare key_less;

        friend class skiplist_map_compact<key_type, data_type, key_compare, traits,
                                          allocator_type>;

        mutable value_type temp_value;

        void move_forward()
        {
            //SL_PRINT((in_dyna ? "d" : "s") << " " << d_iter.key() << " " << s_iter.key());
            if (in_dyna) {
                ++d_iter;
            }
            else {
                do {
                    ++s_iter;
                } while (!s_iter.is_end() && s_iter.is_lazy_deleted());
            }
            if (!d_iter.is_end() && !s_iter.is_end() &&
                !key_less(d_iter.key(), s_iter.key()) &&
                !key_less(s_iter.key(), d_iter.key()))
            {
                do {
                    ++s_iter;
                } while (!s_iter.is_end() && s_iter.is_lazy_deleted());
            }

            in_dyna = s_iter.is_end();
            if (!in_dyna && !d_iter.is_end()) {
                in_dyna = !key_less(s_iter.key(), d_iter.key());
            }
        }

    public:
        inline const_iterator(bool in_dyna, typename sl_type::const_iterator d_iter, typename sl_type::const_iterator s_iter, key_compare key_less)
            : in_dyna(in_dyna), d_iter(d_iter), s_iter(s_iter), key_less(key_less)
        { }

        inline const_iterator()
            : in_dyna(true)
        { }

        inline const_iterator(const key_compare& kcf)
            : in_dyna(true), key_less(kcf)
        { }

        inline const_iterator(const iterator& it)
            : in_dyna(it.in_dyna), d_iter(it.d_iter), s_iter(it.s_iter), key_less(it.key_less)
        { }

        inline reference operator * () const
        {
            temp_value = in_dyna ? value_type(d_iter.key(), d_iter.data())
                                 : value_type(s_iter.key(), s_iter.data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            temp_value = in_dyna ? value_type(d_iter.key(), d_iter.data())
                                 : value_type(s_iter.key(), s_iter.data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            return in_dyna ? d_iter.key() : s_iter.key();
        }

        inline const data_type & data() const
        {
            return in_dyna ? d_iter.data() : s_iter.data();
        }

        inline const_iterator& operator ++ ()
        {
            move_forward();
            return *this;
        }

        inline const_iterator operator ++ (int)
        {
            const_iterator tmp = *this;

            move_forward();
            return tmp;
        }

        inline const_iterator& operator -- ()
        {
            // TODO
            return *this;
        }

        inline const_iterator operator -- (int)
        {
            const_iterator tmp = *this;

            // TODO
            return tmp;
        }

        inline bool operator == (const const_iterator &x) const
        {
            return (x.in_dyna == in_dyna && x.d_iter == d_iter && x.s_iter == s_iter);
        }

        inline bool operator != (const const_iterator &x) const
        {
            return (x.in_dyna != in_dyna || x.d_iter != d_iter || x.s_iter != s_iter);
        }

        inline bool is_incomplete()
        {
            return d_iter.is_invalid() || s_iter.is_invalid();
        }
    };

    class reverse_iterator {
    public:
        typedef typename sl_type::key_type key_type;
        typedef typename sl_type::data_type data_type;
        typedef typename sl_type::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        bool in_dyna;
        typename sl_type::reverse_iterator d_iter;
        typename sl_type::reverse_iterator s_iter;
        key_compare key_less;

        friend class const_reverse_iterator;
        friend class skiplist_map_compact<key_type, data_type, key_compare, traits,
                                          allocator_type>;

        mutable value_type temp_value;

        void move_forward()
        {
            //SL_PRINT((in_dyna ? "d" : "s") << " " << d_iter.key() << " " << s_iter.key());
            if (in_dyna) {
                ++d_iter;
            }
            else {
                do {
                    ++s_iter;
                } while (!s_iter.is_end() && s_iter.is_lazy_deleted());
            }
            if (!d_iter.is_end() && !s_iter.is_end() &&
                !key_less(d_iter.key(), s_iter.key()) &&
                !key_less(s_iter.key(), d_iter.key()))
            {
                do {
                    ++s_iter;
                } while (!s_iter.is_end() && s_iter.is_lazy_deleted());
            }

            in_dyna = s_iter.is_end();
            if (!in_dyna && !d_iter.is_end()) {
                in_dyna = !key_less(d_iter.key(), s_iter.key());
            }
        }

    public:
        inline reverse_iterator(bool in_dyna, typename sl_type::reverse_iterator d_iter, typename sl_type::reverse_iterator s_iter, key_compare key_less)
            : in_dyna(in_dyna), d_iter(d_iter), s_iter(s_iter), key_less(key_less)
        { }

        inline reverse_iterator()
            : in_dyna(true)
        { }

        inline reverse_iterator(const key_compare& kcf)
            : in_dyna(true), key_less(kcf)
        { }

        inline reference operator * () const
        {
            temp_value = in_dyna ? value_type(d_iter.key(), d_iter.data())
                                 : value_type(s_iter.key(), s_iter.data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            temp_value = in_dyna ? value_type(d_iter.key(), d_iter.data())
                                 : value_type(s_iter.key(), s_iter.data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            return in_dyna ? d_iter.key() : s_iter.key();
        }

        inline data_type & data() const
        {
            return in_dyna ? d_iter.data() : s_iter.data();
        }

        inline reverse_iterator& operator ++ ()
        {
            move_forward();
            return *this;
        }

        inline reverse_iterator operator ++ (int)
        {
            reverse_iterator tmp = *this;

            move_forward();
            return tmp;
        }

        inline reverse_iterator& operator -- ()
        {
            // TODO
            return *this;
        }

        inline reverse_iterator operator -- (int)
        {
            reverse_iterator tmp = *this;

            // TODO
            return tmp;
        }

        inline bool operator == (const reverse_iterator &x) const
        {
            return (x.in_dyna == in_dyna && x.d_iter == d_iter && x.s_iter == s_iter);
        }

        inline bool operator != (const reverse_iterator &x) const
        {
            return (x.in_dyna != in_dyna || x.d_iter != d_iter || x.s_iter != s_iter);
        }

        inline bool is_incomplete()
        {
            return d_iter.is_invalid() || s_iter.is_invalid();
        }
    };

    class const_reverse_iterator {
    public:
        typedef typename sl_type::key_type key_type;
        typedef typename sl_type::data_type data_type;
        typedef typename sl_type::value_type value_type;
        typedef value_type& reference;
        typedef value_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

    private:
        bool in_dyna;
        typename sl_type::const_reverse_iterator d_iter;
        typename sl_type::const_reverse_iterator s_iter;
        key_compare key_less;

        friend class skiplist_map_compact<key_type, data_type, key_compare, traits,
                                          allocator_type>;

        mutable value_type temp_value;

        void move_forward()
        {
            //SL_PRINT((in_dyna ? "d" : "s") << " " << d_iter.key() << " " << s_iter.key());
            if (in_dyna) {
                ++d_iter;
            }
            else {
                do {
                    ++s_iter;
                } while (!s_iter.is_end() && s_iter.is_lazy_deleted());
            }
            if (!d_iter.is_end() && !s_iter.is_end() &&
                !key_less(d_iter.key(), s_iter.key()) &&
                !key_less(s_iter.key(), d_iter.key()))
            {
                do {
                    ++s_iter;
                } while (!s_iter.is_end() && s_iter.is_lazy_deleted());
            }

            in_dyna = s_iter.is_end();
            if (!in_dyna && !d_iter.is_end()) {
                in_dyna = !key_less(d_iter.key(), s_iter.key());
            }
        }

    public:
        inline const_reverse_iterator(bool in_dyna, typename sl_type::const_reverse_iterator d_iter, typename sl_type::const_reverse_iterator s_iter, key_compare key_less)
            : in_dyna(in_dyna), d_iter(d_iter), s_iter(s_iter), key_less(key_less)
        { }

        inline const_reverse_iterator()
            : in_dyna(true)
        { }

        inline const_reverse_iterator(const key_compare& kcf)
            : in_dyna(true), key_less(kcf)
        { }

        inline const_reverse_iterator(const reverse_iterator& it)
            : in_dyna(it.in_dyna), d_iter(it.d_iter), s_iter(it.s_iter), key_less(it.key_less)
        { }

        inline reference operator * () const
        {
            temp_value = in_dyna ? value_type(d_iter.key(), d_iter.data())
                                 : value_type(s_iter.key(), s_iter.data());
            return temp_value;
        }

        inline pointer operator -> () const
        {
            temp_value = in_dyna ? value_type(d_iter.key(), d_iter.data())
                                 : value_type(s_iter.key(), s_iter.data());
            return &temp_value;
        }

        inline const key_type & key() const
        {
            return in_dyna ? d_iter.key() : s_iter.key();
        }

        inline const data_type & data() const
        {
            return in_dyna ? d_iter.data() : s_iter.data();
        }

        inline const_reverse_iterator& operator ++ ()
        {
            move_forward();
            return *this;
        }

        inline const_reverse_iterator operator ++ (int)
        {
            const_reverse_iterator tmp = *this;

            move_forward();
            return tmp;
        }

        inline const_reverse_iterator& operator -- ()
        {
            // TODO
            return *this;
        }

        inline const_reverse_iterator operator -- (int)
        {
            const_reverse_iterator tmp = *this;

            // TODO
            return tmp;
        }

        inline bool operator == (const const_reverse_iterator &x) const
        {
            return (x.in_dyna == in_dyna && x.d_iter == d_iter && x.s_iter == s_iter);
        }

        inline bool operator != (const const_reverse_iterator &x) const
        {
            return (x.in_dyna != in_dyna || x.d_iter != d_iter || x.s_iter != s_iter);
        }

        inline bool is_incomplete()
        {
            return d_iter.is_invalid() || s_iter.is_invalid();
        }
    };

private:
    sl_type *dyna_sl;
    sl_type *static_sl;
    bloomfilter bf;

    key_compare m_key_less;
    allocator_type m_allocator;

public:
    explicit inline skiplist_map_compact(const allocator_type& alloc = allocator_type())
        : bf(LITTLEENDIAN, K, BITS_PER_KEY), m_allocator(alloc)
    {
        dyna_sl = new sl_type(alloc);
        static_sl = new sl_type(alloc);

        if (USE_BLOOM_FILTER) {
            bf.reallocate(SL_MERGE_THRESHOLD);
        }
    }

    explicit inline skiplist_map_compact(const key_compare& kcf,
                                 const allocator_type& alloc = allocator_type())
        : bf(LITTLEENDIAN, K, BITS_PER_KEY), m_key_less(kcf), m_allocator(alloc)
    {
        dyna_sl = new sl_type(kcf, alloc);
        static_sl = new sl_type(kcf, alloc);

        if (USE_BLOOM_FILTER) {
            bf.reallocate(SL_MERGE_THRESHOLD);
        }
    }

    inline ~skiplist_map_compact()
    {
        delete dyna_sl;
        delete static_sl;
    }

    void swap(self_type &from)
    {
        std::swap(dyna_sl, from.dyna_sl);
        std::swap(static_sl, from.static_sl);
        std::swap(bf, from.bf);
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
        friend class skiplist_map_compact<key_type, data_type, key_compare, traits,
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

public:
    // clear all nodes except the starting leaf node for empty skip list
    void clear()
    {
        dyna_sl->clear();
        static_sl->clear();
    }

public:
    inline iterator begin()
    {
        typename sl_type::iterator dbegin = dyna_sl->begin();
        typename sl_type::iterator sbegin = static_sl->begin();
        if (sbegin.is_end() || dbegin.is_end()) {
            if (!sbegin.is_end()) {
                while (!sbegin.is_end() && sbegin.is_lazy_deleted()) {
                    ++sbegin;
                }
            }
            return iterator(sbegin.is_end(), dbegin, sbegin, m_key_less);
        }
        while (!sbegin.is_end() && sbegin.is_lazy_deleted()) {
            ++sbegin;
        }
        bool in_dyna =
            sbegin.is_end() ||
            key_greaterequal(sbegin.key(), dbegin.key());
        return iterator(in_dyna, dbegin, sbegin, m_key_less);
    }

    inline iterator end()
    {
        return iterator(true, dyna_sl->end(), static_sl->end(), m_key_less);
    }

    inline const_iterator begin() const
    {
        return const_iterator(begin());
    }

    inline const_iterator end() const
    {
        return const_iterator(end());
    }

    inline reverse_iterator rbegin()
    {
        typename sl_type::reverse_iterator dbegin = dyna_sl->rbegin();
        typename sl_type::reverse_iterator sbegin = static_sl->rbegin();
        if (sbegin.is_end() || dbegin.is_end()) {
            if (!sbegin.is_end()) {
                while (!sbegin.is_end() && sbegin.is_lazy_deleted()) {
                    ++sbegin;
                }
            }
            return reverse_iterator(sbegin.is_end(), dbegin, sbegin, m_key_less);
        }
        while (!sbegin.is_end() && sbegin.is_lazy_deleted()) {
            ++sbegin;
        }
        bool in_dyna =
            sbegin.is_end() ||
            key_lessequal(sbegin.key(), dbegin.key());
        return reverse_iterator(in_dyna, dbegin, sbegin, m_key_less);
    }

    inline reverse_iterator rend()
    {
        return reverse_iterator(true, dyna_sl->rend(), static_sl->rend(), m_key_less);
    }

    inline const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(rbegin());
    }

    inline const_reverse_iterator rend() const
    {
        return const_reverse_iterator(rend());
    }

public:
    // *** Access Functions to the basic stats

    inline size_type dyna_size() const
    {
        return dyna_sl->size();
    }

    inline size_type static_size() const
    {
        return static_sl->size();
    }

    inline size_type size() const
    {
        return dyna_sl->size() + static_sl->size();
    }

    inline size_type bloomfilter_size() const
    {
        return bf.size();
    }

    inline bool empty() const
    {
        return (size() == size_type(0));
    }

    inline size_type max_size() const
    {
        return size_type(-1);
    }

    inline size_type dyna_level() const
    {
        return dyna_sl->level();
    }

    inline size_type static_level() const
    {
        return static_sl->level();
    }

    inline size_type dyna_inner_count() const
    {
        return dyna_sl->inner_count();
    }

    inline size_type static_inner_count() const
    {
        return static_sl->inner_count();
    }

    inline size_type dyna_leaf_count() const
    {
        return dyna_sl->leaf_count();
    }

    inline size_type static_leaf_count() const
    {
        return static_sl->leaf_count();
    }

public:
    // *** Standard Access Functions

    bool exists(const key_type& key) const
    {
        return (find(key) != end());
    }

    iterator find(const key_type& key)
    {
        typename sl_type::iterator it;
        SL_PRINT("finding " << key);
        if (USE_BLOOM_FILTER) {
            if (dyna_sl->size() == 0 || !bf.key_may_match(reinterpret_cast<const char*>(&key), sizeof(key_type))) {
                SL_PRINT("shortcut by bloomfilter");
                it = static_sl->find(key);
                if (!it.is_end() && !it.is_lazy_deleted()) {
                    // NOTE incomplete iterator
                    return iterator(false, typename sl_type::iterator(), it, m_key_less);
                }
                return end();
            }
        }

        it = dyna_sl->find(key);
        if (it.is_end()) {
            it = static_sl->find(key);
            if (!it.is_end() && !it.is_lazy_deleted()) {
                // NOTE incomplete iterator
                return iterator(false, typename sl_type::iterator(), it, m_key_less);
            }
            return end();
        }
        // NOTE incomplete iterator
        return iterator(true, it, typename sl_type::iterator(), m_key_less);
    }

    const_iterator find(const key_type& key) const
    {
        typename sl_type::const_iterator it;
        SL_PRINT("finding " << key);
        if (USE_BLOOM_FILTER) {
            if (dyna_sl->size() == 0 || !bf.key_may_match(reinterpret_cast<const char*>(&key), sizeof(key_type))) {
                SL_PRINT("shortcut by bloomfilter");
                it = static_sl->find(key);
                if (!it.is_end() && !it.is_lazy_deleted()) {
                    // NOTE incomplete iterator
                    return const_iterator(false, typename sl_type::const_iterator(), it, m_key_less);
                }
                return end();
            }
        }

        it = dyna_sl->find(key);
        if (it.is_end()) {
            it = static_sl->find(key);
            if (!it.is_end() && !it.is_lazy_deleted()) {
                // NOTE incomplete iterator
                return const_iterator(false, typename sl_type::const_iterator(), it, m_key_less);
            }
            return end();
        }
        // NOTE incomplete iterator
        return const_iterator(true, it, typename sl_type::const_iterator(), m_key_less);
    }

    void make_iter_complete(iterator& it)
    {
        if (it.in_dyna) {
            key_type key = it.d_iter.key();
            typename sl_type::iterator s_iter = static_sl->upper_bound(key);
            while (!s_iter.is_end() && s_iter.is_lazy_deleted()) {
                ++s_iter;
            }
            it.s_iter = s_iter;
        }
        else {
            key_type key = it.s_iter.key();
            it.d_iter = dyna_sl->upper_bound(key);
        }
    }

    void make_iter_complete(const_iterator& it) const
    {
        if (it.in_dyna) {
            key_type key = it.d_iter.key();
            typename sl_type::iterator s_iter = static_sl->upper_bound(key);
            while (!s_iter.is_end() && s_iter.is_lazy_deleted()) {
                ++s_iter;
            }
            it.s_iter = s_iter;
        }
        else {
            key_type key = it.s_iter.key();
            it.d_iter = dyna_sl->upper_bound(key);
        }
    }

    size_type count(const key_type& key) const
    {
        return (find(key) == end()) ? 0 : 1;
    }

    iterator lower_bound(const key_type& key)
    {
        typename sl_type::iterator dyna_it = dyna_sl->lower_bound(key);
        typename sl_type::iterator static_it = static_sl->lower_bound(key);

        // NOTE skip deleted entries in static stage
        while (!static_it.is_end() && static_it.is_lazy_deleted()) {
            ++static_it;
        }

        bool dyna_end = dyna_it.is_end();
        bool static_end = static_it.is_end();

        if (dyna_end && static_end) {
            return end();
        }
        else if (dyna_end) {
            return iterator(false, dyna_it, static_it, m_key_less);
        }
        else if (static_end) {
            return iterator(true, dyna_it, static_it, m_key_less);
        }
        else if (key_less(dyna_it.key(), static_it.key())) {
            return iterator(true, dyna_it, static_it, m_key_less);
        }
        else if (key_equal(dyna_it.key(), static_it.key())) {
            do {
                ++static_it;
            } while (!static_it.is_end() && static_it.is_lazy_deleted());
            return iterator(true, dyna_it, static_it, m_key_less);
        }
        else {
            return iterator(false, dyna_it, static_it, m_key_less);
        }
    }

    const_iterator lower_bound(const key_type& key) const
    {
        typename sl_type::const_iterator dyna_it = dyna_sl->lower_bound(key);
        typename sl_type::const_iterator static_it = static_sl->lower_bound(key);

        // NOTE skip deleted entries in static stage
        while (!static_it.is_end() && static_it.is_lazy_deleted()) {
            ++static_it;
        }

        bool dyna_end = dyna_it.is_end();
        bool static_end = static_it.is_end();

        if (dyna_end && static_end) {
            return end();
        }
        else if (dyna_end) {
            return const_iterator(false, dyna_it, static_it, m_key_less);
        }
        else if (static_end) {
            return const_iterator(true, dyna_it, static_it, m_key_less);
        }
        else if (key_less(dyna_it.key(), static_it.key())) {
            return const_iterator(true, dyna_it, static_it, m_key_less);
        }
        else if (key_equal(dyna_it.key(), static_it.key())) {
            do {
                ++static_it;
            } while (!static_it.is_end() && static_it.is_lazy_deleted());
            return const_iterator(true, dyna_it, static_it, m_key_less);
        }
        else {
            return const_iterator(false, dyna_it, static_it, m_key_less);
        }
    }

    iterator upper_bound(const key_type& key)
    {
        typename sl_type::iterator dyna_it = dyna_sl->upper_bound(key);
        typename sl_type::iterator static_it = static_sl->upper_bound(key);

        // NOTE skip deleted entries in static stage
        while (!static_it.is_end() && static_it.is_lazy_deleted()) {
            ++static_it;
        }

        bool dyna_end = dyna_it.is_end();
        bool static_end = static_it.is_end();

        if (dyna_end && static_end) {
            return end();
        }
        else if (dyna_end) {
            return iterator(false, dyna_it, static_it, m_key_less);
        }
        else if (static_end) {
            return iterator(true, dyna_it, static_it, m_key_less);
        }
        else if (key_less(dyna_it.key(), static_it.key())) {
            return iterator(true, dyna_it, static_it, m_key_less);
        }
        else if (key_equal(dyna_it.key(), static_it.key())) {
            do {
                ++static_it;
            } while (!static_it.is_end() && static_it.is_lazy_deleted());
            return iterator(true, dyna_it, static_it, m_key_less);
        }
        else {
            return iterator(false, dyna_it, static_it, m_key_less);
        }
    }

    const_iterator upper_bound(const key_type& key) const
    {
        typename sl_type::const_iterator dyna_it = dyna_sl->upper_bound(key);
        typename sl_type::const_iterator static_it = static_sl->upper_bound(key);

        // NOTE skip deleted entries in static stage
        while (!static_it.is_end() && static_it.is_lazy_deleted()) {
            ++static_it;
        }

        bool dyna_end = dyna_it.is_end();
        bool static_end = static_it.is_end();

        if (dyna_end && static_end) {
            return end();
        }
        else if (dyna_end) {
            return const_iterator(false, dyna_it, static_it, m_key_less);
        }
        else if (static_end) {
            return const_iterator(true, dyna_it, static_it, m_key_less);
        }
        else if (key_less(dyna_it.key(), static_it.key())) {
            return const_iterator(true, dyna_it, static_it, m_key_less);
        }
        else if (key_equal(dyna_it.key(), static_it.key())) {
            do {
                ++static_it;
            } while (!static_it.is_end() && static_it.is_lazy_deleted());
            return const_iterator(true, dyna_it, static_it, m_key_less);
        }
        else {
            return const_iterator(false, dyna_it, static_it, m_key_less);
        }
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

    inline skiplist_map_compact(const skiplist_map_compact& other) {

    }
    */

public:
    // NOTE dev-only, to simulate structure after merge

    inline std::pair<iterator, bool> insert_static(const key_type& key, const data_type& data)
    {
        // stub, since only need bool here
        return std::pair<iterator, bool>(end(), (static_sl->insert(key, data)).second);
    }


    // *** Public Insertion Functions

    inline std::pair<iterator, bool> upsert(const pair_type& x)
    {
        if ((SL_MERGE == 1) &&
            ((dyna_sl->size() * SL_MERGE_RATIO) >= static_sl->size()) &&
            (dyna_sl->size() >= SL_MERGE_THRESHOLD))
        {
            merge_dtos();
        }

        return insert_common(x.first, x.second);
    }

    inline std::pair<iterator, bool> insert(const key_type& key, const data_type& data)
    {
        if ((SL_MERGE == 1) &&
            ((dyna_sl->size() * SL_MERGE_RATIO) >= static_sl->size()) &&
            (dyna_sl->size() >= SL_MERGE_THRESHOLD))
        {
            merge_dtos();
        }

        typename sl_type::iterator static_iter = static_sl->find(key);
        if (!static_iter.is_end()) {
            // NOTE incomplete iterator
            return std::pair<iterator, bool>(iterator(false, typename sl_type::iterator(), static_iter, m_key_less), false);
        }

        return insert_common(key, data);
    }

    inline data_type& operator [] (const key_type& key)
    {
        iterator i = upsert(value_type(key, data_type())).first;
        return i.data();
    }

private:
    inline std::pair<iterator, bool> insert_common(const key_type& key, const data_type& data)
    {
        std::pair<typename sl_type::iterator, bool> dyna_retval = dyna_sl->insert_common(key, data);
        // NOTE incomplete iterator
        std::pair<iterator, bool> retval = std::pair<iterator, bool>(iterator(true, dyna_retval.first, typename sl_type::iterator(), m_key_less), dyna_retval.second);

        if (USE_BLOOM_FILTER) {
            bf.insert(reinterpret_cast<const char*>(&key), sizeof(key_type));
        }

        return retval;
    }

public:
    // *** Public Erase Functions

    bool erase_one(const key_type& key)
    {
        if (USE_BLOOM_FILTER) {
            if (dyna_sl->size() == 0 || !bf.key_may_match(reinterpret_cast<const char*>(&key), sizeof(key_type))) {
                return static_sl->lazy_erase_one(key);
            }
        }

        // NOTE commented are incorrect
        //return dyna_sl->erase_one(key) || static_sl->lazy_erase_one(key);
        if (dyna_sl->erase_one(key)) {
            static_sl->lazy_erase_one(key);
            return true;
        }
        else {
            return static_sl->lazy_erase_one(key);
        }
    }

    size_type erase(const key_type& key)
    {
        size_type c = 0;

        if (erase_one(key)) {
            c++;
        }

        return c;
    }

    void erase(iterator iter)
    {
        if (iter.in_dyna) {
            dyna_sl->erase(iter.d_iter);
        }
        else {
            static_sl->lazy_erase(iter.s_iter);
        }
    }

    void erase(reverse_iterator iter)
    {
        if (iter.in_dyna) {
            dyna_sl->erase(iter.d_iter);
        }
        else {
            static_sl->lazy_erase(iter.s_iter);
        }
    }

// NOTE dev-only
public:
    void merge_dtos()
    {
        static_sl->merge(*dyna_sl);

        if (USE_BLOOM_FILTER) {
            bf.reallocate(static_sl->size() / SL_MERGE_RATIO);
        }
    }

#ifdef SL_DEBUG

public:
    // *** Debug Printing

    void print(std::ostream& os) const
    {
        std::cout << "Dynamic stage:" << std::endl << std::endl;
        dyna_sl->print(os);
        std::cout << "Static stage:" << std::endl << std::endl;
        static_sl->print(os);
    }
#endif
};

}

#endif

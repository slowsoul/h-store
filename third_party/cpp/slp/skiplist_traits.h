#ifndef SKIPLIST_TRAITS_H_HEADER
#define SKIPLIST_TRAITS_H_HEADER

#define SL_MAX(a, b)         ((a) < (b) ? (b) : (a))

namespace cmu {

template <typename _Key, typename _Data>
class skiplist_default_map_traits
{
public:
    static const int pagesize = 512;
    static const short order = SL_MAX(8, pagesize / (sizeof(_Key) + sizeof(void *)));
    static const short leaf_order = SL_MAX(8, pagesize / (sizeof(_Key) + sizeof(_Data)));
};

}

#endif

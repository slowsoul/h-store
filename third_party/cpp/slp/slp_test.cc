#include <iostream>

#include "slp.h"

int main() {
    typedef Slp<uint64_t, uint64_t, UINT64_MAX, 32, 0> SkiplistType;
    typedef SlpIterator<uint64_t, uint64_t, UINT64_MAX, 32, 0> IteratorType;
    SkiplistType slmap;

    //int nums[8] = {2,4,6,8,3,5,7,9};
    for (int i = 0; i < 40; i++) {
        slmap.slp_insert(i, i);
        slmap.slp_print();
    }

    std::pair<IteratorType, bool> retval = slmap.search_llb(111);
    assert(retval.second == true);
    assert(retval.first.data() == 112);

    retval = slmap.search_llb(0);
    assert(retval.second == true);
    assert(retval.first.data() == 0);

    retval = slmap.search_llb(199);
    assert(retval.second == false);

    retval = slmap.search_llb(10000);
    assert(retval.second == false);

    return 0;
}

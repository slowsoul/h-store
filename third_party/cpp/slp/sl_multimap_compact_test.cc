#include "skiplist_multimap_compact.h"

int main() {
    typedef cmu::skiplist_multimap_compact<uint64_t, uint64_t> SkiplistType;
    SkiplistType slmap;
    SkiplistType::const_iterator slmap_keyIter;
    SkiplistType::const_reverse_iterator slmap_rev_keyIter;

    // insert test
    std::pair<typename SkiplistType::iterator, bool> retval;

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 3; j++) {
            retval = slmap.insert(i+1, j+1);
            assert(retval.second == true);
        }
    }

    slmap.merge_dtos();

    slmap.print(std::cout);

    for (int i = 16; i < 48; i++) {
        for (int j = 3; j < 6; j++) {
            retval = slmap.insert(i+1, j+1);
            assert(retval.second == true);
        }
    }

    slmap.print(std::cout);

    slmap.find(2).data() = 0;

    // find test
    slmap_keyIter = slmap.find(4);
    assert(slmap_keyIter != slmap.end());
    assert(slmap_keyIter.key() == 4 && slmap_keyIter.data() == 3);

    slmap_keyIter = slmap.find(18);
    assert(slmap_keyIter != slmap.end());
    assert(slmap_keyIter.key() == 18 && slmap_keyIter.data() == 6);

    slmap_keyIter = slmap.find(2);
    assert(slmap_keyIter != slmap.end());
    assert(slmap_keyIter.key() == 2 && slmap_keyIter.data() == 2);

    slmap_keyIter = slmap.find(200);
    assert(slmap_keyIter == slmap.end());

    // lowerbound test
    slmap_keyIter = slmap.lower_bound(7);
    assert(slmap_keyIter.key() == 7 && slmap_keyIter.data() == 3);

    slmap_keyIter = slmap.lower_bound(18);
    assert(slmap_keyIter.key() == 18 && slmap_keyIter.data() == 6);

    slmap_keyIter = slmap.lower_bound(2);
    assert(slmap_keyIter.key() == 2 && slmap_keyIter.data() == 2);

    slmap_keyIter = slmap.upper_bound(2);
    assert(slmap_keyIter.key() == 3 && slmap_keyIter.data() == 3);

    slmap_keyIter = slmap.upper_bound(18);
    assert(slmap_keyIter.key() == 19 && slmap_keyIter.data() == 6);

    // lower bound iterator test
    std::cout << "lower_bound iterator: " << std::endl;
    for (slmap_keyIter = slmap.lower_bound(2);slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    // upper bound iterator test
    std::cout << "upper_bound iterator: " << std::endl;
    for (slmap_keyIter = slmap.upper_bound(16);slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    // equal range iterator test
    std::pair<SkiplistType::iterator, SkiplistType::iterator> res = slmap.equal_range(18);

    std::cout << "equal_range iterator: " << std::endl;
    for (slmap_keyIter = res.first; slmap_keyIter != res.second; ++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    // iterator test
    std::cout << "iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    slmap.merge_dtos();
    slmap.print(std::cout);
}

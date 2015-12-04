#include "skiplist_multimap.h"

int main() {
    typedef cmu::skiplist_multimap<uint64_t, uint64_t> SkiplistType;
    SkiplistType slmap;
    SkiplistType::iterator slmap_keyIter;

    // insert test
    std::pair<typename SkiplistType::iterator, bool> retval;

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            retval = slmap.insert(i, j+1);
            assert(retval.second == true);
        }
    }

    slmap.print(std::cout);

    // find test
    slmap_keyIter = slmap.find(2);
    assert(slmap_keyIter != slmap.end());
    slmap_keyIter.data() = 100;

    slmap_keyIter = slmap.find(200);
    assert(slmap_keyIter == slmap.end());

    // lower_bound test
    slmap_keyIter = slmap.lower_bound(2);
    assert(slmap_keyIter != slmap.end());
    assert(slmap_keyIter.data() == 100);

    // upper bound test
    slmap_keyIter = slmap.upper_bound(2);
    assert(slmap_keyIter != slmap.end());
    assert(slmap_keyIter.key() == 3 && slmap_keyIter.data() == 10);

    // equal_range test
    std::pair<SkiplistType::iterator, SkiplistType::iterator> range = slmap.equal_range(2);
    assert(range.first != slmap.end());
    assert(range.second != slmap.end());
    assert(range.first.key() == 2 && range.first.data() == 100);
    assert(range.second.key() == 3 && range.second.data() == 10);

    range = slmap.equal_range(9);
    assert(range.first != slmap.end());
    assert(range.second == slmap.end());
    assert(range.first.key() == 9 && range.first.data() == 10);

    // iterator test
    std::cout << "iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    for (slmap_keyIter = --slmap.end();;--slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
        if (slmap_keyIter == slmap.begin()) {
            break;
        }
    }

    SkiplistType::const_reverse_iterator slmap_rev_keyIter;
    std::cout << "reverse iterator: " << std::endl;
    for (slmap_rev_keyIter = slmap.rbegin();slmap_rev_keyIter != slmap.rend();++slmap_rev_keyIter) {
        std::cout << slmap_rev_keyIter.key() << ": " << slmap_rev_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    for (slmap_rev_keyIter = --slmap.rend();;--slmap_rev_keyIter) {
        std::cout << slmap_rev_keyIter.key() << ": " << slmap_rev_keyIter.data() << std::endl;
        if (slmap_rev_keyIter == slmap.rbegin()) {
            break;
        }
    }

    // erase key test
    bool erased = slmap.erase(200);
    assert(erased == false);

    slmap.print(std::cout);

    erased = slmap.erase(1);
    assert(erased == true);

    slmap.print(std::cout);

    // iterator test
    std::cout << "after erase, iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    for (slmap_keyIter = --slmap.end();;--slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
        if (slmap_keyIter == slmap.begin()) {
            break;
        }
    }

    erased = slmap.erase(1);
    assert(erased == false);

    slmap.print(std::cout);
}

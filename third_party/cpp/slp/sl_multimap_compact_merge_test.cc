#include "skiplist_multimap.h"
#include "skiplist_multimap_ro.h"

int main() {
    typedef cmu::skiplist_multimap<uint64_t, uint64_t> SkiplistType;
    typedef cmu::skiplist_multimap_ro<uint64_t, uint64_t> SkiplistReadOnlyType;
    SkiplistReadOnlyType::iterator slmapro_iter;

    SkiplistType slmap;
    SkiplistReadOnlyType slmapro;

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 2; j++) {
            slmap.insert(i+1, j+1);
        }
    }

    assert(--slmap.begin() == slmap.begin());
    assert(++slmap.end() == slmap.end());
    assert(--slmap.rbegin() == slmap.rbegin());
    assert(++slmap.rend() == slmap.rend());

    // iterator test
    SkiplistType::const_iterator slmap_keyIter;
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

    slmap.print(std::cout);
    slmapro.merge(slmap);
    slmap.print(std::cout);
    slmapro.print(std::cout);

    assert(--slmapro.begin() == slmapro.begin());
    assert(++slmapro.end() == slmapro.end());
    assert(--slmapro.rbegin() == slmapro.rbegin());
    assert(++slmapro.rend() == slmapro.rend());

    // iterator test
    SkiplistReadOnlyType::const_iterator slmapro_keyIter;
    std::cout << "RO iterator: " << std::endl;
    for (slmapro_keyIter = slmapro.begin();slmapro_keyIter != slmapro.end();++slmapro_keyIter) {
        std::cout << slmapro_keyIter.key() << ": " << slmapro_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    for (slmapro_keyIter = --slmapro.end();;--slmapro_keyIter) {
        std::cout << slmapro_keyIter.key() << ": " << slmapro_keyIter.data() << std::endl;
        if (slmapro_keyIter == slmapro.begin()) {
            break;
        }
    }

    SkiplistReadOnlyType::const_reverse_iterator slmapro_rev_keyIter;
    std::cout << "RO reverse iterator: " << std::endl;
    for (slmapro_rev_keyIter = slmapro.rbegin();slmapro_rev_keyIter != slmapro.rend();++slmapro_rev_keyIter) {
        std::cout << slmapro_rev_keyIter.key() << ": " << slmapro_rev_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    for (slmapro_rev_keyIter = --slmapro.rend();;--slmapro_rev_keyIter) {
        std::cout << slmapro_rev_keyIter.key() << ": " << slmapro_rev_keyIter.data() << std::endl;
        if (slmapro_rev_keyIter == slmapro.rbegin()) {
            break;
        }
    }

    slmapro.find(2).data() = 0;

    slmapro_iter = slmapro.find(2);
    assert(slmapro_iter != slmapro.end());
    assert(slmapro_iter.key() == 2 && slmapro_iter.data() == 1);

    slmapro_iter = slmapro.lower_bound(2);
    assert(slmapro_iter != slmapro.end());
    assert(slmapro_iter.key() == 2 && slmapro_iter.data() == 1);

    slmapro_iter = slmapro.upper_bound(1);
    assert(slmapro_iter != slmapro.end());
    assert(slmapro_iter.key() == 2 && slmapro_iter.data() == 1);

    std::pair<SkiplistReadOnlyType::iterator, SkiplistReadOnlyType::iterator> res = slmapro.equal_range(2);
    assert(res.first.key() == 2 && res.first.data() == 1);
    assert(res.second.key() == 3 && res.second.data() == 2);

    /*
    for (int i = 32; i < 64; i++) {
        for (int j = 0; j < 2; j++) {
            slmap.insert(i+1, j+1);
        }
    }

    slmap.print(std::cout);
    slmapro.merge(slmap);
    slmap.print(std::cout);
    slmapro.print(std::cout);
    */
}

#include "skiplist_map_compact.h"

int main() {
    typedef cmu::skiplist_map_compact<uint64_t, uint64_t> SkiplistType;
    SkiplistType slmap;
    SkiplistType::const_iterator slmap_keyIter;
    SkiplistType::const_reverse_iterator slmap_rev_keyIter;

    std::pair<typename SkiplistType::iterator, bool> retval;

    for (int i = 0; i < 99; i++) {
        if (i % 2 == 0) {
            retval = slmap.insert(i, i+1);
            assert(retval.second == true);
        }
        else {
            retval = slmap.insert_static(i, i+1);
            assert(retval.second == true);
        }
    }
    slmap[3] = 10;
    slmap[5] = 10;
    slmap[97] = 10;
    slmap.erase(1);
    slmap.erase(97);

    std::cout << "before merge: iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    slmap.print(std::cout);

    // merge
    slmap.merge_dtos();

    std::cout << "after merge: iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    slmap.print(std::cout);
}

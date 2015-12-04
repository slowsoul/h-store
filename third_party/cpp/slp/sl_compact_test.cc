#include "skiplist_map_compact.h"

int main() {
    typedef cmu::skiplist_map_compact<uint64_t, uint64_t> SkiplistType;
    SkiplistType slmap;
    SkiplistType::const_iterator slmap_keyIter, tmp_keyIter;
    SkiplistType::const_reverse_iterator slmap_rev_keyIter;

    // insert test
    std::pair<typename SkiplistType::iterator, bool> retval;

    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            retval = slmap.insert(i*2, i*2+1);
            assert(retval.second == true);
        }
        else {
            retval = slmap.insert_static(i*2, i*2+1);
            assert(retval.second == true);
        }
    }

    std::cout << "initial: iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    std::cout << "initial: reverse iterator: " << std::endl;
    for (slmap_rev_keyIter = slmap.rbegin();slmap_rev_keyIter != slmap.rend();++slmap_rev_keyIter) {
        std::cout << slmap_rev_keyIter.key() << ": " << slmap_rev_keyIter.data() << std::endl;
    }

    slmap.print(std::cout);

    // update test
    slmap[4] = 8;
    slmap[10] = 20;
    assert(slmap.find(10).data() == 20);

    // erase key test
    bool erased = slmap.erase(200);
    assert(erased == false);

    slmap.print(std::cout);

    erased = slmap.erase(2);
    assert(erased == true);

    slmap.print(std::cout);

    erased = slmap.erase(2);
    assert(erased == false);

    slmap.print(std::cout);

    erased = slmap.erase(6);
    assert(erased == true);

    erased = slmap.erase(12);
    assert(erased == true);

    slmap.print(std::cout);

    // find test
    slmap_keyIter = slmap.find(4);
    assert(slmap_keyIter != slmap.end());
    assert(slmap_keyIter.is_incomplete());
    tmp_keyIter = slmap.lower_bound(4);
    slmap.make_iter_complete(slmap_keyIter);
    assert(!slmap_keyIter.is_incomplete() && tmp_keyIter == slmap_keyIter);
    std::cout << "completed lower_bound iterator: " << std::endl;
    for (;slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    slmap_keyIter = slmap.find(14);
    assert(slmap_keyIter != slmap.end());
    assert(slmap_keyIter.is_incomplete());
    tmp_keyIter = slmap.lower_bound(14);
    slmap.make_iter_complete(slmap_keyIter);
    assert(!slmap_keyIter.is_incomplete() && tmp_keyIter == slmap_keyIter);
    std::cout << "completed lower_bound iterator: " << std::endl;
    for (;slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    slmap_keyIter = slmap.find(200);
    assert(slmap_keyIter == slmap.end());

    // lowerbound test
    slmap_keyIter = slmap.lower_bound(7);
    assert(slmap_keyIter.key() == 8);

    slmap_keyIter = slmap.lower_bound(8);
    assert(slmap_keyIter.key() == 8);

    slmap_keyIter = slmap.lower_bound(9);
    assert(slmap_keyIter.key() == 10);

    // lower bound iterator test
    std::cout << "lower_bound iterator: " << std::endl;
    for (slmap_keyIter = slmap.lower_bound(14);slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    // upper bound iterator test
    std::cout << "upper_bound iterator: " << std::endl;
    for (slmap_keyIter = slmap.upper_bound(16);slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }

    /*
    std::cout << "reverse iterator: " << std::endl;
    for (slmap_rev_keyIter = slmap.lower_bound(13);slmap_rev_keyIter != slmap.rend();++slmap_rev_keyIter) {
        std::cout << slmap_rev_keyIter.key() << ": " << slmap_rev_keyIter.data() << std::endl;
    }
    */

    // iterator test
    std::cout << "iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    /*
    std::cout << std::endl;
    for (slmap_keyIter = --slmap.end();;--slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
        if (slmap_keyIter == slmap.begin()) {
            break;
        }
    }
    */

    std::cout << "reverse iterator: " << std::endl;
    for (slmap_rev_keyIter = slmap.rbegin();slmap_rev_keyIter != slmap.rend();++slmap_rev_keyIter) {
        std::cout << slmap_rev_keyIter.key() << ": " << slmap_rev_keyIter.data() << std::endl;
    }

    /*
    std::cout << std::endl;
    bool onemore = false;
    for (slmap_rev_keyIter = ++slmap.begin();;--slmap_rev_keyIter) {
        std::cout << slmap_rev_keyIter.key() << ": " << slmap_rev_keyIter.data() << std::endl;
        if (onemore) {
            break;
        }
        if (slmap_rev_keyIter == slmap.end()) {
            onemore = true;
        }
    }
    */

    // erase iterator test
    SkiplistType::iterator slmap_nonconst_keyIter;
    slmap_nonconst_keyIter = slmap.lower_bound(8);
    assert(slmap_nonconst_keyIter != slmap.end());
    slmap.erase(slmap_nonconst_keyIter);
    slmap_nonconst_keyIter = slmap.find(8);
    assert(slmap_nonconst_keyIter == slmap.end());
    slmap.print(std::cout);

    slmap_nonconst_keyIter = slmap.upper_bound(10);
    assert(slmap_nonconst_keyIter != slmap.end());
    slmap.erase(slmap_nonconst_keyIter);
    slmap_nonconst_keyIter = slmap.find(14);
    assert(slmap_nonconst_keyIter == slmap.end());
    slmap.print(std::cout);

    std::cout << "finally: iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    std::cout << "finally: reverse iterator: " << std::endl;
    for (slmap_rev_keyIter = slmap.rbegin();slmap_rev_keyIter != slmap.rend();++slmap_rev_keyIter) {
        std::cout << slmap_rev_keyIter.key() << ": " << slmap_rev_keyIter.data() << std::endl;
    }

    // TODO merge test
    slmap.merge_dtos();

    std::cout << "after merge: iterator: " << std::endl;
    for (slmap_keyIter = slmap.begin();slmap_keyIter != slmap.end();++slmap_keyIter) {
        std::cout << slmap_keyIter.key() << ": " << slmap_keyIter.data() << std::endl;
    }
    std::cout << std::endl;
    std::cout << "after merge: reverse iterator: " << std::endl;
    for (slmap_rev_keyIter = slmap.rbegin();slmap_rev_keyIter != slmap.rend();++slmap_rev_keyIter) {
        std::cout << slmap_rev_keyIter.key() << ": " << slmap_rev_keyIter.data() << std::endl;
    }

    slmap.print(std::cout);
}

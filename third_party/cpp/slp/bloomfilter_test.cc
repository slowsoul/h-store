#include "bloomfilter.h"

#include <iostream>

int main() {
    cmu::bloomfilter bf(true, 2, 8);
    uint64_t a = 4, b = 5, c = 6;

    bf.reallocate(10);
    bf.insert(reinterpret_cast<const char*>(&a), sizeof(uint64_t));
    bf.insert(reinterpret_cast<const char*>(&b), sizeof(uint64_t));

    std::cout << bf.key_may_match(reinterpret_cast<const char*>(&a), sizeof(uint64_t)) << std::endl;
    std::cout << bf.key_may_match(reinterpret_cast<const char*>(&c), sizeof(uint64_t)) << std::endl;

    std::cout << bf.size() << std::endl;
}

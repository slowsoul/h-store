#ifndef BLOOMFILTER_H_HEADER
#define BLOOMFILTER_H_HEADER

#include <cstdlib>
#include <cstdint>
#include <cstring>

namespace cmu {

class bloomfilter {
private:
    bool little_endian;
    size_t bits_per_key;
    size_t k;

    size_t bits;
    char *array;

    inline uint32_t decode_fixed_32(const char *ptr) const
    {
        if (little_endian) {
            // Load the raw bytes
            uint32_t result;
            memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
            return result;
        } else {
            return ((static_cast<uint32_t>(static_cast<unsigned char>(ptr[0])))
            | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[1])) << 8)
            | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[2])) << 16)
            | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[3])) << 24));
        }
    }

    uint32_t hash(const char *data, size_t n, uint32_t seed) const
    {
        // Similar to murmur hash
        const uint32_t m = 0xc6a4a793;
        const uint32_t r = 24;
        const char* limit = data + n;
        uint32_t h = seed ^ (n * m);

        // Pick up four bytes at a time
        while (data + 4 <= limit) {
            uint32_t w = decode_fixed_32(data);
            data += 4;
            h += w;
            h *= m;
            h ^= (h >> 16);
        }

        // Pick up remaining bytes
        switch (limit - data) {
            case 3:
                h += static_cast<unsigned char>(data[2]) << 16;
                //FALLTHROUGH_INTENDED;
            case 2:
                h += static_cast<unsigned char>(data[1]) << 8;
                //FALLTHROUGH_INTENDED;
            case 1:
                h += static_cast<unsigned char>(data[0]);
                h *= m;
                h ^= (h >> r);
                break;
        }
        return h;
    }

    uint32_t bloom_hash(const char *data, size_t n) const
    {
        return hash(data, n, 0xbc9f1d34);
    }

public:
    explicit inline bloomfilter(bool little_endian, int k, int bits_per_key)
        : little_endian(little_endian), k(k), bits_per_key(bits_per_key), bits(0), array(NULL)
    { }

    inline ~bloomfilter()
    {
        if (array != NULL) {
            free(array);
            array = NULL;
        }
    }

    void reallocate(int key_count)
    {
        if (array != NULL) {
            free(array);
            array = NULL;
        }

        size_t bytes = (key_count * bits_per_key + 7) / 8;
        bits = bytes * 8;
        array = (char *)malloc(bytes);
        memset(array, 0, bytes);
    }

    size_t size() const
    {
        return bits / 8;
    }

    void insert(const char *data, size_t n)
    {
        uint32_t h = bloom_hash(data, n);
        const uint32_t delta = (h >> 17) | (h << 15);
        for (size_t j = 0; j < k; j++) {
            const uint32_t bitpos = h % bits;
            array[bitpos / 8] |= (1 << (bitpos % 8));
            h += delta;
        }
    }

    bool key_may_match(const char *data, size_t n) const
    {
        uint32_t h = bloom_hash(data, n);
        const uint32_t delta = (h >> 17) | (h << 15);
        for (size_t j = 0; j < k; j++) {
            const uint32_t bitpos = h % bits;
            if ((array[bitpos / 8] & (1 << (bitpos % 8))) == 0) {
                return false;
            }
            h += delta;
        }
        return true;
    }
};

}

#endif

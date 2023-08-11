#pragma once

#include <stdint.h>
#include <stddef.h>

namespace fce {

namespace codec {

struct zigzag
{
    static constexpr uint64_t encode(int64_t x) { return (2 * x) ^ (x >> 63); }
    static constexpr int64_t  decode(uint64_t x) { return (x >> 1) ^ -(x & 1); }
};

struct varint
{
    static constexpr size_t encode(uint64_t value, uint8_t* output)
    {
        size_t size = 0;
        while (value > 127) {
            output[size] = static_cast<uint8_t>(value & 127) | 128;
            value >>= 7;
            size++;
        }
        output[size++] = static_cast<uint8_t>(value) & 127;
        return size;
    }

    static constexpr uint64_t decode(uint8_t* input, size_t size)
    {
        uint64_t result = 0;
        for (size_t i = 0; i < size; i++) {
            result |= (input[i] & 127) << (7 * i);
            if (!(input[i] & 128))
                break;
        }
        return result;
    }
};

struct grid;

struct hilbert;

} // namespace codec

} // namespace fce

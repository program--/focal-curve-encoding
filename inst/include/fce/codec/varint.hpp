#pragma once
#include <fce/codec/types.hpp>
FCE_CODEC_NS_BEGIN

struct varint
{
    static constexpr size_type encode(unsigned_type value, byte_type* output)
    {
        size_type size = 0;
        while (value > 127) {
            output[size] = static_cast<byte_type>(value & 127) | 128;
            value >>= 7;
            size++;
        }
        output[size++] = static_cast<byte_type>(value) & 127;
        return size;
    }

    static constexpr unsigned_type decode(byte_type* input, size_type size)
    {
        unsigned_type result = 0;
        for (size_type i = 0; i < size; i++) {
            result |= (input[i] & 127) << (7 * i);
            if (!(input[i] & 128))
                break;
        }
        return result;
    }
};

FCE_CODEC_NS_END

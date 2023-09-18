#pragma once
#include <fce/codec/types.hpp>
FCE_CODEC_NS_BEGIN

struct zigzag
{
    static constexpr unsigned_type encode(signed_type x)
    {
        return (2 * x) ^ (x >> 63);
    }

    static constexpr signed_type decode(unsigned_type x)
    {
        return (x >> 1) ^ -(x & 1);
    }
};

FCE_CODEC_NS_END

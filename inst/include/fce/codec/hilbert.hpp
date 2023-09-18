#pragma once
#include <fce/codec/types.hpp>
#include <utility>
FCE_CODEC_NS_BEGIN

struct hilbert
{

    static constexpr unsigned_type
    encode(size_type iterations, unsigned_type x, unsigned_type y)
    {
        unsigned_type index = 0;
        for (unsigned_type s = iterations / 2; s > 0; s /= 2) {
            unsigned_type rx = (x & s) > 0;
            unsigned_type ry = (y & s) > 0;
            index += s * s * ((3 * rx) ^ ry);
            rotate(iterations, x, y, rx, ry);
        }
        return index;
    }

    static constexpr std::pair<unsigned_type, unsigned_type>
    decode(size_type iterations, unsigned_type index)
    {
        unsigned_type t = index;
        unsigned_type x = 0, y = 0;

        for (unsigned_type s = 1; s < iterations; s *= 2) {
            unsigned_type rx = 1 & (t / 2);
            unsigned_type ry = 1 & (t ^ rx);
            rotate(s, x, y, rx, ry);
            x += s * rx;
            y += s * ry;
            t /= 4;
        }

        return std::make_pair(x, y);
    }

  private:
    static constexpr void rotate(
      size_type      n,
      unsigned_type& x,
      unsigned_type& y,
      unsigned_type  rx,
      unsigned_type  ry
    )
    {
        if (ry == 0) {
            if (rx == 1) {
                x = n - 1 - x;
                y = n - 1 - y;
            }

            unsigned_type t = x;
            x               = y;
            y               = t;
        }
    }
};

FCE_CODEC_NS_END

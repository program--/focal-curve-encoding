#pragma once
#include <fce/codec/types.hpp>
#include <cmath>
FCE_CODEC_NS_BEGIN

struct grid
{
    static constexpr unsigned_type
    encode(size_type zoom, coord_type coord, coord_type max, coord_type min)
    {
        const coord_type diff = static_cast<coord_type>(zoom) / (max - min);

        if (coord >= min && coord < max) {
            return floor((coord - min) * diff);
        }

        if (coord == max) {
            return zoom - 1;
        }

        return -1;
    }

    static constexpr coord_type
    decode(size_type zoom, coord_type coord, coord_type max, coord_type min)
    {
        const coord_type diff = (max - min) / static_cast<coord_type>(zoom);
        return min + ((coord + 0.5) * diff);
    }
};

FCE_CODEC_NS_END

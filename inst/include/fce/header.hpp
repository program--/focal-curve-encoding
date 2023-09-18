#pragma once

#include <array>

namespace fce {

struct fce_header
{
    unsigned type   : 3;
    unsigned z_flag : 1;
    unsigned depth  : 6;
    unsigned end    : 6;
    // --- 2 bytes
    std::array<double, 4> bounds;
};

} // namespace fce

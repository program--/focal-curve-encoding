#pragma once

#include "model.hpp"

namespace fce {

struct fce_header
{
    unsigned type   : 3;
    unsigned z_flag : 1;
    unsigned depth  : 6;
    unsigned end    : 6;
    // --- 2 bytes
    std::vector<double> bounds;
};

struct fce_data
{

    std::vector<unsigned char> serialize() const;

  private:
    fce_header                        header_;
    std::vector<fce::model::geometry> data_;
};

} // namespace fce

#undef FCE_DECLARE_DEFAULT_CONSTRUCTORS
#undef FCE_DECLARE_FORWARD_CONSTRUCTOR

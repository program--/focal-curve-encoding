#pragma once

#define FCE_CODEC_NS_BEGIN \
    namespace fce {        \
    namespace codec {

#define FCE_CODEC_NS_END \
    }                    \
    }

#include <cstdint>

FCE_CODEC_NS_BEGIN

using signed_type   = int64_t;
using unsigned_type = uint64_t;
using size_type     = std::size_t;
using byte_type     = uint8_t;
using coord_type    = double;

FCE_CODEC_NS_END

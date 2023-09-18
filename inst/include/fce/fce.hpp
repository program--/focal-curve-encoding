#pragma once

#include "header.hpp"
#include <geos.h>
#include <vector>
#include "visitors.hpp"

namespace fce {

struct fce
{
    using geometry_type = geos::geom::Geometry;
    using pointer       = std::unique_ptr<geometry_type>;

    fce_header make_header(const pointer& geometry)
    {
        using geos_type = geos::geom::GeometryTypeId;

        unsigned type   = 0;
        switch (geometry->getGeometryTypeId()) {
            case geos_type::GEOS_POINT:
                type = 1;
                break;

            case geos_type::GEOS_LINEARRING:
            case geos_type::GEOS_LINESTRING:
                type = 2;
                break;

            case geos_type::GEOS_POLYGON:
                type = 3;
                break;

            case geos_type::GEOS_MULTIPOINT:
                type = 4;
                break;

            case geos_type::GEOS_MULTILINESTRING:
                type = 5;
                break;

            case geos_type::GEOS_MULTIPOLYGON:
                type = 6;
                break;

            case geos_type::GEOS_GEOMETRYCOLLECTION:
                type = 7;
                break;
        }

        auto bbox = geometry->getEnvelope()->getCoordinates();
        return {
            type,
            false,
            31,
            0,
            { bbox->getX(0), bbox->getX(1), bbox->getY(0), bbox->getY(1) }
        };
    }

    template<
      typename CurvePolicy    = codec::hilbert,
      typename CompressPolicy = codec::varint,
      std::enable_if_t<
        codec::is_codec<CurvePolicy> && codec::is_codec<CompressPolicy>,
        bool> = true>
    std::vector<codec::byte_type> encode(pointer geometry)
    {
        fce_header header_ = make_header(geometry);
        visitors::encoder<CurvePolicy, CompressPolicy> visitor{ header_ };
        geometry->applyComponentFilter(visitor);
        return visitor.output();
    }

    template<
      typename CurvePolicy    = codec::hilbert,
      typename CompressPolicy = codec::varint,
      std::enable_if_t<
        codec::is_codec<CurvePolicy> && codec::is_codec<CompressPolicy>,
        bool> = true>
    pointer decode(std::vector<codec::byte_type>& data);
};

} // namespace fce

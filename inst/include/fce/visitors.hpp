#pragma once

#include <geos/geom/Geometry.h>

#include "codec.hpp"

namespace fce {

namespace visitors {

template<typename CurvePolicy, typename CompressPolicy>
struct encoder
{
    void filter(const geos::geom::Geometry* geom)
    {
        encode<geom->getGeometryTypeId()>(geom);
    }

    const auto& output() noexcept { return output_; }

  private:
    using geos_type = geos::geom::GeometryTypeId;

    template<geos_type geometry_type>
    void encode(const geos::geom::Geometry* geom);

    template<>
    void encode<geos_type::GEOS_POINT>(const geos::geom::Geometry* geom);

    template<>
    void encode<geos_type::GEOS_LINESTRING>(const geos::geom::Geometry* geom);

    template<>
    void encode<geos_type::GEOS_POLYGON>(const geos::geom::Geometry* geom);

    template<>
    void encode<geos_type::GEOS_MULTIPOINT>(const geos::geom::Geometry* geom)
    {
        for (codec::size_type i = 0; i < geom->getNumGeometries(); i++) {
            encode<geos_type::GEOS_POINT>(geom->getGeometryN(i));
        }
    }

    template<>
    void encode<geos_type::GEOS_MULTILINESTRING>(
      const geos::geom::Geometry* geom
    )
    {
        for (codec::size_type i = 0; i < geom->getNumGeometries(); i++) {
            encode<geos_type::GEOS_MULTILINESTRING>(geom->getGeometryN(i));
        }
    }

    template<>
    void encode<geos_type::GEOS_MULTIPOLYGON>(const geos::geom::Geometry* geom)
    {
        for (codec::size_type i = 0; i < geom->getNumGeometries(); i++) {
            encode<geos_type::GEOS_POLYGON>(geom->getGeometryN(i));
        }
    }

    template<>
    void encode<geos_type::GEOS_GEOMETRYCOLLECTION>(
      const geos::geom::Geometry* geom
    )
    {
        for (codec::size_type i = 0; i < geom->getNumGeometries(); i++) {
            const auto g = geom->getGeometryN(i);
            encode<g->getGeometryTypeId()>(g);
        }
    }

    std::vector<codec::byte_type> output_;
};

template<typename CurvePolicy, typename CompressPolicy>
struct decoder
{
    void        filter(std::vector<codec::byte_type>&);

    const auto& output() noexcept { return output_; }

  private:
    using geometry_type = geos::geom::Geometry;
    using pointer       = std::unique_ptr<geometry_type>;

    pointer output_;
};

} // namespace visitors

} // namespace fce

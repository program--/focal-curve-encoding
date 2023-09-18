#pragma once
#include <fce/codec/types.hpp>
#include <fce/header.hpp>
#include <fce/codec.hpp>
#include <geos.h>
#include <vector>

FCE_CODEC_NS_BEGIN

template<typename CurvePolicy, typename CompressPolicy>
struct fce
{
    using geometry_type = geos::geom::Geometry;
    using geometry_ptr  = std::unique_ptr<geometry_type>;

    fce(const fce_header& header)
      : header_(&header){};

    fce(const fce_header* const ptr)
      : header_(ptr){};

    std::vector<byte_type> encode(geometry_ptr geometry);

    geometry_ptr           decode(std::vector<byte_type> bytes);

  private:
    auto index_point(geometry_ptr geometry)
    {
        const auto coord = geometry->getCoordinate();
        return index_(coord->x, coord->y);
    }

    auto index_linestring(geometry_ptr geometry)
    {
        const auto                 npts = geometry->getNumGeometries();
        std::vector<unsigned_type> pts;
        pts.reserve(npts);

        for (size_type i = 0; i < npts; i++) {
            const auto ptr = geometry->getGeometryN(i);
            pts.push_back(index_point(ptr));
        }

        return pts;
    }

    auto index_polygon(geometry_ptr geometry)
    {
        const auto nrings = geometry->getNumGeometries();
        std::vector<std::vector<unsigned_type>> pts;
        pts.reserve(nrings);

        for (size_type i = 0; i < nrings; i++) {
            const auto ptr = geometry->getGeometryN(i);
            pts.push_back(index_linestring(ptr));
        }

        return pts;
    }

    auto index_multipoint(geometry_ptr geometry)
    {
        const auto                 count = geometry->getNumGeometries();
        std::vector<unsigned_type> pts;
        pts.reserve(count);

        for (size_type i = 0; i < count; i++) {
            const auto ptr = geometry->getGeometryN(i);
            pts.push_back(index_point(ptr));
        }

        return pts;
    }

    auto index_multilinestring(geometry_ptr geometry)
    {
        const auto count = geometry->getNumGeometries();
        std::vector<std::vector<unsigned_type>> pts;
        pts.reserve(count);

        for (size_type i = 0; i < count; i++) {
            const auto ptr = geometry->getGeometryN(i);
            pts.push_back(index_linestring(ptr));
        }

        return pts;
    }

    auto index_multipolygon(geometry_ptr geometry)
    {
        const auto count = geometry->getNumGeometries();
        std::vector<std::vector<std::vector<unsigned_type>>> pts;
        pts.reserve(count);

        for (size_type i = 0; i < count; i++) {
            const auto ptr = geometry->getGeometryN(i);
            pts.push_back(index_polygon(ptr));
        }

        return pts;
    }

    constexpr unsigned_type index_(coord_type x, coord_type y)
    {
        const auto& [xmin, xmax, ymin, ymax] = header_->bounds;
        const auto gx = codec::grid::encode(header_->depth, x, xmax, xmin);
        const auto gy = codec::grid::encode(header_->depth, y, ymax, ymin);
        return codec::hilbert::encode(header_->depth, gx, gy);
    }

    const fce_header* const header_;
};

FCE_CODEC_NS_END

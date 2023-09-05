#pragma once

#include <geos.h>
#include <geos/geom/Geometry.h>
#include <type_traits>
#include <vector>

namespace fce {

namespace traits {

template<typename T, typename = void>
struct can_encode : public std::false_type
{};

template<typename T>
struct can_encode<T, std::void_t<decltype(T::encode)>> : public std::true_type
{};

template<typename T, typename = void>
struct can_decode : public std::false_type
{};

template<typename T>
struct can_decode<T, std::void_t<decltype(T::decode)>> : public std::true_type
{};

template<typename T>
constexpr bool is_codec = can_encode<T>::value && can_decode<T>::value;

} // namespace traits

namespace codec {

using signed_type   = int64_t;
using unsigned_type = uint64_t;
using size_type     = std::size_t;
using byte_type     = uint8_t;
using coord_type    = double;

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

} // namespace codec

namespace visitors {

template<typename CurvePolicy, typename CompressPolicy>
struct fce
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

} // namespace visitors

struct fce
{
    using geometry_type = geos::geom::Geometry;
    using pointer       = std::unique_ptr<geometry_type>;

    struct header
    {
        unsigned type   : 3;
        unsigned z_flag : 1;
        unsigned depth  : 6;
        unsigned end    : 6;
        // --- 2 bytes
        std::vector<double> bounds;
    };

    template<
      typename CurvePolicy    = codec::hilbert,
      typename CompressPolicy = codec::varint,
      std::enable_if_t<
        traits::is_codec<CurvePolicy> && traits::is_codec<CompressPolicy>,
        bool> = true>
    std::vector<codec::byte_type> encode(pointer geometry)
    {
        visitors::fce<CurvePolicy, CompressPolicy> visitor;
        geometry->applyComponentFilter(visitor);
        return visitor.output();
    }

    template<
      typename CurvePolicy    = codec::hilbert,
      typename CompressPolicy = codec::varint,
      std::enable_if_t<
        traits::is_codec<CurvePolicy> && traits::is_codec<CompressPolicy>,
        bool> = true>
    pointer decode(std::vector<codec::byte_type>&);
};

} // namespace fce

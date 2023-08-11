#pragma once

#include <vector>
#include <variant>

#define FCE_DECLARE_DEFAULT_CONSTRUCTORS(cls) \
    cls()                      = default;     \
    cls(const cls&)            = default;     \
    cls& operator=(const cls&) = default;     \
    cls(cls&&)                 = default;     \
    cls& operator=(cls&&)      = default;

#define FCE_DECLARE_FORWARD_CONSTRUCTOR(cls, mbr) \
    template<typename... Args>                    \
    cls(Args&&... args)                           \
      : mbr(std::forward<Args>(args)...){};

namespace fce {

namespace model {

struct point
{
    using value_type      = double;
    using reference       = value_type&;
    using const_reference = const value_type&;

    FCE_DECLARE_DEFAULT_CONSTRUCTORS(point)

    point(value_type x, value_type y)
      : x_(x)
      , y_(y){};

    reference       x() noexcept { return x_; }
    reference       y() noexcept { return y_; }
    const_reference x() const noexcept { return x_; }
    const_reference y() const noexcept { return y_; }

  protected:
    value_type x_;
    value_type y_;
};

struct linestring
{
    FCE_DECLARE_DEFAULT_CONSTRUCTORS(linestring)
    FCE_DECLARE_FORWARD_CONSTRUCTOR(linestring, points_)

  private:
    std::vector<point> points_;
};

struct polygon
{
    FCE_DECLARE_DEFAULT_CONSTRUCTORS(polygon)
    FCE_DECLARE_FORWARD_CONSTRUCTOR(polygon, rings_)

  private:
    std::vector<linestring> rings_;
};

struct multipoint
{
    FCE_DECLARE_DEFAULT_CONSTRUCTORS(multipoint)
    FCE_DECLARE_FORWARD_CONSTRUCTOR(multipoint, points_)

  private:
    std::vector<point> points_;
};

struct multilinestring
{
    FCE_DECLARE_DEFAULT_CONSTRUCTORS(multilinestring)
    FCE_DECLARE_FORWARD_CONSTRUCTOR(multilinestring, lines_)

  private:
    std::vector<linestring> lines_;
};

struct multipolygon
{
    FCE_DECLARE_DEFAULT_CONSTRUCTORS(multipolygon)
    FCE_DECLARE_FORWARD_CONSTRUCTOR(multipolygon, polygons_)

  private:
    std::vector<polygon> polygons_;
};

using empty = std::monostate;

struct geometry
{

    template<typename Tp>
    geometry(Tp geom)
      : data_(geom){};

  private:
    using variant_type = std::variant<
      empty,
      point,
      linestring,
      polygon,
      multipoint,
      multilinestring,
      multipolygon>;

    variant_type data_;
};

} // namespace model
} // namespace fce

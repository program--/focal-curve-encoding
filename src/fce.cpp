#include "cpp11/named_arg.hpp"
#include "fce/codec/types.hpp"
#include <cpp11.hpp>
#include <fce/codec.hpp>
#include <numeric>

[[cpp11::register]] cpp11::list FCE_encode_(
  cpp11::doubles x,
  cpp11::doubles y,
  size_t         n,
  double         xmin,
  double         xmax,
  double         ymin,
  double         ymax
)
{
    const fce::codec::size_type            pts = x.size();
    std::vector<fce::codec::unsigned_type> h(pts);
    for (fce::codec::size_type i = 0; i < pts; i++) {
        const auto gx = fce::codec::grid::encode(1 << n, x[i], xmax, xmin);
        const auto gy = fce::codec::grid::encode(1 << n, y[i], ymax, ymin);
        h[i]          = fce::codec::hilbert::encode(1 << n, gx, gy);

        // Rprintf("(%f, %f) -> (%zu, %zu) -> (%zu)\n", x[i], y[i], gx, gy,
        // h[i]);
    }

    std::vector<fce::codec::unsigned_type> order(pts);
    std::iota(order.begin(), order.end(), 0);

    std::sort(order.begin(), order.end(), [&](size_t i, size_t j) {
        return h[i] < h[j];
    });

    std::vector<fce::codec::unsigned_type> h_diff(pts);
    std::transform(order.begin(), order.end(), h_diff.begin(), [&](size_t i) {
        return h[i];
    });

    std::adjacent_difference(h_diff.begin(), h_diff.end(), h_diff.begin());

    std::vector<fce::codec::byte_type>     out;
    std::array<fce::codec::byte_type, 512> buf;
    for (auto& v : h_diff) {
        const auto sz = fce::codec::varint::encode(v, buf.data());

        // Rprintf("value %zu = [", v);
        // for (size_t i = 0; i < sz; i++) {
        //     Rprintf("%x", buf[i]);
        // }
        // Rprintf("]\n");

        out.insert(std::end(out), std::begin(buf), std::begin(buf) + sz);
    }

    std::vector<fce::codec::byte_type> order_out;
    for (auto& v : order) {
        const auto sz = fce::codec::varint::encode(v, buf.data());
        order_out.insert(
          std::end(order_out), std::begin(buf), std::begin(buf) + sz
        );
    }

    // Rprintf("Output: ===================\n");
    // for (auto& b : out) {
    //     Rprintf("%x", b);
    // }

    using cpp11::literals::operator""_nm;
    cpp11::writable::raws r_out{ std::move(out) };
    cpp11::writable::raws r_order{ std::move(order_out) };
    cpp11::writable::list output{ "data"_nm = r_out, "order"_nm = r_order };

    return output;
}

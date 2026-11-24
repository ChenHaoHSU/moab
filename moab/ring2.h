#ifndef MOAB_RING2_H_
#define MOAB_RING2_H_

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/numeric/int128.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "boost/geometry.hpp"
#include "boost/polygon/polygon.hpp"
#include "moab/box2.h"
#include "moab/operation.h"
#include "moab/point2.h"

namespace moab {

namespace bg = boost::geometry;
namespace gtl = boost::polygon;

template <typename T>
class Ring2 {
 public:
  // Type aliases.
  using base_type = std::vector<Point2<T>>;
  using coordinate_type = T;
  using point_type = Point2<T>;
  // Area type. int64_t for integral coordinates to avoid overflow.
  using area_type = std::conditional_t<std::is_integral_v<T>, int64_t, T>;
  using iterator_type = typename base_type::const_iterator;
  using const_iterator_type = typename base_type::const_iterator;
  using mutable_iterator_type = typename base_type::iterator;
  using const_reverse_iterator_type =
      typename base_type::const_reverse_iterator;
  using mutable_reverse_iterator_type = typename base_type::reverse_iterator;

  // Constructors.
  Ring2() = default;
  explicit Ring2(const std::vector<Point2<T>>& v) : d_(v) {}
  explicit Ring2(std::vector<Point2<T>>&& v) : d_(std::move(v)) {}
  explicit Ring2(const Box2<T>& b)
      : d_({Point2<T>(b.MinX(), b.MinY()), Point2<T>(b.MaxX(), b.MinY()),
            Point2<T>(b.MaxX(), b.MaxY()), Point2<T>(b.MinX(), b.MaxY()),
            Point2<T>(b.MinX(), b.MinY())}) {}
  Ring2(std::initializer_list<Point2<T>> il) : d_(il) {}
  Ring2(const Ring2&) = default;
  Ring2(Ring2&&) = default;
  ~Ring2() = default;

  // Assignment operators.
  Ring2& operator=(const Ring2&) = default;
  Ring2& operator=(Ring2&&) = default;

  // Accessors.
  std::vector<Point2<T>>& Points() { return d_; }
  const std::vector<Point2<T>>& Points() const { return d_; }
  size_t Size() const { return d_.size(); }
  // Required by boost::polygon::polygon_traits_general.
  size_t size() const { return d_.size(); }
  bool Empty() const { return d_.empty(); }

  // Returns the area of the ring, the same for either orientation. For integer
  // coordinates a fractional area is rounded down (a triangle of area 1.5
  // gives 1).
  area_type Area() const { return static_cast<area_type>(gtl::area(*this)); }
  // Returns the centroid of the ring. For integer coordinates Boost.Geometry
  // computes it with integer arithmetic, so the result is approximate (within
  // about one unit) and not consistently rounded; use Ring2<double> for an
  // exact centroid.
  Point2<T> Centroid() const {
    Point2<T> p;
    bg::centroid(*this, p);
    return p;
  }
  // Returns a point guaranteed to lie inside the ring (on its surface).
  Point2<T> PointOnSurface() const {
    Point2<T> p;
    bg::point_on_surface(*this, p);
    return p;
  }
  // Returns the smallest box that contains the ring. An empty ring has no
  // extent and gives the zero box (0 0)-(0 0).
  Box2<T> BoundingBox() const {
    if (d_.empty()) {
      return Box2<T>();
    }
    Box2<T> b;
    bg::envelope(*this, b);
    return b;
  }
  // Returns true if every edge, including the one from the last point back to
  // the first, is horizontal or vertical (or has zero length). An empty ring
  // is Manhattan.
  bool IsManhattan() const;
  // Returns the maximum boxes that cover the ring. The ring must be Manhattan
  // (see IsManhattan). Boost.Polygon reads a diagonal edge as if it were
  // axis-parallel, so for other rings the boxes are wrong and may miss part
  // of the ring or reach outside it; check IsManhattan first when the input
  // can have diagonal edges, such as shapes read from GDS.
  std::vector<Box2<T>> MaxBoxes() const noexcept {
    std::vector<Box2<T>> boxes;
    gtl::get_max_rectangles(boxes,
                            gtl::view_as<gtl::polygon_90_set_concept>(*this));
    return boxes;
  }

  // Mutators.
  // Avoid using these methods if possible. They are designed for Boost
  // polygon/geometry registration.
  void Clear() { d_.clear(); }
  void Append(const Point2<T>& p) { d_.push_back(p); }
  void Resize(std::size_t n) { d_.resize(n); }

  template <typename InputIt>
  void Assign(InputIt first, InputIt last) {
    d_.assign(first, last);
  }
  void Assign(std::initializer_list<Point2<T>> il) { d_.assign(il); }

  // Operations.
  // Operations - Bloat
  // Returns the ring's max boxes (see MaxBoxes) bloated by 'x' and 'y', as
  // rings. 'x' and 'y' must be non-negative, and the ring must be Manhattan.
  std::vector<Ring2<T>> BloatedRings(T x, T y) const;
  // Operations - Simplify
  // Removes the redundant points of the ring and returns *this: consecutive
  // duplicates, and vertices on the straight line through their neighbors
  // (collinear points, including the tip of a zero-width spike), also across
  // the seam where the ring closes. The ring may be open or closed; the result
  // is closed (its last point repeats its first) and starts at the first input
  // vertex that remains. A ring left with fewer than three vertices has no
  // area and becomes empty. For integer coordinates the collinearity test is
  // exact for any int32 coordinates and for int64 coordinates up to 2^62 in
  // magnitude; for floating-point coordinates a point counts as collinear only
  // if the cross product is exactly zero.
  Ring2& Simplify();

  // Iterators.
  mutable_iterator_type begin() { return d_.begin(); }
  mutable_iterator_type end() { return d_.end(); }
  const_iterator_type begin() const { return d_.cbegin(); }
  const_iterator_type end() const { return d_.cend(); }
  mutable_reverse_iterator_type rbegin() { return d_.rbegin(); }
  mutable_reverse_iterator_type rend() { return d_.rend(); }
  const_reverse_iterator_type rbegin() const { return d_.crbegin(); }
  const_reverse_iterator_type rend() const { return d_.crend(); }

  // Operators.
  // Operator - Subscript
  Point2<T>& operator[](std::size_t i) {
    DCHECK_LT(i, d_.size());
    return d_[i];
  }
  const Point2<T>& operator[](std::size_t i) const {
    DCHECK_LT(i, d_.size());
    return d_[i];
  }
  // Operator - Equality
  // Returns true if the rings cover the same area (see IsEqual), even if their
  // point lists differ, e.g. start at a different point. See the note on
  // AbslHashValue.
  bool operator==(const Ring2& r) const { return moab::IsEqual(*this, r); }
  bool operator!=(const Ring2& r) const { return !(*this == r); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Ring2& r) {
    absl::Format(&sink, "(%s)", absl::StrJoin(r.d_, " "));
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Ring2& r) {
    os << r.ToString();
    return os;
  }

  // Hash.
  // Hashes the bounding box and the area, which rings equal under operator==
  // share however their point lists are written (start point, direction,
  // extra collinear points), so equal rings hash equally. Rings with the same
  // box and area collide. Hashing is O(n) in the number of points. For
  // floating-point coordinates, rings that IsEqual treats as equal within its
  // tolerance may still hash differently.
  template <typename H>
  friend H AbslHashValue(H h, const Ring2& r) {
    return H::combine(std::move(h), r.BoundingBox(), r.Area());
  }

 private:
  // Returns true if 'b' repeats 'a' or lies on the straight line through 'a'
  // and 'c'.
  static bool IsRedundant(const Point2<T>& a, const Point2<T>& b,
                          const Point2<T>& c);

  std::vector<Point2<T>> d_;
};

// Aliases.
using Ring2_i = Ring2<int>;
using Ring2_i32 = Ring2<int32_t>;
using Ring2_i64 = Ring2<int64_t>;

template <typename T>
bool Ring2<T>::IsRedundant(const Point2<T>& a, const Point2<T>& b,
                           const Point2<T>& c) {
  if constexpr (std::is_integral_v<T>) {
    // Products of int32 differences reach 2^64 and of int64 differences
    // 2^126, past int64_t, so compute in 128 bits.
    const absl::int128 cross =
        (absl::int128(b.x()) - a.x()) * (absl::int128(c.y()) - b.y()) -
        (absl::int128(b.y()) - a.y()) * (absl::int128(c.x()) - b.x());
    return cross == 0;
  } else {
    return (b.x() - a.x()) * (c.y() - b.y()) -
               (b.y() - a.y()) * (c.x() - b.x()) ==
           0;
  }
}

template <typename T>
bool Ring2<T>::IsManhattan() const {
  for (std::size_t i = 0; i < d_.size(); ++i) {
    const Point2<T>& a = d_[i];
    const Point2<T>& b = d_[i + 1 < d_.size() ? i + 1 : 0];
    if (a.x() != b.x() && a.y() != b.y()) {
      return false;
    }
  }
  return true;
}

template <typename T>
Ring2<T>& Ring2<T>::Simplify() {
  std::vector<Point2<T>> out;
  out.reserve(d_.size());
  for (const Point2<T>& p : d_) {
    if (!out.empty() && out.back() == p) {
      continue;
    }
    while (out.size() >= 2 && IsRedundant(out[out.size() - 2], out.back(), p)) {
      out.pop_back();
    }
    out.push_back(p);
  }
  // Drop the closing point, then the vertices around the seam (the last and
  // the first) while they are redundant.
  if (out.size() >= 2 && out.back() == out.front()) {
    out.pop_back();
  }
  std::size_t first = 0;
  bool changed = true;
  while (changed && out.size() - first >= 3) {
    changed = false;
    if (IsRedundant(out[out.size() - 2], out.back(), out[first])) {
      out.pop_back();
      changed = true;
    } else if (IsRedundant(out.back(), out[first], out[first + 1])) {
      ++first;
      changed = true;
    }
  }
  out.erase(out.begin(), out.begin() + first);
  if (out.size() < 3) {
    out.clear();
  } else {
    out.push_back(out.front());
  }
  d_ = std::move(out);
  return *this;
}

template <typename T>
std::vector<Ring2<T>> Ring2<T>::BloatedRings(T x, T y) const {
  DCHECK(x >= 0 && y >= 0) << "Bloat values must be non-negative.";
  std::vector<Box2<T>> bloated_boxes;
  for (moab::Box2<T> box : MaxBoxes()) {
    bloated_boxes.push_back(box.Bloat(x, y));
  }
  std::vector<Ring2<T>> bloated_rings;
  moab::Assign(bloated_rings, bloated_boxes);
  return bloated_rings;
}

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

template <typename T>
struct tag<moab::Ring2<T>> {
  using type = ring_tag;
};

template <typename T>
struct clear<moab::Ring2<T>> {
  static inline void apply(moab::Ring2<T>& r) { r.Clear(); }
};

template <typename T>
struct push_back<moab::Ring2<T>> {
  using point_type = typename moab::Ring2<T>::point_type;

  static inline void apply(moab::Ring2<T>& r, const point_type& p) {
    r.Append(p);
  }
};

template <typename T>
struct resize<moab::Ring2<T>> {
  static inline void apply(moab::Ring2<T>& r, std::size_t n) { r.Resize(n); }
};

template <typename T>
struct point_order<moab::Ring2<T>> {
  static const order_selector value = counterclockwise;
};

template <typename T>
struct closure<moab::Ring2<T>> {
  static const closure_selector value = closed;
};

}  // namespace boost::geometry::traits

// Boost range traits.
namespace boost {

template <typename T>
struct range_mutable_iterator<moab::Ring2<T>> {
  using type = typename moab::Ring2<T>::mutable_iterator_type;
};

template <typename T>
struct range_const_iterator<moab::Ring2<T>> {
  using type = typename moab::Ring2<T>::const_iterator_type;
};

template <typename T>
struct range_size<moab::Ring2<T>> {
  using type = std::size_t;
};

}  // namespace boost

// Boost polygon traits.
namespace boost::polygon {

template <typename T>
struct geometry_concept<moab::Ring2<T>> {
  using type = polygon_concept;
};

template <typename T>
struct polygon_mutable_traits<moab::Ring2<T>> {
  template <typename iT>
  static inline moab::Ring2<T>& set_points(moab::Ring2<T>& r, iT input_begin,
                                           iT input_end) {
    r.Clear();
    while (input_begin != input_end) {
      r.Append(moab::Point2<T>(input_begin->x(), input_begin->y()));
      ++input_begin;
    }
    return r;
  }
};

}  // namespace boost::polygon

#endif  // MOAB_RING2_H_
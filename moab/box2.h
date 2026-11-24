#ifndef MOAB_BOX2_H_
#define MOAB_BOX2_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "absl/log/check.h"
#include "absl/status/status.h"
#include "absl/status/status_macros.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/geometry/core/access.hpp"
#include "boost/geometry/core/coordinate_dimension.hpp"
#include "boost/geometry/core/coordinate_system.hpp"
#include "boost/geometry/core/cs.hpp"
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/geometries/concepts/box_concept.hpp"
#include "boost/polygon/polygon.hpp"
#include "moab/box2.pb.h"
#include "moab/coordinate_conversion.h"
#include "moab/interval.h"
#include "moab/point2.h"
#include "moab/point2.pb.h"

namespace moab {

namespace gtl = boost::polygon;

// Axis-aligned box with min corner (xl, yl) and max corner (xh, yh), where
// xl <= xh and yl <= yh.
//
// The corners can only be changed through the constructors, the setters,
// Bloat, Shrink and Encompass, which keep them ordered.
// Boost's traits write coordinates directly and one at a time, and some
// Boost.Geometry algorithms start from an inside-out box, so a box may be
// inverted while Boost updates it; reading its size then is a bug, which Width
// and Height (and so Area and the perimeters) check in debug builds.
template <typename T>
class Box2 {
 public:
  // Type aliases. (Required by Boost geometry/polygon traits.)
  using coordinate_type = T;
  using point_type = Point2<T>;
  using interval_type = Interval<T>;
  // Area type. int64_t for integral coordinates to avoid overflow.
  using area_type = std::conditional_t<std::is_integral_v<T>, int64_t, T>;

  // Constructors.
  Box2() : d_({Point2<T>(0, 0), Point2<T>(0, 0)}) {}
  explicit Box2(const Point2<T>& p1, const Point2<T>& p2) { Set(p1, p2); }
  explicit Box2(T xl, T yl, T xh, T yh) { Set(xl, yl, xh, yh); }
  // Aborts if 'proto' is invalid (see FromProto). Prefer FromProto.
  explicit Box2(const Box2Proto& proto) { CHECK_OK(SetFromProto(proto)); }
  Box2(const Box2&) = default;
  Box2(Box2&&) = default;
  ~Box2() = default;

  // Assignment operators.
  Box2& operator=(const Box2&) = default;
  Box2& operator=(Box2&&) = default;

  // Accessors.
  const Point2<T>& ll() const { return d_[0]; }
  const Point2<T>& ur() const { return d_[1]; }

  const Point2<T>& MinCorner() const { return d_[0]; }
  const Point2<T>& MaxCorner() const { return d_[1]; }

  T xl() const { return d_[0].x(); }
  T yl() const { return d_[0].y(); }
  T xh() const { return d_[1].x(); }
  T yh() const { return d_[1].y(); }

  T MinX() const { return d_[0].x(); }
  T MinY() const { return d_[0].y(); }
  T MaxX() const { return d_[1].x(); }
  T MaxY() const { return d_[1].y(); }

  T Width() const {
    DCHECK_LE(d_[0].x(), d_[1].x()) << "Box is inverted in x.";
    return d_[1].x() - d_[0].x();
  }
  T Height() const {
    DCHECK_LE(d_[0].y(), d_[1].y()) << "Box is inverted in y.";
    return d_[1].y() - d_[0].y();
  }

  std::tuple<T, T, T, T> ToTuple() const {
    return std::make_tuple(d_[0].x(), d_[0].y(), d_[1].x(), d_[1].y());
  }

  // Center.
  T CenterX() const { return Center().x(); }
  T CenterY() const { return Center().y(); }
  Point2<T> Center() const {
    Point2<T> p;
    gtl::center(p, *this);
    return p;
  }

  area_type Area() const {
    return static_cast<area_type>(Width()) * static_cast<area_type>(Height());
  }
  T HalfPerimeter() const { return (Width() + Height()); }
  T Perimeter() const { return 2 * (Width() + Height()); }

  // Mutators.
  void Set(T xl, T yl, T xh, T yh) {
    // Ensure that the box is valid. Automatically swap if necessary.
    d_[0].Set((xl < xh ? xl : xh), (yl < yh ? yl : yh));
    d_[1].Set((xl < xh ? xh : xl), (yl < yh ? yh : yl));
  }
  void Set(const Point2<T>& ll, const Point2<T>& ur) {
    Set(ll.x(), ll.y(), ur.x(), ur.y());
  }

  void set_xl(T xl) { Set(xl, yl(), xh(), yh()); }
  void set_yl(T yl) { Set(xl(), yl, xh(), yh()); }
  void set_xh(T xh) { Set(xl(), yl(), xh, yh()); }
  void set_yh(T yh) { Set(xl(), yl(), xh(), yh); }
  void set_ll(const Point2<T>& p) { Set(p.x(), p.y(), xh(), yh()); }
  void set_ur(const Point2<T>& p) { Set(xl(), yl(), p.x(), p.y()); }

  void SetMinCorner(const Point2<T>& p) { Set(p.x(), p.y(), xh(), yh()); }
  void SetMaxCorner(const Point2<T>& p) { Set(xl(), yl(), p.x(), p.y()); }

  // Operations.
  // Operations - Shift (It's safe. No need to check for validity.)
  void Shift(T dx, T dy) {
    d_[0].Shift(dx, dy);
    d_[1].Shift(dx, dy);
  }
  void ShiftX(T dx) {
    d_[0].ShiftX(dx);
    d_[1].ShiftX(dx);
  }
  void ShiftY(T dy) {
    d_[0].ShiftY(dy);
    d_[1].ShiftY(dy);
  }
  // Operations - Bloat
  // Moves both sides of an axis outward by the given amount, or inward if it
  // is negative. An axis shrunk past zero width collapses to zero width at its
  // center, lo + (hi - lo) / 2 (rounded toward lo for integers), instead of
  // inverting.
  Box2& Bloat(T d) { return Bloat(d, d); }
  Box2& Bloat(T dx, T dy) {
    BloatX(dx);
    BloatY(dy);
    return *this;
  }
  Box2& BloatX(T dx) {
    BloatAxis(0, dx);
    return *this;
  }
  Box2& BloatY(T dy) {
    BloatAxis(1, dy);
    return *this;
  }
  // Operations - Shrink
  // Moves both sides of an axis inward by the given amount. Shrink(d) is
  // Bloat(-d), so an axis shrunk past zero width collapses to its center.
  Box2& Shrink(T d) { return Bloat(-d); }
  Box2& Shrink(T dx, T dy) { return Bloat(-dx, -dy); }
  Box2& ShrinkX(T dx) { return BloatX(-dx); }
  Box2& ShrinkY(T dy) { return BloatY(-dy); }
  // Operations - Encompass
  bool Encompass(const Point2<T>& p) { return gtl::encompass(*this, p); }
  bool Encompass(const Box2& b) { return gtl::encompass(*this, b); }

  // Operators.
  // Operators - Subscript
  const Point2<T>& operator[](std::size_t i) const {
    DCHECK_LT(i, d_.size());
    return d_[i];
  }
  // Operators - Equality
  bool operator==(const Box2& b) const {
    return d_[0] == b.d_[0] && d_[1] == b.d_[1];
  }
  bool operator!=(const Box2& b) const { return !(*this == b); }
  // Operators - Inequality
  bool operator<(const Box2& b) const {
    return d_[0] != b.d_[0] ? d_[0] < b.d_[0] : d_[1] < b.d_[1];
  }
  bool operator>(const Box2& b) const { return b < *this; }
  bool operator<=(const Box2& b) const { return !(*this > b); }
  bool operator>=(const Box2& b) const { return !(*this < b); }

  // Utilities.
  // Returns the smallest box that contains all 'points', or std::nullopt if
  // 'points' is empty.
  template <typename Container>
  static std::optional<Box2> BoundingBox(const Container& points);
  static std::optional<Box2> BoundingBox(
      std::initializer_list<Point2<T>> points) {
    return BoundingBox<std::initializer_list<Point2<T>>>(points);
  }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Box2& b) {
    absl::Format(&sink, "(%v %v)", b.d_[0], b.d_[1]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Box2& b) {
    os << b.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Box2& b) {
    return H::combine(std::move(h), b.d_[0], b.d_[1]);
  }

  // Protobuf.
  // Returns a Box2Proto.
  Box2Proto ToProto() const;
  // Sets the Box2 from a Box2Proto. On error, returns the error from FromProto
  // and leaves the Box2 unchanged.
  absl::Status SetFromProto(const Box2Proto& proto);
  // Returns the Box2 in 'proto'. Returns InvalidArgument if no coordinate
  // type is set, or if a coordinate is not finite, does not fit in T, or is
  // not a whole number for an integral T.
  static absl::StatusOr<Box2> FromProto(const Box2Proto& proto);

 private:
  // Boost's traits write coordinates directly; see the class comment.
  template <typename Geometry, std::size_t Index, std::size_t Dimension>
  friend struct boost::geometry::traits::indexed_access;
  template <typename Rectangle>
  friend struct boost::polygon::rectangle_mutable_traits;

  // Moves both sides of axis 'dim' outward by 'd' (inward if negative), and
  // collapses the axis to its center rather than inverting it.
  void BloatAxis(std::size_t dim, T d) {
    const T lo = d_[0][dim] - d;
    const T hi = d_[1][dim] + d;
    if (lo <= hi) {
      d_[0][dim] = lo;
      d_[1][dim] = hi;
    } else {
      const T center = d_[0][dim] + (d_[1][dim] - d_[0][dim]) / 2;
      d_[0][dim] = center;
      d_[1][dim] = center;
    }
  }

  std::array<Point2<T>, 2> d_;
};

// Protobuf.
template <typename T>
Box2Proto Box2<T>::ToProto() const {
  Box2Proto proto;
  if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
    proto.mutable_box_int32()->mutable_min_corner()->set_x(d_[0].x());
    proto.mutable_box_int32()->mutable_min_corner()->set_y(d_[0].y());
    proto.mutable_box_int32()->mutable_max_corner()->set_x(d_[1].x());
    proto.mutable_box_int32()->mutable_max_corner()->set_y(d_[1].y());
  } else if constexpr (std::is_same_v<T, int64_t>) {
    proto.mutable_box_int64()->mutable_min_corner()->set_x(d_[0].x());
    proto.mutable_box_int64()->mutable_min_corner()->set_y(d_[0].y());
    proto.mutable_box_int64()->mutable_max_corner()->set_x(d_[1].x());
    proto.mutable_box_int64()->mutable_max_corner()->set_y(d_[1].y());
  } else if constexpr (std::is_same_v<T, float>) {
    proto.mutable_box_float()->mutable_min_corner()->set_x(d_[0].x());
    proto.mutable_box_float()->mutable_min_corner()->set_y(d_[0].y());
    proto.mutable_box_float()->mutable_max_corner()->set_x(d_[1].x());
    proto.mutable_box_float()->mutable_max_corner()->set_y(d_[1].y());
  } else if constexpr (std::is_same_v<T, double>) {
    proto.mutable_box_double()->mutable_min_corner()->set_x(d_[0].x());
    proto.mutable_box_double()->mutable_min_corner()->set_y(d_[0].y());
    proto.mutable_box_double()->mutable_max_corner()->set_x(d_[1].x());
    proto.mutable_box_double()->mutable_max_corner()->set_y(d_[1].y());
  } else {
    static_assert(std::is_same_v<T, int>, "Unsupported type.");
  }
  return proto;
}

template <typename T>
absl::Status Box2<T>::SetFromProto(const Box2Proto& proto) {
  ABSL_ASSIGN_OR_RETURN(*this, FromProto(proto));
  return absl::OkStatus();
}

template <typename T>
absl::StatusOr<Box2<T>> Box2<T>::FromProto(const Box2Proto& proto) {
  const auto from = [](const auto& p) -> absl::StatusOr<Box2<T>> {
    ABSL_ASSIGN_OR_RETURN(
        const T xl, internal::ConvertCoordinate<T>(p.min_corner().x(),
                                                   "[Box2] min_corner.x"));
    ABSL_ASSIGN_OR_RETURN(
        const T yl, internal::ConvertCoordinate<T>(p.min_corner().y(),
                                                   "[Box2] min_corner.y"));
    ABSL_ASSIGN_OR_RETURN(
        const T xh, internal::ConvertCoordinate<T>(p.max_corner().x(),
                                                   "[Box2] max_corner.x"));
    ABSL_ASSIGN_OR_RETURN(
        const T yh, internal::ConvertCoordinate<T>(p.max_corner().y(),
                                                   "[Box2] max_corner.y"));
    return Box2<T>(xl, yl, xh, yh);
  };
  switch (proto.type_variant_case()) {
    case Box2Proto::kBoxInt32:
      return from(proto.box_int32());
    case Box2Proto::kBoxInt64:
      return from(proto.box_int64());
    case Box2Proto::kBoxFloat:
      return from(proto.box_float());
    case Box2Proto::kBoxDouble:
      return from(proto.box_double());
    case Box2Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return absl::InvalidArgumentError(
      "[Box2] The proto has no coordinate type set.");
}

template <typename T>
template <typename Container>
std::optional<Box2<T>> Box2<T>::BoundingBox(const Container& points) {
  static_assert(std::is_same_v<typename Container::value_type, Point2<T>>,
                "Container must hold Point2<T> elements.");
  if (points.begin() == points.end()) {
    return std::nullopt;
  }
  Box2<T> box(*points.begin(), *points.begin());
  for (const Point2<T>& p : points) {
    box.Encompass(p);
  }
  return box;
}

// Aliases.
using Box2_i = Box2<int>;
using Box2_i32 = Box2<int32_t>;
using Box2_i64 = Box2<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

template <typename T>
struct tag<moab::Box2<T>> {
  using type = box_tag;
};

template <typename T>
struct point_type<moab::Box2<T>> {
  using type = typename moab::Box2<T>::point_type;
};

// Index 0: min corner, Index 1: max corner.
// Dimension 0: x, Dimension 1: y.
template <typename T, std::size_t Index, std::size_t Dimension>
struct indexed_access<moab::Box2<T>, Index, Dimension> {
  using box_type = moab::Box2<T>;
  using coordinate_type = typename box_type::coordinate_type;

  static inline coordinate_type get(const box_type& b) {
    return b[Index][Dimension];
  }
  static inline void set(box_type& b, const coordinate_type& value) {
    b.d_[Index][Dimension] = value;
  }
};

}  // namespace boost::geometry::traits

// Boost polygon traits.
namespace boost::polygon {

template <typename T>
struct geometry_concept<moab::Box2<T>> {
  using type = rectangle_concept;
};

template <typename T>
struct rectangle_traits<
    moab::Box2<T>,
    typename gtl_same_type<typename moab::Box2<T>::interval_type,
                           typename moab::Box2<T>::interval_type>::type> {
  using coordinate_type = typename moab::Box2<T>::coordinate_type;
  using interval_type = typename moab::Box2<T>::interval_type;

  static inline interval_type get(const moab::Box2<T>& rectangle,
                                  orientation_2d orient) {
    return interval_mutable_traits<interval_type>::construct(
        rectangle[0][orient.to_int()], rectangle[1][orient.to_int()]);
  }
};

template <typename T>
struct rectangle_mutable_traits<moab::Box2<T>> {
  using coordinate_type = typename moab::Box2<T>::coordinate_type;
  using interval_type = typename moab::Box2<T>::interval_type;

  template <typename T2>
  static inline void set(moab::Box2<T>& rectangle, orientation_2d orient,
                         const T2& interval) {
    rectangle.d_[0][orient.to_int()] = interval_traits<T2>::get(interval, LOW);
    rectangle.d_[1][orient.to_int()] = interval_traits<T2>::get(interval, HIGH);
  }
  template <typename T2, typename T3>
  static inline moab::Box2<T> construct(const T2& interval_horizontal,
                                        const T3& interval_vertical) {
    return moab::Box2<T>(interval_traits<T2>::get(interval_horizontal, LOW),
                         interval_traits<T3>::get(interval_vertical, LOW),
                         interval_traits<T2>::get(interval_horizontal, HIGH),
                         interval_traits<T3>::get(interval_vertical, HIGH));
  }
};

}  // namespace boost::polygon

#endif  // MOAB_BOX2_H_

#ifndef MOAB_SEGMENT2_H_
#define MOAB_SEGMENT2_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include "absl/hash/hash.h"
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
#include "boost/geometry/geometries/concepts/segment_concept.hpp"
#include "boost/polygon/polygon.hpp"
#include "moab/coordinate_conversion.h"
#include "moab/point2.h"
#include "moab/segment2.pb.h"

namespace moab {

template <typename T>
class Segment2 {
 public:
  // Type aliases. (Required by Boost geometry/polygon traits.)
  using coordinate_type = T;
  using point_type = Point2<T>;

  // Constructors.
  constexpr Segment2() : d_({Point2<T>(0, 0), Point2<T>(0, 0)}) {}
  explicit constexpr Segment2(const Point2<T>& p0, const Point2<T>& p1) {
    Set(p0, p1);
  }
  explicit constexpr Segment2(T x0, T y0, T x1, T y1) { Set(x0, y0, x1, y1); }
  // Aborts if 'proto' is invalid (see FromProto). Prefer FromProto.
  explicit Segment2(const Segment2Proto& proto) {
    CHECK_OK(SetFromProto(proto));
  }
  Segment2(const Segment2&) = default;
  Segment2(Segment2&&) = default;
  ~Segment2() = default;

  // Assignment operators.
  Segment2& operator=(const Segment2&) = default;
  Segment2& operator=(Segment2&&) = default;

  // Accessors.
  constexpr Point2<T>& p0() { return d_[0]; }
  constexpr const Point2<T>& p0() const { return d_[0]; }
  constexpr Point2<T>& p1() { return d_[1]; }
  constexpr const Point2<T>& p1() const { return d_[1]; }
  constexpr const Point2<T>* data() const { return d_.data(); }

  constexpr std::size_t Size() const { return d_.size(); }

  constexpr std::pair<Point2<T>, Point2<T>> ToPair() const {
    return {d_[0], d_[1]};
  }

  // Returns the Manhattan (L1) length, i.e. p0().ManhattanDistance(p1()). Not
  // the Euclidean length.
  constexpr T ManhattanLength() const { return d_[0].ManhattanDistance(d_[1]); }

  // The minimum and maximum coordinates over the two endpoints. The endpoints
  // are not sorted, so xl() is not necessarily p0().x().
  constexpr T xl() const {
    return d_[0].x() < d_[1].x() ? d_[0].x() : d_[1].x();
  }
  constexpr T yl() const {
    return d_[0].y() < d_[1].y() ? d_[0].y() : d_[1].y();
  }
  constexpr T xh() const {
    return d_[0].x() > d_[1].x() ? d_[0].x() : d_[1].x();
  }
  constexpr T yh() const {
    return d_[0].y() > d_[1].y() ? d_[0].y() : d_[1].y();
  }

  constexpr T MinX() const { return xl(); }
  constexpr T MinY() const { return yl(); }
  constexpr T MaxX() const { return xh(); }
  constexpr T MaxY() const { return yh(); }

  // Returns the lexicographically smaller (MinPoint) or larger (MaxPoint)
  // endpoint, comparing x then y. When the endpoints are equal, both return
  // p1().
  constexpr Point2<T>& MinPoint() { return d_[0] < d_[1] ? d_[0] : d_[1]; }
  constexpr const Point2<T>& MinPoint() const {
    return d_[0] < d_[1] ? d_[0] : d_[1];
  }
  constexpr Point2<T>& MaxPoint() { return d_[0] > d_[1] ? d_[0] : d_[1]; }
  constexpr const Point2<T>& MaxPoint() const {
    return d_[0] > d_[1] ? d_[0] : d_[1];
  }

  // Mutators.
  constexpr void Set(T x0, T y0, T x1, T y1) {
    d_[0].Set(x0, y0);
    d_[1].Set(x1, y1);
  }
  constexpr void Set(const Point2<T>& p0, const Point2<T>& p1) {
    d_[0] = p0;
    d_[1] = p1;
  }
  constexpr void SetP0(const Point2<T>& p) { d_[0] = p; }
  constexpr void SetP1(const Point2<T>& p) { d_[1] = p; }
  constexpr void SetP(std::size_t i, const Point2<T>& p) {
    DCHECK(i < 2) << "Invalid index. i: " << i;
    d_[i] = p;
  }

  // Operations.
  constexpr void Shift(T dx, T dy) {
    d_[0].Shift(dx, dy);
    d_[1].Shift(dx, dy);
  }
  constexpr void ShiftX(T dx) {
    d_[0].ShiftX(dx);
    d_[1].ShiftX(dx);
  }
  constexpr void ShiftY(T dy) {
    d_[0].ShiftY(dy);
    d_[1].ShiftY(dy);
  }

  // Operators.
  // Operators - Subscript
  constexpr Point2<T>& operator[](std::size_t i) {
    DCHECK_LT(i, d_.size());
    return d_[i];
  }
  constexpr const Point2<T>& operator[](std::size_t i) const {
    DCHECK_LT(i, d_.size());
    return d_[i];
  }
  // Operators - Equality
  constexpr bool operator==(const Segment2& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1];
  }
  constexpr bool operator!=(const Segment2& p) const { return !(*this == p); }
  // Operators - Inequality
  // Lexicographic order, for sorting and ordered containers; not a geometric
  // comparison. Compares p0, then p1.
  constexpr bool operator<(const Segment2& p) const {
    return d_[0] != p.d_[0] ? d_[0] < p.d_[0] : d_[1] < p.d_[1];
  }
  constexpr bool operator>(const Segment2& p) const { return p < *this; }
  constexpr bool operator<=(const Segment2& p) const { return !(*this > p); }
  constexpr bool operator>=(const Segment2& p) const { return !(*this < p); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Segment2& s) {
    absl::Format(&sink, "(%v %v)", s.d_[0], s.d_[1]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Segment2& s) {
    os << s.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Segment2& s) {
    return H::combine(std::move(h), s.d_[0], s.d_[1]);
  }

  // Protobuf.
  // Returns a Segment2Proto.
  Segment2Proto ToProto() const noexcept;
  // Sets the Segment2 from a Segment2Proto. On error, returns the error from
  // FromProto and leaves the Segment2 unchanged.
  absl::Status SetFromProto(const Segment2Proto& proto);
  // Returns the Segment2 in 'proto'. Returns InvalidArgument if no coordinate
  // type is set, or if a coordinate is not finite, does not fit in T, or is
  // not a whole number for an integral T.
  static absl::StatusOr<Segment2> FromProto(const Segment2Proto& proto);

 private:
  std::array<Point2<T>, 2> d_;  // <p0, p1>
};

// Protobuf.
template <typename T>
Segment2Proto Segment2<T>::ToProto() const noexcept {
  Segment2Proto proto;
  if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
    proto.mutable_segment_int32()->mutable_p0()->set_x(d_[0].x());
    proto.mutable_segment_int32()->mutable_p0()->set_y(d_[0].y());
    proto.mutable_segment_int32()->mutable_p1()->set_x(d_[1].x());
    proto.mutable_segment_int32()->mutable_p1()->set_y(d_[1].y());
  } else if constexpr (std::is_same_v<T, int64_t>) {
    proto.mutable_segment_int64()->mutable_p0()->set_x(d_[0].x());
    proto.mutable_segment_int64()->mutable_p0()->set_y(d_[0].y());
    proto.mutable_segment_int64()->mutable_p1()->set_x(d_[1].x());
    proto.mutable_segment_int64()->mutable_p1()->set_y(d_[1].y());
  } else if constexpr (std::is_same_v<T, float>) {
    proto.mutable_segment_float()->mutable_p0()->set_x(d_[0].x());
    proto.mutable_segment_float()->mutable_p0()->set_y(d_[0].y());
    proto.mutable_segment_float()->mutable_p1()->set_x(d_[1].x());
    proto.mutable_segment_float()->mutable_p1()->set_y(d_[1].y());
  } else if constexpr (std::is_same_v<T, double>) {
    proto.mutable_segment_double()->mutable_p0()->set_x(d_[0].x());
    proto.mutable_segment_double()->mutable_p0()->set_y(d_[0].y());
    proto.mutable_segment_double()->mutable_p1()->set_x(d_[1].x());
    proto.mutable_segment_double()->mutable_p1()->set_y(d_[1].y());
  } else {
    static_assert(std::is_same_v<T, int>, "Unsupported type.");
  }
  return proto;
}

template <typename T>
absl::Status Segment2<T>::SetFromProto(const Segment2Proto& proto) {
  ABSL_ASSIGN_OR_RETURN(*this, FromProto(proto));
  return absl::OkStatus();
}

template <typename T>
absl::StatusOr<Segment2<T>> Segment2<T>::FromProto(const Segment2Proto& proto) {
  const auto from = [](const auto& p) -> absl::StatusOr<Segment2<T>> {
    ABSL_ASSIGN_OR_RETURN(const T x0, internal::ConvertCoordinate<T>(
                                          p.p0().x(), "[Segment2] p0.x"));
    ABSL_ASSIGN_OR_RETURN(const T y0, internal::ConvertCoordinate<T>(
                                          p.p0().y(), "[Segment2] p0.y"));
    ABSL_ASSIGN_OR_RETURN(const T x1, internal::ConvertCoordinate<T>(
                                          p.p1().x(), "[Segment2] p1.x"));
    ABSL_ASSIGN_OR_RETURN(const T y1, internal::ConvertCoordinate<T>(
                                          p.p1().y(), "[Segment2] p1.y"));
    return Segment2<T>(x0, y0, x1, y1);
  };
  switch (proto.type_variant_case()) {
    case Segment2Proto::kSegmentInt32:
      return from(proto.segment_int32());
    case Segment2Proto::kSegmentInt64:
      return from(proto.segment_int64());
    case Segment2Proto::kSegmentFloat:
      return from(proto.segment_float());
    case Segment2Proto::kSegmentDouble:
      return from(proto.segment_double());
    case Segment2Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return absl::InvalidArgumentError(
      "[Segment2] The proto has no coordinate type set.");
}

// Aliases.
using Segment2_i = Segment2<int>;
using Segment2_i32 = Segment2<int32_t>;
using Segment2_i64 = Segment2<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

template <typename T>
struct tag<moab::Segment2<T>> {
  using type = segment_tag;
};

template <typename T>
struct point_type<moab::Segment2<T>> {
  using type = typename moab::Segment2<T>::point_type;
};

template <typename T, std::size_t Index, std::size_t Dimension>
struct indexed_access<moab::Segment2<T>, Index, Dimension> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;

  static inline coordinate_type get(const moab::Segment2<T>& s) {
    return s[Index][Dimension];
  }
  static inline void set(moab::Segment2<T>& s, const coordinate_type& value) {
    s[Index][Dimension] = value;
  }
};

}  // namespace boost::geometry::traits

// Boost polygon traits.
// LOW is p0 and HIGH is p1, as in boost::polygon::segment_data. The endpoints
// are not reordered, so setting LOW then HIGH writes p0 then p1.
namespace boost::polygon {

template <typename T>
struct geometry_concept<moab::Segment2<T>> {
  using type = segment_concept;
};

template <typename T>
struct segment_traits<moab::Segment2<T>> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;

  static inline point_type get(const moab::Segment2<T>& s, direction_1d dir) {
    return s[dir.to_int()];
  }
};

template <typename T>
struct segment_mutable_traits<moab::Segment2<T>> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;

  static inline void set(moab::Segment2<T>& s, direction_1d dir,
                         const point_type& p) {
    s[dir.to_int()] = p;
  }
  static inline moab::Segment2<T> construct(const point_type& p0,
                                            const point_type& p1) {
    return moab::Segment2<T>(p0, p1);
  }
};

}  // namespace boost::polygon

#endif  // MOAB_SEGMENT2_H_

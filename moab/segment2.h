#ifndef MOAB_SEGMENT2_H_
#define MOAB_SEGMENT2_H_

#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

#include "absl/hash/hash.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/geometry/core/access.hpp"
#include "boost/geometry/core/coordinate_dimension.hpp"
#include "boost/geometry/core/coordinate_system.hpp"
#include "boost/geometry/core/cs.hpp"
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/geometries/concepts/segment_concept.hpp"
#include "boost/polygon/polygon.hpp"
#include "moab/point2.h"
#include "moab/segment2.pb.h"

namespace moab {

template <typename T>
class Segment2 {
 public:
  // Type aliases. (Required by Boost geometry/poylgon traits.)
  using coordinate_type = T;
  using point_type = Point2<T>;

  // Constructors.
  Segment2() : d_({Point2<T>(0, 0), Point2<T>(0, 0)}) {}
  explicit Segment2(const Point2<T>& p0, const Point2<T>& p1) { Set(p0, p1); }
  explicit Segment2(T x0, T y0, T x1, T y1) { Set(x0, y0, x1, y1); }
  explicit Segment2(const Segment2Proto& proto) { SetFromProto(proto); }
  Segment2(const Segment2&) = default;
  Segment2(Segment2&&) = default;
  ~Segment2() = default;

  // Assignment operators.
  Segment2& operator=(const Segment2&) = default;
  Segment2& operator=(Segment2&&) = default;

  // Accessors.
  Point2<T>& p0() { return d_[0]; }
  const Point2<T>& p0() const { return d_.at(0); }
  Point2<T>& p1() { return d_[1]; }
  const Point2<T>& p1() const { return d_.at(1); }
  Point2<T>* data() { return d_.data(); }
  const Point2<T>* data() const { return d_.data(); }

  constexpr std::size_t Size() const { return d_.size(); }

  std::pair<Point2<T>, Point2<T>> ToPair() const { return {d_[0], d_[1]}; }

  T Length() const { return d_[0].Distance(d_[1]); }

  T xl() const { return d_[0].x() < d_[1].x() ? d_[0].x() : d_[1].x(); }
  T yl() const { return d_[0].y() < d_[1].y() ? d_[0].y() : d_[1].y(); }
  T xh() const { return d_[0].x() > d_[1].x() ? d_[0].x() : d_[1].x(); }
  T yh() const { return d_[0].y() > d_[1].y() ? d_[0].y() : d_[1].y(); }

  T MinX() const { return xl(); }
  T MinY() const { return yl(); }
  T MaxX() const { return xh(); }
  T MaxY() const { return yh(); }

  Point2<T>& MinPoint() { return d_[0] < d_[1] ? d_[0] : d_[1]; }
  const Point2<T>& MinPoint() const { return d_[0] < d_[1] ? d_[0] : d_[1]; }
  Point2<T>& MaxPoint() { return d_[0] > d_[1] ? d_[0] : d_[1]; }
  const Point2<T>& MaxPoint() const { return d_[0] > d_[1] ? d_[0] : d_[1]; }

  // Mutators.
  void Set(T x0, T y0, T x1, T y1) {
    d_[0].Set(x0, y0);
    d_[1].Set(x1, y1);
  }
  void Set(const Point2<T>& p0, const Point2<T>& p1) {
    d_[0] = p0;
    d_[1] = p1;
  }
  void SetP0(const Point2<T>& p) { d_[0] = p; }
  void SetP1(const Point2<T>& p) { d_[1] = p; }
  void SetP(std::size_t i, const Point2<T>& p) {
    DCHECK(i < 2) << "Invalid index. i: " << i;
    d_[i] = p;
  }

  // Operations.
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

  // Operators.
  // Operators - Subscript
  Point2<T>& operator[](std::size_t i) { return d_[i]; }
  const Point2<T>& operator[](std::size_t i) const { return d_.at(i); }
  // Operators - Equality
  bool operator==(const Segment2& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1];
  }
  bool operator!=(const Segment2& p) const { return !(*this == p); }
  // Operators - Inequality
  bool operator<(const Segment2& p) const {
    return d_[0] != p.d_[0] ? d_[0] < p.d_[0] : d_[1] < p.d_[1];
  }
  bool operator>(const Segment2& p) const { return p < *this; }
  bool operator<=(const Segment2& p) const { return !(*this > p); }
  bool operator>=(const Segment2& p) const { return !(*this < p); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Segment2& s) {
    absl::Format(&sink, "(%v %v)", s.d_.at(0), s.d_.at(1));
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Segment2& s) {
    os << s.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Segment2& s) {
    return H::combine(std::move(h), s.d_.at(0), s.d_.at(1));
  }

  // Protobuf.
  // Returns a Segment2Proto.
  Segment2Proto ToProto() const;
  // Sets the Segment2 from a Segment2Proto.
  void SetFromProto(const Segment2Proto& proto);

 private:
  std::array<Point2<T>, 2> d_;  // <p0, p1>
};

// Protobuf.
template <typename T>
Segment2Proto Segment2<T>::ToProto() const {
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
void Segment2<T>::SetFromProto(const Segment2Proto& proto) {
  if (proto.has_segment_int32()) {
    Set(proto.segment_int32().p0().x(), proto.segment_int32().p0().y(),
        proto.segment_int32().p1().x(), proto.segment_int32().p1().y());
  } else if (proto.has_segment_int64()) {
    Set(proto.segment_int64().p0().x(), proto.segment_int64().p0().y(),
        proto.segment_int64().p1().x(), proto.segment_int64().p1().y());
  } else if (proto.has_segment_float()) {
    Set(proto.segment_float().p0().x(), proto.segment_float().p0().y(),
        proto.segment_float().p1().x(), proto.segment_float().p1().y());
  } else if (proto.has_segment_double()) {
    Set(proto.segment_double().p0().x(), proto.segment_double().p0().y(),
        proto.segment_double().p1().x(), proto.segment_double().p1().y());
  } else {
    LOG(FATAL) << "Unsupported type.";
  }
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

// Boost poylgon traits.
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
    return dir == LOW ? s.MinPoint() : s.MaxPoint();
  }
};

template <typename T>
struct segment_mutable_traits<moab::Segment2<T>> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;

  static inline void set(moab::Segment2<T>& s, direction_1d dir,
                         const point_type& p) {
    point_type& point = (dir == LOW ? s.MinPoint() : s.MaxPoint());
    point = p;
  }
  static inline moab::Segment2<T> construct(const point_type& p0,
                                            const point_type& p1) {
    return moab::Segment2<T>(p0, p1);
  }
};

}  // namespace boost::polygon

#endif  // MOAB_SEGMENT2_H_

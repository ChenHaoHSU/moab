#ifndef MOAB_SEGMENT3_H_
#define MOAB_SEGMENT3_H_

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
#include "moab/point3.h"
#include "moab/segment3.pb.h"

namespace moab {

template <typename T>
class Segment3 {
 public:
  // Type aliases. (Required by Boost geometry traits.)
  using coordinate_type = T;
  using point_type = Point3<T>;

  // Constructors.
  Segment3() : d_({Point3<T>(0, 0, 0), Point3<T>(0, 0, 0)}) {}
  explicit Segment3(const Point3<T>& p0, const Point3<T>& p1) { Set(p0, p1); }
  explicit Segment3(T xl, T yl, T zl, T xh, T yh, T zh) {
    Set(xl, yl, zl, xh, yh, zh);
  }
  explicit Segment3(const Segment3Proto& proto) { SetFromProto(proto); }
  Segment3(const Segment3&) = default;
  Segment3(Segment3&&) = default;
  ~Segment3() = default;

  // Assignment operators.
  Segment3& operator=(const Segment3&) = default;
  Segment3& operator=(Segment3&&) = default;

  // Accessors.
  Point3<T>& p0() { return d_[0]; }
  const Point3<T>& p0() const { return d_.at(0); }
  Point3<T>& p1() { return d_[1]; }
  const Point3<T>& p1() const { return d_.at(1); }
  Point3<T>* data() { return d_.data(); }
  const Point3<T>* data() const { return d_.data(); }

  constexpr std::size_t Size() const { return d_.size(); }

  std::pair<Point3<T>, Point3<T>> ToPair() const { return {d_[0], d_[1]}; }

  T Length() const { return d_[0].Distance(d_[1]); }

  T xl() const { return d_[0].x() < d_[1].x() ? d_[0].x() : d_[1].x(); }
  T yl() const { return d_[0].y() < d_[1].y() ? d_[0].y() : d_[1].y(); }
  T zl() const { return d_[0].z() < d_[1].z() ? d_[0].z() : d_[1].z(); }
  T xh() const { return d_[0].x() > d_[1].x() ? d_[0].x() : d_[1].x(); }
  T yh() const { return d_[0].y() > d_[1].y() ? d_[0].y() : d_[1].y(); }
  T zh() const { return d_[0].z() > d_[1].z() ? d_[0].z() : d_[1].z(); }

  T MinX() const { return xl(); }
  T MinY() const { return yl(); }
  T MinZ() const { return zl(); }
  T MaxX() const { return xh(); }
  T MaxY() const { return yh(); }
  T MaxZ() const { return zh(); }

  Point3<T>& MinPoint() { return d_[0] < d_[1] ? d_[0] : d_[1]; }
  const Point3<T>& MinPoint() const { return d_[0] < d_[1] ? d_[0] : d_[1]; }
  Point3<T>& MaxPoint() { return d_[0] > d_[1] ? d_[0] : d_[1]; }
  const Point3<T>& MaxPoint() const { return d_[0] > d_[1] ? d_[0] : d_[1]; }

  // Mutators.
  void Set(T x0, T y0, T z0, T x1, T y1, T z1) {
    d_[0].Set(x0, y0, z0);
    d_[1].Set(x1, y1, z1);
  }
  void Set(const Point3<T>& p0, const Point3<T>& p1) {
    d_[0] = p0;
    d_[1] = p1;
  }
  void SetP0(const Point3<T>& p) { d_[0] = p; }
  void SetP1(const Point3<T>& p) { d_[1] = p; }
  void SetP(std::size_t i, const Point3<T>& p) {
    DCHECK(i < 2) << "Invalid SetP Index i" << i;
    d_[i] = p;
  }

  // Operations.
  // It's safe. No need to check for validity.
  void Shift(T dx, T dy, T dz) {
    d_[0].Shift(dx, dy, dz);
    d_[1].Shift(dx, dy, dz);
  }
  void ShiftX(T dx) {
    d_[0].ShiftX(dx);
    d_[1].ShiftX(dx);
  }
  void ShiftY(T dy) {
    d_[0].ShiftY(dy);
    d_[1].ShiftY(dy);
  }
  void ShiftZ(T dz) {
    d_[0].ShiftZ(dz);
    d_[1].ShiftZ(dz);
  }

  // Operators.
  // Operators - Subscript
  Point3<T>& operator[](std::size_t i) { return d_[i]; }
  const Point3<T>& operator[](std::size_t i) const { return d_.at(i); }
  // Operators - Equality
  bool operator==(const Segment3& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1];
  }
  bool operator!=(const Segment3& p) const { return !(*this == p); }
  // Operators - Inequality
  bool operator<(const Segment3& p) const {
    return d_[0] != p.d_[0] ? d_[0] < p.d_[0] : d_[1] < p.d_[1];
  }
  bool operator>(const Segment3& p) const { return p < *this; }
  bool operator<=(const Segment3& p) const { return !(*this > p); }
  bool operator>=(const Segment3& p) const { return !(*this < p); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Segment3& s) {
    absl::Format(&sink, "(%v %v)", s.d_.at(0), s.d_.at(1));
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Segment3& s) {
    os << s.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Segment3& s) {
    return H::combine(std::move(h), s.d_.at(0), s.d_.at(1));
  }

  // Protobuf.
  // Returns a Segment3Proto.
  Segment3Proto ToProto() const;
  // Sets the Segment3 from a Segment3Proto.
  void SetFromProto(const Segment3Proto& proto);

 private:
  std::array<Point3<T>, 2> d_;  // <p0, p1>
};

// Protobuf.
template <typename T>
Segment3Proto Segment3<T>::ToProto() const {
  Segment3Proto proto;
  if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
    proto.mutable_segment_int32()->mutable_p0()->set_x(d_[0].x());
    proto.mutable_segment_int32()->mutable_p0()->set_y(d_[0].y());
    proto.mutable_segment_int32()->mutable_p0()->set_z(d_[0].z());
    proto.mutable_segment_int32()->mutable_p1()->set_x(d_[1].x());
    proto.mutable_segment_int32()->mutable_p1()->set_y(d_[1].y());
    proto.mutable_segment_int32()->mutable_p1()->set_z(d_[1].z());
  } else if constexpr (std::is_same_v<T, int64_t>) {
    proto.mutable_segment_int64()->mutable_p0()->set_x(d_[0].x());
    proto.mutable_segment_int64()->mutable_p0()->set_y(d_[0].y());
    proto.mutable_segment_int64()->mutable_p0()->set_z(d_[0].z());
    proto.mutable_segment_int64()->mutable_p1()->set_x(d_[1].x());
    proto.mutable_segment_int64()->mutable_p1()->set_y(d_[1].y());
    proto.mutable_segment_int64()->mutable_p1()->set_z(d_[1].z());
  } else if constexpr (std::is_same_v<T, float>) {
    proto.mutable_segment_float()->mutable_p0()->set_x(d_[0].x());
    proto.mutable_segment_float()->mutable_p0()->set_y(d_[0].y());
    proto.mutable_segment_float()->mutable_p0()->set_z(d_[0].z());
    proto.mutable_segment_float()->mutable_p1()->set_x(d_[1].x());
    proto.mutable_segment_float()->mutable_p1()->set_y(d_[1].y());
    proto.mutable_segment_float()->mutable_p1()->set_z(d_[1].z());
  } else if constexpr (std::is_same_v<T, double>) {
    proto.mutable_segment_double()->mutable_p0()->set_x(d_[0].x());
    proto.mutable_segment_double()->mutable_p0()->set_y(d_[0].y());
    proto.mutable_segment_double()->mutable_p0()->set_z(d_[0].z());
    proto.mutable_segment_double()->mutable_p1()->set_x(d_[1].x());
    proto.mutable_segment_double()->mutable_p1()->set_y(d_[1].y());
    proto.mutable_segment_double()->mutable_p1()->set_z(d_[1].z());
  } else {
    static_assert(std::is_same_v<T, int>, "Unsupported type.");
  }
  return proto;
}

template <typename T>
void Segment3<T>::SetFromProto(const Segment3Proto& proto) {
  if (proto.has_segment_int32()) {
    Set(proto.segment_int32().p0().x(), proto.segment_int32().p0().y(),
        proto.segment_int32().p0().z(), proto.segment_int32().p1().x(),
        proto.segment_int32().p1().y(), proto.segment_int32().p1().z());
  } else if (proto.has_segment_int64()) {
    Set(proto.segment_int64().p0().x(), proto.segment_int64().p0().y(),
        proto.segment_int64().p0().z(), proto.segment_int64().p1().x(),
        proto.segment_int64().p1().y(), proto.segment_int64().p1().z());
  } else if (proto.has_segment_float()) {
    Set(proto.segment_float().p0().x(), proto.segment_float().p0().y(),
        proto.segment_float().p0().z(), proto.segment_float().p1().x(),
        proto.segment_float().p1().y(), proto.segment_float().p1().z());
  } else if (proto.has_segment_double()) {
    Set(proto.segment_double().p0().x(), proto.segment_double().p0().y(),
        proto.segment_double().p0().z(), proto.segment_double().p1().x(),
        proto.segment_double().p1().y(), proto.segment_double().p1().z());
  } else {
    LOG(FATAL) << "Unsupported type.";
  }
}

// Aliases.
using Segment3_i = Segment3<int>;
using Segment3_i32 = Segment3<int32_t>;
using Segment3_i64 = Segment3<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

template <typename T>
struct tag<moab::Segment3<T>> {
  using type = segment_tag;
};

template <typename T>
struct point_type<moab::Segment3<T>> {
  using type = typename moab::Segment3<T>::point_type;
};

template <typename T, std::size_t Index, std::size_t Dimension>
struct indexed_access<moab::Segment3<T>, Index, Dimension> {
  using coordinate_type = typename moab::Segment3<T>::coordinate_type;

  static inline coordinate_type get(const moab::Segment3<T>& s) {
    return s[Index][Dimension];
  }
  static inline void set(moab::Segment3<T>& s, const coordinate_type& value) {
    s[Index][Dimension] = value;
  }
};

}  // namespace boost::geometry::traits

#endif  // MOAB_SEGMENT3_H_

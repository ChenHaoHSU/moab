#ifndef MOAB_INTERVAL_H_
#define MOAB_INTERVAL_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

#include "absl/log/check.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/polygon/polygon.hpp"

namespace moab {

// Closed interval [lo, hi], with lo <= hi.
//
// The ends can only be changed through the constructor and the setters, which
// keep lo <= hi by swapping the ends when needed. Boost.Polygon's traits write
// the ends directly and one at a time, so an interval may be inverted while
// Boost updates it; reading its size then is a bug, which Length, Size and
// Contains check in debug builds.
template <typename T>
class Interval {
 public:
  // Type aliases. (Required by Boost polygon traits.)
  using coordinate_type = T;

  // Constructors.
  Interval() : d_({0, 0}) {}
  explicit Interval(T lo, T hi) : d_() { Set(lo, hi); }
  Interval(const Interval& i) = default;
  Interval(Interval&& i) = default;
  ~Interval() = default;

  // Assignment operators.
  Interval& operator=(const Interval&) = default;
  Interval& operator=(Interval&&) = default;
  // Accessors.
  T lo() const { return d_[0]; }
  T hi() const { return d_[1]; }
  const T* data() const { return d_.data(); }

  T Min() const { return d_[0]; }
  T Max() const { return d_[1]; }

  T Length() const {
    DCHECK_LE(d_[0], d_[1]) << "Interval is inverted.";
    return d_[1] - d_[0];
  }
  T Size() const { return Length(); }

  // Mutators.
  // Sets the ends to 'lo' and 'hi', swapping them if lo > hi.
  void Set(T lo, T hi) {
    d_[0] = lo < hi ? lo : hi;
    d_[1] = lo < hi ? hi : lo;
  }

  // Sets one end and keeps lo <= hi by swapping: if 'v' passes the other end,
  // the ends swap, so set_lo(20) on [0 10] gives [10 20].
  void set_lo(T v) { Set(v, hi()); }
  void set_hi(T v) { Set(lo(), v); }

  // Same as set_lo and set_hi.
  void SetMin(T v) { Set(v, hi()); }
  void SetMax(T v) { Set(lo(), v); }

  // Queries.
  bool Contains(T v) const {
    DCHECK_LE(d_[0], d_[1]) << "Interval is inverted.";
    return lo() <= v && v <= hi();
  }
  bool Contains(const Interval& i) const {
    DCHECK_LE(d_[0], d_[1]) << "Interval is inverted.";
    DCHECK_LE(i.d_[0], i.d_[1]) << "Interval is inverted.";
    return lo() <= i.lo() && i.hi() <= hi();
  }

  // Operations.
  void Shift(T d) {
    d_[0] += d;
    d_[1] += d;
  }

  // Operators.
  // Operators - Subscript.
  const T& operator[](std::size_t i) const {
    DCHECK_LT(i, d_.size());
    return d_[i];
  }
  // Operators - Equality.
  bool operator==(const Interval& i) const {
    return d_[0] == i.d_[0] && d_[1] == i.d_[1];
  }
  bool operator!=(const Interval& i) const { return !(*this == i); }
  // Operators - Inequality.
  // Lexicographic order, for sorting and ordered containers; not a geometric
  // comparison. Compares lo, then hi.
  bool operator<(const Interval& i) const {
    return d_[0] != i.d_[0] ? d_[0] < i.d_[0] : d_[1] < i.d_[1];
  }
  bool operator>(const Interval& i) const { return i < *this; }
  bool operator<=(const Interval& i) const { return !(*this > i); }
  bool operator>=(const Interval& i) const { return !(*this < i); }
  // Operators - Arithmetic.
  Interval& operator+=(T d) {
    Shift(d);
    return *this;
  }
  Interval& operator-=(T d) {
    Shift(-d);
    return *this;
  }
  Interval operator+(T d) const {
    Interval i(*this);
    i += d;
    return i;
  }
  Interval operator-(T d) const {
    Interval i(*this);
    i -= d;
    return i;
  }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Interval& i) {
    absl::Format(&sink, "[%v %v]", i.d_[0], i.d_[1]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Interval& i) {
    os << i.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Interval& i) {
    return H::combine(std::move(h), i.d_[0], i.d_[1]);
  }

 private:
  // Boost's traits write the ends directly; see the class comment.
  template <typename U>
  friend struct boost::polygon::interval_mutable_traits;

  std::array<T, 2> d_;  // <low, high>
};

using Interval_i = Interval<int>;
using Interval_i32 = Interval<int32_t>;
using Interval_i64 = Interval<int64_t>;

}  // namespace moab

// Boost polygon traits.
namespace boost::polygon {

template <typename T>
struct geometry_concept<moab::Interval<T>> {
  using type = interval_concept;
};

template <typename T>
struct interval_traits<moab::Interval<T>> {
  using coordinate_type = typename moab::Interval<T>::coordinate_type;

  static inline coordinate_type get(const moab::Interval<T>& i,
                                    direction_1d dir) {
    return i[dir.to_int()];
  }
};

template <typename T>
struct interval_mutable_traits<moab::Interval<T>> {
  using coordinate_type = typename moab::Interval<T>::coordinate_type;

  static inline void set(moab::Interval<T>& i, direction_1d dir,
                         coordinate_type value) {
    i.d_[dir.to_int()] = value;
  }
  static inline moab::Interval<T> construct(coordinate_type lo,
                                            coordinate_type hi) {
    return moab::Interval<T>(lo, hi);
  }
};

}  // namespace boost::polygon

#endif  // MOAB_INTERVAL_H_
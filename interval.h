#ifndef MOAB_INTERVAL_H_
#define MOAB_INTERVAL_H_

#include <array>
#include <cstdint>

#include "absl/log/check.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/polygon/polygon.hpp"

namespace moab {

// Closed interval [lo, hi].
template <typename T>
class Interval {
 public:
  // Type aliases.
  using coordinate_type = T;

  // Constructors.
  Interval() : d_({0, 0}) {}
  explicit Interval(coordinate_type lo, coordinate_type hi) : d_() {
    Set(lo, hi);
  }
  Interval(const Interval& i) = default;
  Interval(Interval&& i) = default;
  ~Interval() = default;

  // Accessors.
  coordinate_type lo() const { return d_[0]; }
  coordinate_type hi() const { return d_[1]; }
  coordinate_type* data() { return d_.data(); }
  const coordinate_type* data() const { return d_.data(); }

  coordinate_type Min() const { return d_[0]; }
  coordinate_type Max() const { return d_[1]; }

  coordinate_type Length() const { return d_[1] - d_[0]; }
  coordinate_type Size() const { return d_[1] - d_[0]; }

  // Mutators.
  void Set(coordinate_type lo, coordinate_type hi) {
    DCHECK(lo <= hi) << "Invalid interval. low: " << lo << ", high: " << hi;
    d_[0] = lo;
    d_[1] = hi;
  }

  void set_lo(coordinate_type v) { Set(v, hi()); }
  void set_hi(coordinate_type v) { Set(lo(), v); }

  void SetMin(coordinate_type v) { Set(v, hi()); }
  void SetMax(coordinate_type v) { Set(lo(), v); }

  // Queries.
  bool Contains(coordinate_type v) const { return lo() <= v && v <= hi(); }
  bool Contains(const Interval& i) const {
    return lo() <= i.lo() && i.hi() <= hi();
  }

  // Operations.
  void Shift(coordinate_type d) {
    d_[0] += d;
    d_[1] += d;
  }

  // Operators.
  // Operators - Assignment.
  Interval& operator=(const Interval& i) {
    d_ = i.d_;
    return *this;
  }
  Interval& operator=(Interval&& i) {
    d_ = std::move(i.d_);
    return *this;
  }
  // Operators - Subscript.
  coordinate_type& operator[](std::size_t i) { return d_[i]; }
  const coordinate_type& operator[](std::size_t i) const { return d_.at(i); }
  // Operators - Equality.
  bool operator==(const Interval& i) const {
    return d_[0] == i.d_[0] && d_[1] == i.d_[1];
  }
  bool operator!=(const Interval& i) const { return !(*this == i); }
  // Operators - Inequality.
  bool operator<(const Interval& i) const {
    return d_[0] != i.d_[0] ? d_[0] < i.d_[0] : d_[1] < i.d_[1];
  }
  bool operator>(const Interval& i) const { return i < *this; }
  bool operator<=(const Interval& i) const { return !(*this > i); }
  bool operator>=(const Interval& i) const { return !(*this < i); }
  // Operators - Arithmetic.
  Interval& operator+=(coordinate_type d) {
    Shift(d);
    return *this;
  }
  Interval& operator-=(coordinate_type d) {
    Shift(-d);
    return *this;
  }
  Interval operator+(coordinate_type d) const {
    Interval i(*this);
    i += d;
    return i;
  }
  Interval operator-(coordinate_type d) const {
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
  std::array<coordinate_type, 2> d_;  // <low, high>
};  // class Interval

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
  using interval_type = moab::Interval<T>;
  using coordinate_type = typename interval_type::coordinate_type;

  static inline coordinate_type get(const interval_type& i, direction_1d dir) {
    return i[dir.to_int()];
  }
};

template <typename T>
struct interval_mutable_traits<moab::Interval<T>> {
  using interval_type = moab::Interval<T>;
  using coordinate_type = typename interval_type::coordinate_type;

  static inline void set(interval_type& i, direction_1d dir,
                         coordinate_type value) {
    i[dir.to_int()] = value;
  }
  static inline moab::Interval<T> construct(coordinate_type lo,
                                            coordinate_type hi) {
    return interval_type(lo, hi);
  }
};

}  // namespace boost::polygon

#endif  // MOAB_INTERVAL_H_
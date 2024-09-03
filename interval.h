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
  // Type aliases. (Required by Boost polygon traits.)
  using coordinate_type = T;

  // Constructors.
  Interval() : d_({0, 0}) {}
  explicit Interval(T lo, T hi) : d_() { Set(lo, hi); }
  Interval(const Interval& i) = default;
  Interval(Interval&& i) = default;
  ~Interval() = default;

  // Accessors.
  T lo() const { return d_[0]; }
  T hi() const { return d_[1]; }
  T* data() { return d_.data(); }
  const T* data() const { return d_.data(); }

  T Min() const { return d_[0]; }
  T Max() const { return d_[1]; }

  T Length() const { return d_[1] - d_[0]; }
  T Size() const { return d_[1] - d_[0]; }

  // Mutators.
  void Set(T lo, T hi) {
    DCHECK(lo <= hi) << "Invalid interval. low: " << lo << ", high: " << hi;
    d_[0] = lo;
    d_[1] = hi;
  }

  void set_lo(T v) { Set(v, hi()); }
  void set_hi(T v) { Set(lo(), v); }

  void SetMin(T v) { Set(v, hi()); }
  void SetMax(T v) { Set(lo(), v); }

  // Queries.
  bool Contains(T v) const { return lo() <= v && v <= hi(); }
  bool Contains(const Interval& i) const {
    return lo() <= i.lo() && i.hi() <= hi();
  }

  // Operations.
  void Shift(T d) {
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
  T& operator[](std::size_t i) { return d_[i]; }
  const T& operator[](std::size_t i) const { return d_.at(i); }
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
  std::array<T, 2> d_;  // <low, high>
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
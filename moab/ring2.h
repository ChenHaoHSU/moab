#ifndef MOAB_RING2_H_
#define MOAB_RING2_H_

#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

#include "absl/strings/str_join.h"

#include "box2.h"
#include "operation.h"
#include "point2.h"

namespace moab {

namespace bg = boost::geometry;

template <typename T>
class Ring2 {
 public:
  // Type aliases.
  using base_type = std::vector<Point2<T>>;
  using coordinate_type = T;
  using point_type = Point2<T>;
  using iterator_type = typename base_type::iterator;
  using const_iterator_type = typename base_type::const_iterator;
  using mutable_iterator_type = typename base_type::iterator;

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
  bool Empty() const { return d_.empty(); }

  Point2<T> Centroid() const {
    Point2<T> p;
    bg::centroid(*this, p);
    return p;
  }

  // Mutators.
  // Avoid using these methods if possible. It is designed for Boost
  // polygon/geometry registration.
  void Clear() { d_.clear(); }
  void Append(const Point2<T>& p) { d_.push_back(p); }
  void Resize(std::size_t n) { d_.resize(n); }

  template <typename InputIt>
  void Assign(InputIt first, InputIt last) {
    d_.assign(first, last);
  }
  void Assign(std::initializer_list<Point2<T>> il) { d_.assign(il); }

  // Iterators.
  iterator_type begin() { return d_.begin(); }
  iterator_type end() { return d_.end(); }
  const_iterator_type begin() const { return d_.begin(); }
  const_iterator_type end() const { return d_.end(); }
  iterator_type rbegin() { return d_.rbegin(); }
  iterator_type rend() { return d_.rend(); }
  const_iterator_type rbegin() const { return d_.rbegin(); }
  const_iterator_type rend() const { return d_.rend(); }

  // Operators.
  // Operator - Subscript
  Point2<T>& operator[](std::size_t i) { return d_[i]; }
  const Point2<T>& operator[](std::size_t i) const { return d_[i]; }
  // Operator - Equality
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
  // This is NOT a perfect hash function. Be careful when using this hash
  // function. It is possible that two equivalent rings with different point
  // sets have different hash values. They are equal under operator== (the
  // IsEqual function) but have different hash values.
  template <typename H>
  friend H AbslHashValue(H h, const Ring2& r) {
    h = H::combine(std::move(h), r.d_);
    return h;
  }

 private:
  std::vector<Point2<T>> d_;
};  // class Ring2

using Ring2_i = Ring2<int>;
using Ring2_i32 = Ring2<int32_t>;
using Ring2_i64 = Ring2<int64_t>;

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

template <typename T>
inline typename range_mutable_iterator<moab::Ring2<T>>::type range_begin(
    moab::Ring2<T>& r) {
  return r.begin();
}

template <typename T>
inline typename range_mutable_iterator<moab::Ring2<T>>::type range_end(
    moab::Ring2<T>& r) {
  return r.end();
}

template <typename T>
inline typename range_const_iterator<moab::Ring2<T>>::type range_begin(
    const moab::Ring2<T>& r) {
  return r.begin();
}

template <typename T>
inline typename range_const_iterator<moab::Ring2<T>>::type range_end(
    const moab::Ring2<T>& r) {
  return r.end();
}

template <typename T>
inline typename range_const_iterator<moab::Ring2<T>>::type range_calculate_size(
    const moab::Ring2<T>& r) {
  return r.Size();
}

}  // namespace boost

// Boost polygon traits.
namespace boost::polygon {

template <>
struct geometry_concept<moab::Ring2<int>> {
  using type = polygon_concept;
};

template <>
struct polygon_traits<moab::Ring2<int>> {
  using coordinate_type = int;
  using iterator_type = typename moab::Ring2<int>::const_iterator_type;

  static inline iterator_type begin_points(const moab::Ring2<int>& r) {
    return r.begin();
  }
  static inline iterator_type end_points(const moab::Ring2<int>& r) {
    return r.end();
  }
  static inline unsigned int size(const moab::Ring2<int>& r) {
    return r.Size();
  }
  static inline winding_direction winding(const moab::Ring2<int>& r) {
    return winding_direction::unknown_winding;
  }
};

template <>
struct polygon_mutable_traits<moab::Ring2<int>> {
  template <typename iT>
  static inline moab::Ring2<int>& set_points(moab::Ring2<int>& r,
                                             iT input_begin, iT input_end) {
    r.Clear();
    while (input_begin != input_end) {
      r.Append(moab::Point2<int>(input_begin->x(), input_begin->y()));
      ++input_begin;
    }
    return r;
  }
};

}  // namespace boost::polygon

#endif  // MOAB_RING2_H_
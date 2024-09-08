#ifndef MOAB_RTREE_H_
#define MOAB_RTREE_H_

#include <cstdint>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/index/parameters.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "box2.h"
#include "point2.h"

namespace moab {

namespace bgi = boost::geometry::index;

template <typename Value>
class Rtree {
 public:
  // Rtree type.
  using rtree_type = bgi::rtree<Value, bgi::rstar<16>, bgi::indexable<Value>,
                                bgi::equal_to<Value>>;

  // Constructors.
  Rtree() = default;
  Rtree(const Rtree& r) = default;
  Rtree(Rtree&& r) = default;
  ~Rtree() = default;

  // Accessors.
  // Returns the number of values in the R-tree.
  std::size_t Size() const { return rtree_.size(); }
  // Returns true if the R-tree is empty.
  bool Empty() const { return rtree_.empty(); }
  // Returns the number of values equal to v.
  std::size_t Count(const Value& v) const { return rtree_.count(v); }

  // Mutators.
  // Clears the R-tree.
  void Clear() { rtree_.clear(); }
  // Inserts a value.
  void Insert(const Value& v) { rtree_.insert(v); }
  // Inserts a range of values.
  template <typename Iterator>
  void insert(Iterator first, Iterator last) {
    rtree_.insert(first, last);
  }

  // Removes only one value, not all equal values.
  void Remove(const Value& v) { rtree_.remove(v); }
  // Removes only one value for each one passed in the range, not
  // all equal values.
  template <typename Iterator>
  void Remove(Iterator first, Iterator last) {
    rtree_.remove(first, last);
  }

  // Operators.
  // Copy assignment.
  Rtree& operator=(const Rtree& r) {
    rtree_ = r.rtree_;
    return *this;
  }
  // Move assignment.
  Rtree& operator=(Rtree&& r) {
    rtree_ = std::move(r.rtree_);
    return *this;
  }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Rtree& r) {
    absl::Format(&sink, "<Size=%d, Values=[%s]>", r.Size(),
                 absl::StrJoin(r.rtree_, ", "));
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Rtree& r) {
    os << r.ToString();
    return os;
  }

 private:
  rtree_type rtree_;
};

// Box R-tree.
using RtreeBox2_i = Rtree<Box2_i>;
using RtreeBox2_i32 = Rtree<Box2_i32>;
using RtreeBox2_i64 = Rtree<Box2_i64>;

// Box R-tree map.
using RtreeBoxMap2_i = Rtree<std::pair<Box2_i, int>>;

}  // namespace moab

#endif  // MOAB_RTREE_H_

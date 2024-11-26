#ifndef MOAB_RTREE_H_
#define MOAB_RTREE_H_

#include <cstdint>
#include <initializer_list>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/index/parameters.hpp"
#include "boost/geometry/index/rtree.hpp"
#include "boost/iterator/function_output_iterator.hpp"

#include "box2.h"
#include "operation.h"
#include "point2.h"

namespace moab {

namespace bgi = boost::geometry::index;

// Primary template for std::pair, which is false for all types
template <typename T>
struct is_pair : std::false_type {};

// Specialization for std::pair
template <typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : std::true_type {};

// Helper variable template for easier usage (C++14 and later)
template <typename T>
constexpr bool is_pair_v = is_pair<T>::value;

// Primary template for std::tuple, which is false for all types
template <typename T>
struct is_tuple : std::false_type {};

// Specialization for std::tuple
template <typename... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type {};

// Helper variable template for easier usage (C++14 and later)
template <typename T>
constexpr bool is_tuple_v = is_tuple<T>::value;

namespace index {

constexpr auto Contains = [](const auto& g) { return bgi::contains(g); };
constexpr auto CoveredBy = [](const auto& g) { return bgi::covered_by(g); };
constexpr auto Covers = [](const auto& g) { return bgi::covers(g); };
constexpr auto Disjoint = [](const auto& g) { return bgi::disjoint(g); };
constexpr auto Intersects = [](const auto& g) { return bgi::intersects(g); };
constexpr auto Overlaps = [](const auto& g) { return bgi::overlaps(g); };
constexpr auto Within = [](const auto& g) { return bgi::within(g); };

// Touches is not part of Boost.Geometry.Index, so we need to implement it.
// Boost.Geometry has a touches function, but it is not part of the index
// module.
constexpr auto Touches = [](const auto& g) constexpr {
  return bgi::satisfies([&g](const auto& v) {
    using T = std::decay_t<decltype(v)>;
    if constexpr (is_pair_v<T> || is_tuple_v<T>) {
      return moab::IsTouch(std::get<0>(v), g);
    } else {
      return moab::IsTouch(v, g);
    }
  });
};

// StrictlyIntersects is not part of Boost.Geometry, so we need to implement it.
// It is similar to Intersects, but it returns true only if the geometries have
// a non-empty intersection, i.e., intersects && !touches.
constexpr auto StrictlyIntersects = [](const auto& g) constexpr {
  return bgi::satisfies([&g](const auto& v) {
    using T = std::decay_t<decltype(v)>;
    if constexpr (is_pair_v<T> || is_tuple_v<T>) {
      return moab::IsStrictlyIntersect(std::get<0>(v), g);
    } else {
      return moab::IsStrictlyIntersect(v, g);
    }
  });
};

}  // namespace index

template <typename T>
class Rtree {
 public:
  // Type aliases.
  using rtree_type =
      bgi::rtree<T, bgi::rstar<16, 4>, bgi::indexable<T>, bgi::equal_to<T>>;
  using const_iterator_type = typename rtree_type::const_iterator;

  // Constructors.
  // Default constructor.
  Rtree() = default;
  // Range constructor using iterators.
  template <typename Iterator>
  explicit Rtree(Iterator first, Iterator last) : rtree_(first, last) {}
  // Range constructor.
  template <typename Range>
  explicit Rtree(const Range& rng) : rtree_(rng) {}
  // Initializer list constructor.
  Rtree(std::initializer_list<T> il) : rtree_(il) {}
  // Copy constructor.
  Rtree(const Rtree&) = default;
  // Move constructor.
  Rtree(Rtree&&) = default;
  // Destructor.
  ~Rtree() = default;

  // Assignment operators.
  Rtree& operator=(const Rtree&) = default;
  Rtree& operator=(Rtree&&) = default;

  // Iterators.
  // Returns a const iterator to the first value.
  const_iterator_type begin() const { return rtree_.begin(); }
  // Returns a const iterator to the end.
  const_iterator_type end() const { return rtree_.end(); }

  // Accessors.
  // Returns the number of values in the R-tree.
  std::size_t Size() const { return rtree_.size(); }
  // Returns true if the R-tree is empty.
  bool Empty() const { return rtree_.empty(); }
  // Returns the number of values equal to v.
  std::size_t Count(const T& v) const { return rtree_.count(v); }

  // Mutators.
  // Clears the R-tree.
  void Clear() { rtree_.clear(); }
  // Inserts a value.
  void Insert(const T& v) { rtree_.insert(v); }
  // Inserts a range of values using iterators.
  template <typename Iterator>
  void Insert(Iterator first, Iterator last) {
    rtree_.insert(first, last);
  }
  // Inserts a range of values.
  template <typename ConvertibleOrRange>
  void Insert(const ConvertibleOrRange& conv_or_rng) {
    rtree_.insert(conv_or_rng);
  }
  // Inserts a key-value pair (only for R-tree maps).
  template <typename U = T>
  std::enable_if_t<is_pair_v<U>> Insert(const typename U::first_type& k,
                                        const typename U::second_type& v) {
    rtree_.insert(std::make_pair(k, v));
  }
  // Inserts a key-values tuple (only for R-tree multi-maps).
  template <typename U = T, typename... Args>
  std::enable_if_t<is_tuple_v<U>> Insert(Args&&... args) {
    rtree_.insert(std::make_tuple(std::forward<Args>(args)...));
  }

  // Removes only one value, not all equal values.
  void Remove(const T& v) { rtree_.remove(v); }
  // Removes only one value for each one passed in the range using iterators,
  // not all equal values.
  template <typename Iterator>
  void Remove(Iterator first, Iterator last) {
    rtree_.remove(first, last);
  }
  // Removes only one value for each one passed in the range, not all equal
  // values.
  template <typename ConvertibleOrRange>
  void Remove(const ConvertibleOrRange& conv_or_rng) {
    rtree_.remove(conv_or_rng);
  }

  // Queries.
  template <typename Predicates>
  std::vector<T> Query(const Predicates& predicates) const {
    std::vector<T> result;
    rtree_.query(predicates,
                 boost::make_function_output_iterator(
                     [&result](const T& v) { result.push_back(v); }));
    return result;
  }
  template <typename Indexable>
  std::vector<T> QueryContains(const Indexable& indexable) const {
    return Query(index::Contains(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryCoveredBy(const Indexable& indexable) const {
    return Query(index::CoveredBy(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryCovers(const Indexable& indexable) const {
    return Query(index::Covers(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryDisjoint(const Indexable& indexable) const {
    return Query(index::Disjoint(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryIntersects(const Indexable& indexable) const {
    return Query(index::Intersects(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryOverlaps(const Indexable& indexable) const {
    return Query(index::Overlaps(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryWithin(const Indexable& indexable) const {
    return Query(index::Within(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryTouches(const Indexable& indexable) const {
    return Query(index::Touches(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryStrictlyIntersects(const Indexable& indexable) const {
    return Query(index::StrictlyIntersects(indexable));
  }

  // Rtree map / multi-map queries.
  template <std::size_t I, typename Predicates>
  auto Query(const Predicates& predicates) const {
    static_assert(is_pair_v<T> || is_tuple_v<T>,
                  "T is neither a pair nor a tuple");
    if constexpr (is_pair_v<T> || is_tuple_v<T>) {
      static_assert(I < std::tuple_size_v<T>, "I is out of bounds");
      std::vector<std::tuple_element_t<I, T>> result;
      rtree_.query(predicates,
                   boost::make_function_output_iterator([&result](const T& v) {
                     result.push_back(std::get<I>(v));
                   }));
      return result;
    }
  }
  // Queries (only for R-tree maps / multi-maps).
  // For std::pair, I = 0: key, I = 1: value.
  // For std::tuple, I = 0, 1, 2, ...: element.
  template <std::size_t I, typename Indexable>
  auto QueryContains(const Indexable& indexable) const {
    return Query<I>(index::Contains(indexable));
  }
  template <std::size_t I, typename Indexable>
  auto QueryCoveredBy(const Indexable& indexable) const {
    return Query<I>(index::CoveredBy(indexable));
  }
  template <std::size_t I, typename Indexable>
  auto QueryCovers(const Indexable& indexable) const {
    return Query<I>(index::Covers(indexable));
  }
  template <std::size_t I, typename Indexable>
  auto QueryDisjoint(const Indexable& indexable) const {
    return Query<I>(index::Disjoint(indexable));
  }
  template <std::size_t I, typename Indexable>
  auto QueryIntersects(const Indexable& indexable) const {
    return Query<I>(index::Intersects(indexable));
  }
  template <std::size_t I, typename Indexable>
  auto QueryOverlaps(const Indexable& indexable) const {
    return Query<I>(index::Overlaps(indexable));
  }
  template <std::size_t I, typename Indexable>
  auto QueryWithin(const Indexable& indexable) const {
    return Query<I>(index::Within(indexable));
  }
  template <std::size_t I, typename Indexable>
  auto QueryTouches(const Indexable& indexable) const {
    return Query<I>(index::Touches(indexable));
  }
  template <std::size_t I, typename Indexable>
  auto QueryStrictlyIntersects(const Indexable& indexable) const {
    return Query<I>(index::StrictlyIntersects(indexable));
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

// Type aliases.
// Box R-tree.
using RtreeBox2_i = Rtree<Box2<int>>;
using RtreeBox2_i32 = Rtree<Box2<int32_t>>;
using RtreeBox2_i64 = Rtree<Box2<int64_t>>;
// Box R-tree map (pair).
template <typename T>
using RtreeMapBox2_i = Rtree<std::pair<Box2<int>, T>>;
template <typename T>
using RtreeMapBox2_i32 = Rtree<std::pair<Box2<int32_t>, T>>;
template <typename T>
using RtreeMapBox2_i64 = Rtree<std::pair<Box2<int64_t>, T>>;
// Box R-tree multi-map (tuple).
template <typename... Args>
using RtreeMultiMapBox2_i = Rtree<std::tuple<Box2<int>, Args...>>;
template <typename... Args>
using RtreeMultiMapBox2_i32 = Rtree<std::tuple<Box2<int32_t>, Args...>>;
template <typename... Args>
using RtreeMultiMapBox2_i64 = Rtree<std::tuple<Box2<int64_t>, Args...>>;
// Point R-tree.
using RtreePoint2_i = Rtree<Point2<int>>;
using RtreePoint2_i32 = Rtree<Point2<int32_t>>;
using RtreePoint2_i64 = Rtree<Point2<int64_t>>;
// Point R-tree map (pair).
template <typename T>
using RtreeMapPoint2_i = Rtree<std::pair<Point2<int>, T>>;
template <typename T>
using RtreeMapPoint2_i32 = Rtree<std::pair<Point2<int32_t>, T>>;
template <typename T>
using RtreeMapPoint2_i64 = Rtree<std::pair<Point2<int64_t>, T>>;
// Point R-tree multi-map (tuple).
template <typename... Args>
using RtreeMultiMapPoint2_i = Rtree<std::tuple<Point2<int>, Args...>>;
template <typename... Args>
using RtreeMultiMapPoint2_i32 = Rtree<std::tuple<Point2<int32_t>, Args...>>;
template <typename... Args>
using RtreeMultiMapPoint2_i64 = Rtree<std::tuple<Point2<int64_t>, Args...>>;

}  // namespace moab

#endif  // MOAB_RTREE_H_

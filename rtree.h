#ifndef MOAB_RTREE_H_
#define MOAB_RTREE_H_

#include <cstdint>
#include <initializer_list>
#include <string>
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
#include "point2.h"

namespace moab {

namespace bgi = boost::geometry::index;

namespace index {

constexpr auto Contains = [](const auto& g) { return bgi::contains(g); };
constexpr auto CoveredBy = [](const auto& g) { return bgi::covered_by(g); };
constexpr auto Covers = [](const auto& g) { return bgi::covers(g); };
constexpr auto Disjoint = [](const auto& g) { return bgi::disjoint(g); };
constexpr auto Intersects = [](const auto& g) { return bgi::intersects(g); };
constexpr auto Overlaps = [](const auto& g) { return bgi::overlaps(g); };
constexpr auto Within = [](const auto& g) { return bgi::within(g); };

}  // namespace index

template <typename>
struct is_pair : std::false_type {};

template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type {};

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
  Rtree(const Rtree& r) = default;
  // Move constructor.
  Rtree(Rtree&& r) = default;
  // Destructor.
  ~Rtree() = default;

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
  // Insert a key-value pair (only for R-tree maps).
  template <typename P = T>
  std::enable_if_t<is_pair<P>::value> Insert(const typename P::first_type& k,
                                             const typename P::second_type& v) {
    rtree_.insert(std::make_pair(k, v));
  }

  // Removes only one value, not all equal values.
  void Remove(const T& v) { rtree_.remove(v); }
  // Removes only one value for each one passed in the range using iterators,
  // not all equal values.
  template <typename Iterator>
  void Remove(Iterator first, Iterator last) {
    rtree_.remove(first, last);
  }
  // Removes only one value for each one passed in the range, not
  // all equal values.
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
  std::vector<T> QueryOverlaps(const Indexable& indexable) const {
    return Query(index::Overlaps(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryIntersects(const Indexable& indexable) const {
    return Query(index::Intersects(indexable));
  }
  template <typename Indexable>
  std::vector<T> QueryWithin(const Indexable& indexable) const {
    return Query(index::Within(indexable));
  }

  // Key/Value queries (only for R-tree maps).
  template <std::size_t I, typename Predicates>
  auto Query(const Predicates& predicates) const {
    static_assert(is_pair<T>::value, "T is not a std::pair");
    if constexpr (is_pair<T>::value) {
      static_assert(I < 2, "I is out of bounds");
      std::vector<std::tuple_element_t<I, T>> result;
      rtree_.query(predicates,
                   boost::make_function_output_iterator([&result](const T& v) {
                     result.push_back(std::get<I>(v));
                   }));
      return result;
    }
  }
  // Keys queries (only for R-tree maps).
  template <typename Indexable>
  auto QueryContainsKey(const Indexable& indexable) const {
    return Query<0>(index::Contains(indexable));
  }
  template <typename Indexable>
  auto QueryCoveredByKey(const Indexable& indexable) const {
    return Query<0>(index::CoveredBy(indexable));
  }
  template <typename Indexable>
  auto QueryCoversKey(const Indexable& indexable) const {
    return Query<0>(index::Covers(indexable));
  }
  template <typename Indexable>
  auto QueryDisjointKey(const Indexable& indexable) const {
    return Query<0>(index::Disjoint(indexable));
  }
  template <typename Indexable>
  auto QueryOverlapsKey(const Indexable& indexable) const {
    return Query<0>(index::Overlaps(indexable));
  }
  template <typename Indexable>
  auto QueryIntersectsKey(const Indexable& indexable) const {
    return Query<0>(index::Intersects(indexable));
  }
  template <typename Indexable>
  auto QueryWithinKey(const Indexable& indexable) const {
    return Query<0>(index::Within(indexable));
  }
  // Value queries (only for R-tree maps).
  template <typename Indexable>
  auto QueryContainsValue(const Indexable& indexable) const {
    return Query<1>(index::Contains(indexable));
  }
  template <typename Indexable>
  auto QueryCoveredByValue(const Indexable& indexable) const {
    return Query<1>(index::CoveredBy(indexable));
  }
  template <typename Indexable>
  auto QueryCoversValue(const Indexable& indexable) const {
    return Query<1>(index::Covers(indexable));
  }
  template <typename Indexable>
  auto QueryDisjointValue(const Indexable& indexable) const {
    return Query<1>(index::Disjoint(indexable));
  }
  template <typename Indexable>
  auto QueryOverlapsValue(const Indexable& indexable) const {
    return Query<1>(index::Overlaps(indexable));
  }
  template <typename Indexable>
  auto QueryIntersectsValue(const Indexable& indexable) const {
    return Query<1>(index::Intersects(indexable));
  }
  template <typename Indexable>
  auto QueryWithinValue(const Indexable& indexable) const {
    return Query<1>(index::Within(indexable));
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
// Box R-tree map.
template <typename T>
using RtreeBoxMap2_i = Rtree<std::pair<Box2<int>, T>>;
template <typename T>
using RtreeBoxMap2_i32 = Rtree<std::pair<Box2<int32_t>, T>>;
template <typename T>
using RtreeBoxMap2_i64 = Rtree<std::pair<Box2<int64_t>, T>>;
// Point R-tree.
using RtreePoint2_i = Rtree<Point2<int>>;
using RtreePoint2_i32 = Rtree<Point2<int32_t>>;
using RtreePoint2_i64 = Rtree<Point2<int64_t>>;
// Point R-tree map.
template <typename T>
using RtreePointMap2_i = Rtree<std::pair<Point2<int>, T>>;
template <typename T>
using RtreePointMap2_i32 = Rtree<std::pair<Point2<int32_t>, T>>;
template <typename T>
using RtreePointMap2_i64 = Rtree<std::pair<Point2<int64_t>, T>>;

}  // namespace moab

#endif  // MOAB_RTREE_H_

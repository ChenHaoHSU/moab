#include "rtree.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "boost/geometry/index/predicates.hpp"
#include "moab/box2.h"
#include "moab/operation.h"
#include "moab/point2.h"
#include "moab/segment2.h"

// The tests follow the order of rtree.h: the pair and tuple traits, the index
// predicates, then the Rtree class (constructors, iterators, accessors,
// mutators, queries, map and multi-map queries, string conversion) and the
// type aliases. Rtree is a template on the stored value, so the tests cover
// the three value kinds: plain trees (Box2_i, Point2_i, Segment2_i), maps
// (std::pair of a key and a value) and multi-maps (std::tuple of a key and
// values).

namespace moab {
namespace {

using ::testing::ElementsAre;
using ::testing::FieldsAre;
using ::testing::IsEmpty;
using ::testing::Pair;
using ::testing::StrEq;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

// =============================================================
// Helpers for the relation queries
// =============================================================

// The boxes of the relation query tests, by their relation to QueryBox(). The
// index of a box is its value in maps and multi-maps.
std::vector<Box2_i> RelationBoxes() {
  return {
      Box2_i(0, 0, 2, 2),    // 0: inside, sharing two edges
      Box2_i(2, 2, 6, 6),    // 1: partial overlap
      Box2_i(4, 0, 6, 2),    // 2: touching the right edge from outside
      Box2_i(8, 8, 9, 9),    // 3: apart
      Box2_i(-1, -1, 5, 5),  // 4: around, containing the query box
      Box2_i(0, 0, 4, 4),    // 5: equal
  };
}

Box2_i QueryBox() { return Box2_i(0, 0, 4, 4); }

// The points of the relation query tests, by their relation to QueryBox().
std::vector<Point2_i> RelationPoints() {
  return {
      Point2_i(0, 0),  // 0: corner
      Point2_i(2, 2),  // 1: inside
      Point2_i(5, 5),  // 2: outside
      Point2_i(4, 2),  // 3: on the right edge
  };
}

// The segments of the relation query tests.
std::vector<Segment2_i> RelationSegments() {
  return {
      Segment2_i(Point2_i(0, 0), Point2_i(10, 0)),   // 0: along the x axis
      Segment2_i(Point2_i(0, 0), Point2_i(0, 10)),   // 1: along the y axis
      Segment2_i(Point2_i(0, 0), Point2_i(10, 10)),  // 2: diagonal
      Segment2_i(Point2_i(2, 2), Point2_i(4, 4)),    // 3: on the diagonal
  };
}

// Returns the values of 'rtree' in iteration order.
template <typename T>
std::vector<T> Values(const Rtree<T>& rtree) {
  return std::vector<T>(rtree.begin(), rtree.end());
}

// The third element of the multi-map tuple with value 'id'.
std::string Name(int id) { return "v" + std::to_string(id); }

// One enumerator for each Query* method and index predicate.
enum class Relation {
  kContains,
  kCoveredBy,
  kCovers,
  kDisjoint,
  kIntersects,
  kOverlaps,
  kWithin,
  kTouches,
  kStrictlyIntersects,
};

// Calls the Query* method of relation R.
template <Relation R, typename Tree, typename Geometry>
auto QueryValues(const Tree& rtree, const Geometry& g) {
  if constexpr (R == Relation::kContains) {
    return rtree.QueryContains(g);
  } else if constexpr (R == Relation::kCoveredBy) {
    return rtree.QueryCoveredBy(g);
  } else if constexpr (R == Relation::kCovers) {
    return rtree.QueryCovers(g);
  } else if constexpr (R == Relation::kDisjoint) {
    return rtree.QueryDisjoint(g);
  } else if constexpr (R == Relation::kIntersects) {
    return rtree.QueryIntersects(g);
  } else if constexpr (R == Relation::kOverlaps) {
    return rtree.QueryOverlaps(g);
  } else if constexpr (R == Relation::kWithin) {
    return rtree.QueryWithin(g);
  } else if constexpr (R == Relation::kTouches) {
    return rtree.QueryTouches(g);
  } else {
    return rtree.QueryStrictlyIntersects(g);
  }
}

// Calls the Query*<I> method of relation R.
template <Relation R, std::size_t I, typename Tree, typename Geometry>
auto QueryElements(const Tree& rtree, const Geometry& g) {
  if constexpr (R == Relation::kContains) {
    return rtree.template QueryContains<I>(g);
  } else if constexpr (R == Relation::kCoveredBy) {
    return rtree.template QueryCoveredBy<I>(g);
  } else if constexpr (R == Relation::kCovers) {
    return rtree.template QueryCovers<I>(g);
  } else if constexpr (R == Relation::kDisjoint) {
    return rtree.template QueryDisjoint<I>(g);
  } else if constexpr (R == Relation::kIntersects) {
    return rtree.template QueryIntersects<I>(g);
  } else if constexpr (R == Relation::kOverlaps) {
    return rtree.template QueryOverlaps<I>(g);
  } else if constexpr (R == Relation::kWithin) {
    return rtree.template QueryWithin<I>(g);
  } else if constexpr (R == Relation::kTouches) {
    return rtree.template QueryTouches<I>(g);
  } else {
    return rtree.template QueryStrictlyIntersects<I>(g);
  }
}

// Returns the index predicate of relation R.
template <Relation R, typename Geometry>
auto Predicate(const Geometry& g) {
  if constexpr (R == Relation::kContains) {
    return index::Contains(g);
  } else if constexpr (R == Relation::kCoveredBy) {
    return index::CoveredBy(g);
  } else if constexpr (R == Relation::kCovers) {
    return index::Covers(g);
  } else if constexpr (R == Relation::kDisjoint) {
    return index::Disjoint(g);
  } else if constexpr (R == Relation::kIntersects) {
    return index::Intersects(g);
  } else if constexpr (R == Relation::kOverlaps) {
    return index::Overlaps(g);
  } else if constexpr (R == Relation::kWithin) {
    return index::Within(g);
  } else if constexpr (R == Relation::kTouches) {
    return index::Touches(g);
  } else {
    return index::StrictlyIntersects(g);
  }
}

// Checks relation R of query geometry g on three trees holding 'keys': a plain
// tree, a map from each key to its index, and a multi-map from each key to its
// index and Name(index). 'ids' are the indices of the keys expected to match.
// Every Query*, Query*<I>, Query(predicate) and Query<I>(predicate) the value
// kind has is checked.
template <Relation R, typename Key, typename Geometry>
void ExpectQuery(const std::vector<Key>& keys, const Geometry& g,
                 const std::vector<int>& ids) {
  std::vector<Key> expected_keys;
  std::vector<std::string> expected_names;
  std::vector<std::pair<Key, int>> expected_pairs;
  std::vector<std::tuple<Key, int, std::string>> expected_tuples;
  for (int id : ids) {
    expected_keys.push_back(keys[id]);
    expected_names.push_back(Name(id));
    expected_pairs.emplace_back(keys[id], id);
    expected_tuples.emplace_back(keys[id], id, Name(id));
  }
  std::vector<std::pair<Key, int>> pairs;
  std::vector<std::tuple<Key, int, std::string>> tuples;
  for (int id = 0; id < static_cast<int>(keys.size()); ++id) {
    pairs.emplace_back(keys[id], id);
    tuples.emplace_back(keys[id], id, Name(id));
  }
  {
    SCOPED_TRACE("plain tree");
    const Rtree<Key> rtree(keys);
    EXPECT_THAT(QueryValues<R>(rtree, g),
                UnorderedElementsAreArray(expected_keys));
    EXPECT_THAT(rtree.Query(Predicate<R>(g)),
                UnorderedElementsAreArray(expected_keys));
  }
  {
    SCOPED_TRACE("map");
    const Rtree<std::pair<Key, int>> rtree(pairs);
    EXPECT_THAT(QueryValues<R>(rtree, g),
                UnorderedElementsAreArray(expected_pairs));
    EXPECT_THAT((QueryElements<R, 0>(rtree, g)),
                UnorderedElementsAreArray(expected_keys));
    EXPECT_THAT((QueryElements<R, 1>(rtree, g)),
                UnorderedElementsAreArray(ids));
    EXPECT_THAT(rtree.template Query<1>(Predicate<R>(g)),
                UnorderedElementsAreArray(ids));
  }
  {
    SCOPED_TRACE("multi-map");
    const Rtree<std::tuple<Key, int, std::string>> rtree(tuples);
    EXPECT_THAT(QueryValues<R>(rtree, g),
                UnorderedElementsAreArray(expected_tuples));
    EXPECT_THAT((QueryElements<R, 0>(rtree, g)),
                UnorderedElementsAreArray(expected_keys));
    EXPECT_THAT((QueryElements<R, 1>(rtree, g)),
                UnorderedElementsAreArray(ids));
    EXPECT_THAT((QueryElements<R, 2>(rtree, g)),
                UnorderedElementsAreArray(expected_names));
    EXPECT_THAT(rtree.template Query<2>(Predicate<R>(g)),
                UnorderedElementsAreArray(expected_names));
  }
}

// =============================================================
// Pair and tuple traits
// =============================================================

TEST(RtreeTraits, IsPairIsTrueOnlyForStdPair) {
  static_assert(is_pair<std::pair<Box2_i, int>>::value);
  static_assert(is_pair_v<std::pair<int, std::string>>);
  static_assert(!is_pair_v<Box2_i>);
  static_assert(!is_pair_v<std::tuple<Box2_i, int>>);
}

TEST(RtreeTraits, IsTupleIsTrueOnlyForStdTuple) {
  static_assert(is_tuple<std::tuple<Box2_i, int>>::value);
  static_assert(is_tuple_v<std::tuple<Box2_i>>);
  static_assert(is_tuple_v<std::tuple<Box2_i, int, std::string>>);
  static_assert(!is_tuple_v<Box2_i>);
  static_assert(!is_tuple_v<std::pair<Box2_i, int>>);
}

// =============================================================
// Index predicates
// =============================================================

// Each predicate is checked on all three value kinds with every query method
// in the Queries and Map and multi-map queries sections. These tests check
// what the predicates themselves promise.

// Touches and StrictlyIntersects capture g by value, so a predicate made
// from a temporary geometry stays valid.
TEST(RtreeIndexPredicates, TouchesAndStrictlyIntersectsKeepACopyOfTheGeometry) {
  const RtreeBox2_i rtree(RelationBoxes());
  const auto touches = index::Touches(Box2_i(0, 0, 4, 4));
  const auto strictly_intersects =
      index::StrictlyIntersects(Box2_i(0, 0, 4, 4));

  EXPECT_THAT(rtree.Query(touches), ElementsAre(Box2_i(4, 0, 6, 2)));
  EXPECT_THAT(rtree.Query(strictly_intersects),
              UnorderedElementsAre(Box2_i(0, 0, 2, 2), Box2_i(2, 2, 6, 6),
                                   Box2_i(-1, -1, 5, 5), Box2_i(0, 0, 4, 4)));
}

// The predicates combine with each other and with Boost.Geometry.Index
// predicates through && and !.
TEST(RtreeIndexPredicates, CombineWithAndAndNot) {
  const RtreeBox2_i rtree(RelationBoxes());
  const Box2_i q = QueryBox();

  // Intersects {0, 1, 2, 4, 5} without Within {0, 5}.
  EXPECT_THAT(rtree.Query(index::Intersects(q) && !index::Within(q)),
              UnorderedElementsAre(Box2_i(2, 2, 6, 6), Box2_i(4, 0, 6, 2),
                                   Box2_i(-1, -1, 5, 5)));
  // StrictlyIntersects {0, 1, 4, 5} without Within {0, 5}.
  EXPECT_THAT(rtree.Query(index::StrictlyIntersects(q) && !index::Within(q)),
              UnorderedElementsAre(Box2_i(2, 2, 6, 6), Box2_i(-1, -1, 5, 5)));
  // Touches {2}, and box 2 is within (0 0)-(6 6).
  EXPECT_THAT(
      rtree.Query(index::Touches(q) && index::Within(Box2_i(0, 0, 6, 6))),
      ElementsAre(Box2_i(4, 0, 6, 2)));
}

// =============================================================
// Constructors and assignment
// =============================================================

TEST(RtreeConstructors, DefaultConstructorIsEmpty) {
  RtreeBox2_i rtree;

  EXPECT_EQ(rtree.Size(), 0u);
  EXPECT_TRUE(rtree.Empty());
}

TEST(RtreeConstructors, IteratorPairConstructor) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 3u);
  EXPECT_FALSE(rtree.Empty());
  EXPECT_THAT(Values(rtree), UnorderedElementsAreArray(boxes));
}

TEST(RtreeConstructors, RangeConstructor) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree(boxes);

  EXPECT_EQ(rtree.Size(), 3u);
  EXPECT_FALSE(rtree.Empty());
  EXPECT_THAT(Values(rtree), UnorderedElementsAreArray(boxes));
}

TEST(RtreeConstructors, InitializerListConstructor) {
  RtreeBox2_i rtree = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                       Box2_i(4, 4, 5, 5)};

  EXPECT_EQ(rtree.Size(), 3u);
  EXPECT_FALSE(rtree.Empty());
  EXPECT_THAT(Values(rtree),
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                   Box2_i(4, 4, 5, 5)));
}

TEST(RtreeConstructors, MapAndMultiMapConstructors) {
  const std::vector<std::pair<Point2_i, int>> pairs = {{Point2_i(0, 0), 0},
                                                       {Point2_i(2, 2), 1}};
  EXPECT_THAT(Values(RtreeMapPoint2_i<int>(pairs.begin(), pairs.end())),
              UnorderedElementsAreArray(pairs));
  EXPECT_THAT(Values(RtreeMapPoint2_i<int>(pairs)),
              UnorderedElementsAreArray(pairs));
  EXPECT_THAT(
      Values(RtreeMapPoint2_i<int>{{Point2_i(0, 0), 0}, {Point2_i(2, 2), 1}}),
      UnorderedElementsAreArray(pairs));

  using MultiMap = RtreeMultiMapPoint2_i<int, std::string>;
  const std::vector<std::tuple<Point2_i, int, std::string>> tuples = {
      {Point2_i(0, 0), 0, "zero"}, {Point2_i(2, 2), 1, "one"}};
  EXPECT_THAT(Values(MultiMap(tuples.begin(), tuples.end())),
              UnorderedElementsAreArray(tuples));
  EXPECT_THAT(Values(MultiMap(tuples)), UnorderedElementsAreArray(tuples));
  EXPECT_THAT(
      Values(MultiMap{{Point2_i(0, 0), 0, "zero"}, {Point2_i(2, 2), 1, "one"}}),
      UnorderedElementsAreArray(tuples));
}

TEST(RtreeConstructors, CopyConstructor) {
  RtreeBox2_i rtree1;
  rtree1.Insert(Box2_i(0, 0, 1, 1));
  rtree1.Insert(Box2_i(2, 2, 3, 3));
  rtree1.Insert(Box2_i(4, 4, 5, 5));

  RtreeBox2_i rtree2(rtree1);

  EXPECT_EQ(rtree1.Size(), 3u);
  EXPECT_FALSE(rtree1.Empty());
  EXPECT_EQ(rtree2.Size(), 3u);
  EXPECT_FALSE(rtree2.Empty());
}

TEST(RtreeConstructors, CopyIsIndependentOfTheOriginal) {
  RtreeBox2_i rtree1 = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)};
  RtreeBox2_i rtree2(rtree1);

  rtree2.Insert(Box2_i(4, 4, 5, 5));
  rtree1.Remove(Box2_i(0, 0, 1, 1));

  EXPECT_THAT(Values(rtree1), ElementsAre(Box2_i(2, 2, 3, 3)));
  EXPECT_THAT(Values(rtree2),
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                   Box2_i(4, 4, 5, 5)));
}

TEST(RtreeConstructors, MoveConstructor) {
  RtreeBox2_i rtree1;
  rtree1.Insert(Box2_i(0, 0, 1, 1));
  rtree1.Insert(Box2_i(2, 2, 3, 3));
  rtree1.Insert(Box2_i(4, 4, 5, 5));

  RtreeBox2_i rtree2(std::move(rtree1));

  // Boost.Geometry.Index leaves a moved-from R-tree empty.
  EXPECT_EQ(rtree1.Size(), 0u);  // NOLINT(bugprone-use-after-move)
  EXPECT_TRUE(rtree1.Empty());
  EXPECT_EQ(rtree2.Size(), 3u);
  EXPECT_FALSE(rtree2.Empty());
}

TEST(RtreeConstructors, CopyAssignment) {
  RtreeBox2_i rtree1;
  rtree1.Insert(Box2_i(0, 0, 1, 1));
  rtree1.Insert(Box2_i(2, 2, 3, 3));
  rtree1.Insert(Box2_i(4, 4, 5, 5));

  RtreeBox2_i rtree2;
  rtree2 = rtree1;

  EXPECT_EQ(rtree1.Size(), 3u);
  EXPECT_FALSE(rtree1.Empty());
  EXPECT_EQ(rtree2.Size(), 3u);
  EXPECT_FALSE(rtree2.Empty());
}

TEST(RtreeConstructors, CopyAssignmentReplacesTheValues) {
  const RtreeBox2_i rtree1 = {Box2_i(0, 0, 1, 1)};
  RtreeBox2_i rtree2 = {Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)};

  rtree2 = rtree1;

  EXPECT_THAT(Values(rtree2), ElementsAre(Box2_i(0, 0, 1, 1)));
}

TEST(RtreeConstructors, MoveAssignment) {
  RtreeBox2_i rtree1;
  rtree1.Insert(Box2_i(0, 0, 1, 1));
  rtree1.Insert(Box2_i(2, 2, 3, 3));
  rtree1.Insert(Box2_i(4, 4, 5, 5));

  RtreeBox2_i rtree2;
  rtree2 = std::move(rtree1);

  // Boost.Geometry.Index leaves a moved-from R-tree empty.
  EXPECT_EQ(rtree1.Size(), 0u);  // NOLINT(bugprone-use-after-move)
  EXPECT_TRUE(rtree1.Empty());
  EXPECT_EQ(rtree2.Size(), 3u);
  EXPECT_FALSE(rtree2.Empty());
}

// =============================================================
// Iterators
// =============================================================

// With a few values, the R-tree has one leaf that keeps the insertion order.
TEST(RtreeIterators, RangeBasedForLoopVisitsEveryValue) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::vector<Box2_i> boxes;
  for (const Box2_i& box : rtree) {
    boxes.push_back(box);
  }

  EXPECT_THAT(boxes, ElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                 Box2_i(4, 4, 5, 5)));
}

TEST(RtreeIterators, BeginToEndVisitsEveryValue) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::vector<Box2_i> boxes;
  for (auto iter = rtree.begin(); iter != rtree.end(); ++iter) {
    const Box2_i& box = *iter;
    boxes.push_back(box);
  }

  EXPECT_THAT(boxes, ElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                 Box2_i(4, 4, 5, 5)));
}

TEST(RtreeIterators, EmptyTreeBeginIsEnd) {
  const RtreeBox2_i rtree;

  EXPECT_TRUE(rtree.begin() == rtree.end());
}

// Many values spread over several nodes are each visited once.
TEST(RtreeIterators, IterationVisitsEveryValueOfALargeTree) {
  std::vector<Point2_i> points;
  for (int x = 0; x < 20; ++x) {
    for (int y = 0; y < 20; ++y) points.push_back(Point2_i(x, y));
  }
  const RtreePoint2_i rtree(points);

  const std::vector<Point2_i> visited(rtree.begin(), rtree.end());

  EXPECT_THAT(visited, UnorderedElementsAreArray(points));
}

// =============================================================
// Accessors
// =============================================================

TEST(RtreeAccessors, SizeCountsEveryInsertedValue) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));
  rtree.Insert(Box2_i(1, 4, 4, 5));

  EXPECT_EQ(rtree.Size(), 4u);
}

TEST(RtreeAccessors, EmptyOnlyWithoutValues) {
  RtreeBox2_i rtree;

  EXPECT_TRUE(rtree.Empty());

  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));

  EXPECT_FALSE(rtree.Empty());
}

TEST(RtreeAccessors, CountCountsEqualValues) {
  struct Case {
    std::string name;
    std::vector<Box2_i> inserted;
    Box2_i value;
    std::size_t expected;
  };
  const std::vector<Box2_i> distinct = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                        Box2_i(4, 4, 5, 5)};
  const std::vector<Box2_i> repeated = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                        Box2_i(4, 4, 5, 5), Box2_i(4, 4, 5, 5),
                                        Box2_i(4, 4, 5, 5), Box2_i(2, 2, 3, 3)};
  const std::vector<Case> cases = {
      {"distinct, first", distinct, Box2_i(0, 0, 1, 1), 1},
      {"distinct, second", distinct, Box2_i(2, 2, 3, 3), 1},
      {"distinct, third", distinct, Box2_i(4, 4, 5, 5), 1},
      {"distinct, absent", distinct, Box2_i(1, 1, 2, 2), 0},
      {"repeated once", repeated, Box2_i(0, 0, 1, 1), 1},
      {"repeated twice", repeated, Box2_i(2, 2, 3, 3), 2},
      {"repeated three times", repeated, Box2_i(4, 4, 5, 5), 3},
      {"repeated, absent", repeated, Box2_i(1, 1, 2, 2), 0},
      {"overlapping but not equal", repeated, Box2_i(0, 0, 2, 2), 0},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    RtreeBox2_i rtree;
    for (const Box2_i& b : c.inserted) rtree.Insert(b);
    EXPECT_EQ(rtree.Count(c.value), c.expected);
  }
}

// A map or multi-map value is counted only when all its elements are equal.
TEST(RtreeAccessors, CountComparesWholeMapAndMultiMapValues) {
  const RtreeMapBox2_i<int> map = {{Box2_i(0, 0, 1, 1), 0},
                                   {Box2_i(0, 0, 1, 1), 0},
                                   {Box2_i(0, 0, 1, 1), 1}};
  EXPECT_EQ(map.Count({Box2_i(0, 0, 1, 1), 0}), 2u);
  EXPECT_EQ(map.Count({Box2_i(0, 0, 1, 1), 1}), 1u);
  EXPECT_EQ(map.Count({Box2_i(0, 0, 1, 1), 2}), 0u);

  const RtreeMultiMapBox2_i<int, std::string> multi_map = {
      {Box2_i(0, 0, 1, 1), 0, "zero"}, {Box2_i(0, 0, 1, 1), 0, "nil"}};
  EXPECT_EQ(multi_map.Count({Box2_i(0, 0, 1, 1), 0, "zero"}), 1u);
  EXPECT_EQ(multi_map.Count({Box2_i(0, 0, 1, 1), 0, "one"}), 0u);
}

// =============================================================
// Mutators: Clear
// =============================================================

TEST(RtreeClear, ClearRemovesEveryValue) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));
  rtree.Clear();

  EXPECT_EQ(rtree.Size(), 0u);
  EXPECT_TRUE(rtree.Empty());
  EXPECT_THAT(rtree.QueryIntersects(Box2_i(0, 0, 5, 5)), IsEmpty());
}

TEST(RtreeClear, ClearOnEmptyTreeKeepsItEmpty) {
  RtreeBox2_i rtree;
  rtree.Clear();

  EXPECT_EQ(rtree.Size(), 0u);
  EXPECT_TRUE(rtree.Empty());
}

TEST(RtreeClear, InsertAfterClear) {
  RtreeBox2_i rtree = {Box2_i(0, 0, 1, 1)};
  rtree.Clear();
  rtree.Insert(Box2_i(2, 2, 3, 3));

  EXPECT_THAT(Values(rtree), ElementsAre(Box2_i(2, 2, 3, 3)));
}

// =============================================================
// Mutators: Insert
// =============================================================

TEST(RtreeInsert, InsertValueKeepsDuplicates) {
  struct Case {
    std::string name;
    std::vector<Box2_i> inserted;
    std::size_t expected_size;
  };
  const std::vector<Case> cases = {
      {"distinct values",
       {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)},
       3},
      {"duplicate values",
       {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3), Box2_i(2, 2, 3, 3),
        Box2_i(4, 4, 5, 5), Box2_i(4, 4, 5, 5)},
       5},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    RtreeBox2_i rtree;
    for (const Box2_i& b : c.inserted) rtree.Insert(b);
    EXPECT_EQ(rtree.Size(), c.expected_size);
    EXPECT_THAT(Values(rtree), UnorderedElementsAreArray(c.inserted));
  }
}

// Insert(first, last) and Insert(range) insert every element, duplicates
// included.
TEST(RtreeInsert, InsertIteratorPairAndRange) {
  struct Case {
    std::string name;
    std::vector<Box2_i> inserted;
    std::size_t expected_size;
  };
  const std::vector<Case> cases = {
      {"distinct values",
       {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)},
       3},
      {"duplicate values",
       {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3), Box2_i(2, 2, 3, 3),
        Box2_i(4, 4, 5, 5), Box2_i(4, 4, 5, 5)},
       5},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    {
      SCOPED_TRACE("iterator pair");
      RtreeBox2_i rtree;
      rtree.Insert(c.inserted.begin(), c.inserted.end());
      EXPECT_EQ(rtree.Size(), c.expected_size);
      EXPECT_THAT(Values(rtree), UnorderedElementsAreArray(c.inserted));
    }
    {
      SCOPED_TRACE("range");
      RtreeBox2_i rtree;
      rtree.Insert(c.inserted);
      EXPECT_EQ(rtree.Size(), c.expected_size);
      EXPECT_THAT(Values(rtree), UnorderedElementsAreArray(c.inserted));
    }
  }
}

TEST(RtreeInsert, InsertAddsToExistingValues) {
  RtreePoint2_i rtree = {Point2_i(0, 0)};
  const std::vector<Point2_i> more = {Point2_i(1, 1), Point2_i(2, 2)};

  rtree.Insert(more.begin(), more.end());
  rtree.Insert(more);

  EXPECT_THAT(Values(rtree), UnorderedElementsAre(
                                 Point2_i(0, 0), Point2_i(1, 1), Point2_i(2, 2),
                                 Point2_i(1, 1), Point2_i(2, 2)));
}

TEST(RtreeInsert, InsertEmptyRangeChangesNothing) {
  RtreeSegment2_i rtree;
  const std::vector<Segment2_i> none;

  rtree.Insert(none.begin(), none.end());
  rtree.Insert(none);

  EXPECT_TRUE(rtree.Empty());
}

// The single-argument template overload also takes a value convertible to T.
TEST(RtreeInsert, InsertConvertibleValue) {
  RtreeMapBox2_i<int> rtree;
  const std::pair<Box2_i, int16_t> value(Box2_i(0, 0, 1, 1), 7);

  rtree.Insert(value);

  EXPECT_THAT(Values(rtree), ElementsAre(Pair(Box2_i(0, 0, 1, 1), 7)));
}

TEST(RtreeInsert, InsertPointsAndSegments) {
  RtreePoint2_i points;
  points.Insert(Point2_i(0, 0));
  points.Insert(Point2_i(2, 2));
  EXPECT_THAT(Values(points),
              UnorderedElementsAre(Point2_i(0, 0), Point2_i(2, 2)));

  RtreeSegment2_i segments;
  segments.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 0)));
  EXPECT_THAT(Values(segments),
              ElementsAre(Segment2_i(Point2_i(0, 0), Point2_i(10, 0))));
}

TEST(RtreeInsert, InsertKeyValuePair) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  EXPECT_EQ(rtree.Size(), 3u);
  EXPECT_THAT(Values(rtree), UnorderedElementsAre(Pair(Box2_i(0, 0, 1, 1), 0),
                                                  Pair(Box2_i(2, 2, 3, 3), 1),
                                                  Pair(Box2_i(4, 4, 5, 5), 2)));
}

// The key and the value are converted to the pair's element types.
TEST(RtreeInsert, InsertKeyValuePairConvertsTheValue) {
  RtreeMapPoint2_i<std::string> rtree;
  rtree.Insert(Point2_i(0, 0), "origin");
  rtree.Insert({Point2_i(1, 1), "one"});

  EXPECT_THAT(Values(rtree),
              UnorderedElementsAre(Pair(Point2_i(0, 0), "origin"),
                                   Pair(Point2_i(1, 1), "one")));
}

TEST(RtreeInsert, InsertKeyValuesTuple) {
  RtreeMultiMapBox2_i<int, std::string> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0, "zero");
  rtree.Insert(Box2_i(2, 2, 3, 3), 1, std::string("one"));
  rtree.Insert(std::make_tuple(Box2_i(4, 4, 5, 5), 2, std::string("two")));

  EXPECT_THAT(Values(rtree),
              UnorderedElementsAre(FieldsAre(Box2_i(0, 0, 1, 1), 0, "zero"),
                                   FieldsAre(Box2_i(2, 2, 3, 3), 1, "one"),
                                   FieldsAre(Box2_i(4, 4, 5, 5), 2, "two")));
}

TEST(RtreeInsert, InsertTupleLvalue) {
  RtreeMultiMapBox2_i<int, std::string> rtree;
  std::tuple<Box2_i, int, std::string> value(Box2_i(0, 0, 1, 1), 0, "zero");
  const std::tuple<Box2_i, int, std::string> const_value(Box2_i(2, 2, 3, 3), 1,
                                                         "one");

  rtree.Insert(value);
  rtree.Insert(const_value);

  EXPECT_EQ(rtree.Size(), 2u);
  EXPECT_EQ(rtree.Count(value), 1u);
  EXPECT_THAT(rtree.QueryIntersects(Box2_i(0, 0, 3, 3)),
              UnorderedElementsAre(FieldsAre(Box2_i(0, 0, 1, 1), 0, "zero"),
                                   FieldsAre(Box2_i(2, 2, 3, 3), 1, "one")));
}

TEST(RtreeInsert, InsertTupleRange) {
  std::vector<std::tuple<Box2_i, int, std::string>> values = {
      {Box2_i(0, 0, 1, 1), 0, "zero"},
      {Box2_i(2, 2, 3, 3), 1, "one"},
      {Box2_i(4, 4, 5, 5), 2, "two"}};
  RtreeMultiMapBox2_i<int, std::string> rtree;

  rtree.Insert(values);

  EXPECT_EQ(rtree.Size(), 3u);
  EXPECT_THAT(Values(rtree), UnorderedElementsAreArray(values));
}

// A two-element tuple must not take an iterator pair as (key, value).
TEST(RtreeInsert, InsertIteratorPairTwoElementTuple) {
  std::vector<std::tuple<Box2_i, int>> values = {{Box2_i(0, 0, 1, 1), 0},
                                                 {Box2_i(2, 2, 3, 3), 1}};
  RtreeMultiMapBox2_i<int> rtree;

  rtree.Insert(values.begin(), values.end());
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  EXPECT_EQ(rtree.Size(), 3u);
  EXPECT_THAT(rtree.QueryIntersects<1>(Box2_i(0, 0, 5, 5)),
              UnorderedElementsAre(0, 1, 2));
}

// A one-element tuple holds only the key.
TEST(RtreeInsert, InsertOneElementTuple) {
  RtreeMultiMapPoint2_i<> rtree;
  rtree.Insert(Point2_i(1, 2));

  EXPECT_THAT(Values(rtree), ElementsAre(FieldsAre(Point2_i(1, 2))));
}

// =============================================================
// Mutators: Remove
// =============================================================

TEST(RtreeRemove, RemoveValue) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_EQ(rtree.Size(), 3u);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1u);

  rtree.Remove(Box2_i(0, 0, 1, 1));

  EXPECT_EQ(rtree.Size(), 2u);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0u);
  EXPECT_THAT(Values(rtree),
              UnorderedElementsAre(Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)));
}

// Each Remove removes one of the equal values; once none is left, Remove
// changes nothing.
TEST(RtreeRemove, RemoveValueRemovesOneOfEqualValues) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_EQ(rtree.Size(), 5u);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 3u);

  struct Step {
    std::string name;
    std::size_t expected_size;
    std::size_t expected_count;
  };
  const std::vector<Step> steps = {
      {"remove 1", 4, 2},
      {"remove 2", 3, 1},
      {"remove 3", 2, 0},
      {"remove 4, nothing removed", 2, 0},
  };
  for (const Step& s : steps) {
    SCOPED_TRACE(s.name);
    rtree.Remove(Box2_i(0, 0, 1, 1));
    EXPECT_EQ(rtree.Size(), s.expected_size);
    EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), s.expected_count);
  }
}

// Remove(first, last) and Remove(range) remove one value for each element.
// 'remove' calls one of the two overloads.
void ExpectRemoveRange(
    const std::function<void(RtreeBox2_i&, const std::vector<Box2_i>&)>&
        remove) {
  {
    SCOPED_TRACE("distinct values");
    std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                 Box2_i(4, 4, 5, 5)};
    RtreeBox2_i rtree(boxes);

    EXPECT_EQ(rtree.Size(), 3u);
    EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1u);

    remove(rtree, std::vector<Box2_i>(boxes.begin(), boxes.begin() + 1));

    EXPECT_EQ(rtree.Size(), 2u);
    EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 1u);
    EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 1u);

    remove(rtree, boxes);

    EXPECT_EQ(rtree.Size(), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 0u);
  }
  {
    SCOPED_TRACE("duplicate values and absent values");
    std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                 Box2_i(4, 4, 5, 5), Box2_i(0, 0, 1, 1),
                                 Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)};
    RtreeBox2_i rtree(boxes);

    EXPECT_EQ(rtree.Size(), 6u);
    EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 2u);
    EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 2u);
    EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 2u);

    std::vector<Box2_i> removed_boxes = {
        Box2_i(0, 0, 1, 1),     Box2_i(2, 2, 3, 3),     Box2_i(4, 4, 5, 5),
        Box2_i(10, 10, 11, 11), Box2_i(12, 12, 13, 13), Box2_i(14, 14, 15, 15)};

    remove(rtree, removed_boxes);

    EXPECT_EQ(rtree.Size(), 3u);
    EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1u);
    EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 1u);
    EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 1u);
    EXPECT_EQ(rtree.Count(Box2_i(10, 10, 11, 11)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(12, 12, 13, 13)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(14, 14, 15, 15)), 0u);

    remove(rtree, boxes);

    EXPECT_EQ(rtree.Size(), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(10, 10, 11, 11)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(12, 12, 13, 13)), 0u);
    EXPECT_EQ(rtree.Count(Box2_i(14, 14, 15, 15)), 0u);
  }
}

TEST(RtreeRemove, RemoveIteratorPairRemovesOneValuePerElement) {
  ExpectRemoveRange([](RtreeBox2_i& rtree, const std::vector<Box2_i>& boxes) {
    rtree.Remove(boxes.begin(), boxes.end());
  });
}

TEST(RtreeRemove, RemoveRangeRemovesOneValuePerElement) {
  ExpectRemoveRange([](RtreeBox2_i& rtree, const std::vector<Box2_i>& boxes) {
    rtree.Remove(boxes);
  });
}

// The single-argument template overload also takes a value convertible to T.
TEST(RtreeRemove, RemoveConvertibleValue) {
  RtreeMapBox2_i<int> rtree = {{Box2_i(0, 0, 1, 1), 7},
                               {Box2_i(2, 2, 3, 3), 8}};

  rtree.Remove(std::pair<Box2_i, int16_t>(Box2_i(0, 0, 1, 1), 7));

  EXPECT_THAT(Values(rtree), ElementsAre(Pair(Box2_i(2, 2, 3, 3), 8)));
}

// A map or multi-map value is removed only when all its elements are equal.
TEST(RtreeRemove, RemoveComparesWholeMapAndMultiMapValues) {
  RtreeMapBox2_i<int> map = {{Box2_i(0, 0, 1, 1), 0}};
  map.Remove({Box2_i(0, 0, 1, 1), 1});
  EXPECT_EQ(map.Size(), 1u);
  map.Remove({Box2_i(0, 0, 1, 1), 0});
  EXPECT_TRUE(map.Empty());

  RtreeMultiMapBox2_i<int, std::string> multi_map = {
      {Box2_i(0, 0, 1, 1), 0, "zero"}, {Box2_i(0, 0, 1, 1), 0, "nil"}};
  multi_map.Remove({Box2_i(0, 0, 1, 1), 0, "zero"});
  EXPECT_THAT(Values(multi_map),
              ElementsAre(FieldsAre(Box2_i(0, 0, 1, 1), 0, "nil")));
}

TEST(RtreeRemove, RemoveFromEmptyTreeChangesNothing) {
  RtreePoint2_i rtree;
  const std::vector<Point2_i> points = {Point2_i(0, 0)};

  rtree.Remove(Point2_i(0, 0));
  rtree.Remove(points.begin(), points.end());
  rtree.Remove(points);

  EXPECT_TRUE(rtree.Empty());
}

// =============================================================
// Queries
// =============================================================

TEST(RtreeQuery, QueryIntersectsPoint) {
  struct Case {
    std::string name;
    Point2_i query;
    std::vector<Box2_i> expected;
  };
  const std::vector<Case> cases = {
      {"corner of the first box", Point2_i(1, 1), {Box2_i(0, 0, 1, 1)}},
      {"corner of the second box", Point2_i(2, 2), {Box2_i(2, 2, 3, 3)}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    RtreeBox2_i rtree;
    rtree.Insert(Box2_i(0, 0, 1, 1));
    rtree.Insert(Box2_i(2, 2, 3, 3));
    rtree.Insert(Box2_i(4, 4, 5, 5));

    std::vector<Box2_i> boxes = rtree.QueryIntersects(c.query);

    EXPECT_THAT(boxes, UnorderedElementsAreArray(c.expected));
  }
}

TEST(RtreeQuery, QueryIntersectsBox) {
  struct Case {
    std::string name;
    std::vector<Box2_i> inserted;
    Box2_i query;
    std::vector<Box2_i> expected;
  };
  const std::vector<Case> cases = {
      {"touching two boxes at corners",
       {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)},
       Box2_i(1, 1, 2, 2),
       {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)}},
      {"every duplicate is returned",
       {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3), Box2_i(2, 2, 3, 3),
        Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)},
       Box2_i(0, 0, 3, 3),
       {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3), Box2_i(2, 2, 3, 3),
        Box2_i(2, 2, 3, 3)}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    RtreeBox2_i rtree;
    for (const Box2_i& b : c.inserted) rtree.Insert(b);

    std::vector<Box2_i> boxes = rtree.QueryIntersects(c.query);

    EXPECT_THAT(boxes, UnorderedElementsAreArray(c.expected));
  }
}

TEST(RtreeQuery, QueryNotIntersects) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::vector<Box2_i> boxes =
      rtree.Query(!index::Intersects(Box2_i(0, 0, 3, 3)));

  EXPECT_THAT(boxes, UnorderedElementsAre(Box2_i(4, 4, 5, 5)));
}

TEST(RtreeQuery, QueryIntersectsPointsInBox) {
  RtreePoint2_i rtree;
  rtree.Insert(Point2_i(0, 0));
  rtree.Insert(Point2_i(2, 2));
  rtree.Insert(Point2_i(4, 4));

  std::vector<Point2_i> points = rtree.QueryIntersects(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(points, UnorderedElementsAre(Point2_i(0, 0), Point2_i(2, 2)));
}

// Query takes any Boost.Geometry.Index predicate, not only the index ones.
TEST(RtreeQuery, QueryWithBoostPredicates) {
  const RtreePoint2_i rtree(RelationPoints());

  // Distances from (0 0): 0, 2.8, 7.1 and 4.5.
  EXPECT_THAT(rtree.Query(bgi::nearest(Point2_i(0, 0), 2)),
              UnorderedElementsAre(Point2_i(0, 0), Point2_i(2, 2)));
  EXPECT_THAT(
      rtree.Query(
          bgi::satisfies([](const Point2_i& p) { return p.x() == p.y(); })),
      UnorderedElementsAre(Point2_i(0, 0), Point2_i(2, 2), Point2_i(5, 5)));
}

TEST(RtreeQuery, QueryOnEmptyTreeReturnsNothing) {
  const RtreeBox2_i rtree;

  EXPECT_THAT(rtree.QueryIntersects(Box2_i(0, 0, 10, 10)), IsEmpty());
  EXPECT_THAT(rtree.QueryDisjoint(Box2_i(0, 0, 10, 10)), IsEmpty());
}

// An exception thrown by a predicate leaves Query, and the R-tree keeps its
// values. The predicate accepts the first value it sees and throws on the
// second, so the exception comes after a value was already collected.
TEST(RtreeQuery, QueryPropagatesPredicateException) {
  const RtreeBox2_i rtree = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)};
  int calls = 0;
  const auto throw_on_second = bgi::satisfies([&calls](const Box2_i&) {
    if (++calls > 1) throw std::runtime_error("predicate failed");
    return true;
  });

  EXPECT_THROW(rtree.Query(throw_on_second), std::runtime_error);
  EXPECT_EQ(calls, 2);
  EXPECT_EQ(rtree.Size(), 2u);
}

// ForEach visits the values Query returns, in the same order, without
// collecting them.
TEST(RtreeQueryWithoutVector, ForEachVisitsWhatQueryReturns) {
  const RtreeMapBox2_i<int> rtree = [] {
    RtreeMapBox2_i<int> r;
    const std::vector<Box2_i> boxes = RelationBoxes();
    for (int i = 0; i < static_cast<int>(boxes.size()); ++i) {
      r.Insert(boxes[i], i);
    }
    return r;
  }();
  for (const Box2_i& q :
       {QueryBox(), Box2_i(8, 8, 8, 8), Box2_i(20, 20, 21, 21)}) {
    SCOPED_TRACE(q.ToString());
    std::vector<std::pair<Box2_i, int>> visited;
    rtree.ForEach(
        index::Intersects(q),
        [&visited](const std::pair<Box2_i, int>& v) { visited.push_back(v); });
    EXPECT_EQ(visited, rtree.QueryIntersects(q));
  }
}

TEST(RtreeQueryWithoutVector, AnyOfAndCountIfMatchQuery) {
  const RtreeBox2_i rtree(RelationBoxes());
  struct Case {
    std::string name;
    Box2_i query;
  };
  const std::vector<Case> cases = {
      {"several matches", QueryBox()},
      {"one match", Box2_i(8, 8, 8, 8)},
      {"no match", Box2_i(20, 20, 21, 21)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    const std::vector<Box2_i> matches = rtree.QueryIntersects(c.query);
    EXPECT_EQ(rtree.AnyOf(index::Intersects(c.query)), !matches.empty());
    EXPECT_EQ(rtree.CountIf(index::Intersects(c.query)), matches.size());
  }
  // Combined and non-index predicates work too.
  EXPECT_EQ(rtree.CountIf(index::Intersects(QueryBox()) &&
                          !index::Within(QueryBox())),
            3u);
  EXPECT_FALSE(rtree.AnyOf(index::Touches(Box2_i(8, 8, 9, 9))));
  // CountIf takes predicates; Count still counts equal values.
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 2, 2)), 1u);
}

TEST(RtreeQueryWithoutVector, EmptyTree) {
  const RtreeBox2_i rtree;
  int calls = 0;
  const auto count_calls = [&calls](const Box2_i&) { ++calls; };
  rtree.ForEach(index::Intersects(QueryBox()), count_calls);
  EXPECT_EQ(calls, 0);
  // The same callback on a tree with values is called once per match.
  RtreeBox2_i(RelationBoxes())
      .ForEach(index::Intersects(Box2_i(8, 8, 8, 8)), count_calls);
  EXPECT_EQ(calls, 1);
  EXPECT_FALSE(rtree.AnyOf(index::Intersects(QueryBox())));
  EXPECT_EQ(rtree.CountIf(index::Intersects(QueryBox())), 0u);
  EXPECT_THAT(rtree.Nearest(Point2_i(0, 0), 3), IsEmpty());
}

// Nearest returns the closest values first. Distances from (0 0) to the
// relation points: 0, 2.8, 7.1 and 4.5.
TEST(RtreeNearest, ClosestFirst) {
  const RtreePoint2_i rtree(RelationPoints());
  EXPECT_THAT(rtree.Nearest(Point2_i(0, 0), 3),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 2), Point2_i(4, 2)));
  EXPECT_THAT(rtree.Nearest(Point2_i(0, 0), 10),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 2), Point2_i(4, 2),
                          Point2_i(5, 5)));
  EXPECT_THAT(rtree.Nearest(Point2_i(0, 0), 0), IsEmpty());
}

// For maps the distance is to the key, and a key that overlaps the query is
// at distance 0.
TEST(RtreeNearest, MapUsesTheKey) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(10, 0, 11, 1), 0);
  rtree.Insert(Box2_i(0, 0, 4, 4), 1);
  rtree.Insert(Box2_i(5, 0, 6, 1), 2);
  EXPECT_THAT(
      rtree.Nearest(Point2_i(2, 2), 2),
      ElementsAre(Pair(Box2_i(0, 0, 4, 4), 1), Pair(Box2_i(5, 0, 6, 1), 2)));
}

// Relation boxes 0 inside, 1 partial, 2 touching, 3 apart, 4 around and
// 5 equal against QueryBox() (0 0)-(4 4), and against its corner (0 0). Every
// query method is checked on a plain tree, a map and a multi-map.
TEST(RtreeBoxQueries, Contains) {
  ExpectQuery<Relation::kContains>(RelationBoxes(), QueryBox(), {4, 5});
  // The corner is on the border of 0 and 5, and inside 4.
  ExpectQuery<Relation::kContains>(RelationBoxes(), Point2_i(0, 0), {4});
}

TEST(RtreeBoxQueries, CoveredBy) {
  ExpectQuery<Relation::kCoveredBy>(RelationBoxes(), QueryBox(), {0, 5});
}

TEST(RtreeBoxQueries, Covers) {
  ExpectQuery<Relation::kCovers>(RelationBoxes(), QueryBox(), {4, 5});
  // Unlike Contains, a box covers a point on its border.
  ExpectQuery<Relation::kCovers>(RelationBoxes(), Point2_i(0, 0), {0, 4, 5});
}

// Boost.Geometry.Index cannot check a node box against a point for
// disjoint (covered_by of a box by a point is not implemented), so Disjoint
// takes no Point2 query for any value kind.
TEST(RtreeBoxQueries, Disjoint) {
  ExpectQuery<Relation::kDisjoint>(RelationBoxes(), QueryBox(), {3});
}

TEST(RtreeBoxQueries, Intersects) {
  ExpectQuery<Relation::kIntersects>(RelationBoxes(), QueryBox(),
                                     {0, 1, 2, 4, 5});
  ExpectQuery<Relation::kIntersects>(RelationBoxes(), Point2_i(0, 0),
                                     {0, 4, 5});
}

// Containment and equal boxes do not overlap.
TEST(RtreeBoxQueries, Overlaps) {
  ExpectQuery<Relation::kOverlaps>(RelationBoxes(), QueryBox(), {1});
}

TEST(RtreeBoxQueries, Within) {
  ExpectQuery<Relation::kWithin>(RelationBoxes(), QueryBox(), {0, 5});
}

// Box 0 shares two edges with the query box but their interiors overlap, so
// only box 2 touches it.
TEST(RtreeBoxQueries, Touches) {
  ExpectQuery<Relation::kTouches>(RelationBoxes(), QueryBox(), {2});
  ExpectQuery<Relation::kTouches>(RelationBoxes(), Box2_i(6, 6, 8, 8), {1, 3});
}

TEST(RtreeBoxQueries, StrictlyIntersects) {
  ExpectQuery<Relation::kStrictlyIntersects>(RelationBoxes(), QueryBox(),
                                             {0, 1, 4, 5});
}

// Relation points 0 corner, 1 inside, 2 outside and 3 on an edge against
// QueryBox() (0 0)-(4 4), and against point 1 (2 2). Overlaps and touches are
// not defined for points, and Boost.Geometry does not implement touches for a
// point and a box.
TEST(RtreePointQueries, Contains) {
  ExpectQuery<Relation::kContains>(RelationPoints(), Point2_i(2, 2), {1});
}

TEST(RtreePointQueries, CoveredBy) {
  ExpectQuery<Relation::kCoveredBy>(RelationPoints(), QueryBox(), {0, 1, 3});
}

TEST(RtreePointQueries, Covers) {
  ExpectQuery<Relation::kCovers>(RelationPoints(), Point2_i(2, 2), {1});
}

TEST(RtreePointQueries, Disjoint) {
  ExpectQuery<Relation::kDisjoint>(RelationPoints(), QueryBox(), {2});
}

TEST(RtreePointQueries, Intersects) {
  ExpectQuery<Relation::kIntersects>(RelationPoints(), QueryBox(), {0, 1, 3});
  ExpectQuery<Relation::kIntersects>(RelationPoints(), Point2_i(2, 2), {1});
}

// Unlike CoveredBy, a point on the border of the box is not within it.
TEST(RtreePointQueries, Within) {
  ExpectQuery<Relation::kWithin>(RelationPoints(), QueryBox(), {1});
}

// Relation segments 0 x axis, 1 y axis, 2 diagonal (0 0)-(10 10) and
// 3 (2 2)-(4 4) on the diagonal. Boost.Geometry does not implement covered_by
// or within for a segment in a box, nor overlaps for two segments, so the
// segment trees have no CoveredBy, Within or Overlaps queries.
TEST(RtreeSegmentQueries, Contains) {
  // (2 2) is inside segment 2 and an end point of segment 3.
  ExpectQuery<Relation::kContains>(RelationSegments(), Point2_i(2, 2), {2});
}

TEST(RtreeSegmentQueries, Covers) {
  ExpectQuery<Relation::kCovers>(RelationSegments(), Point2_i(2, 2), {2, 3});
}

TEST(RtreeSegmentQueries, Disjoint) {
  ExpectQuery<Relation::kDisjoint>(RelationSegments(), Box2_i(1, 1, 3, 3),
                                   {0, 1});
}

TEST(RtreeSegmentQueries, Intersects) {
  ExpectQuery<Relation::kIntersects>(RelationSegments(), Box2_i(1, 1, 3, 3),
                                     {2, 3});
  ExpectQuery<Relation::kIntersects>(RelationSegments(), Point2_i(2, 2),
                                     {2, 3});
  ExpectQuery<Relation::kIntersects>(
      RelationSegments(), Segment2_i(Point2_i(0, 5), Point2_i(10, 5)), {1, 2});
}

// A point touches a segment at an end point.
TEST(RtreeSegmentQueries, Touches) {
  ExpectQuery<Relation::kTouches>(RelationSegments(), Point2_i(2, 2), {3});
}

TEST(RtreeSegmentQueries, StrictlyIntersects) {
  ExpectQuery<Relation::kStrictlyIntersects>(RelationSegments(), Point2_i(2, 2),
                                             {2});
}

// =============================================================
// Map and multi-map queries
// =============================================================

TEST(RtreeMapQuery, QueryIntersectsReturnsPairs) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert({Box2_i(0, 0, 1, 1), 0});
  rtree.Insert({Box2_i(2, 2, 3, 3), 1});
  rtree.Insert({Box2_i(4, 4, 5, 5), 2});

  std::vector<std::pair<Box2_i, int>> boxes =
      rtree.QueryIntersects(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(Pair(Box2_i(0, 0, 1, 1), 0),
                                          Pair(Box2_i(2, 2, 3, 3), 1)));
}

TEST(RtreeMapQuery, QueryIntersectsKeys) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert({Box2_i(0, 0, 1, 1), 0});
  rtree.Insert({Box2_i(2, 2, 3, 3), 1});
  rtree.Insert({Box2_i(4, 4, 5, 5), 2});

  std::vector<Box2_i> boxes = rtree.QueryIntersects<0>(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)));
}

TEST(RtreeMapQuery, QueryIntersectsValues) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<int> boxes = rtree.QueryIntersects<1>(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(0, 1));
}

TEST(RtreeMapQuery, QueryTouchesPredicateValues) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<int> boxes =
      rtree.Query<1>(moab::index::Touches(Box2_i(0, 1, 3, 2)));

  EXPECT_THAT(boxes, UnorderedElementsAre(0, 1));
}

TEST(RtreeMapQuery, QueryTouchesValues) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<int> boxes = rtree.QueryTouches<1>(Box2_i(0, 0, 3, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(1));
}

TEST(RtreeMapQuery, QueryTouchesKeys) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<Box2_i> boxes = rtree.QueryTouches<0>(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)));
}

TEST(RtreeMapQuery, QueryStrictlyIntersectsValues) {
  struct Case {
    std::string name;
    Box2_i query;
    std::vector<int> expected;
  };
  const std::vector<Case> cases = {
      {"touching two boxes at corners", Box2_i(1, 1, 2, 2), {}},
      {"overlapping the first box, touching the second",
       Box2_i(0, -1, 3, 2),
       {0}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    RtreeMapBox2_i<int> rtree;
    rtree.Insert(Box2_i(0, 0, 1, 1), 0);
    rtree.Insert(Box2_i(2, 2, 3, 3), 1);
    rtree.Insert(Box2_i(4, 4, 5, 5), 2);

    std::vector<int> boxes = rtree.QueryStrictlyIntersects<1>(c.query);

    EXPECT_THAT(boxes, UnorderedElementsAreArray(c.expected));
  }
}

// Boxes degenerated to segments and a point: a box covers the parts of
// itself, including its end points, but nothing that sticks out.
TEST(RtreeMapQuery, QueryCoversDegenerateBoxes) {
  struct Case {
    std::string name;
    Box2_i query;
    std::vector<int> expected;
  };
  const std::vector<Case> cases = {
      {"whole horizontal box", Box2_i(Point2_i(0, 0), Point2_i(10, 0)), {0}},
      {"inner part", Box2_i(Point2_i(1, 0), Point2_i(9, 0)), {0}},
      {"part from the start", Box2_i(Point2_i(0, 0), Point2_i(9, 0)), {0}},
      {"part to the end", Box2_i(Point2_i(5, 0), Point2_i(10, 0)), {0}},
      {"inner point", Box2_i(Point2_i(5, 0), Point2_i(5, 0)), {0}},
      {"sticking out at the end", Box2_i(Point2_i(5, 0), Point2_i(15, 0)), {}},
      {"starting at the end", Box2_i(Point2_i(10, 0), Point2_i(15, 0)), {}},
      {"sticking out at both ends",
       Box2_i(Point2_i(-5, 0), Point2_i(15, 0)),
       {}},
      {"the point box", Box2_i(Point2_i(10, 10), Point2_i(10, 10)), {2}},
  };
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(Point2_i(0, 0), Point2_i(10, 0)), 0);
  rtree.Insert(Box2_i(Point2_i(0, 0), Point2_i(0, 10)), 1);
  rtree.Insert(Box2_i(Point2_i(10, 10), Point2_i(10, 10)), 2);
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    std::vector<int> values = rtree.QueryCovers<1>(c.query);
    EXPECT_THAT(values, UnorderedElementsAreArray(c.expected));
  }
}

TEST(RtreeMapQuery, PointMapQueryIntersects) {
  RtreeMapPoint2_i<int> rtree;
  rtree.Insert({Point2_i(0, 0), 0});
  rtree.Insert({Point2_i(2, 2), 1});
  rtree.Insert({Point2_i(4, 4), 2});

  std::vector<std::pair<Point2_i, int>> points =
      rtree.QueryIntersects(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(points, UnorderedElementsAre(Pair(Point2_i(0, 0), 0),
                                           Pair(Point2_i(2, 2), 1)));
}

TEST(RtreeMapQuery, PointMapQueryIntersectsKeys) {
  std::vector<std::pair<Point2_i, int>> point_values = {
      {Point2_i(0, 0), 0}, {Point2_i(2, 2), 1}, {Point2_i(4, 4), 2}};
  RtreeMapPoint2_i<int> rtree;
  rtree.Insert(point_values);

  std::vector<Point2_i> points = rtree.QueryIntersects<0>(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(points, UnorderedElementsAre(Point2_i(0, 0), Point2_i(2, 2)));
}

TEST(RtreeMapQuery, PointMapQueryIntersectsValues) {
  RtreeMapPoint2_i<int> rtree;
  rtree.Insert(Point2_i(0, 0), 0);
  rtree.Insert(Point2_i(2, 2), 1);
  rtree.Insert(Point2_i(4, 4), 2);

  std::vector<int> points = rtree.QueryIntersects<1>(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(points, UnorderedElementsAre(0, 1));
}

TEST(RtreeMapQuery, SegmentMapQueryIntersectsSegment) {
  struct Case {
    std::string name;
    Segment2_i query;
    std::vector<int> expected;
  };
  const std::vector<Case> cases = {
      {"horizontal through the middle",
       Segment2_i(Point2_i(0, 5), Point2_i(10, 5)),
       {1, 2}},
      {"vertical through the middle",
       Segment2_i(Point2_i(5, 0), Point2_i(5, 10)),
       {0, 2}},
      {"ending at the common start",
       Segment2_i(Point2_i(-5, 0), Point2_i(0, 0)),
       {0, 1, 2}},
      {"starting at the end of the x axis segment",
       Segment2_i(Point2_i(10, 0), Point2_i(20, 0)),
       {0}},
      {"collinear with the x axis segment",
       Segment2_i(Point2_i(5, 0), Point2_i(15, 0)),
       {0}},
  };
  RtreeMapSegment2_i<int> rtree;
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 0)), 0);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(0, 10)), 1);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 10)), 2);
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    std::vector<int> values = rtree.QueryIntersects<1>(c.query);
    EXPECT_THAT(values, UnorderedElementsAreArray(c.expected));
  }
}

TEST(RtreeMapQuery, SegmentMapQueryIntersectsBox) {
  RtreeMapSegment2_i<int> rtree;
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 0)), 0);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(0, 10)), 1);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 10)), 2);

  std::vector<int> values = rtree.QueryIntersects<1>(Box2_i(0, 5, 10, 10));

  EXPECT_THAT(values, UnorderedElementsAre(1, 2));
}

TEST(RtreeMultiMapQuery, QueryIntersectsReturnsTuples) {
  RtreeMultiMapBox2_i<int, std::string> rtree;
  rtree.Insert({Box2_i(0, 0, 1, 1), 0, "zero"});
  rtree.Insert({Box2_i(2, 2, 3, 3), 1, "one"});
  rtree.Insert({Box2_i(4, 4, 5, 5), 2, "two"});

  std::vector<std::tuple<Box2_i, int, std::string>> boxes =
      rtree.QueryIntersects(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(boxes,
              UnorderedElementsAre(FieldsAre(Box2_i(0, 0, 1, 1), 0, "zero"),
                                   FieldsAre(Box2_i(2, 2, 3, 3), 1, "one")));
}

TEST(RtreeMultiMapQuery, QueryIntersectsEachElement) {
  RtreeMultiMapBox2_i<int, std::string> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0, "zero");
  rtree.Insert(Box2_i(2, 2, 3, 3), 1, "one");
  rtree.Insert(Box2_i(4, 4, 5, 5), 2, "two");

  std::vector<Box2_i> boxes = rtree.QueryIntersects<0>(Box2_i(0, 0, 2, 2));
  EXPECT_THAT(boxes,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)));

  std::vector<int> ids = rtree.QueryIntersects<1>(Box2_i(0, 0, 2, 2));
  EXPECT_THAT(ids, UnorderedElementsAre(0, 1));

  std::vector<std::string> names = rtree.QueryIntersects<2>(Box2_i(0, 0, 2, 2));
  EXPECT_THAT(names, UnorderedElementsAre("zero", "one"));
}

// Query<I> takes any Boost.Geometry.Index predicate too.
TEST(RtreeMultiMapQuery, QueryElementWithBoostPredicates) {
  const RtreeMultiMapPoint2_i<int, std::string> rtree = {
      {Point2_i(0, 0), 0, "zero"},
      {Point2_i(2, 2), 1, "one"},
      {Point2_i(5, 5), 2, "two"}};

  EXPECT_THAT(rtree.Query<2>(bgi::nearest(Point2_i(4, 4), 1)),
              ElementsAre("two"));
  EXPECT_THAT(rtree.Query<1>(bgi::satisfies(
                  [](const auto& v) { return std::get<1>(v) > 0; })),
              UnorderedElementsAre(1, 2));
}

// An exception thrown by a predicate leaves Query<I>, and the R-tree keeps
// its values. As above, the predicate throws on the second value.
TEST(RtreeMultiMapQuery, QueryElementPropagatesPredicateException) {
  const RtreeMultiMapBox2_i<int> rtree = {{Box2_i(0, 0, 1, 1), 0},
                                          {Box2_i(2, 2, 3, 3), 1}};
  int calls = 0;
  const auto throw_on_second =
      bgi::satisfies([&calls](const std::tuple<Box2_i, int>&) {
        if (++calls > 1) throw std::runtime_error("predicate failed");
        return true;
      });

  EXPECT_THROW(rtree.Query<1>(throw_on_second), std::runtime_error);
  EXPECT_EQ(calls, 2);
  EXPECT_EQ(rtree.Size(), 2u);
}

// =============================================================
// Queries against a linear scan
// =============================================================

// Brute-force equivalence tests for QueryStrictlyIntersects and QueryTouches.
// Boost.Geometry implements touches only for box-box among the types here,
// so these queries need box keys and box queries.
// The data is grid-aligned so that many shapes abut exactly, which exercises
// the touching cases as well as the overlapping ones.

Box2_i RandomGridBox(std::mt19937& rng, int extent, int max_size) {
  std::uniform_int_distribution<int> pos(0, extent / 10);
  std::uniform_int_distribution<int> size(1, max_size / 10);
  const int x = pos(rng) * 10;
  const int y = pos(rng) * 10;
  return Box2_i(x, y, x + size(rng) * 10, y + size(rng) * 10);
}

std::vector<Box2_i> RandomGridBoxes(std::mt19937& rng, int n, int extent,
                                    int max_size) {
  std::vector<Box2_i> boxes;
  for (int i = 0; i < n; ++i) {
    boxes.push_back(RandomGridBox(rng, extent, max_size));
  }
  return boxes;
}

template <typename T>
std::vector<T> Sorted(std::vector<T> v) {
  std::sort(v.begin(), v.end());
  return v;
}

// Returns the values whose key (the value itself, or its first element)
// satisfies `pred`, found by a linear scan.
template <typename Value, typename Pred>
std::vector<Value> Scan(const std::vector<Value>& values, Pred pred) {
  std::vector<Value> result;
  for (const Value& v : values) {
    if constexpr (is_pair_v<Value> || is_tuple_v<Value>) {
      if (pred(std::get<0>(v))) result.push_back(v);
    } else {
      if (pred(v)) result.push_back(v);
    }
  }
  return result;
}

// Checks both queries against the scan for every query geometry, and that
// the data produces both touching and strictly intersecting matches.
template <typename Value, typename Query>
void ExpectMatchesScan(const std::vector<Value>& values,
                       const std::vector<Query>& queries) {
  Rtree<Value> rtree(values);
  std::size_t num_strict = 0;
  std::size_t num_touch = 0;
  for (const Query& q : queries) {
    const std::vector<Value> strict =
        Scan(values, [&q](const auto& k) { return IsStrictlyIntersect(k, q); });
    const std::vector<Value> touch =
        Scan(values, [&q](const auto& k) { return IsTouch(k, q); });
    EXPECT_EQ(Sorted(rtree.QueryStrictlyIntersects(q)), Sorted(strict))
        << "query " << q;
    EXPECT_EQ(Sorted(rtree.QueryTouches(q)), Sorted(touch)) << "query " << q;
    num_strict += strict.size();
    num_touch += touch.size();
  }
  EXPECT_GT(num_strict, 0u);
  EXPECT_GT(num_touch, 0u);
}

TEST(RtreeBruteForceTest, BoxValuesBoxQueries) {
  std::mt19937 rng(1);
  ExpectMatchesScan(RandomGridBoxes(rng, 2000, 2000, 100),
                    RandomGridBoxes(rng, 300, 2000, 300));
}

TEST(RtreeBruteForceTest, BoxMapValuesBoxQueries) {
  std::mt19937 rng(2);
  std::vector<std::pair<Box2_i, int>> values;
  for (const Box2_i& b : RandomGridBoxes(rng, 2000, 2000, 100)) {
    values.emplace_back(b, static_cast<int>(values.size()));
  }
  const std::vector<Box2_i> queries = RandomGridBoxes(rng, 300, 2000, 300);
  ExpectMatchesScan(values, queries);

  // The Query<I> overloads return the same matches, projected.
  RtreeMapBox2_i<int> rtree(values);
  for (const Box2_i& q : queries) {
    std::vector<int> expected;
    for (const auto& [box, id] : rtree.QueryStrictlyIntersects(q)) {
      expected.push_back(id);
    }
    EXPECT_EQ(rtree.QueryStrictlyIntersects<1>(q), expected);
    expected.clear();
    for (const auto& [box, id] : rtree.QueryTouches(q)) {
      expected.push_back(id);
    }
    EXPECT_EQ(rtree.QueryTouches<1>(q), expected);
  }
}

TEST(RtreeBruteForceTest, TupleValuesBoxQueries) {
  std::mt19937 rng(3);
  std::vector<std::tuple<Box2_i, int, int>> values;
  for (const Box2_i& b : RandomGridBoxes(rng, 1000, 1500, 100)) {
    const int id = static_cast<int>(values.size());
    values.emplace_back(b, id, -id);
  }
  ExpectMatchesScan(values, RandomGridBoxes(rng, 200, 1500, 300));
}

// =============================================================
// String conversion
// =============================================================

TEST(RtreeStringConversion, ToString) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_THAT(
      rtree.ToString(),
      StrEq("<Size=3, Values=[((0 0) (1 1)), ((2 2) (3 3)), ((4 4) (5 5))]>"));
}

TEST(RtreeStringConversion, AbslStringify) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::string s = absl::StrFormat("%v", rtree);

  EXPECT_THAT(
      s,
      StrEq("<Size=3, Values=[((0 0) (1 1)), ((2 2) (3 3)), ((4 4) (5 5))]>"));
}

TEST(RtreeStringConversion, StreamOutput) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::ostringstream ss;
  ss << rtree;

  EXPECT_THAT(
      ss.str(),
      StrEq("<Size=3, Values=[((0 0) (1 1)), ((2 2) (3 3)), ((4 4) (5 5))]>"));
}

TEST(RtreeStringConversion, EveryValueKindAndEmptyTree) {
  struct Case {
    std::string name;
    std::string actual;
    std::string expected;
  };
  const RtreePoint2_i points = {Point2_i(1, 2), Point2_i(3, 4)};
  const RtreeSegment2_i segments = {
      Segment2_i(Point2_i(0, 0), Point2_i(10, 0))};
  const RtreeBox2_i empty;
  std::ostringstream points_stream;
  points_stream << points;
  const std::vector<Case> cases = {
      {"points ToString", points.ToString(), "<Size=2, Values=[(1 2), (3 4)]>"},
      {"points AbslStringify", absl::StrFormat("%v", points),
       "<Size=2, Values=[(1 2), (3 4)]>"},
      {"points operator<<", points_stream.str(),
       "<Size=2, Values=[(1 2), (3 4)]>"},
      {"segments", segments.ToString(), "<Size=1, Values=[((0 0) (10 0))]>"},
      {"empty", empty.ToString(), "<Size=0, Values=[]>"},
      {"empty AbslStringify", absl::StrFormat("%v", empty),
       "<Size=0, Values=[]>"},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.actual, c.expected);
  }
}

// =============================================================
// Type aliases
// =============================================================

TEST(RtreeTypeAliases, AliasesNameTheValueTypes) {
  static_assert(std::is_same_v<RtreeBox2_i, Rtree<Box2<int>>>);
  static_assert(std::is_same_v<RtreeBox2_i32, Rtree<Box2<int32_t>>>);
  static_assert(std::is_same_v<RtreeBox2_i64, Rtree<Box2<int64_t>>>);
  static_assert(
      std::is_same_v<RtreeMapBox2_i<int>, Rtree<std::pair<Box2<int>, int>>>);
  static_assert(std::is_same_v<RtreeMapBox2_i32<int>,
                               Rtree<std::pair<Box2<int32_t>, int>>>);
  static_assert(std::is_same_v<RtreeMapBox2_i64<int>,
                               Rtree<std::pair<Box2<int64_t>, int>>>);
  static_assert(std::is_same_v<RtreeMultiMapBox2_i<int, char>,
                               Rtree<std::tuple<Box2<int>, int, char>>>);
  static_assert(std::is_same_v<RtreeMultiMapBox2_i32<int, char>,
                               Rtree<std::tuple<Box2<int32_t>, int, char>>>);
  static_assert(std::is_same_v<RtreeMultiMapBox2_i64<int, char>,
                               Rtree<std::tuple<Box2<int64_t>, int, char>>>);

  static_assert(std::is_same_v<RtreePoint2_i, Rtree<Point2<int>>>);
  static_assert(std::is_same_v<RtreePoint2_i32, Rtree<Point2<int32_t>>>);
  static_assert(std::is_same_v<RtreePoint2_i64, Rtree<Point2<int64_t>>>);
  static_assert(std::is_same_v<RtreeMapPoint2_i<int>,
                               Rtree<std::pair<Point2<int>, int>>>);
  static_assert(std::is_same_v<RtreeMapPoint2_i32<int>,
                               Rtree<std::pair<Point2<int32_t>, int>>>);
  static_assert(std::is_same_v<RtreeMapPoint2_i64<int>,
                               Rtree<std::pair<Point2<int64_t>, int>>>);
  static_assert(std::is_same_v<RtreeMultiMapPoint2_i<int, char>,
                               Rtree<std::tuple<Point2<int>, int, char>>>);
  static_assert(std::is_same_v<RtreeMultiMapPoint2_i32<int, char>,
                               Rtree<std::tuple<Point2<int32_t>, int, char>>>);
  static_assert(std::is_same_v<RtreeMultiMapPoint2_i64<int, char>,
                               Rtree<std::tuple<Point2<int64_t>, int, char>>>);

  static_assert(std::is_same_v<RtreeSegment2_i, Rtree<Segment2<int>>>);
  static_assert(std::is_same_v<RtreeSegment2_i32, Rtree<Segment2<int32_t>>>);
  static_assert(std::is_same_v<RtreeSegment2_i64, Rtree<Segment2<int64_t>>>);
  static_assert(std::is_same_v<RtreeMapSegment2_i<int>,
                               Rtree<std::pair<Segment2<int>, int>>>);
  static_assert(std::is_same_v<RtreeMapSegment2_i32<int>,
                               Rtree<std::pair<Segment2<int32_t>, int>>>);
  static_assert(std::is_same_v<RtreeMapSegment2_i64<int>,
                               Rtree<std::pair<Segment2<int64_t>, int>>>);
  static_assert(std::is_same_v<RtreeMultiMapSegment2_i<int, char>,
                               Rtree<std::tuple<Segment2<int>, int, char>>>);
  static_assert(
      std::is_same_v<RtreeMultiMapSegment2_i32<int, char>,
                     Rtree<std::tuple<Segment2<int32_t>, int, char>>>);
  static_assert(
      std::is_same_v<RtreeMultiMapSegment2_i64<int, char>,
                     Rtree<std::tuple<Segment2<int64_t>, int, char>>>);
}

// 64-bit coordinates beyond the 32-bit range work like small ones.
TEST(RtreeTypeAliases, Int64TreeHoldsLargeCoordinates) {
  constexpr int64_t kBig = int64_t{1} << 40;
  RtreeBox2_i64 rtree;
  rtree.Insert(Box2_i64(kBig, kBig, kBig + 10, kBig + 10));
  rtree.Insert(Box2_i64(0, 0, 10, 10));

  EXPECT_THAT(
      rtree.QueryIntersects(Box2_i64(kBig + 5, kBig + 5, kBig + 20, kBig + 20)),
      ElementsAre(Box2_i64(kBig, kBig, kBig + 10, kBig + 10)));
}

}  // namespace
}  // namespace moab

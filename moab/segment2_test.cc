#include "segment2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/str_format.h"
#include "boost/geometry/core/access.hpp"
#include "boost/geometry/core/point_type.hpp"
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/core/tags.hpp"
#include "boost/polygon/polygon.hpp"
#include "moab/point2.h"
#include "moab/segment2.pb.h"

// The tests follow the order of segment2.h.

namespace moab {
namespace {

using ::absl_testing::IsOk;
using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::HasSubstr;
using ::testing::Pair;
using ::testing::StrEq;

namespace bg = ::boost::geometry;
namespace gtl = ::boost::polygon;

// Every coordinate type a Segment2 supports. The typed tests use small
// integers, which are exact in all of them.
using CoordinateTypes = ::testing::Types<int32_t, int64_t, float, double>;

// =============================================================
// Helpers for the protobuf tests
// =============================================================

// Sets the endpoints of a Segment2ProtoInt32/Int64/Float/Double.
template <typename Message, typename V>
void SetEndpoints(Message* m, V x0, V y0, V x1, V y1) {
  m->mutable_p0()->set_x(x0);
  m->mutable_p0()->set_y(y0);
  m->mutable_p1()->set_x(x1);
  m->mutable_p1()->set_y(y1);
}

// Returns a Segment2Proto holding (x0 y0) (x1 y1) in the named variant.
Segment2Proto Int32Proto(int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
  Segment2Proto proto;
  SetEndpoints(proto.mutable_segment_int32(), x0, y0, x1, y1);
  return proto;
}
Segment2Proto Int64Proto(int64_t x0, int64_t y0, int64_t x1, int64_t y1) {
  Segment2Proto proto;
  SetEndpoints(proto.mutable_segment_int64(), x0, y0, x1, y1);
  return proto;
}
Segment2Proto FloatProto(float x0, float y0, float x1, float y1) {
  Segment2Proto proto;
  SetEndpoints(proto.mutable_segment_float(), x0, y0, x1, y1);
  return proto;
}
Segment2Proto DoubleProto(double x0, double y0, double x1, double y1) {
  Segment2Proto proto;
  SetEndpoints(proto.mutable_segment_double(), x0, y0, x1, y1);
  return proto;
}

// The variant ToProto writes for coordinate type T.
template <typename T>
constexpr Segment2Proto::TypeVariantCase VariantOf() {
  if constexpr (std::is_same_v<T, int32_t>) {
    return Segment2Proto::kSegmentInt32;
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return Segment2Proto::kSegmentInt64;
  } else if constexpr (std::is_same_v<T, float>) {
    return Segment2Proto::kSegmentFloat;
  } else {
    static_assert(std::is_same_v<T, double>);
    return Segment2Proto::kSegmentDouble;
  }
}

// The variant message of 'proto' that ToProto writes for coordinate type T.
template <typename T>
const auto& VariantMessage(const Segment2Proto& proto) {
  if constexpr (std::is_same_v<T, int32_t>) {
    return proto.segment_int32();
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return proto.segment_int64();
  } else if constexpr (std::is_same_v<T, float>) {
    return proto.segment_float();
  } else {
    static_assert(std::is_same_v<T, double>);
    return proto.segment_double();
  }
}

// =============================================================
// Constructors
// =============================================================

template <typename T>
class Segment2Constructors : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2Constructors, CoordinateTypes);

TYPED_TEST(Segment2Constructors, DefaultIsTheOrigin) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s;

  EXPECT_EQ(s.p0(), P2(0, 0));
  EXPECT_EQ(s.p1(), P2(0, 0));
}

TYPED_TEST(Segment2Constructors, TwoPointsKeepsTheirOrder) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));

  EXPECT_EQ(s.p0(), P2(1, 2));
  EXPECT_EQ(s.p1(), P2(3, 4));
}

TYPED_TEST(Segment2Constructors, FourCoordinatesAreX0Y0X1Y1) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(1, 2, 3, 4);

  EXPECT_EQ(s.p0(), P2(1, 2));
  EXPECT_EQ(s.p1(), P2(3, 4));

  // The endpoints are not sorted.
  S2 reversed(3, 4, 1, 2);

  EXPECT_EQ(reversed.p0(), P2(3, 4));
  EXPECT_EQ(reversed.p1(), P2(1, 2));
}

TYPED_TEST(Segment2Constructors, ProtoReadsTheEndpoints) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(Int32Proto(1, 2, 3, 4));

  EXPECT_EQ(s.p0(), P2(1, 2));
  EXPECT_EQ(s.p1(), P2(3, 4));
}

TYPED_TEST(Segment2Constructors, CopyKeepsTheEndpoints) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s1(P2(1, 2), P2(3, 4));
  S2 s2(s1);

  EXPECT_EQ(s2.p0(), P2(1, 2));
  EXPECT_EQ(s2.p1(), P2(3, 4));
}

TYPED_TEST(Segment2Constructors, MoveKeepsTheEndpoints) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s1(P2(1, 2), P2(3, 4));
  S2 s2(std::move(s1));

  EXPECT_EQ(s2.p0(), P2(1, 2));
  EXPECT_EQ(s2.p1(), P2(3, 4));
}

// The proto constructor uses CHECK_OK, so it aborts in every build mode.
TEST(Segment2ConstructorsDeathTest, InvalidProtoAborts) {
  const Segment2Proto proto;

  EXPECT_DEATH(
      {
        const Segment2_i s(proto);
        (void)s;
      },
      "The proto has no coordinate type set");
}

// =============================================================
// Assignment operators
// =============================================================

template <typename T>
class Segment2AssignmentOperators : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2AssignmentOperators, CoordinateTypes);

TYPED_TEST(Segment2AssignmentOperators, CopyKeepsTheEndpoints) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s1(P2(1, 2), P2(3, 4));
  S2 s2;

  s2 = s1;

  EXPECT_EQ(s2.p0(), P2(1, 2));
  EXPECT_EQ(s2.p1(), P2(3, 4));
}

TYPED_TEST(Segment2AssignmentOperators, MoveKeepsTheEndpoints) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s1(P2(1, 2), P2(3, 4));
  S2 s2;

  s2 = std::move(s1);

  EXPECT_EQ(s2.p0(), P2(1, 2));
  EXPECT_EQ(s2.p1(), P2(3, 4));
}

// =============================================================
// Accessors
// =============================================================

template <typename T>
class Segment2Accessors : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2Accessors, CoordinateTypes);

TYPED_TEST(Segment2Accessors, P0P1ReturnMutableReferences) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));

  EXPECT_EQ(s.p0(), P2(1, 2));
  EXPECT_EQ(s.p1(), P2(3, 4));

  s.p0() = P2(5, 6);
  s.p1() = P2(7, 8);

  EXPECT_EQ(s, S2(5, 6, 7, 8));
}

TYPED_TEST(Segment2Accessors, P0P1Const) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  const S2 s(P2(1, 2), P2(3, 4));

  EXPECT_EQ(s.p0(), P2(1, 2));
  EXPECT_EQ(s.p1(), P2(3, 4));
}

// data() is read-only, even on a non-const segment.
TYPED_TEST(Segment2Accessors, DataIsReadOnlyP0ThenP1) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  static_assert(
      std::is_same_v<decltype(std::declval<S2&>().data()), const P2*>);
  S2 s(P2(1, 2), P2(3, 4));
  const P2* data = s.data();

  EXPECT_EQ(data[0], P2(1, 2));
  EXPECT_EQ(data[1], P2(3, 4));
}

TYPED_TEST(Segment2Accessors, DataConst) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  const S2 s(P2(1, 2), P2(3, 4));
  const P2* data = s.data();

  EXPECT_EQ(data[0], P2(1, 2));
  EXPECT_EQ(data[1], P2(3, 4));
}

TYPED_TEST(Segment2Accessors, SizeIsTwo) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));

  EXPECT_EQ(s.Size(), 2u);
}

TYPED_TEST(Segment2Accessors, ToPairIsP0ThenP1) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  std::pair<P2, P2> p = s.ToPair();

  EXPECT_THAT(p, Pair(P2(1, 2), P2(3, 4)));
}

TYPED_TEST(Segment2Accessors, LengthIsManhattan) {
  using S2 = Segment2<TypeParam>;
  struct Case {
    std::string name;
    S2 s;
    TypeParam expected;
  };
  const std::vector<Case> cases = {
      {"diagonal", S2(1, 2, 3, 4), 4},
      {"reversed diagonal", S2(3, 4, 1, 2), 4},
      // The Euclidean length of a 3-4 diagonal would be 5.
      {"3-4 diagonal is not Euclidean", S2(0, 0, 3, -4), 7},
      {"horizontal", S2(-2, 1, 3, 1), 5},
      {"vertical", S2(1, 3, 1, -2), 5},
      {"degenerate", S2(2, 2, 2, 2), 0},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.s.Length(), c.expected);
  }
}

// xl()/yl()/xh()/yh() and MinX()/MinY()/MaxX()/MaxY() are per-axis minima and
// maxima over the two unsorted endpoints.
TYPED_TEST(Segment2Accessors, MinMaxCoordinatesAreOverBothEndpoints) {
  using S2 = Segment2<TypeParam>;
  struct Case {
    std::string name;
    S2 s;
    TypeParam xl, yl, xh, yh;
  };
  const std::vector<Case> cases = {
      {"p0 is the low corner", S2(1, 2, 3, 4), 1, 2, 3, 4},
      {"p1 is the low corner", S2(3, 4, 1, 2), 1, 2, 3, 4},
      {"x from p0, y from p1", S2(1, 4, 3, 2), 1, 2, 3, 4},
      {"x from p1, y from p0", S2(3, 2, 1, 4), 1, 2, 3, 4},
      {"negative coordinates", S2(-1, 5, -6, -2), -6, -2, -1, 5},
      {"degenerate", S2(2, 3, 2, 3), 2, 3, 2, 3},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.s.xl(), c.xl);
    EXPECT_EQ(c.s.yl(), c.yl);
    EXPECT_EQ(c.s.xh(), c.xh);
    EXPECT_EQ(c.s.yh(), c.yh);
    EXPECT_EQ(c.s.MinX(), c.xl);
    EXPECT_EQ(c.s.MinY(), c.yl);
    EXPECT_EQ(c.s.MaxX(), c.xh);
    EXPECT_EQ(c.s.MaxY(), c.yh);
  }
}

// MinPoint()/MaxPoint() return a reference to the lexicographically smaller
// or larger endpoint (x, then y); both return p1() when the endpoints are
// equal.
TYPED_TEST(Segment2Accessors, MinMaxPointAreLexicographic) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  struct Case {
    std::string name;
    S2 s;
    std::size_t min_index;
    std::size_t max_index;
  };
  const std::vector<Case> cases = {
      {"p0 has the smaller x", S2(1, 5, 2, 0), 0, 1},
      {"p1 has the smaller x", S2(2, 0, 1, 5), 1, 0},
      {"same x, p0 has the smaller y", S2(1, 2, 1, 3), 0, 1},
      {"same x, p1 has the smaller y", S2(1, 3, 1, 2), 1, 0},
      {"equal endpoints return p1", S2(1, 1, 1, 1), 1, 1},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    S2 s = c.s;
    const S2& const_s = s;

    EXPECT_EQ(&s.MinPoint(), &s[c.min_index]);
    EXPECT_EQ(&s.MaxPoint(), &s[c.max_index]);
    EXPECT_EQ(&const_s.MinPoint(), &const_s[c.min_index]);
    EXPECT_EQ(&const_s.MaxPoint(), &const_s[c.max_index]);
    EXPECT_EQ(const_s.MinPoint(), c.s[c.min_index]);
    EXPECT_EQ(const_s.MaxPoint(), c.s[c.max_index]);
  }

  // The non-const versions write through to the endpoint.
  S2 s(2, 0, 1, 5);
  s.MinPoint() = P2(0, 0);
  s.MaxPoint() = P2(9, 9);

  EXPECT_EQ(s, S2(9, 9, 0, 0));
}

// =============================================================
// Mutators
// =============================================================

template <typename T>
class Segment2Mutators : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2Mutators, CoordinateTypes);

TYPED_TEST(Segment2Mutators, SetByFourCoordinates) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  s.Set(5, 6, 7, 8);

  EXPECT_EQ(s.p0(), P2(5, 6));
  EXPECT_EQ(s.p1(), P2(7, 8));
}

TYPED_TEST(Segment2Mutators, SetByTwoPoints) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  s.Set(P2(5, 6), P2(7, 8));

  EXPECT_EQ(s.p0(), P2(5, 6));
  EXPECT_EQ(s.p1(), P2(7, 8));
}

TYPED_TEST(Segment2Mutators, SetP0P1SetOneEndpointEach) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));

  s.SetP0(P2(5, 6));

  EXPECT_EQ(s, S2(5, 6, 3, 4));

  s.SetP1(P2(7, 8));

  EXPECT_EQ(s.p0(), P2(5, 6));
  EXPECT_EQ(s.p1(), P2(7, 8));
}

TYPED_TEST(Segment2Mutators, SetPByIndex) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));

  s.SetP(0, P2(5, 6));

  EXPECT_EQ(s, S2(5, 6, 3, 4));

  s.SetP(1, P2(7, 8));

  EXPECT_EQ(s.p0(), P2(5, 6));
  EXPECT_EQ(s.p1(), P2(7, 8));
}

// SetP checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(Segment2MutatorsDeathTest, SetPOutOfRange) {
  Segment2_i s(0, 0, 1, 1);

  EXPECT_DEATH(s.SetP(2, Point2_i(5, 6)), "Invalid index. i: 2");
}
#endif

// =============================================================
// Operations
// =============================================================

template <typename T>
class Segment2Operations : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2Operations, CoordinateTypes);

TYPED_TEST(Segment2Operations, ShiftMovesBothEndpoints) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  s.Shift(1, 2);

  EXPECT_EQ(s.p0(), P2(2, 4));
  EXPECT_EQ(s.p1(), P2(4, 6));

  s.Shift(-3, -1);

  EXPECT_EQ(s, S2(-1, 3, 1, 5));
}

TYPED_TEST(Segment2Operations, ShiftXMovesOnlyX) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  s.ShiftX(1);

  EXPECT_EQ(s.p0(), P2(2, 2));
  EXPECT_EQ(s.p1(), P2(4, 4));
}

TYPED_TEST(Segment2Operations, ShiftYMovesOnlyY) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  s.ShiftY(1);

  EXPECT_EQ(s.p0(), P2(1, 3));
  EXPECT_EQ(s.p1(), P2(3, 5));
}

// =============================================================
// Operators
// =============================================================

template <typename T>
class Segment2Operators : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2Operators, CoordinateTypes);

TYPED_TEST(Segment2Operators, SubscriptReadsP0AndP1) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  const S2& const_s = s;

  EXPECT_EQ(s[0], P2(1, 2));
  EXPECT_EQ(s[1], P2(3, 4));
  EXPECT_EQ(&const_s[0], &s.p0());
  EXPECT_EQ(&const_s[1], &s.p1());
}

TYPED_TEST(Segment2Operators, SubscriptMutation) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  s[0] = P2(5, 6);
  s[1] = P2(7, 8);

  EXPECT_EQ(s[0], P2(5, 6));
  EXPECT_EQ(s[1], P2(7, 8));
}

TYPED_TEST(Segment2Operators, EqualityComparesBothEndpointsInOrder) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  struct Case {
    std::string name;
    S2 lhs;
    S2 rhs;
    bool equal;
  };
  const S2 s(P2(1, 2), P2(3, 4));
  const std::vector<Case> cases = {
      {"same endpoints", s, S2(P2(1, 2), P2(3, 4)), true},
      {"both endpoints differ", s, S2(P2(5, 6), P2(7, 8)), false},
      {"p1.y differs", s, S2(P2(1, 2), P2(3, 5)), false},
      {"p1.x differs", s, S2(P2(1, 2), P2(4, 4)), false},
      {"p0 differs", s, S2(P2(0, 2), P2(3, 4)), false},
      {"swapped endpoints", s, S2(P2(3, 4), P2(1, 2)), false},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.lhs == c.rhs, c.equal);
    EXPECT_EQ(c.lhs != c.rhs, !c.equal);
    if (c.equal) {
      EXPECT_EQ(c.lhs, c.rhs);
    } else {
      EXPECT_NE(c.lhs, c.rhs);
    }
  }
}

// operator< compares p0, then p1; >, <= and >= follow from it.
TYPED_TEST(Segment2Operators, OrderingComparesP0ThenP1) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  struct Case {
    std::string name;
    S2 lhs;
    S2 rhs;
    bool less;
    bool greater;
  };
  const S2 s1(P2(1, 2), P2(3, 4));
  const S2 s2(P2(3, 4), P2(5, 6));
  const std::vector<Case> cases = {
      {"smaller p0", s1, s2, true, false},
      {"larger p0", s2, s1, false, true},
      {"equal", s1, S2(P2(1, 2), P2(3, 4)), false, false},
      {"smaller p0 wins over larger p1", S2(1, 2, 9, 9), S2(1, 3, 0, 0), true,
       false},
      {"larger p0 wins over smaller p1", S2(1, 3, 0, 0), S2(1, 2, 9, 9), false,
       true},
      {"equal p0, smaller p1", S2(1, 2, 3, 4), S2(1, 2, 3, 5), true, false},
      {"equal p0, larger p1", S2(1, 2, 4, 0), S2(1, 2, 3, 5), false, true},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.lhs < c.rhs, c.less);
    EXPECT_EQ(c.lhs > c.rhs, c.greater);
    EXPECT_EQ(c.lhs <= c.rhs, !c.greater);
    EXPECT_EQ(c.lhs >= c.rhs, !c.less);
    // The reverse comparisons.
    EXPECT_EQ(c.rhs > c.lhs, c.less);
    EXPECT_EQ(c.rhs < c.lhs, c.greater);
    EXPECT_EQ(c.rhs >= c.lhs, !c.greater);
    EXPECT_EQ(c.rhs <= c.lhs, !c.less);
  }
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(Segment2OperatorsDeathTest, SubscriptOutOfRange) {
  Segment2_i v(0, 0, 1, 1);
  const Segment2_i& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

// =============================================================
// String conversion
// =============================================================

template <typename T>
class Segment2StringConversion : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2StringConversion, CoordinateTypes);

TYPED_TEST(Segment2StringConversion, ToString) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));

  EXPECT_THAT(s.ToString(), StrEq("((1 2) (3 4))"));
  EXPECT_THAT(S2(-1, 0, 5, -6).ToString(), StrEq("((-1 0) (5 -6))"));
}

TYPED_TEST(Segment2StringConversion, SupportsAbslStringify) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 seg(P2(1, 2), P2(3, 4));
  std::string s = absl::StrFormat("%v", seg);

  EXPECT_THAT(s, StrEq("((1 2) (3 4))"));
}

TYPED_TEST(Segment2StringConversion, OstreamWritesToString) {
  using S2 = Segment2<TypeParam>;
  std::ostringstream os;

  os << S2(1, 2, 3, 4) << ";";

  EXPECT_THAT(os.str(), StrEq("((1 2) (3 4));"));
}

TEST(Segment2StringConversionFloatingPoint, PrintsFractions) {
  EXPECT_THAT(Segment2<double>(0.5, 1, 2, 3.5).ToString(),
              StrEq("((0.5 1) (2 3.5))"));
  EXPECT_THAT(Segment2<float>(0.5f, 1, 2, 3.5f).ToString(),
              StrEq("((0.5 1) (2 3.5))"));
}

// =============================================================
// Hash
// =============================================================

template <typename T>
class Segment2Hash : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2Hash, CoordinateTypes);

TYPED_TEST(Segment2Hash, SupportsAbslHash) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      S2(),
      S2(1, 2, 3, 4),
      S2(1, 2, 3, 4),
      S2(P2(1, 2), P2(3, 4)),
      S2(P2(4, 5), P2(7, 6)),
      S2(3, 4, 1, 2),
  }));
}

// =============================================================
// Protobuf
// =============================================================

template <typename T>
class Segment2Protobuf : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2Protobuf, CoordinateTypes);

// ToProto writes the variant of T: segment_int32 for int32_t, and so on.
TYPED_TEST(Segment2Protobuf, ToProtoWritesTheVariantOfT) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(P2(1, 2), P2(3, 4));
  Segment2Proto proto = s.ToProto();

  EXPECT_EQ(proto.type_variant_case(), VariantOf<TypeParam>());
  const auto& m = VariantMessage<TypeParam>(proto);
  EXPECT_EQ(m.p0().x(), 1);
  EXPECT_EQ(m.p0().y(), 2);
  EXPECT_EQ(m.p1().x(), 3);
  EXPECT_EQ(m.p1().y(), 4);
}

TYPED_TEST(Segment2Protobuf, SetFromProto) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s;
  ASSERT_THAT(s.SetFromProto(Int32Proto(1, 2, 3, 4)), IsOk());

  EXPECT_EQ(s.p0(), P2(1, 2));
  EXPECT_EQ(s.p1(), P2(3, 4));
}

TYPED_TEST(Segment2Protobuf, SetFromProtoNotSetLeavesValueUnchanged) {
  using S2 = Segment2<TypeParam>;
  S2 v = S2(1, 2, 3, 4);

  EXPECT_THAT(v.SetFromProto(Segment2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, S2(1, 2, 3, 4));
}

TYPED_TEST(Segment2Protobuf, FromProtoRoundTrip) {
  using S2 = Segment2<TypeParam>;
  EXPECT_THAT(S2::FromProto(S2(4, 3, 2, 1).ToProto()),
              IsOkAndHolds(S2(4, 3, 2, 1)));
  EXPECT_THAT(S2::FromProto(S2(-4, 0, 2, -1).ToProto()),
              IsOkAndHolds(S2(-4, 0, 2, -1)));
}

// FromProto reads every variant into every coordinate type.
TYPED_TEST(Segment2Protobuf, FromProtoReadsEveryVariant) {
  using S2 = Segment2<TypeParam>;
  struct Case {
    std::string name;
    Segment2Proto proto;
  };
  const std::vector<Case> cases = {
      {"int32", Int32Proto(4, -3, 2, -1)},
      {"int64", Int64Proto(4, -3, 2, -1)},
      {"float", FloatProto(4, -3, 2, -1)},
      {"double", DoubleProto(4, -3, 2, -1)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_THAT(S2::FromProto(c.proto), IsOkAndHolds(S2(4, -3, 2, -1)));
    S2 s;
    EXPECT_THAT(s.SetFromProto(c.proto), IsOk());
    EXPECT_EQ(s, S2(4, -3, 2, -1));
  }
}

TYPED_TEST(Segment2Protobuf, FromProtoNotSet) {
  using S2 = Segment2<TypeParam>;
  EXPECT_THAT(S2::FromProto(Segment2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment2] The proto has no coordinate type "
                                 "set.")));
}

TEST(Segment2ProtobufFloatingPoint, FromProtoRoundTripKeepsFractions) {
  EXPECT_THAT(
      Segment2<double>::FromProto(Segment2<double>(0.5, 1, 2, 3.5).ToProto()),
      IsOkAndHolds(Segment2<double>(0.5, 1, 2, 3.5)));
  EXPECT_THAT(
      Segment2<float>::FromProto(Segment2<float>(0.5f, 1, 2, 3.5f).ToProto()),
      IsOkAndHolds(Segment2<float>(0.5f, 1, 2, 3.5f)));
}

// Each coordinate is converted on its own, and the error names it.
TEST(Segment2ProtobufErrors, FromProtoIntoInt32NamesTheBadCoordinate) {
  struct Case {
    std::string name;
    Segment2Proto proto;
    std::string message;
  };
  const std::vector<Case> cases = {
      {"p0.x is not a whole number", FloatProto(0.5f, 0, 0, 0),
       "[Segment2] p0.x: coordinate 0.5 is not a whole number, so it does not "
       "fit in int32."},
      {"p0.y is not finite",
       DoubleProto(0, std::numeric_limits<double>::infinity(), 0, 0),
       "[Segment2] p0.y: coordinate inf is not finite."},
      {"p1.x is above the int32 range", Int64Proto(0, 0, int64_t{1} << 40, 0),
       "[Segment2] p1.x: coordinate 1099511627776 does not fit in int32."},
      {"p1.y is below the int32 range",
       Int64Proto(0, 0, 0, -(int64_t{1} << 40)),
       "[Segment2] p1.y: coordinate -1099511627776 does not fit in int32."},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_THAT(
        Segment2_i::FromProto(c.proto),
        StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr(c.message)));
    Segment2_i s(1, 2, 3, 4);
    EXPECT_THAT(
        s.SetFromProto(c.proto),
        StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr(c.message)));
    EXPECT_EQ(s, Segment2_i(1, 2, 3, 4));
  }
}

TEST(Segment2ProtobufErrors, FromProtoDoubleTooLargeForFloat) {
  EXPECT_THAT(Segment2<float>::FromProto(DoubleProto(0, 0, 0, 1e300)),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment2] p1.y: coordinate 1e+300 does not "
                                 "fit in float.")));
}

// =============================================================
// Aliases
// =============================================================

TEST(Segment2Aliases, NameTheIntegerSegments) {
  static_assert(std::is_same_v<Segment2_i, Segment2<int>>);
  static_assert(std::is_same_v<Segment2_i32, Segment2<int32_t>>);
  static_assert(std::is_same_v<Segment2_i64, Segment2<int64_t>>);
}

// =============================================================
// Boost geometry traits
// =============================================================

template <typename T>
class Segment2BoostGeometry : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2BoostGeometry, CoordinateTypes);

TYPED_TEST(Segment2BoostGeometry, IsASegmentOfPoint2) {
  using S2 = Segment2<TypeParam>;
  static_assert(std::is_same_v<bg::tag_t<S2>, bg::segment_tag>);
  static_assert(std::is_same_v<bg::point_type_t<S2>, Point2<TypeParam>>);
}

// Index 0 is p0 and index 1 is p1; dimension 0 is x and dimension 1 is y.
TYPED_TEST(Segment2BoostGeometry, IndexedGet) {
  using S2 = Segment2<TypeParam>;
  const S2 s(1, 2, 3, 4);

  EXPECT_EQ((bg::get<0, 0>(s)), 1);
  EXPECT_EQ((bg::get<0, 1>(s)), 2);
  EXPECT_EQ((bg::get<1, 0>(s)), 3);
  EXPECT_EQ((bg::get<1, 1>(s)), 4);
}

TYPED_TEST(Segment2BoostGeometry, IndexedSet) {
  using S2 = Segment2<TypeParam>;
  S2 s(1, 2, 3, 4);

  bg::set<0, 0>(s, TypeParam{5});

  EXPECT_EQ(s, S2(5, 2, 3, 4));

  bg::set<0, 1>(s, TypeParam{6});
  bg::set<1, 0>(s, TypeParam{7});
  bg::set<1, 1>(s, TypeParam{8});

  EXPECT_EQ(s, S2(5, 6, 7, 8));
}

// =============================================================
// Boost polygon traits
// =============================================================

template <typename T>
class Segment2BoostPolygon : public ::testing::Test {};
TYPED_TEST_SUITE(Segment2BoostPolygon, CoordinateTypes);

// Boost.Polygon sees p0 as LOW and p1 as HIGH, without reordering.
TYPED_TEST(Segment2BoostPolygon, LowHigh) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(10, 0, 0, 0);

  EXPECT_EQ(gtl::low(s), P2(10, 0));
  EXPECT_EQ(gtl::high(s), P2(0, 0));
}

TYPED_TEST(Segment2BoostPolygon, SetLowThenHigh) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(0, 0, 10, 0);

  gtl::set(s, gtl::LOW, P2(20, 0));
  gtl::set(s, gtl::HIGH, P2(30, 0));

  EXPECT_EQ(s, S2(20, 0, 30, 0));
}

TYPED_TEST(Segment2BoostPolygon, SetHighThenLow) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(0, 0, 10, 0);

  gtl::set(s, gtl::HIGH, P2(-10, 0));
  gtl::set(s, gtl::LOW, P2(-20, 0));

  EXPECT_EQ(s, S2(-20, 0, -10, 0));
}

TYPED_TEST(Segment2BoostPolygon, SetDegenerate) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(5, 5, 5, 5);

  gtl::set(s, gtl::LOW, P2(0, 0));
  gtl::set(s, gtl::HIGH, P2(9, 9));

  EXPECT_EQ(s, S2(0, 0, 9, 9));
}

TYPED_TEST(Segment2BoostPolygon, ConstructKeepsTheOrder) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s = gtl::construct<S2>(P2(3, 4), P2(1, 2));

  EXPECT_EQ(s, S2(3, 4, 1, 2));
}

TYPED_TEST(Segment2BoostPolygon, Convolve) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(0, 0, 10, 0);

  gtl::convolve(s, P2(100, 0));

  EXPECT_EQ(s, S2(100, 0, 110, 0));
}

TYPED_TEST(Segment2BoostPolygon, ConvolveReversed) {
  using S2 = Segment2<TypeParam>;
  using P2 = Point2<TypeParam>;
  S2 s(10, 0, 0, 0);

  gtl::convolve(s, P2(0, 7));

  EXPECT_EQ(s, S2(10, 7, 0, 7));
}

TYPED_TEST(Segment2BoostPolygon, Move) {
  using S2 = Segment2<TypeParam>;
  S2 s(0, 0, 10, 5);

  gtl::move(s, gtl::HORIZONTAL, 3);
  gtl::move(s, gtl::VERTICAL, -2);

  EXPECT_EQ(s, S2(3, -2, 13, 3));
}

TYPED_TEST(Segment2BoostPolygon, ScaleUp) {
  using S2 = Segment2<TypeParam>;
  S2 s(1, 2, 3, 4);

  gtl::scale_up(s, 10);

  EXPECT_EQ(s, S2(10, 20, 30, 40));
}

}  // namespace
}  // namespace moab

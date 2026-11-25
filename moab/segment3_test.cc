#include "segment3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
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
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/core/tags.hpp"
#include "boost/geometry/geometries/concepts/check.hpp"
#include "moab/point3.h"
#include "moab/segment3.pb.h"

// The tests follow the order of segment3.h.

namespace moab {
namespace {

namespace bg = boost::geometry;

using ::absl_testing::IsOk;
using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::ElementsAre;
using ::testing::HasSubstr;
using ::testing::Pair;
using ::testing::StrEq;

using CoordinateTypes = ::testing::Types<int32_t, int64_t, float, double>;

// The proto variants FromProto reads, in declaration order.
constexpr std::array<Segment3Proto::TypeVariantCase, 4> kProtoVariants = {
    Segment3Proto::kSegmentInt32, Segment3Proto::kSegmentInt64,
    Segment3Proto::kSegmentFloat, Segment3Proto::kSegmentDouble};

// The proto variant ToProto writes for coordinate type T.
template <typename T>
constexpr Segment3Proto::TypeVariantCase VariantOf() {
  if constexpr (std::is_same_v<T, int32_t>) {
    return Segment3Proto::kSegmentInt32;
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return Segment3Proto::kSegmentInt64;
  } else if constexpr (std::is_same_v<T, float>) {
    return Segment3Proto::kSegmentFloat;
  } else {
    return Segment3Proto::kSegmentDouble;
  }
}

// Returns a Segment3Proto with 'variant' set to the coordinates
// {p0.x, p0.y, p0.z, p1.x, p1.y, p1.z}. Each coordinate is cast to the field
// type of the variant.
Segment3Proto MakeProto(Segment3Proto::TypeVariantCase variant,
                        const std::array<double, 6>& c) {
  Segment3Proto proto;
  const auto fill = [&c](auto* s) {
    using Field = decltype(s->p0().x());
    s->mutable_p0()->set_x(static_cast<Field>(c[0]));
    s->mutable_p0()->set_y(static_cast<Field>(c[1]));
    s->mutable_p0()->set_z(static_cast<Field>(c[2]));
    s->mutable_p1()->set_x(static_cast<Field>(c[3]));
    s->mutable_p1()->set_y(static_cast<Field>(c[4]));
    s->mutable_p1()->set_z(static_cast<Field>(c[5]));
  };
  switch (variant) {
    case Segment3Proto::kSegmentInt32:
      fill(proto.mutable_segment_int32());
      break;
    case Segment3Proto::kSegmentInt64:
      fill(proto.mutable_segment_int64());
      break;
    case Segment3Proto::kSegmentFloat:
      fill(proto.mutable_segment_float());
      break;
    case Segment3Proto::kSegmentDouble:
      fill(proto.mutable_segment_double());
      break;
    case Segment3Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return proto;
}

// Returns the coordinates {p0.x, p0.y, p0.z, p1.x, p1.y, p1.z} of whichever
// variant 'proto' has set, or all zeros if none is set.
std::array<double, 6> ProtoCoordinates(const Segment3Proto& proto) {
  const auto read = [](const auto& s) {
    return std::array<double, 6>{
        static_cast<double>(s.p0().x()), static_cast<double>(s.p0().y()),
        static_cast<double>(s.p0().z()), static_cast<double>(s.p1().x()),
        static_cast<double>(s.p1().y()), static_cast<double>(s.p1().z())};
  };
  switch (proto.type_variant_case()) {
    case Segment3Proto::kSegmentInt32:
      return read(proto.segment_int32());
    case Segment3Proto::kSegmentInt64:
      return read(proto.segment_int64());
    case Segment3Proto::kSegmentFloat:
      return read(proto.segment_float());
    case Segment3Proto::kSegmentDouble:
      return read(proto.segment_double());
    case Segment3Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return {};
}

// =============================================================
// Constructors
// =============================================================

template <typename T>
class Segment3Constructors : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3Constructors, CoordinateTypes);

TYPED_TEST(Segment3Constructors, DefaultIsAllZeros) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s;

  EXPECT_EQ(s.p0(), P(0, 0, 0));
  EXPECT_EQ(s.p1(), P(0, 0, 0));
}

TYPED_TEST(Segment3Constructors, TwoPoints) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));

  EXPECT_EQ(s.p0(), P(1, 2, 3));
  EXPECT_EQ(s.p1(), P(4, 5, 6));
}

TYPED_TEST(Segment3Constructors, SixCoordinatesAreP0ThenP1) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(1, 2, 3, 4, 5, 6);

  EXPECT_EQ(s.p0(), P(1, 2, 3));
  EXPECT_EQ(s.p1(), P(4, 5, 6));
}

// The constructor keeps the endpoints as given; it does not sort them.
TYPED_TEST(Segment3Constructors, SixCoordinatesAreNotSorted) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(6, 5, 4, 3, 2, 1);

  EXPECT_EQ(s.p0(), P(6, 5, 4));
  EXPECT_EQ(s.p1(), P(3, 2, 1));
}

TYPED_TEST(Segment3Constructors, ProtoOfEachVariant) {
  using P = Point3<TypeParam>;
  for (const Segment3Proto::TypeVariantCase variant : kProtoVariants) {
    SCOPED_TRACE(variant);
    Segment3<TypeParam> s(MakeProto(variant, {1, 2, 3, 4, 5, 6}));

    EXPECT_EQ(s.p0(), P(1, 2, 3));
    EXPECT_EQ(s.p1(), P(4, 5, 6));
  }
}

TYPED_TEST(Segment3Constructors, Copy) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s1(P(1, 2, 3), P(4, 5, 6));
  Segment3<TypeParam> s2(s1);

  EXPECT_EQ(s2.p0(), P(1, 2, 3));
  EXPECT_EQ(s2.p1(), P(4, 5, 6));
}

TYPED_TEST(Segment3Constructors, Move) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s1(P(1, 2, 3), P(4, 5, 6));
  Segment3<TypeParam> s2(std::move(s1));

  EXPECT_EQ(s2.p0(), P(1, 2, 3));
  EXPECT_EQ(s2.p1(), P(4, 5, 6));
}

// The proto constructor CHECKs the result of SetFromProto, in every build.
TEST(Segment3ConstructorsDeathTest, InvalidProtoAborts) {
  EXPECT_DEATH(
      {
        Segment3_i s((Segment3Proto()));
        (void)s;
      },
      "no coordinate type set");
}

// =============================================================
// Assignment operators
// =============================================================

template <typename T>
class Segment3Assignment : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3Assignment, CoordinateTypes);

TYPED_TEST(Segment3Assignment, Copy) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s1(P(1, 2, 3), P(4, 5, 6));
  Segment3<TypeParam> s2;

  s2 = s1;

  EXPECT_EQ(s2.p0(), P(1, 2, 3));
  EXPECT_EQ(s2.p1(), P(4, 5, 6));
}

TYPED_TEST(Segment3Assignment, Move) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s1(P(1, 2, 3), P(4, 5, 6));
  Segment3<TypeParam> s2;

  s2 = std::move(s1);

  EXPECT_EQ(s2.p0(), P(1, 2, 3));
  EXPECT_EQ(s2.p1(), P(4, 5, 6));
}

// =============================================================
// Accessors
// =============================================================

template <typename T>
class Segment3Accessors : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3Accessors, CoordinateTypes);

TYPED_TEST(Segment3Accessors, P0P1) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));

  EXPECT_EQ(s.p0(), P(1, 2, 3));
  EXPECT_EQ(s.p1(), P(4, 5, 6));
}

TYPED_TEST(Segment3Accessors, P0P1Const) {
  using P = Point3<TypeParam>;
  const Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));

  EXPECT_EQ(s.p0(), P(1, 2, 3));
  EXPECT_EQ(s.p1(), P(4, 5, 6));
}

TYPED_TEST(Segment3Accessors, P0P1ReturnWritableReferences) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));

  s.p0() = P(7, 8, 9);
  s.p1() = P(10, 11, 12);

  EXPECT_EQ(s, Segment3<TypeParam>(7, 8, 9, 10, 11, 12));
}

// data() is read-only, even on a non-const segment.
TYPED_TEST(Segment3Accessors, Data) {
  using P = Point3<TypeParam>;
  static_assert(
      std::is_same_v<decltype(std::declval<Segment3<TypeParam>&>().data()),
                     const P*>);
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  const P* data = s.data();

  EXPECT_EQ(data[0], P(1, 2, 3));
  EXPECT_EQ(data[1], P(4, 5, 6));
}

TYPED_TEST(Segment3Accessors, DataConst) {
  using P = Point3<TypeParam>;
  const Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  const P* data = s.data();

  EXPECT_EQ(data[0], P(1, 2, 3));
  EXPECT_EQ(data[1], P(4, 5, 6));
}

TYPED_TEST(Segment3Accessors, SizeIsTwo) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));

  EXPECT_EQ(s.Size(), 2u);
}

TYPED_TEST(Segment3Accessors, ToPair) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  std::pair<P, P> p = s.ToPair();

  EXPECT_THAT(p, Pair(P(1, 2, 3), P(4, 5, 6)));
}

// ManhattanLength() is |dx| + |dy| + |dz|, whatever the direction of the
// segment.
TYPED_TEST(Segment3Accessors, ManhattanLength) {
  using S = Segment3<TypeParam>;
  struct Case {
    std::string name;
    S s;
    TypeParam expected;
  };
  const std::vector<Case> cases = {
      {"increasing coordinates", S(1, 2, 3, 4, 5, 6), 9},
      {"decreasing coordinates", S(4, 5, 6, 1, 2, 3), 9},
      {"mixed directions", S(0, 0, 0, 3, -4, 5), 12},
      {"not Euclidean (which would be 5)", S(0, 0, 0, 3, 4, 0), 7},
      {"degenerate", S(1, 1, 1, 1, 1, 1), 0},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.s.ManhattanLength(), c.expected);
  }
}

// xl() .. zh() and MinX() .. MaxZ() are the min and max over both endpoints,
// per axis, whatever the order of the endpoints.
TYPED_TEST(Segment3Accessors, CoordinateBoundsOverBothEndpoints) {
  using S = Segment3<TypeParam>;
  struct Case {
    std::string name;
    S s;
    std::array<TypeParam, 6> expected;  // {xl, yl, zl, xh, yh, zh}
  };
  const std::vector<Case> cases = {
      {"p0 is the low corner", S(1, 2, 3, 4, 5, 6), {1, 2, 3, 4, 5, 6}},
      {"p1 is the low corner", S(4, 5, 6, 1, 2, 3), {1, 2, 3, 4, 5, 6}},
      {"mixed per axis", S(1, 5, 3, 4, 2, 6), {1, 2, 3, 4, 5, 6}},
      {"mixed per axis, swapped", S(4, 2, 6, 1, 5, 3), {1, 2, 3, 4, 5, 6}},
      {"negative coordinates", S(0, -1, 2, -3, 4, -5), {-3, -1, -5, 0, 4, 2}},
      {"degenerate", S(7, 7, 7, 7, 7, 7), {7, 7, 7, 7, 7, 7}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_THAT((std::array<TypeParam, 6>{c.s.xl(), c.s.yl(), c.s.zl(),
                                          c.s.xh(), c.s.yh(), c.s.zh()}),
                ElementsAre(c.expected[0], c.expected[1], c.expected[2],
                            c.expected[3], c.expected[4], c.expected[5]));
    EXPECT_THAT((std::array<TypeParam, 6>{c.s.MinX(), c.s.MinY(), c.s.MinZ(),
                                          c.s.MaxX(), c.s.MaxY(), c.s.MaxZ()}),
                ElementsAre(c.expected[0], c.expected[1], c.expected[2],
                            c.expected[3], c.expected[4], c.expected[5]));
  }
}

// MinPoint() and MaxPoint() return a reference to the lexicographically
// smaller and larger endpoint (x, then y, then z). For equal endpoints, both
// return p1().
TYPED_TEST(Segment3Accessors, MinPointMaxPointPickAnEndpoint) {
  using S = Segment3<TypeParam>;
  struct Case {
    std::string name;
    S s;
    bool min_is_p0;
    bool max_is_p0;
  };
  const std::vector<Case> cases = {
      {"x decides, p0 smaller", S(1, 9, 9, 2, 0, 0), true, false},
      {"x decides, p1 smaller", S(2, 0, 0, 1, 9, 9), false, true},
      {"equal x, y decides", S(1, 2, 9, 1, 3, 0), true, false},
      {"equal x and y, z decides", S(1, 2, 4, 1, 2, 3), false, true},
      {"equal endpoints both return p1", S(1, 2, 3, 1, 2, 3), false, false},
  };
  for (Case c : cases) {
    SCOPED_TRACE(c.name);
    S& s = c.s;
    const S& const_s = c.s;
    const auto* const expected_min = c.min_is_p0 ? &s.p0() : &s.p1();
    const auto* const expected_max = c.max_is_p0 ? &s.p0() : &s.p1();

    EXPECT_EQ(&s.MinPoint(), expected_min);
    EXPECT_EQ(&s.MaxPoint(), expected_max);
    EXPECT_EQ(&const_s.MinPoint(), expected_min);
    EXPECT_EQ(&const_s.MaxPoint(), expected_max);
  }
}

TYPED_TEST(Segment3Accessors, MinPointMaxPointReturnWritableReferences) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(4, 5, 6, 1, 2, 3);

  s.MinPoint() = P(0, 0, 0);  // p1
  s.MaxPoint() = P(9, 9, 9);  // p0

  EXPECT_EQ(s, Segment3<TypeParam>(9, 9, 9, 0, 0, 0));
}

// =============================================================
// Mutators
// =============================================================

template <typename T>
class Segment3Mutators : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3Mutators, CoordinateTypes);

TYPED_TEST(Segment3Mutators, SetBySixCoordinates) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.Set(7, 8, 9, 10, 11, 12);

  EXPECT_EQ(s.p0(), P(7, 8, 9));
  EXPECT_EQ(s.p1(), P(10, 11, 12));
}

TYPED_TEST(Segment3Mutators, SetByTwoPoints) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.Set(P(7, 8, 9), P(10, 11, 12));

  EXPECT_EQ(s.p0(), P(7, 8, 9));
  EXPECT_EQ(s.p1(), P(10, 11, 12));
}

TYPED_TEST(Segment3Mutators, SetP0P1) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.SetP0(P(7, 8, 9));
  s.SetP1(P(10, 11, 12));

  EXPECT_EQ(s.p0(), P(7, 8, 9));
  EXPECT_EQ(s.p1(), P(10, 11, 12));
}

TYPED_TEST(Segment3Mutators, SetP) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.SetP(0, P(7, 8, 9));
  s.SetP(1, P(10, 11, 12));

  EXPECT_EQ(s.p0(), P(7, 8, 9));
  EXPECT_EQ(s.p1(), P(10, 11, 12));
}

// SetP checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(Segment3MutatorsDeathTest, SetPOutOfRange) {
  Segment3_i s(0, 0, 0, 1, 1, 1);

  EXPECT_DEATH(s.SetP(2, Point3_i(7, 8, 9)), "Invalid index. i: 2");
}
#endif

// =============================================================
// Operations
// =============================================================

template <typename T>
class Segment3Operations : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3Operations, CoordinateTypes);

TYPED_TEST(Segment3Operations, ShiftMovesBothEndpoints) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.Shift(1, 2, 3);

  EXPECT_EQ(s.p0(), P(2, 4, 6));
  EXPECT_EQ(s.p1(), P(5, 7, 9));
}

TYPED_TEST(Segment3Operations, ShiftX) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.ShiftX(1);

  EXPECT_EQ(s.p0(), P(2, 2, 3));
  EXPECT_EQ(s.p1(), P(5, 5, 6));
}

TYPED_TEST(Segment3Operations, ShiftY) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.ShiftY(1);

  EXPECT_EQ(s.p0(), P(1, 3, 3));
  EXPECT_EQ(s.p1(), P(4, 6, 6));
}

TYPED_TEST(Segment3Operations, ShiftZ) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.ShiftZ(1);

  EXPECT_EQ(s.p0(), P(1, 2, 4));
  EXPECT_EQ(s.p1(), P(4, 5, 7));
}

TYPED_TEST(Segment3Operations, ShiftByNegativeOffsets) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s.Shift(-1, -2, -3);

  EXPECT_EQ(s.p0(), P(0, 0, 0));
  EXPECT_EQ(s.p1(), P(3, 3, 3));
}

// =============================================================
// Operators
// =============================================================

template <typename T>
class Segment3Operators : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3Operators, CoordinateTypes);

TYPED_TEST(Segment3Operators, SubscriptAccess) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));

  EXPECT_EQ(s[0], P(1, 2, 3));
  EXPECT_EQ(s[1], P(4, 5, 6));
}

TYPED_TEST(Segment3Operators, SubscriptAccessConst) {
  using P = Point3<TypeParam>;
  const Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));

  EXPECT_EQ(s[0], P(1, 2, 3));
  EXPECT_EQ(s[1], P(4, 5, 6));
}

TYPED_TEST(Segment3Operators, SubscriptMutation) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  s[0] = P(7, 8, 9);
  s[1] = P(10, 11, 12);

  EXPECT_EQ(s[0], P(7, 8, 9));
  EXPECT_EQ(s[1], P(10, 11, 12));
}

TYPED_TEST(Segment3Operators, Equality) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s1(P(1, 2, 3), P(4, 5, 6));
  Segment3<TypeParam> s2(P(1, 2, 3), P(4, 5, 6));
  Segment3<TypeParam> s3(P(7, 8, 9), P(10, 11, 12));

  EXPECT_TRUE(s1 == s2);
  EXPECT_FALSE(s1 == s3);
  EXPECT_EQ(s1, s2);
}

TYPED_TEST(Segment3Operators, NotEquality) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s1(P(1, 2, 3), P(4, 5, 6));
  Segment3<TypeParam> s2(P(1, 2, 3), P(4, 5, 6));
  Segment3<TypeParam> s3(P(7, 8, 9), P(10, 11, 12));
  Segment3<TypeParam> s4(P(1, 2, 3), P(3, 5, 6));
  Segment3<TypeParam> s5(P(1, 2, 3), P(4, 4, 4));

  EXPECT_FALSE(s1 != s2);
  EXPECT_TRUE(s1 != s3);
  EXPECT_TRUE(s1 != s4);
  EXPECT_TRUE(s1 != s5);
  EXPECT_NE(s1, s3);
  EXPECT_NE(s1, s4);
  EXPECT_NE(s1, s5);
}

// The segments are equal only when both endpoints are equal in order: the
// reversed segment is a different value.
TYPED_TEST(Segment3Operators, ReversedSegmentIsNotEqual) {
  using S = Segment3<TypeParam>;

  EXPECT_NE(S(1, 2, 3, 4, 5, 6), S(4, 5, 6, 1, 2, 3));
}

// operator< and friends order lexicographically: p0 first, then p1.
TYPED_TEST(Segment3Operators, RelationalOrderComparesP0ThenP1) {
  using S = Segment3<TypeParam>;
  struct Case {
    std::string name;
    S a;
    S b;
    int order;  // -1 if a < b, 0 if a == b, 1 if a > b.
  };
  const std::vector<Case> cases = {
      {"p0 smaller", S(1, 2, 3, 4, 5, 6), S(4, 5, 6, 7, 8, 9), -1},
      {"equal", S(1, 2, 3, 4, 5, 6), S(1, 2, 3, 4, 5, 6), 0},
      {"same p0, p1 decides", S(1, 2, 3, 4, 5, 6), S(1, 2, 3, 4, 5, 7), -1},
      {"p0 decides before p1", S(1, 2, 3, 9, 9, 9), S(1, 2, 4, 0, 0, 0), -1},
      {"p0 larger", S(2, 0, 0, 0, 0, 0), S(1, 9, 9, 9, 9, 9), 1},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.a < c.b, c.order < 0);
    EXPECT_EQ(c.a > c.b, c.order > 0);
    EXPECT_EQ(c.a <= c.b, c.order <= 0);
    EXPECT_EQ(c.a >= c.b, c.order >= 0);
    EXPECT_EQ(c.b < c.a, c.order > 0);
    EXPECT_EQ(c.b > c.a, c.order < 0);
    EXPECT_EQ(c.b <= c.a, c.order >= 0);
    EXPECT_EQ(c.b >= c.a, c.order <= 0);
  }
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(Segment3OperatorsDeathTest, SubscriptOutOfRange) {
  Segment3_i v(0, 0, 0, 1, 1, 1);
  const Segment3_i& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

// =============================================================
// String conversion
// =============================================================

// Small whole numbers print the same for every coordinate type.
template <typename T>
class Segment3StringConversion : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3StringConversion, CoordinateTypes);

TYPED_TEST(Segment3StringConversion, ToString) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));

  EXPECT_THAT(s.ToString(), StrEq("((1 2 3) (4 5 6))"));
}

TYPED_TEST(Segment3StringConversion, SupportsAbslStringify) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> seg(P(1, 2, 3), P(4, 5, 6));
  std::string s = absl::StrFormat("%v", seg);

  EXPECT_THAT(s, StrEq("((1 2 3) (4 5 6))"));
}

TYPED_TEST(Segment3StringConversion, StreamOperator) {
  std::ostringstream os;
  os << Segment3<TypeParam>(1, 2, 3, -4, -5, -6);

  EXPECT_THAT(os.str(), StrEq("((1 2 3) (-4 -5 -6))"));
}

TEST(Segment3StringConversionFloat, FractionsPrintShortest) {
  EXPECT_THAT(Segment3<double>(0.5, 1.25, 0, 0, 0, -2.5).ToString(),
              StrEq("((0.5 1.25 0) (0 0 -2.5))"));
}

// =============================================================
// Hash
// =============================================================

template <typename T>
class Segment3Hash : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3Hash, CoordinateTypes);

TYPED_TEST(Segment3Hash, SupportsAbslHash) {
  using P = Point3<TypeParam>;
  using S = Segment3<TypeParam>;
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      S(),
      S(1, 2, 3, 4, 5, 6),
      S(1, 2, 3, 4, 5, 6),
      S(P(1, 2, 3), P(4, 5, 6)),
      S(P(7, 8, 9), P(10, 11, 12)),
      S(4, 5, 6, 1, 2, 3),
  }));
}

// =============================================================
// Protobuf
// =============================================================

template <typename T>
class Segment3Protobuf : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3Protobuf, CoordinateTypes);

// ToProto sets the variant that matches T.
TYPED_TEST(Segment3Protobuf, ToProto) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s(P(1, 2, 3), P(4, 5, 6));
  Segment3Proto proto = s.ToProto();

  EXPECT_EQ(proto.type_variant_case(), VariantOf<TypeParam>());
  EXPECT_THAT(ProtoCoordinates(proto), ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(Segment3ProtobufInt32, ToProtoFields) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3Proto proto = s.ToProto();

  EXPECT_TRUE(proto.has_segment_int32());
  EXPECT_EQ(proto.segment_int32().p0().x(), 1);
  EXPECT_EQ(proto.segment_int32().p0().y(), 2);
  EXPECT_EQ(proto.segment_int32().p0().z(), 3);
  EXPECT_EQ(proto.segment_int32().p1().x(), 4);
  EXPECT_EQ(proto.segment_int32().p1().y(), 5);
  EXPECT_EQ(proto.segment_int32().p1().z(), 6);
}

TYPED_TEST(Segment3Protobuf, SetFromProto) {
  using P = Point3<TypeParam>;
  Segment3<TypeParam> s;
  ASSERT_THAT(
      s.SetFromProto(MakeProto(VariantOf<TypeParam>(), {1, 2, 3, 4, 5, 6})),
      IsOk());

  EXPECT_EQ(s.p0(), P(1, 2, 3));
  EXPECT_EQ(s.p1(), P(4, 5, 6));
}

TYPED_TEST(Segment3Protobuf, SetFromProtoNotSetLeavesValueUnchanged) {
  using S = Segment3<TypeParam>;
  S v = S(1, 2, 3, 4, 5, 6);

  EXPECT_THAT(v.SetFromProto(Segment3Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, S(1, 2, 3, 4, 5, 6));
}

TYPED_TEST(Segment3Protobuf, FromProtoRoundTrip) {
  using S = Segment3<TypeParam>;

  EXPECT_THAT(S::FromProto(S(6, 5, 4, 3, 2, 1).ToProto()),
              IsOkAndHolds(S(6, 5, 4, 3, 2, 1)));
}

// FromProto reads every variant into every T, as long as the values fit.
TYPED_TEST(Segment3Protobuf, FromProtoOfEachVariant) {
  using S = Segment3<TypeParam>;
  for (const Segment3Proto::TypeVariantCase variant : kProtoVariants) {
    SCOPED_TRACE(variant);

    EXPECT_THAT(S::FromProto(MakeProto(variant, {6, 5, 4, -3, -2, -1})),
                IsOkAndHolds(S(6, 5, 4, -3, -2, -1)));
  }
}

TYPED_TEST(Segment3Protobuf, FromProtoNotSet) {
  EXPECT_THAT(Segment3<TypeParam>::FromProto(Segment3Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment3] The proto has no coordinate type "
                                 "set.")));
}

TEST(Segment3ProtobufErrors, FromProtoNotWholeNumber) {
  Segment3Proto proto;
  proto.mutable_segment_double()->mutable_p0()->set_z(2.25);

  EXPECT_THAT(Segment3_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment3] p0.z: coordinate 2.25 is not a "
                                 "whole number")));
}

// The error names the coordinate that failed to convert.
TEST(Segment3ProtobufErrors, FromProtoNamesTheBadCoordinate) {
  const std::array<std::string, 6> names = {"p0.x", "p0.y", "p0.z",
                                            "p1.x", "p1.y", "p1.z"};
  for (std::size_t i = 0; i < names.size(); ++i) {
    SCOPED_TRACE(names[i]);
    std::array<double, 6> c = {};
    c[i] = 0.5;

    EXPECT_THAT(
        Segment3_i64::FromProto(MakeProto(Segment3Proto::kSegmentFloat, c)),
        StatusIs(absl::StatusCode::kInvalidArgument,
                 HasSubstr(absl::StrFormat(
                     "[Segment3] %s: coordinate 0.5 is not a whole number",
                     names[i]))));
  }
}

TEST(Segment3ProtobufErrors, FromProtoConversionErrors) {
  constexpr double kNan = std::numeric_limits<double>::quiet_NaN();
  constexpr double kInf = std::numeric_limits<double>::infinity();
  struct Case {
    std::string name;
    absl::Status status;
    std::string expected;
  };
  const std::vector<Case> cases = {
      {"int64 too large for int32",
       Segment3_i32::FromProto(MakeProto(Segment3Proto::kSegmentInt64,
                                         {0, 0, 0, 0, 1099511627776.0, 0}))
           .status(),
       "[Segment3] p1.y: coordinate 1099511627776 does not fit in int32."},
      {"double too large for float",
       Segment3<float>::FromProto(
           MakeProto(Segment3Proto::kSegmentDouble, {0, 0, 0, 0, 0, 1e300}))
           .status(),
       "[Segment3] p1.z: coordinate 1e+300 does not fit in float."},
      {"NaN into double",
       Segment3<double>::FromProto(
           MakeProto(Segment3Proto::kSegmentDouble, {kNan, 0, 0, 0, 0, 0}))
           .status(),
       "[Segment3] p0.x: coordinate nan is not finite."},
      {"infinity into int64",
       Segment3_i64::FromProto(
           MakeProto(Segment3Proto::kSegmentFloat, {0, 0, 0, -kInf, 0, 0}))
           .status(),
       "[Segment3] p1.x: coordinate -inf is not finite."},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_THAT(c.status, StatusIs(absl::StatusCode::kInvalidArgument,
                                   HasSubstr(c.expected)));
  }
}

TEST(Segment3ProtobufErrors, SetFromProtoErrorLeavesValueUnchanged) {
  Segment3_i s(1, 2, 3, 4, 5, 6);

  EXPECT_THAT(s.SetFromProto(MakeProto(Segment3Proto::kSegmentDouble,
                                       {7, 8, 9, 10, 11, 0.5})),
              StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr("p1.z")));
  EXPECT_EQ(s, Segment3_i(1, 2, 3, 4, 5, 6));
}

// =============================================================
// Boost geometry traits
// =============================================================

template <typename T>
class Segment3BoostGeometry : public ::testing::Test {};
TYPED_TEST_SUITE(Segment3BoostGeometry, CoordinateTypes);

TYPED_TEST(Segment3BoostGeometry, IsASegment) {
  using S = Segment3<TypeParam>;
  bg::concepts::check<S>();
  bg::concepts::check<const S>();
  static_assert(std::is_same_v<typename bg::tag<S>::type, bg::segment_tag>);
  static_assert(std::is_same_v<typename bg::traits::point_type<S>::type,
                               Point3<TypeParam>>);
}

// bg::get<Index, Dimension> reads coordinate Dimension of endpoint Index.
TYPED_TEST(Segment3BoostGeometry, IndexedGet) {
  const Segment3<TypeParam> s(1, 2, 3, 4, 5, 6);

  EXPECT_EQ((bg::get<0, 0>(s)), 1);
  EXPECT_EQ((bg::get<0, 1>(s)), 2);
  EXPECT_EQ((bg::get<0, 2>(s)), 3);
  EXPECT_EQ((bg::get<1, 0>(s)), 4);
  EXPECT_EQ((bg::get<1, 1>(s)), 5);
  EXPECT_EQ((bg::get<1, 2>(s)), 6);
}

TYPED_TEST(Segment3BoostGeometry, IndexedSet) {
  Segment3<TypeParam> s;

  bg::set<0, 0>(s, 1);
  bg::set<0, 1>(s, 2);
  bg::set<0, 2>(s, 3);
  bg::set<1, 0>(s, 4);
  bg::set<1, 1>(s, 5);
  bg::set<1, 2>(s, 6);

  EXPECT_EQ(s, Segment3<TypeParam>(1, 2, 3, 4, 5, 6));
}

// The value-type operations are constexpr, so tech constants can be built at
// compile time.
constexpr Segment3<int> ShiftedSegment() {
  Segment3<int> s(0, 0, 0, 1, 2, 3);
  s.Shift(1, 1, 1);
  s.SetP(0, Point3<int>(0, 0, 0));
  return s;
}

TEST(Segment3Constexpr, OperationsRunAtCompileTime) {
  static_assert(ShiftedSegment() == Segment3<int>(0, 0, 0, 2, 3, 4));
  static_assert(Segment3<int>(0, 0, 0, 3, 4, 0).ManhattanLength() == 7);
  static_assert(Segment3<int>(3, 4, 5, 1, 2, 3).zl() == 3);
  static_assert(Segment3<int>(3, 4, 5, 1, 2, 3).MaxPoint() ==
                Point3<int>(3, 4, 5));
}

}  // namespace
}  // namespace moab

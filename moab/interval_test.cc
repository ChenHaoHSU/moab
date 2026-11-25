#include "interval.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"
#include "boost/polygon/polygon.hpp"

// The tests follow the order of interval.h.
//
// The class members are typed tests over every coordinate type. Interval only
// has integer aliases, but nothing in the class depends on integers, and
// Interval<float> and Interval<double> compile and behave the same for the
// small whole numbers used here. The Boost.Polygon traits are tested for the
// integer types only: Boost.Polygon is an integer-coordinate library and the
// interval is registered for it through the integer aliases.

namespace moab {
namespace {

using ::testing::Pair;
using ::testing::StrEq;

using CoordinateTypes = ::testing::Types<int32_t, int64_t, float, double>;
using BoostCoordinateTypes = ::testing::Types<int32_t, int64_t>;

// The ends of 'i' as <lo, hi>.
template <typename T>
std::pair<T, T> Ends(const Interval<T>& i) {
  return {i.lo(), i.hi()};
}

// =============================================================
// Type aliases
// =============================================================

template <typename T>
class TypeAliases : public ::testing::Test {};
TYPED_TEST_SUITE(TypeAliases, CoordinateTypes);

TYPED_TEST(TypeAliases, CoordinateTypeIsT) {
  static_assert(
      std::is_same_v<typename Interval<TypeParam>::coordinate_type, TypeParam>);
}

TEST(IntegerTypeAliases, NameTheIntegerIntervals) {
  static_assert(std::is_same_v<Interval_i, Interval<int>>);
  static_assert(std::is_same_v<Interval_i32, Interval<int32_t>>);
  static_assert(std::is_same_v<Interval_i64, Interval<int64_t>>);
}

// =============================================================
// Constructors and assignment operators
// =============================================================

template <typename T>
class Constructors : public ::testing::Test {};
TYPED_TEST_SUITE(Constructors, CoordinateTypes);

TYPED_TEST(Constructors, DefaultIsZeroZero) {
  Interval<TypeParam> i;

  EXPECT_THAT(Ends(i), Pair(0, 0));
}

TYPED_TEST(Constructors, LoHi) {
  Interval<TypeParam> i(1, 2);

  EXPECT_THAT(Ends(i), Pair(1, 2));
}

TYPED_TEST(Constructors, LoHiSwapsInvertedEnds) {
  Interval<TypeParam> i(2, 1);

  EXPECT_THAT(Ends(i), Pair(1, 2));
}

TYPED_TEST(Constructors, Copy) {
  Interval<TypeParam> i1(1, 2);
  Interval<TypeParam> i2(i1);

  EXPECT_THAT(Ends(i2), Pair(1, 2));
}

TYPED_TEST(Constructors, Move) {
  Interval<TypeParam> i1(1, 2);
  Interval<TypeParam> i2(std::move(i1));

  EXPECT_THAT(Ends(i2), Pair(1, 2));
}

TYPED_TEST(Constructors, CopyAssignment) {
  Interval<TypeParam> i1(1, 2);
  Interval<TypeParam> i2;

  i2 = i1;

  EXPECT_THAT(Ends(i2), Pair(1, 2));
}

TYPED_TEST(Constructors, MoveAssignment) {
  Interval<TypeParam> i1(1, 2);
  Interval<TypeParam> i2;

  i2 = std::move(i1);

  EXPECT_THAT(Ends(i2), Pair(1, 2));
}

// =============================================================
// Accessors
// =============================================================

template <typename T>
class Accessors : public ::testing::Test {};
TYPED_TEST_SUITE(Accessors, CoordinateTypes);

TYPED_TEST(Accessors, LoHi) {
  Interval<TypeParam> i(1, 2);

  EXPECT_EQ(i.lo(), 1);
  EXPECT_EQ(i.hi(), 2);
}

TYPED_TEST(Accessors, Data) {
  Interval<TypeParam> i(1, 2);
  const TypeParam* d = i.data();

  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
}

TYPED_TEST(Accessors, MinMax) {
  Interval<TypeParam> i(1, 2);

  EXPECT_EQ(i.Min(), 1);
  EXPECT_EQ(i.Max(), 2);
}

TYPED_TEST(Accessors, LengthAndSize) {
  struct Case {
    std::string name;
    Interval<TypeParam> i;
    TypeParam expected;
  };
  const std::vector<Case> cases = {
      {"positive ends", Interval<TypeParam>(1, 5), 4},
      {"negative ends", Interval<TypeParam>(-7, -2), 5},
      {"across zero", Interval<TypeParam>(-3, 4), 7},
      {"a point has length zero", Interval<TypeParam>(3, 3), 0},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.i.Length(), c.expected);
    EXPECT_EQ(c.i.Size(), c.expected);
  }
}

// The ends are read-only outside the setters, so an interval can't be
// inverted through them.
TYPED_TEST(Accessors, EndAccessorsAreReadOnly) {
  using I = Interval<TypeParam>;
  static_assert(
      std::is_same_v<decltype(std::declval<I&>()[0]), const TypeParam&>);
  static_assert(
      std::is_same_v<decltype(std::declval<I&>().data()), const TypeParam*>);
}

// =============================================================
// Mutators
// =============================================================

template <typename T>
class Mutators : public ::testing::Test {};
TYPED_TEST_SUITE(Mutators, CoordinateTypes);

TYPED_TEST(Mutators, SetLoHi) {
  Interval<TypeParam> i;

  i.Set(1, 2);

  EXPECT_THAT(Ends(i), Pair(1, 2));
}

TYPED_TEST(Mutators, SetLoHiSwapsInvertedEnds) {
  Interval<TypeParam> i;

  i.Set(2, 1);

  EXPECT_THAT(Ends(i), Pair(1, 2));
}

// set_lo and SetMin set the low end, set_hi and SetMax the high end. A value
// past the other end swaps the ends, so lo <= hi always holds.
TYPED_TEST(Mutators, SettersKeepLoAtMostHi) {
  using I = Interval<TypeParam>;
  using Setter = void (I::*)(TypeParam);
  struct Case {
    std::string name;
    I start;
    Setter setter;
    TypeParam value;
    TypeParam lo;
    TypeParam hi;
  };
  const std::vector<Case> cases = {
      {"set_lo below lo", I(1, 2), &I::set_lo, 0, 0, 2},
      {"set_lo inside", I(0, 10), &I::set_lo, 5, 5, 10},
      {"set_lo to hi", I(0, 10), &I::set_lo, 10, 10, 10},
      {"set_lo past hi swaps", I(0, 10), &I::set_lo, 20, 10, 20},
      {"set_hi above hi", I(1, 2), &I::set_hi, 3, 1, 3},
      {"set_hi inside", I(0, 10), &I::set_hi, 5, 0, 5},
      {"set_hi to lo", I(0, 10), &I::set_hi, 0, 0, 0},
      {"set_hi past lo swaps", I(0, 10), &I::set_hi, -5, -5, 0},
      {"SetMin below min", I(1, 2), &I::SetMin, 0, 0, 2},
      {"SetMin past max swaps", I(0, 10), &I::SetMin, 20, 10, 20},
      {"SetMax above max", I(1, 2), &I::SetMax, 3, 1, 3},
      {"SetMax past min swaps", I(0, 10), &I::SetMax, -5, -5, 0},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    I i = c.start;
    (i.*c.setter)(c.value);
    EXPECT_THAT(Ends(i), Pair(c.lo, c.hi));
  }
}

// =============================================================
// Queries
// =============================================================

template <typename T>
class Queries : public ::testing::Test {};
TYPED_TEST_SUITE(Queries, CoordinateTypes);

TYPED_TEST(Queries, ContainsValueIncludesBothEnds) {
  const Interval<TypeParam> i(1, 3);
  struct Case {
    std::string name;
    TypeParam value;
    bool expected;
  };
  const std::vector<Case> cases = {
      {"lo", 1, true},
      {"inside", 2, true},
      {"hi", 3, true},
      {"just below lo", 0, false},
      {"just above hi", 4, false},
      {"negative", -1, false},
      {"far above", 10, false},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(i.Contains(c.value), c.expected);
  }
}

TYPED_TEST(Queries, ContainsInterval) {
  using I = Interval<TypeParam>;
  const I i(1, 5);
  struct Case {
    std::string name;
    I other;
    bool expected;
  };
  const std::vector<Case> cases = {
      {"sharing lo", I(1, 2), true},
      {"sharing hi", I(3, 5), true},
      {"equal", I(1, 5), true},
      {"apart", I(7, 9), false},
      {"larger on both sides", I(0, 9), false},
      {"a point outside", I(9, 9), false},
      {"overlapping lo", I(0, 3), false},
      {"overlapping hi", I(3, 6), false},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(i.Contains(c.other), c.expected);
  }
}

// =============================================================
// Operations
// =============================================================

template <typename T>
class Operations : public ::testing::Test {};
TYPED_TEST_SUITE(Operations, CoordinateTypes);

TYPED_TEST(Operations, Shift) {
  Interval<TypeParam> i(1, 2);

  i.Shift(3);

  EXPECT_THAT(Ends(i), Pair(4, 5));
}

TYPED_TEST(Operations, ShiftNegative) {
  Interval<TypeParam> i(1, 2);

  i.Shift(-3);

  EXPECT_THAT(Ends(i), Pair(-2, -1));
}

// =============================================================
// Operators
// =============================================================

template <typename T>
class Operators : public ::testing::Test {};
TYPED_TEST_SUITE(Operators, CoordinateTypes);

TYPED_TEST(Operators, SubscriptAccess) {
  Interval<TypeParam> i(1, 2);

  EXPECT_EQ(i[0], 1);
  EXPECT_EQ(i[1], 2);
}

TYPED_TEST(Operators, Equality) {
  using I = Interval<TypeParam>;
  const I lhs(1, 2);
  struct Case {
    std::string name;
    I rhs;
    bool equal;
  };
  const std::vector<Case> cases = {
      {"same ends", I(1, 2), true},
      {"different hi", I(1, 3), false},
      {"different lo", I(0, 2), false},
      {"apart", I(7, 9), false},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(lhs == c.rhs, c.equal);
    EXPECT_EQ(lhs != c.rhs, !c.equal);
    if (c.equal) {
      EXPECT_EQ(lhs, c.rhs);
    } else {
      EXPECT_NE(lhs, c.rhs);
    }
  }
}

// The order is lexicographic on (lo, hi). Each case checks all four operators
// both ways round.
TYPED_TEST(Operators, Inequality) {
  using I = Interval<TypeParam>;
  struct Case {
    std::string name;
    I lhs;
    I rhs;
    int order;  // -1: lhs < rhs, 0: equal, 1: lhs > rhs.
  };
  const std::vector<Case> cases = {
      {"lower lo", I(1, 2), I(2, 3), -1},
      {"same lo, lower hi", I(1, 2), I(1, 3), -1},
      {"equal", I(1, 2), I(1, 2), 0},
      {"lo decides before hi", I(1, 5), I(2, 3), -1},
      {"higher lo", I(2, 3), I(1, 9), 1},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.lhs < c.rhs, c.order < 0);
    EXPECT_EQ(c.lhs <= c.rhs, c.order <= 0);
    EXPECT_EQ(c.lhs > c.rhs, c.order > 0);
    EXPECT_EQ(c.lhs >= c.rhs, c.order >= 0);
    EXPECT_EQ(c.rhs < c.lhs, c.order > 0);
    EXPECT_EQ(c.rhs <= c.lhs, c.order >= 0);
    EXPECT_EQ(c.rhs > c.lhs, c.order < 0);
    EXPECT_EQ(c.rhs >= c.lhs, c.order <= 0);
  }
}

TYPED_TEST(Operators, AdditionAssignment) {
  Interval<TypeParam> i(1, 2);

  i += 1;

  EXPECT_THAT(Ends(i), Pair(2, 3));
}

TYPED_TEST(Operators, SubtractionAssignment) {
  Interval<TypeParam> i(1, 2);

  i -= 1;

  EXPECT_THAT(Ends(i), Pair(0, 1));
}

TYPED_TEST(Operators, AdditionLeavesOperandUnchanged) {
  Interval<TypeParam> i(1, 2);

  Interval<TypeParam> j = i + 1;

  EXPECT_THAT(Ends(i), Pair(1, 2));
  EXPECT_THAT(Ends(j), Pair(2, 3));
}

TYPED_TEST(Operators, SubtractionLeavesOperandUnchanged) {
  Interval<TypeParam> i(1, 2);

  Interval<TypeParam> j = i - 1;

  EXPECT_THAT(Ends(i), Pair(1, 2));
  EXPECT_THAT(Ends(j), Pair(0, 1));
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
template <typename T>
class OperatorsDeathTest : public ::testing::Test {};
TYPED_TEST_SUITE(OperatorsDeathTest, CoordinateTypes);

TYPED_TEST(OperatorsDeathTest, SubscriptOutOfRange) {
  Interval<TypeParam> v(1, 2);
  const Interval<TypeParam>& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

// =============================================================
// String conversion
// =============================================================

template <typename T>
class StringConversion : public ::testing::Test {};
TYPED_TEST_SUITE(StringConversion, CoordinateTypes);

TYPED_TEST(StringConversion, ToString) {
  EXPECT_THAT(Interval<TypeParam>(1, 2).ToString(), StrEq("[1 2]"));
  EXPECT_THAT(Interval<TypeParam>(-3, -2).ToString(), StrEq("[-3 -2]"));
}

TYPED_TEST(StringConversion, SupportsAbslStringify) {
  Interval<TypeParam> i(1, 2);
  std::string s = absl::StrFormat("%v", i);

  EXPECT_THAT(s, StrEq("[1 2]"));
}

TYPED_TEST(StringConversion, StreamOperator) {
  std::ostringstream os;

  os << Interval<TypeParam>(1, 2) << " " << Interval<TypeParam>(-3, 4);

  EXPECT_THAT(os.str(), StrEq("[1 2] [-3 4]"));
}

// The ends print with %v, which is %g for floating-point types: a short
// fraction prints exactly and a large value in exponent form, while an
// integer prints every digit.
TEST(StringConversionFormats, FloatingPointEnds) {
  EXPECT_THAT(Interval<double>(0.5, 1.5).ToString(), StrEq("[0.5 1.5]"));
  EXPECT_THAT(Interval<float>(-0.25f, 0.125f).ToString(),
              StrEq("[-0.25 0.125]"));
  EXPECT_THAT(Interval<double>(0, 1e10).ToString(), StrEq("[0 1e+10]"));
}

TEST(StringConversionFormats, IntegerEnds) {
  EXPECT_THAT(Interval_i64(0, 10000000000).ToString(),
              StrEq("[0 10000000000]"));
  EXPECT_THAT(Interval_i32(-2147483647 - 1, 2147483647).ToString(),
              StrEq("[-2147483648 2147483647]"));
}

// =============================================================
// Hash
// =============================================================

template <typename T>
class Hash : public ::testing::Test {};
TYPED_TEST_SUITE(Hash, CoordinateTypes);

TYPED_TEST(Hash, SupportsAbslHash) {
  using I = Interval<TypeParam>;
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      I(),
      I(0, 0),
      I(1, 2),
      I(3, 4),
      I(1, 2),
      I(-3, -2),
      I(-3, 4),
      I(10, 100),
  }));
}

// =============================================================
// Boost.Polygon traits
// =============================================================

// Boost.Polygon is an integer-coordinate library, so these run for the
// integer types only.
template <typename T>
class BoostPolygon : public ::testing::Test {};
TYPED_TEST_SUITE(BoostPolygon, BoostCoordinateTypes);

TYPED_TEST(BoostPolygon, IsAnIntervalConcept) {
  static_assert(
      std::is_same_v<
          typename boost::polygon::geometry_concept<Interval<TypeParam>>::type,
          boost::polygon::interval_concept>);
  static_assert(std::is_same_v<typename boost::polygon::interval_traits<
                                   Interval<TypeParam>>::coordinate_type,
                               TypeParam>);
  static_assert(std::is_same_v<typename boost::polygon::interval_mutable_traits<
                                   Interval<TypeParam>>::coordinate_type,
                               TypeParam>);
}

TYPED_TEST(BoostPolygon, GetReadsTheEnds) {
  const Interval<TypeParam> i(-3, 7);

  EXPECT_EQ(boost::polygon::low(i), -3);
  EXPECT_EQ(boost::polygon::high(i), 7);
  EXPECT_EQ(boost::polygon::get(i, boost::polygon::LOW), -3);
  EXPECT_EQ(boost::polygon::get(i, boost::polygon::HIGH), 7);
  EXPECT_EQ(boost::polygon::delta(i), 10);
}

TYPED_TEST(BoostPolygon, SetWritesOneEnd) {
  Interval<TypeParam> i(0, 10);

  boost::polygon::low(i, 2);
  boost::polygon::high(i, 8);

  EXPECT_THAT(Ends(i), Pair(2, 8));
}

// Unlike set_lo and set_hi, the Boost setters write the end as given, so the
// interval can be inverted until Boost writes the other end.
TYPED_TEST(BoostPolygon, SetWritesTheEndRaw) {
  Interval<TypeParam> past_hi(0, 10);
  Interval<TypeParam> past_lo(0, 10);

  boost::polygon::low(past_hi, 20);
  boost::polygon::high(past_lo, -5);

  EXPECT_THAT(Ends(past_hi), Pair(20, 10));
  EXPECT_THAT(Ends(past_lo), Pair(0, -5));
}

// construct goes through the constructor, which orders the ends.
TYPED_TEST(BoostPolygon, Construct) {
  using I = Interval<TypeParam>;

  EXPECT_EQ(boost::polygon::construct<I>(1, 2), I(1, 2));
  EXPECT_THAT(Ends(boost::polygon::construct<I>(2, 1)), Pair(1, 2));
}

// Boost.Polygon writes the two ends one at a time. These cases move both ends
// past the old high end, so they fail if a write ever reorders the ends.
TYPED_TEST(BoostPolygon, Move) {
  Interval<TypeParam> i(0, 10);

  boost::polygon::move(i, 20);

  EXPECT_EQ(i, Interval<TypeParam>(20, 30));
}

TYPED_TEST(BoostPolygon, ScaleUp) {
  Interval<TypeParam> i(1, 2);

  boost::polygon::scale_up(i, 10);

  EXPECT_EQ(i, Interval<TypeParam>(10, 20));
}

// Length, Size and Contains check the end order in debug builds. Boost's raw
// writes are the only way to produce an inverted interval.
#ifndef NDEBUG
template <typename T>
class InvariantDeathTest : public ::testing::Test {};
TYPED_TEST_SUITE(InvariantDeathTest, BoostCoordinateTypes);

TYPED_TEST(InvariantDeathTest, InvertedIntervalSize) {
  using I = Interval<TypeParam>;
  I i(0, 10);
  boost::polygon::low(i, 20);

  EXPECT_DEATH((void)i.Length(), "Interval is inverted");
  EXPECT_DEATH((void)i.Size(), "Interval is inverted");
  EXPECT_DEATH((void)i.Contains(5), "Interval is inverted");
  EXPECT_DEATH((void)I(0, 30).Contains(i), "Interval is inverted");
}
#endif

// The value-type operations are constexpr, so tech constants can be built at
// compile time.
constexpr Interval<int> ShiftedAndExtended() {
  Interval<int> i(10, 0);
  i.Shift(5);
  i.set_hi(20);
  return i;
}

TEST(IntervalConstexpr, OperationsRunAtCompileTime) {
  static_assert(Interval<int>(10, 0).lo() == 0);
  static_assert(ShiftedAndExtended() == Interval<int>(5, 20));
  static_assert(Interval<int>(0, 10).Length() == 10);
  static_assert(Interval<int>(0, 10).Contains(Interval<int>(2, 3)));
  static_assert((Interval<int>(0, 10) + 2)[1] == 12);
  static_assert(Interval<int>(0, 10) < Interval<int>(1, 2));
}

}  // namespace
}  // namespace moab

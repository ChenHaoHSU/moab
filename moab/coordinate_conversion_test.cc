#include "coordinate_conversion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/status/statusor.h"

// The tests follow the order of coordinate_conversion.h: CoordinateTypeName,
// then ConvertCoordinate for every pair of coordinate types, and then its
// branches in order: non-finite values, floating-point targets, integral
// targets from floating point, and integral targets from integers.

namespace moab::internal {
namespace {

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::FloatEq;
using ::testing::HasSubstr;

constexpr double kInf = std::numeric_limits<double>::infinity();
constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

constexpr int32_t kInt32Min = std::numeric_limits<int32_t>::min();
constexpr int32_t kInt32Max = std::numeric_limits<int32_t>::max();
constexpr int64_t kInt64Min = std::numeric_limits<int64_t>::min();
constexpr int64_t kInt64Max = std::numeric_limits<int64_t>::max();
constexpr float kFloatMax = std::numeric_limits<float>::max();
constexpr double kDoubleMax = std::numeric_limits<double>::max();

// A conversion to the coordinate type To from the proto type From.
template <typename To, typename From>
struct Conversion {
  using T = To;
  using S = From;
};

// Every pair of coordinate types.
using AllConversions =
    ::testing::Types<Conversion<int32_t, int32_t>, Conversion<int32_t, int64_t>,
                     Conversion<int32_t, float>, Conversion<int32_t, double>,
                     Conversion<int64_t, int32_t>, Conversion<int64_t, int64_t>,
                     Conversion<int64_t, float>, Conversion<int64_t, double>,
                     Conversion<float, int32_t>, Conversion<float, int64_t>,
                     Conversion<float, float>, Conversion<float, double>,
                     Conversion<double, int32_t>, Conversion<double, int64_t>,
                     Conversion<double, float>, Conversion<double, double>>;

// The pairs with a floating-point source.
using FromFloatingPointConversions =
    ::testing::Types<Conversion<int32_t, float>, Conversion<int32_t, double>,
                     Conversion<int64_t, float>, Conversion<int64_t, double>,
                     Conversion<float, float>, Conversion<float, double>,
                     Conversion<double, float>, Conversion<double, double>>;

// The pairs with an integral target and a floating-point source.
using IntegralFromFloatingPointConversions =
    ::testing::Types<Conversion<int32_t, float>, Conversion<int32_t, double>,
                     Conversion<int64_t, float>, Conversion<int64_t, double>>;

// Names a typed test after its pair, e.g. "int32_from_double".
struct ConversionName {
  template <typename C>
  static std::string GetName(int) {
    return std::string(CoordinateTypeName<typename C::T>()) + "_from_" +
           std::string(CoordinateTypeName<typename C::S>());
  }
};

// A conversion that must succeed, with its result and the expected value.
template <typename T>
struct OkCase {
  std::string name;
  absl::StatusOr<T> result;
  T expected;
};

template <typename T>
void ExpectOk(const std::vector<OkCase<T>>& cases) {
  for (const OkCase<T>& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_THAT(c.result, IsOkAndHolds(c.expected));
  }
}

// A conversion that must fail, with its status and the expected message.
struct ErrorCase {
  std::string name;
  absl::Status status;
  std::string message;
};

void ExpectErrors(const std::vector<ErrorCase>& cases) {
  for (const ErrorCase& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_THAT(c.status,
                StatusIs(absl::StatusCode::kInvalidArgument, c.message));
  }
}

// =============================================================
// CoordinateTypeName
// =============================================================

TEST(CoordinateTypeName, Names) {
  EXPECT_EQ(CoordinateTypeName<int32_t>(), "int32");
  EXPECT_EQ(CoordinateTypeName<int64_t>(), "int64");
  EXPECT_EQ(CoordinateTypeName<float>(), "float");
  EXPECT_EQ(CoordinateTypeName<double>(), "double");
}

TEST(CoordinateTypeName, OtherTypesAreUnknown) {
  EXPECT_EQ(CoordinateTypeName<int16_t>(), "unknown");
  EXPECT_EQ(CoordinateTypeName<long double>(), "unknown");
}

// =============================================================
// ConvertCoordinate: every pair of coordinate types
// =============================================================

template <typename C>
class ConvertCoordinateAllPairs : public ::testing::Test {};
TYPED_TEST_SUITE(ConvertCoordinateAllPairs, AllConversions, ConversionName);

// Small whole numbers are exact in every type.
TYPED_TEST(ConvertCoordinateAllPairs, SmallWholeNumbersConvertExactly) {
  using T = typename TypeParam::T;
  using S = typename TypeParam::S;
  for (const int v : {-7, -1, 0, 1, 42}) {
    SCOPED_TRACE(v);
    EXPECT_THAT(ConvertCoordinate<T>(static_cast<S>(v), "x"),
                IsOkAndHolds(static_cast<T>(v)));
  }
}

// =============================================================
// ConvertCoordinate: non-finite values
// =============================================================

template <typename C>
class ConvertCoordinateNotFinite : public ::testing::Test {};
TYPED_TEST_SUITE(ConvertCoordinateNotFinite, FromFloatingPointConversions,
                 ConversionName);

// NaN and infinities are rejected for every target, floating-point or not.
TYPED_TEST(ConvertCoordinateNotFinite, Rejected) {
  using T = typename TypeParam::T;
  using S = typename TypeParam::S;
  ExpectErrors({
      {"nan",
       ConvertCoordinate<T>(std::numeric_limits<S>::quiet_NaN(), "x").status(),
       "x: coordinate nan is not finite."},
      {"+inf",
       ConvertCoordinate<T>(std::numeric_limits<S>::infinity(), "x").status(),
       "x: coordinate inf is not finite."},
      {"-inf",
       ConvertCoordinate<T>(-std::numeric_limits<S>::infinity(), "x").status(),
       "x: coordinate -inf is not finite."},
  });
}

TEST(FromFloatingPoint, NotFinite) {
  EXPECT_THAT(ConvertCoordinate<int32_t>(kNaN, "[Segment2] p0.y"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment2] p0.y: coordinate nan is not "
                                 "finite.")));
  EXPECT_THAT(ConvertCoordinate<int64_t>(kInf, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(ConvertCoordinate<double>(-kInf, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(ConvertCoordinate<float>(kNaN, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

// =============================================================
// ConvertCoordinate: floating-point targets
// =============================================================

TEST(FloatingPointFromAny, Converts) {
  EXPECT_THAT(ConvertCoordinate<double>(int32_t{-5}, "x"), IsOkAndHolds(-5.0));
  EXPECT_THAT(ConvertCoordinate<double>(int64_t{1} << 40, "x"),
              IsOkAndHolds(1099511627776.0));
  EXPECT_THAT(ConvertCoordinate<double>(0.1f, "x"),
              IsOkAndHolds(static_cast<double>(0.1f)));
  // Precision loss is accepted for a floating-point target.
  EXPECT_THAT(ConvertCoordinate<float>(0.1, "x"), IsOkAndHolds(FloatEq(0.1f)));
}

// A floating-point target takes fractions, and any value in its range, even
// when it rounds to the nearest representable value.
TEST(FloatingPointFromAny, ToFloat) {
  ExpectOk<float>({
      {"fraction from double", ConvertCoordinate<float>(0.5, "x"), 0.5f},
      {"fraction from float", ConvertCoordinate<float>(-0.25f, "x"), -0.25f},
      {"2^24 + 1 from int32 rounds to 2^24",
       ConvertCoordinate<float>(int32_t{16777217}, "x"), 16777216.0f},
      {"int32 max rounds to 2^31", ConvertCoordinate<float>(kInt32Max, "x"),
       2147483648.0f},
      {"int32 min", ConvertCoordinate<float>(kInt32Min, "x"), -2147483648.0f},
      {"int64 max rounds to 2^63", ConvertCoordinate<float>(kInt64Max, "x"),
       9223372036854775808.0f},
      {"int64 min", ConvertCoordinate<float>(kInt64Min, "x"),
       -9223372036854775808.0f},
      {"float max from double",
       ConvertCoordinate<float>(static_cast<double>(kFloatMax), "x"),
       kFloatMax},
      {"float lowest from double",
       ConvertCoordinate<float>(-static_cast<double>(kFloatMax), "x"),
       -kFloatMax},
      {"float max from float", ConvertCoordinate<float>(kFloatMax, "x"),
       kFloatMax},
      {"a double too small for float underflows to zero",
       ConvertCoordinate<float>(1e-300, "x"), 0.0f},
  });
}

TEST(FloatingPointFromAny, ToDouble) {
  ExpectOk<double>({
      {"fraction from float", ConvertCoordinate<double>(-0.25f, "x"), -0.25},
      {"fraction from double", ConvertCoordinate<double>(0.5, "x"), 0.5},
      {"int32 max", ConvertCoordinate<double>(kInt32Max, "x"), 2147483647.0},
      {"int32 min", ConvertCoordinate<double>(kInt32Min, "x"), -2147483648.0},
      {"2^53 + 1 from int64 rounds to 2^53",
       ConvertCoordinate<double>(int64_t{9007199254740993}, "x"),
       9007199254740992.0},
      {"int64 max rounds to 2^63", ConvertCoordinate<double>(kInt64Max, "x"),
       9223372036854775808.0},
      {"int64 min", ConvertCoordinate<double>(kInt64Min, "x"),
       -9223372036854775808.0},
      {"float max from float", ConvertCoordinate<double>(kFloatMax, "x"),
       static_cast<double>(kFloatMax)},
      {"double max from double", ConvertCoordinate<double>(kDoubleMax, "x"),
       kDoubleMax},
      {"double lowest from double", ConvertCoordinate<double>(-kDoubleMax, "x"),
       -kDoubleMax},
  });
}

TEST(FloatingPointFromAny, OutOfRange) {
  EXPECT_THAT(ConvertCoordinate<float>(1e300, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("does not fit in float")));
  EXPECT_THAT(ConvertCoordinate<float>(-1e300, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

// Only a double can be out of float's range. %v prints a floating-point value
// like %g.
TEST(FloatingPointFromAny, OutOfRangeMessages) {
  ExpectErrors({
      {"1e300", ConvertCoordinate<float>(1e300, "[Point2] y").status(),
       "[Point2] y: coordinate 1e+300 does not fit in float."},
      {"-1e300", ConvertCoordinate<float>(-1e300, "x").status(),
       "x: coordinate -1e+300 does not fit in float."},
      {"twice float max",
       ConvertCoordinate<float>(2.0 * static_cast<double>(kFloatMax), "x")
           .status(),
       "x: coordinate 6.80565e+38 does not fit in float."},
      {"double max", ConvertCoordinate<float>(kDoubleMax, "x").status(),
       "x: coordinate 1.79769e+308 does not fit in float."},
  });
}

// =============================================================
// ConvertCoordinate: integral targets from floating point
// =============================================================

TEST(IntegralFromFloatingPoint, WholeNumbers) {
  EXPECT_THAT(ConvertCoordinate<int32_t>(3.0, "x"), IsOkAndHolds(3));
  EXPECT_THAT(ConvertCoordinate<int32_t>(-2.0f, "x"), IsOkAndHolds(-2));
  EXPECT_THAT(ConvertCoordinate<int32_t>(-2147483648.0, "x"),
              IsOkAndHolds(std::numeric_limits<int32_t>::min()));
  EXPECT_THAT(ConvertCoordinate<int32_t>(2147483647.0, "x"),
              IsOkAndHolds(std::numeric_limits<int32_t>::max()));
  EXPECT_THAT(ConvertCoordinate<int64_t>(4611686018427387904.0, "x"),
              IsOkAndHolds(int64_t{1} << 62));
}

// The range is [-2^k, 2^k): -2^k converts, and so does the largest
// floating-point value below 2^k.
TEST(IntegralFromFloatingPoint, Limits) {
  ExpectOk<int32_t>({
      {"-2^31 from float", ConvertCoordinate<int32_t>(-2147483648.0f, "x"),
       kInt32Min},
      {"largest float below 2^31",
       ConvertCoordinate<int32_t>(2147483520.0f, "x"), 2147483520},
      {"negative zero", ConvertCoordinate<int32_t>(-0.0, "x"), 0},
  });
  ExpectOk<int64_t>({
      {"-2^63 from float",
       ConvertCoordinate<int64_t>(-9223372036854775808.0f, "x"), kInt64Min},
      {"largest float below 2^63",
       ConvertCoordinate<int64_t>(9223371487098961920.0f, "x"),
       int64_t{9223371487098961920}},
      {"-2^63 from double",
       ConvertCoordinate<int64_t>(-9223372036854775808.0, "x"), kInt64Min},
      {"largest double below 2^63",
       ConvertCoordinate<int64_t>(9223372036854774784.0, "x"),
       int64_t{9223372036854774784}},
      {"2^62 from float",
       ConvertCoordinate<int64_t>(4611686018427387904.0f, "x"),
       int64_t{1} << 62},
  });
}

TEST(IntegralFromFloatingPoint, NotWholeNumber) {
  EXPECT_THAT(ConvertCoordinate<int32_t>(1.5, "[Point2] x"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] x: coordinate 1.5 is not a whole "
                                 "number, so it does not fit in int32.")));
  EXPECT_THAT(ConvertCoordinate<int64_t>(-0.25, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(IntegralFromFloatingPoint, NotWholeNumberMessages) {
  ExpectErrors({
      {"int32 from float", ConvertCoordinate<int32_t>(0.5f, "x").status(),
       "x: coordinate 0.5 is not a whole number, so it does not fit in "
       "int32."},
      {"int64 from float", ConvertCoordinate<int64_t>(-2.5f, "x").status(),
       "x: coordinate -2.5 is not a whole number, so it does not fit in "
       "int64."},
      {"int64 from double", ConvertCoordinate<int64_t>(-0.25, "x").status(),
       "x: coordinate -0.25 is not a whole number, so it does not fit in "
       "int64."},
      // In range, but not whole: the whole-number check still applies.
      {"just below int32 max + 1",
       ConvertCoordinate<int32_t>(2147483647.5, "x").status(),
       "x: coordinate 2.14748e+09 is not a whole number, so it does not fit "
       "in int32."},
  });
}

template <typename C>
class ConvertCoordinateNotWholeNumber : public ::testing::Test {};
TYPED_TEST_SUITE(ConvertCoordinateNotWholeNumber,
                 IntegralFromFloatingPointConversions, ConversionName);

TYPED_TEST(ConvertCoordinateNotWholeNumber, Rejected) {
  using T = typename TypeParam::T;
  using S = typename TypeParam::S;
  for (const S v : {S{0.5}, S{-0.5}, S{1.5}, S{-2.25}, S{100.125}}) {
    SCOPED_TRACE(v);
    EXPECT_THAT(ConvertCoordinate<T>(v, "x"),
                StatusIs(absl::StatusCode::kInvalidArgument,
                         HasSubstr("is not a whole number")));
  }
}

TEST(IntegralFromFloatingPoint, OutOfRange) {
  EXPECT_THAT(ConvertCoordinate<int32_t>(2147483648.0, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("does not fit in int32")));
  EXPECT_THAT(ConvertCoordinate<int32_t>(-2147483649.0, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(ConvertCoordinate<int64_t>(9223372036854775808.0, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("does not fit in int64")));
}

// The range check comes before the whole-number check, so a large fraction
// is reported as out of range. %v prints a floating-point value like %g.
TEST(IntegralFromFloatingPoint, OutOfRangeMessages) {
  ExpectErrors({
      {"int32 from double 2^31",
       ConvertCoordinate<int32_t>(2147483648.0, "x").status(),
       "x: coordinate 2.14748e+09 does not fit in int32."},
      {"int32 from double below -2^31",
       ConvertCoordinate<int32_t>(-2147483649.0, "x").status(),
       "x: coordinate -2.14748e+09 does not fit in int32."},
      {"int32 from double, out of range and not whole",
       ConvertCoordinate<int32_t>(-2147483648.5, "x").status(),
       "x: coordinate -2.14748e+09 does not fit in int32."},
      {"int32 from float 2^31",
       ConvertCoordinate<int32_t>(2147483648.0f, "x").status(),
       "x: coordinate 2.14748e+09 does not fit in int32."},
      {"int32 from the float below -2^31",
       ConvertCoordinate<int32_t>(-2147483904.0f, "x").status(),
       "x: coordinate -2.14748e+09 does not fit in int32."},
      {"int32 from float 1e20", ConvertCoordinate<int32_t>(1e20f, "x").status(),
       "x: coordinate 1e+20 does not fit in int32."},
      {"int64 from double 2^63",
       ConvertCoordinate<int64_t>(9223372036854775808.0, "x").status(),
       "x: coordinate 9.22337e+18 does not fit in int64."},
      {"int64 from the double below -2^63",
       ConvertCoordinate<int64_t>(-9223372036854777856.0, "x").status(),
       "x: coordinate -9.22337e+18 does not fit in int64."},
      {"int64 from float 2^63",
       ConvertCoordinate<int64_t>(9223372036854775808.0f, "x").status(),
       "x: coordinate 9.22337e+18 does not fit in int64."},
      {"int64 from float -1e19",
       ConvertCoordinate<int64_t>(-1e19f, "x").status(),
       "x: coordinate -1e+19 does not fit in int64."},
      {"int64 from double max",
       ConvertCoordinate<int64_t>(kDoubleMax, "x").status(),
       "x: coordinate 1.79769e+308 does not fit in int64."},
  });
}

// =============================================================
// ConvertCoordinate: integral targets from integers
// =============================================================

TEST(IntegralFromIntegral, InRange) {
  EXPECT_THAT(ConvertCoordinate<int32_t>(int64_t{-7}, "x"), IsOkAndHolds(-7));
  EXPECT_THAT(ConvertCoordinate<int32_t>(
                  int64_t{std::numeric_limits<int32_t>::max()}, "x"),
              IsOkAndHolds(std::numeric_limits<int32_t>::max()));
  EXPECT_THAT(ConvertCoordinate<int32_t>(
                  int64_t{std::numeric_limits<int32_t>::min()}, "x"),
              IsOkAndHolds(std::numeric_limits<int32_t>::min()));
  EXPECT_THAT(ConvertCoordinate<int64_t>(int32_t{42}, "x"), IsOkAndHolds(42));
}

// A wider or equal target takes every value of the source.
TEST(IntegralFromIntegral, Limits) {
  ExpectOk<int32_t>({
      {"int32 min", ConvertCoordinate<int32_t>(kInt32Min, "x"), kInt32Min},
      {"int32 max", ConvertCoordinate<int32_t>(kInt32Max, "x"), kInt32Max},
  });
  ExpectOk<int64_t>({
      {"int32 min", ConvertCoordinate<int64_t>(kInt32Min, "x"), kInt32Min},
      {"int32 max", ConvertCoordinate<int64_t>(kInt32Max, "x"), kInt32Max},
      {"int64 min", ConvertCoordinate<int64_t>(kInt64Min, "x"), kInt64Min},
      {"int64 max", ConvertCoordinate<int64_t>(kInt64Max, "x"), kInt64Max},
  });
}

TEST(IntegralFromIntegral, OutOfRange) {
  EXPECT_THAT(ConvertCoordinate<int32_t>(int64_t{1} << 40, "[Box2] xh"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] xh: coordinate 1099511627776 does "
                                 "not fit in int32.")));
  EXPECT_THAT(ConvertCoordinate<int32_t>(
                  int64_t{std::numeric_limits<int32_t>::min()} - 1, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

// Only an int64 can be out of int32's range.
TEST(IntegralFromIntegral, OutOfRangeMessages) {
  ExpectErrors({
      {"int32 max + 1",
       ConvertCoordinate<int32_t>(int64_t{kInt32Max} + 1, "x").status(),
       "x: coordinate 2147483648 does not fit in int32."},
      {"int32 min - 1",
       ConvertCoordinate<int32_t>(int64_t{kInt32Min} - 1, "x").status(),
       "x: coordinate -2147483649 does not fit in int32."},
      {"int64 max", ConvertCoordinate<int32_t>(kInt64Max, "x").status(),
       "x: coordinate 9223372036854775807 does not fit in int32."},
      {"int64 min", ConvertCoordinate<int32_t>(kInt64Min, "x").status(),
       "x: coordinate -9223372036854775808 does not fit in int32."},
      // 2^32 truncates to 0 in int32, which a sign check alone would miss.
      {"2^32", ConvertCoordinate<int32_t>(int64_t{1} << 32, "x").status(),
       "x: coordinate 4294967296 does not fit in int32."},
  });
}

}  // namespace
}  // namespace moab::internal

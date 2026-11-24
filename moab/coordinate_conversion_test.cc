#include "coordinate_conversion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"

namespace moab::internal {
namespace {

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::FloatEq;
using ::testing::HasSubstr;

constexpr double kInf = std::numeric_limits<double>::infinity();
constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

TEST(CoordinateTypeName, Names) {
  EXPECT_EQ(CoordinateTypeName<int32_t>(), "int32");
  EXPECT_EQ(CoordinateTypeName<int64_t>(), "int64");
  EXPECT_EQ(CoordinateTypeName<float>(), "float");
  EXPECT_EQ(CoordinateTypeName<double>(), "double");
}

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

TEST(IntegralFromIntegral, OutOfRange) {
  EXPECT_THAT(ConvertCoordinate<int32_t>(int64_t{1} << 40, "[Box2] xh"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] xh: coordinate 1099511627776 does "
                                 "not fit in int32.")));
  EXPECT_THAT(ConvertCoordinate<int32_t>(
                  int64_t{std::numeric_limits<int32_t>::min()} - 1, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

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

TEST(IntegralFromFloatingPoint, NotWholeNumber) {
  EXPECT_THAT(ConvertCoordinate<int32_t>(1.5, "[Point2] x"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] x: coordinate 1.5 is not a whole "
                                 "number, so it does not fit in int32.")));
  EXPECT_THAT(ConvertCoordinate<int64_t>(-0.25, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
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

TEST(FloatingPointFromAny, Converts) {
  EXPECT_THAT(ConvertCoordinate<double>(int32_t{-5}, "x"), IsOkAndHolds(-5.0));
  EXPECT_THAT(ConvertCoordinate<double>(int64_t{1} << 40, "x"),
              IsOkAndHolds(1099511627776.0));
  EXPECT_THAT(ConvertCoordinate<double>(0.1f, "x"),
              IsOkAndHolds(static_cast<double>(0.1f)));
  // Precision loss is accepted for a floating-point target.
  EXPECT_THAT(ConvertCoordinate<float>(0.1, "x"), IsOkAndHolds(FloatEq(0.1f)));
}

TEST(FloatingPointFromAny, OutOfRange) {
  EXPECT_THAT(ConvertCoordinate<float>(1e300, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("does not fit in float")));
  EXPECT_THAT(ConvertCoordinate<float>(-1e300, "x"),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

}  // namespace
}  // namespace moab::internal

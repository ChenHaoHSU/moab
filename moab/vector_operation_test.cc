#include "vector_operation.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "absl/log/log.h"
#include "moab/vector2.h"

namespace moab {

using ::testing::UnorderedElementsAre;

TEST(VectorOperators, MagnitudeSquared) {
  Vector2_i v(3, 4);
  EXPECT_EQ(MagSqr(v), 25);
}

TEST(VectorOperators, MagnitudeSquaredLarge) {
  Vector2_i v(30000, 40000);
  EXPECT_EQ(MagSqr(v), 2500000000);
}

TEST(VectorOperators, Magnitude) {
  Vector2<double> v(3.0, 4.0);
  EXPECT_EQ(Mag(v), 5.0);
}

TEST(VectorOperators, MagnitudeInt) {
  Vector2_i v(3, 4);
  EXPECT_EQ(Mag(v), 5.0);
}

TEST(VectorOperators, Normalized) {
  Vector2<double> v(3.0, 4.0);
  Vector2<double> normalized = Normalized(v);
  EXPECT_DOUBLE_EQ(normalized.dx(), 0.6);
  EXPECT_DOUBLE_EQ(normalized.dy(), 0.8);
}

TEST(VectorOperators, Normalize) {
  Vector2<double> v(3.0, 4.0);
  Normalize(v);

  EXPECT_DOUBLE_EQ(v.dx(), 0.6);
  EXPECT_DOUBLE_EQ(v.dy(), 0.8);
}

TEST(VectorOperators, DotProduct) {
  Vector2_i v1(1, 2);
  Vector2_i v2(3, 4);
  EXPECT_EQ(Dot(v1, v2), 11);
}

TEST(VectorOperators, CrossProduct) {
  Vector2_i v1(1, 2);
  Vector2_i v2(3, 4);
  EXPECT_EQ(Cross(v1, v2), -2);
}

TEST(VectorOperators, Cos) {
  Vector2_i v1(1, 1);
  Vector2_i v2(1, 0);
  EXPECT_DOUBLE_EQ(Cos(v1, v2), std::sqrt(2) / 2);
}

TEST(VectorOperators, CosLarge) {
  Vector2_i v1(300000, 300000);
  Vector2_i v2(300000, 0);
  EXPECT_DOUBLE_EQ(Cos(v1, v2), std::sqrt(2) / 2);
}

TEST(VectorOperators, Angle) {
  Vector2_i v1(1, 1);
  Vector2_i v2(1, 0);
  EXPECT_DOUBLE_EQ(Angle(v1, v2), M_PI / 4);
}

TEST(VectorOperators, Rotate) {
  Vector2_i v(1, 0);
  Vector2<double> rotated = Rotate(v, M_PI / 2);

  EXPECT_NEAR(rotated.dx(), 0.0, 1e-9);
  EXPECT_NEAR(rotated.dy(), 1.0, 1e-9);
}

TEST(VectorOperators, IsParallel) {
  Vector2_i v1(1, 1);
  Vector2_i v2(2, 2);
  Vector2_i v3(1, 0);

  EXPECT_TRUE(IsParallel(v1, v2));
  EXPECT_FALSE(IsParallel(v1, v3));
}

TEST(VectorOperators, IsOrthogonal) {
  Vector2_i v1(1, 0);
  Vector2_i v2(0, 1);
  Vector2_i v3(1, 1);

  EXPECT_TRUE(IsOrthogonal(v1, v2));
  EXPECT_FALSE(IsOrthogonal(v1, v3));
}

TEST(VectorOperators, Project) {
  Vector2_i v1(2, 3);
  Vector2_i v2(1, 0);
  Vector2_i projection = Project(v1, v2);

  EXPECT_EQ(projection.dx(), 2);
  EXPECT_EQ(projection.dy(), 0);
}

}  // namespace moab

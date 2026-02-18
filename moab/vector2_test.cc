#include "moab/vector2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"
#include "moab/vector2.pb.h"

namespace moab {

using ::testing::Pair;
using ::testing::StrEq;

TEST(Constructors, Default) {
  Vector2_i v;

  EXPECT_EQ(v.dx(), 0);
  EXPECT_EQ(v.dy(), 0);
}

TEST(Constructors, XY) {
  Vector2_i v(1, 2);

  EXPECT_EQ(v.dx(), 1);
  EXPECT_EQ(v.dy(), 2);
}

TEST(Constructors, Proto) {
  Vector2Proto proto;
  proto.mutable_vector_int32()->set_dx(1);
  proto.mutable_vector_int32()->set_dy(2);

  Vector2_i v(proto);

  EXPECT_EQ(v.dx(), 1);
  EXPECT_EQ(v.dy(), 2);
}

TEST(Constructors, Copy) {
  Vector2_i v1(1, 2);
  Vector2_i v2(v1);

  EXPECT_EQ(v2.dx(), 1);
  EXPECT_EQ(v2.dy(), 2);
}

TEST(Constructors, Move) {
  Vector2_i v(Vector2_i(1, 2));

  EXPECT_EQ(v.dx(), 1);
  EXPECT_EQ(v.dy(), 2);
}

TEST(AssignmentOperators, Copy) {
  Vector2_i v1(1, 2);
  Vector2_i v2;

  v2 = v1;

  EXPECT_EQ(v2.dx(), 1);
  EXPECT_EQ(v2.dy(), 2);
}

TEST(AssignmentOperators, Move) {
  Vector2_i v1(1, 2);
  Vector2_i v2;

  v2 = std::move(v1);

  EXPECT_EQ(v2.dx(), 1);
  EXPECT_EQ(v2.dy(), 2);
}

TEST(Accessors, XY) {
  Vector2_i v(1, 2);

  EXPECT_EQ(v.dx(), 1);
  EXPECT_EQ(v.dy(), 2);
}

TEST(Accessors, Data) {
  Vector2_i v(1, 2);
  const int* d = v.data();

  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
}

TEST(Accessors, Size) {
  Vector2_i v(1, 2);

  EXPECT_EQ(v.Size(), 2);
}

TEST(Accessors, ToPair) {
  Vector2_i v(1, 2);
  std::pair<int, int> v_pair = v.ToPair();

  EXPECT_THAT(v_pair, Pair(1, 2));
}

TEST(Mutators, Set) {
  Vector2_i v(1, 2);
  v.Set(3, 4);

  EXPECT_EQ(v.dx(), 3);
  EXPECT_EQ(v.dy(), 4);
}

TEST(Mutators, SetdX) {
  Vector2_i v(1, 2);
  v.SetdX(3);

  EXPECT_EQ(v.dx(), 3);
  EXPECT_EQ(v.dy(), 2);
}

TEST(Mutators, SetdY) {
  Vector2_i v(1, 2);
  v.SetdY(4);

  EXPECT_EQ(v.dx(), 1);
  EXPECT_EQ(v.dy(), 4);
}

TEST(Mutators, SetDim) {
  Vector2_i v(1, 2);
  v.SetDim(0, 3);
  v.SetDim(1, 4);

  EXPECT_EQ(v.dx(), 3);
  EXPECT_EQ(v.dy(), 4);
}

TEST(Operations, Rotate90) {
  Vector2_i v(1, 2);
  v.Rotate90();

  EXPECT_EQ(v.dx(), -2);
  EXPECT_EQ(v.dy(), 1);
}

TEST(Operations, Rotate180) {
  Vector2_i v(1, 2);
  v.Rotate180();

  EXPECT_EQ(v.dx(), -1);
  EXPECT_EQ(v.dy(), -2);
}

TEST(Operators, SubscriptAccess) {
  Vector2_i v(1, 2);

  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 2);
}

TEST(Operators, SubscriptMutation) {
  Vector2_i v(1, 2);
  v[0] = 3;
  v[1] = 4;

  EXPECT_EQ(v[0], 3);
  EXPECT_EQ(v[1], 4);
}

TEST(Operators, Equality) {
  Vector2_i v1(1, 2);
  Vector2_i v2(1, 2);
  Vector2_i v3(1, 3);

  EXPECT_TRUE(v1 == v2);
  EXPECT_FALSE(v1 == v3);
  EXPECT_EQ(v1, v2);
}

TEST(Operators, NotEquality) {
  Vector2_i v1(1, 2);
  Vector2_i v2(1, 2);
  Vector2_i v3(1, 3);
  Vector2_i v4(2, 2);
  Vector2_i v5(3, 4);

  EXPECT_FALSE(v1 != v2);
  EXPECT_TRUE(v1 != v3);
  EXPECT_TRUE(v1 != v4);
  EXPECT_TRUE(v1 != v5);
  EXPECT_NE(v1, v3);
  EXPECT_NE(v1, v4);
  EXPECT_NE(v1, v5);
}

TEST(Operators, Inequality1) {
  Vector2_i v1(1, 2);
  Vector2_i v2(2, 1);

  EXPECT_TRUE(v1 < v2);
  EXPECT_TRUE(v2 > v1);
  EXPECT_TRUE(v1 <= v2);
  EXPECT_TRUE(v2 >= v1);
}

TEST(Operators, Inequality2) {
  Vector2_i v1(1, 2);
  Vector2_i v2(1, 3);

  EXPECT_TRUE(v1 < v2);
  EXPECT_TRUE(v2 > v1);
  EXPECT_TRUE(v1 <= v2);
  EXPECT_TRUE(v2 >= v1);
}

TEST(Operators, Inequality3) {
  Vector2_i v1(1, 2);
  Vector2_i v2(2, 3);

  EXPECT_TRUE(v1 < v2);
  EXPECT_TRUE(v2 > v1);
  EXPECT_TRUE(v1 <= v2);
  EXPECT_TRUE(v2 >= v1);
}

TEST(Operators, VectorAdditionAssignment) {
  Vector2_i v1(1, 2);
  Vector2_i v2(3, 4);
  v1 += v2;

  EXPECT_EQ(v1.dx(), 4);
  EXPECT_EQ(v1.dy(), 6);
  EXPECT_EQ(v2.dx(), 3);
  EXPECT_EQ(v2.dy(), 4);
}

TEST(Operators, VectorSubstractionAssignment) {
  Vector2_i v1(1, 2);
  Vector2_i v2(3, 4);
  v1 -= v2;

  EXPECT_EQ(v1.dx(), -2);
  EXPECT_EQ(v1.dy(), -2);
  EXPECT_EQ(v2.dx(), 3);
  EXPECT_EQ(v2.dy(), 4);
}

TEST(Operators, VectorAddition) {
  Vector2_i v1(1, 2);
  Vector2_i v2(3, 4);
  Vector2_i v3 = (v1 + v2);

  EXPECT_EQ(v1.dx(), 1);
  EXPECT_EQ(v1.dy(), 2);
  EXPECT_EQ(v2.dx(), 3);
  EXPECT_EQ(v2.dy(), 4);
  EXPECT_EQ(v3.dx(), 4);
  EXPECT_EQ(v3.dy(), 6);
}

TEST(Operators, VectorSubstraction) {
  Vector2_i v1(1, 2);
  Vector2_i v2(3, 4);
  Vector2_i v3 = (v1 - v2);

  EXPECT_EQ(v1.dx(), 1);
  EXPECT_EQ(v1.dy(), 2);
  EXPECT_EQ(v2.dx(), 3);
  EXPECT_EQ(v2.dy(), 4);
  EXPECT_EQ(v3.dx(), -2);
  EXPECT_EQ(v3.dy(), -2);
}

TEST(Operators, VectorPositive) {
  Vector2_i v1(1, 2);
  Vector2_i v2 = +v1;

  EXPECT_EQ(v1.dx(), 1);
  EXPECT_EQ(v1.dy(), 2);
  EXPECT_EQ(v2.dx(), 1);
  EXPECT_EQ(v2.dy(), 2);
}

TEST(Operators, VectorNegative) {
  Vector2_i v1(1, 2);
  Vector2_i v2 = -v1;

  EXPECT_EQ(v1.dx(), 1);
  EXPECT_EQ(v1.dy(), 2);
  EXPECT_EQ(v2.dx(), -1);
  EXPECT_EQ(v2.dy(), -2);
}

TEST(Operators, IntegerAdditionAssignment) {
  Vector2_i v1(1, 2);
  v1 += 1;

  EXPECT_EQ(v1.dx(), 2);
  EXPECT_EQ(v1.dy(), 3);
}

TEST(Operators, IntegerSubstractionAssignment) {
  Vector2_i v1(1, 2);
  v1 -= 1;

  EXPECT_EQ(v1.dx(), 0);
  EXPECT_EQ(v1.dy(), 1);
}

TEST(Operators, IntegerMultiplicationAssignment) {
  Vector2_i v1(1, 2);
  v1 *= 2;

  EXPECT_EQ(v1.dx(), 2);
  EXPECT_EQ(v1.dy(), 4);
}

TEST(Operators, IntegerDivisionAssignment) {
  Vector2_i v1(2, 4);
  v1 /= 2;

  EXPECT_EQ(v1.dx(), 1);
  EXPECT_EQ(v1.dy(), 2);
}

TEST(Operators, IntegerAddition) {
  Vector2_i v1(1, 2);
  Vector2_i v2 = v1 + 1;

  EXPECT_EQ(v1.dx(), 1);
  EXPECT_EQ(v1.dy(), 2);
  EXPECT_EQ(v2.dx(), 2);
  EXPECT_EQ(v2.dy(), 3);
}

TEST(Operators, IntegerSubstraction) {
  Vector2_i v1(1, 2);
  Vector2_i v2 = v1 - 1;

  EXPECT_EQ(v1.dx(), 1);
  EXPECT_EQ(v1.dy(), 2);
  EXPECT_EQ(v2.dx(), 0);
  EXPECT_EQ(v2.dy(), 1);
}

TEST(Operators, IntegerMultiplication) {
  Vector2_i v1(1, 2);
  Vector2_i v2 = v1 * 2;

  EXPECT_EQ(v1.dx(), 1);
  EXPECT_EQ(v1.dy(), 2);
  EXPECT_EQ(v2.dx(), 2);
  EXPECT_EQ(v2.dy(), 4);
}

TEST(Operators, IntegerDivision) {
  Vector2_i v1(2, 4);
  Vector2_i v2 = v1 / 2;

  EXPECT_EQ(v1.dx(), 2);
  EXPECT_EQ(v1.dy(), 4);
  EXPECT_EQ(v2.dx(), 1);
  EXPECT_EQ(v2.dy(), 2);
}

TEST(StringConversion, ToString) {
  Vector2_i v(1, 2);

  EXPECT_THAT(v.ToString(), StrEq("(1 2)"));
}

TEST(StringConversion, SupportsAbslStringify) {
  Vector2_i v(1, 2);
  std::string s = absl::StrFormat("%v", v);

  EXPECT_THAT(s, StrEq("(1 2)"));
}

TEST(Hash, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Vector2_i(),
      Vector2_i(1, 2),
      Vector2_i(0, 0),
      Vector2_i(2, 3),
      Vector2_i(0, -1),
  }));
}

TEST(Protobuf, ToProto) {
  Vector2_i v(1, 2);
  Vector2Proto proto = v.ToProto();

  EXPECT_TRUE(proto.has_vector_int32());
  EXPECT_EQ(proto.vector_int32().dx(), 1);
  EXPECT_EQ(proto.vector_int32().dy(), 2);
}

TEST(Protobuf, SetFromProto) {
  Vector2Proto proto;
  proto.mutable_vector_int32()->set_dx(1);
  proto.mutable_vector_int32()->set_dy(2);

  Vector2_i p;
  p.SetFromProto(proto);

  EXPECT_EQ(p.dx(), 1);
  EXPECT_EQ(p.dy(), 2);
}

}  // namespace moab
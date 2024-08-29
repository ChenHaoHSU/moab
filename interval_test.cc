#include "interval.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"

namespace moab {

using ::testing::Pair;
using ::testing::StrEq;

TEST(Constructors, Default) {
  Interval_i i;

  EXPECT_EQ(i.lo(), 0);
  EXPECT_EQ(i.hi(), 0);
}

TEST(Constructors, LoHi) {
  Interval_i i(1, 2);

  EXPECT_EQ(i.lo(), 1);
  EXPECT_EQ(i.hi(), 2);
}

TEST(Constructors, Copy) {
  Interval_i i1(1, 2);
  Interval_i i2(i1);

  EXPECT_EQ(i2.lo(), 1);
  EXPECT_EQ(i2.hi(), 2);
}

TEST(Constructors, Move) {
  Interval_i i1(1, 2);
  Interval_i i2(std::move(i1));

  EXPECT_EQ(i2.lo(), 1);
  EXPECT_EQ(i2.hi(), 2);
}

TEST(Accessors, LoHi) {
  Interval_i i(1, 2);

  EXPECT_EQ(i.lo(), 1);
  EXPECT_EQ(i.hi(), 2);
}

TEST(Accessors, Data) {
  Interval_i i(1, 2);
  const int* d = i.data();

  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
}

TEST(Accessors, MinMax) {
  Interval_i i(1, 2);

  EXPECT_EQ(i.Min(), 1);
  EXPECT_EQ(i.Max(), 2);
}

TEST(Accessors, Length) {
  Interval_i i(1, 5);

  EXPECT_EQ(i.Length(), 4);
}

TEST(Accessors, Size) {
  Interval_i i(1, 5);

  EXPECT_EQ(i.Size(), 4);
}

TEST(Mutators, SetLoHi) {
  Interval_i i;

  i.Set(1, 2);

  EXPECT_EQ(i.lo(), 1);
  EXPECT_EQ(i.hi(), 2);
}

TEST(Mutators, SetLo) {
  Interval_i i(1, 2);

  i.set_lo(0);

  EXPECT_EQ(i.lo(), 0);
  EXPECT_EQ(i.hi(), 2);
}

TEST(Mutators, SetHi) {
  Interval_i i(1, 2);

  i.set_hi(3);

  EXPECT_EQ(i.lo(), 1);
  EXPECT_EQ(i.hi(), 3);
}

TEST(Mutators, SetMin) {
  Interval_i i(1, 2);

  i.SetMin(0);

  EXPECT_EQ(i.lo(), 0);
  EXPECT_EQ(i.hi(), 2);
}

TEST(Mutators, SetMax) {
  Interval_i i(1, 2);

  i.SetMax(3);

  EXPECT_EQ(i.lo(), 1);
  EXPECT_EQ(i.hi(), 3);
}

TEST(Queries, ContainsValue) {
  Interval_i i(1, 3);

  EXPECT_TRUE(i.Contains(1));
  EXPECT_TRUE(i.Contains(2));
  EXPECT_TRUE(i.Contains(3));
  EXPECT_FALSE(i.Contains(0));
  EXPECT_FALSE(i.Contains(4));
  EXPECT_FALSE(i.Contains(-1));
  EXPECT_FALSE(i.Contains(10));
}

TEST(Queries, ContainsInterval) {
  Interval_i i1(1, 5);
  Interval_i i2(1, 2);
  Interval_i i3(3, 5);
  Interval_i i4(1, 5);
  Interval_i i5(7, 9);
  Interval_i i6(0, 9);
  Interval_i i7(9, 9);

  EXPECT_TRUE(i1.Contains(i2));
  EXPECT_TRUE(i1.Contains(i3));
  EXPECT_TRUE(i1.Contains(i4));
  EXPECT_FALSE(i1.Contains(i5));
  EXPECT_FALSE(i1.Contains(i6));
  EXPECT_FALSE(i1.Contains(i7));
}

TEST(Operations, Shift) {
  Interval_i i(1, 2);

  i.Shift(3);

  EXPECT_EQ(i.lo(), 4);
  EXPECT_EQ(i.hi(), 5);
}

TEST(Operators, AssignmentCopy) {
  Interval_i i1(1, 2);
  Interval_i i2;

  i2 = i1;

  EXPECT_EQ(i2.lo(), 1);
  EXPECT_EQ(i2.hi(), 2);
}

TEST(Operators, AssignmentMove) {
  Interval_i i1(1, 2);
  Interval_i i2;

  i2 = std::move(i1);

  EXPECT_EQ(i2.lo(), 1);
  EXPECT_EQ(i2.hi(), 2);
}

TEST(Operators, Equality) {
  Interval_i i1(1, 2);
  Interval_i i2(1, 2);
  Interval_i i3(1, 3);
  Interval_i i4(0, 2);

  EXPECT_TRUE(i1 == i2);
  EXPECT_FALSE(i1 == i3);
  EXPECT_FALSE(i1 == i4);
  EXPECT_EQ(i1, i2);
}

TEST(Operators, NotEquality) {
  Interval_i i1(1, 2);
  Interval_i i2(1, 2);
  Interval_i i3(1, 3);
  Interval_i i4(0, 2);
  Interval_i i5(7, 9);

  EXPECT_FALSE(i1 != i2);
  EXPECT_TRUE(i1 != i3);
  EXPECT_TRUE(i1 != i4);
  EXPECT_TRUE(i1 != i5);
  EXPECT_NE(i1, i3);
  EXPECT_NE(i1, i4);
  EXPECT_NE(i1, i5);
}

TEST(Operators, Inequality1) {
  Interval_i i1(1, 2);
  Interval_i i2(2, 3);

  EXPECT_TRUE(i1 < i2);
  EXPECT_TRUE(i1 <= i2);
  EXPECT_TRUE(i2 > i1);
  EXPECT_TRUE(i2 >= i1);
  EXPECT_FALSE(i1 > i2);
  EXPECT_FALSE(i1 >= i2);
  EXPECT_FALSE(i2 < i1);
  EXPECT_FALSE(i2 <= i1);
}

TEST(Operators, Inequality2) {
  Interval_i i1(1, 2);
  Interval_i i2(1, 3);

  EXPECT_TRUE(i1 < i2);
  EXPECT_TRUE(i1 <= i2);
  EXPECT_TRUE(i2 > i1);
  EXPECT_TRUE(i2 >= i1);
  EXPECT_FALSE(i1 > i2);
  EXPECT_FALSE(i1 >= i2);
  EXPECT_FALSE(i2 < i1);
  EXPECT_FALSE(i2 <= i1);
}

TEST(Operators, Inequality3) {
  Interval_i i1(1, 2);
  Interval_i i2(1, 2);

  EXPECT_FALSE(i1 < i2);
  EXPECT_TRUE(i1 <= i2);
  EXPECT_FALSE(i2 > i1);
  EXPECT_TRUE(i2 >= i1);
  EXPECT_FALSE(i1 > i2);
  EXPECT_TRUE(i1 >= i2);
  EXPECT_FALSE(i2 < i1);
  EXPECT_TRUE(i2 <= i1);
}

TEST(Operators, SubscriptAccess) {
  Interval_i i(1, 2);

  EXPECT_EQ(i[0], 1);
  EXPECT_EQ(i[1], 2);
}

TEST(Operators, SubscriptMutation) {
  Interval_i i(1, 2);
  i[0] = 3;
  i[1] = 4;

  EXPECT_EQ(i[0], 3);
  EXPECT_EQ(i[1], 4);
}

TEST(Operators, AdditionAssignment) {
  Interval_i i(1, 2);
  i += 1;

  EXPECT_EQ(i.lo(), 2);
  EXPECT_EQ(i.hi(), 3);
}

TEST(Operators, SubstractionAssignment) {
  Interval_i i(1, 2);
  i -= 1;

  EXPECT_EQ(i.lo(), 0);
  EXPECT_EQ(i.hi(), 1);
}

TEST(Operators, Addition) {
  Interval_i i(1, 2);
  Interval_i j = i + 1;

  EXPECT_EQ(i.lo(), 1);
  EXPECT_EQ(i.hi(), 2);
  EXPECT_EQ(j.lo(), 2);
  EXPECT_EQ(j.hi(), 3);
}

TEST(Operators, Substraction) {
  Interval_i i(1, 2);
  Interval_i j = i - 1;

  EXPECT_EQ(i.lo(), 1);
  EXPECT_EQ(i.hi(), 2);
  EXPECT_EQ(j.lo(), 0);
  EXPECT_EQ(j.hi(), 1);
}

TEST(StringConversion, ToString) {
  Interval_i i(1, 2);

  EXPECT_THAT(i.ToString(), StrEq("[1 2]"));
}

TEST(StringConversion, SupportsAbslStringify) {
  Interval_i i(1, 2);
  std::string s = absl::StrFormat("%v", i);

  EXPECT_THAT(s, StrEq("[1 2]"));
}

TEST(Hash, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Interval_i(),
      Interval_i(0, 0),
      Interval_i(1, 2),
      Interval_i(3, 4),
      Interval_i(1, 2),
      Interval_i(-3, -2),
      Interval_i(-3, 4),
      Interval_i(10, 100),
  }));
}

}  // namespace moab
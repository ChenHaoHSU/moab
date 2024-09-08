#include "rtree.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "absl/log/log.h"
#include "boost/geometry.hpp"
#include "box2.h"
#include "point2.h"

namespace moab {

using ::testing::ElementsAre;
using ::testing::Pair;
using ::testing::StrEq;

TEST(RtreeBoxTest, DefaultConstructor) {
  RtreeBox2_i rtree;

  EXPECT_EQ(rtree.Size(), 0);
  EXPECT_TRUE(rtree.Empty());
}

TEST(RtreeBoxTest, CopyConstructor) {
  RtreeBox2_i rtree1;
  rtree1.Insert(Box2_i(0, 0, 1, 1));
  rtree1.Insert(Box2_i(2, 2, 3, 3));
  rtree1.Insert(Box2_i(4, 4, 5, 5));

  RtreeBox2_i rtree2(rtree1);

  EXPECT_EQ(rtree1.Size(), 3);
  EXPECT_FALSE(rtree1.Empty());
  EXPECT_EQ(rtree2.Size(), 3);
  EXPECT_FALSE(rtree2.Empty());
}

TEST(RtreeBoxTest, MoveConstructor) {
  RtreeBox2_i rtree1;
  rtree1.Insert(Box2_i(0, 0, 1, 1));
  rtree1.Insert(Box2_i(2, 2, 3, 3));
  rtree1.Insert(Box2_i(4, 4, 5, 5));

  RtreeBox2_i rtree2(std::move(rtree1));

  EXPECT_EQ(rtree1.Size(), 0);
  EXPECT_TRUE(rtree1.Empty());
  EXPECT_EQ(rtree2.Size(), 3);
  EXPECT_FALSE(rtree2.Empty());
}

TEST(RtreeBoxTest, Size) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));
  rtree.Insert(Box2_i(1, 4, 4, 5));

  EXPECT_EQ(rtree.Size(), 4);
}

TEST(RtreeBoxTest, Empty1) {
  RtreeBox2_i rtree;

  EXPECT_TRUE(rtree.Empty());
}

TEST(RtreeBoxTest, Empty2) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));

  EXPECT_FALSE(rtree.Empty());
}

TEST(RtreeBoxTest, InsertValue1) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_EQ(rtree.Size(), 3);
}

TEST(RtreeBoxTest, InsertValue2) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_EQ(rtree.Size(), 5);
}

TEST(RtreeBoxTest, InsertIterator1) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree;
  rtree.insert(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 3);
}

TEST(RtreeBoxTest, InsertIterator2) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree;
  rtree.insert(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 5);
}

TEST(RtreeBoxTest, RemoveValue1) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1);

  rtree.Remove(Box2_i(0, 0, 1, 1));

  EXPECT_EQ(rtree.Size(), 2);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);
}

TEST(RtreeBoxTest, RemoveValue2) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_EQ(rtree.Size(), 5);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 3);

  // Remove 1.
  rtree.Remove(Box2_i(0, 0, 1, 1));

  EXPECT_EQ(rtree.Size(), 4);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 2);

  // Romve 2.
  rtree.Remove(Box2_i(0, 0, 1, 1));

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1);

  // Romve 3.
  rtree.Remove(Box2_i(0, 0, 1, 1));

  EXPECT_EQ(rtree.Size(), 2);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);

  // Romve 4. Nothing removed.
  rtree.Remove(Box2_i(0, 0, 1, 1));

  EXPECT_EQ(rtree.Size(), 2);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);
}

TEST(RtreeBoxTest, RemoveIterator1) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree;
  rtree.insert(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1);

  rtree.Remove(boxes.begin(), boxes.begin() + 1);

  EXPECT_EQ(rtree.Size(), 2);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 1);

  rtree.Remove(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 0);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 0);
}

TEST(RtreeBoxTest, RemoveIterator2) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5), Box2_i(0, 0, 1, 1),
                               Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree;
  rtree.insert(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 6);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 2);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 2);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 2);

  std::vector<Box2_i> removed_boxes = {
      Box2_i(0, 0, 1, 1),     Box2_i(2, 2, 3, 3),     Box2_i(4, 4, 5, 5),
      Box2_i(10, 10, 11, 11), Box2_i(12, 12, 13, 13), Box2_i(14, 14, 15, 15)};

  rtree.Remove(removed_boxes.begin(), removed_boxes.end());

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(10, 10, 11, 11)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(12, 12, 13, 13)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(14, 14, 15, 15)), 0);

  rtree.Remove(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 0);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(10, 10, 11, 11)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(12, 12, 13, 13)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(14, 14, 15, 15)), 0);
}

TEST(RtreeBoxTest, Count1) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(1, 1, 2, 2)), 0);
}

TEST(RtreeBoxTest, Count2) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));
  rtree.Insert(Box2_i(4, 4, 5, 5));
  rtree.Insert(Box2_i(4, 4, 5, 5));
  rtree.Insert(Box2_i(2, 2, 3, 3));

  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 2);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 3);
  EXPECT_EQ(rtree.Count(Box2_i(1, 1, 2, 2)), 0);
}

TEST(RtreeBoxTest, Clear1) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));
  rtree.Clear();

  EXPECT_EQ(rtree.Size(), 0);
  EXPECT_TRUE(rtree.Empty());
}

TEST(RtreeBoxTest, Clear2) {
  RtreeBox2_i rtree;
  rtree.Clear();

  EXPECT_EQ(rtree.Size(), 0);
  EXPECT_TRUE(rtree.Empty());
}

TEST(RtreeBoxTest, CopyAssignment) {
  RtreeBox2_i rtree1;
  rtree1.Insert(Box2_i(0, 0, 1, 1));
  rtree1.Insert(Box2_i(2, 2, 3, 3));
  rtree1.Insert(Box2_i(4, 4, 5, 5));

  RtreeBox2_i rtree2;
  rtree2 = rtree1;

  EXPECT_EQ(rtree1.Size(), 3);
  EXPECT_FALSE(rtree1.Empty());
  EXPECT_EQ(rtree2.Size(), 3);
  EXPECT_FALSE(rtree2.Empty());
}

TEST(RtreeBoxTest, MoveAssignment) {
  RtreeBox2_i rtree1;
  rtree1.Insert(Box2_i(0, 0, 1, 1));
  rtree1.Insert(Box2_i(2, 2, 3, 3));
  rtree1.Insert(Box2_i(4, 4, 5, 5));

  RtreeBox2_i rtree2;
  rtree2 = std::move(rtree1);

  EXPECT_EQ(rtree1.Size(), 0);
  EXPECT_TRUE(rtree1.Empty());
  EXPECT_EQ(rtree2.Size(), 3);
  EXPECT_FALSE(rtree2.Empty());
}

TEST(RtreeBoxTest, ToString) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  EXPECT_THAT(
      rtree.ToString(),
      StrEq("<Size=3, Values=[((0 0) (1 1)), ((2 2) (3 3)), ((4 4) (5 5))]>"));
}

TEST(RtreeBoxTest, SupportsAbslStringify) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::string s = absl::StrFormat("%v", rtree);

  EXPECT_THAT(
      s,
      StrEq("<Size=3, Values=[((0 0) (1 1)), ((2 2) (3 3)), ((4 4) (5 5))]>"));
}

}  // namespace moab
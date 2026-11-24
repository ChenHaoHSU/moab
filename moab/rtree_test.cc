#include "rtree.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "absl/log/log.h"
#include "boost/geometry.hpp"
#include "moab/box2.h"
#include "moab/point2.h"
#include "moab/segment2.h"

namespace moab {

using ::testing::ElementsAre;
using ::testing::FieldsAre;
using ::testing::Pair;
using ::testing::StrEq;
using ::testing::UnorderedElementsAre;

TEST(RtreeBoxTest, DefaultConstructor) {
  RtreeBox2_i rtree;

  EXPECT_EQ(rtree.Size(), 0);
  EXPECT_TRUE(rtree.Empty());
}

TEST(RtreeBoxTest, ConstructorWithIterator) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_FALSE(rtree.Empty());
}

TEST(RtreeBoxTest, ConstructorRange) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree(boxes);

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_FALSE(rtree.Empty());
}

TEST(RtreeBoxTest, ConstructorWithInitializerList) {
  RtreeBox2_i rtree = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                       Box2_i(4, 4, 5, 5)};

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_FALSE(rtree.Empty());
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
  rtree.Insert(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 3);
}

TEST(RtreeBoxTest, InsertIterator2) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree;
  rtree.Insert(boxes.begin(), boxes.end());

  EXPECT_EQ(rtree.Size(), 5);
}

TEST(RtreeBoxTest, InsertIterator3) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree;
  rtree.Insert(boxes);

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
  rtree.Insert(boxes.begin(), boxes.end());

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
  rtree.Insert(boxes.begin(), boxes.end());

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

TEST(RtreeBoxTest, RemoveRange1) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree;
  rtree.Insert(boxes);

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1);

  std::vector<Box2_i> removed_boxes(boxes.begin(), boxes.begin() + 1);
  rtree.Remove(removed_boxes);

  EXPECT_EQ(rtree.Size(), 2);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 1);

  rtree.Remove(boxes);

  EXPECT_EQ(rtree.Size(), 0);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 0);
}

TEST(RtreeBoxTest, RemoveRange2) {
  std::vector<Box2_i> boxes = {Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                               Box2_i(4, 4, 5, 5), Box2_i(0, 0, 1, 1),
                               Box2_i(2, 2, 3, 3), Box2_i(4, 4, 5, 5)};
  RtreeBox2_i rtree;
  rtree.Insert(boxes);

  EXPECT_EQ(rtree.Size(), 6);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 2);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 2);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 2);

  std::vector<Box2_i> removed_boxes = {
      Box2_i(0, 0, 1, 1),     Box2_i(2, 2, 3, 3),     Box2_i(4, 4, 5, 5),
      Box2_i(10, 10, 11, 11), Box2_i(12, 12, 13, 13), Box2_i(14, 14, 15, 15)};

  rtree.Remove(removed_boxes);

  EXPECT_EQ(rtree.Size(), 3);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 1);
  EXPECT_EQ(rtree.Count(Box2_i(10, 10, 11, 11)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(12, 12, 13, 13)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(14, 14, 15, 15)), 0);

  rtree.Remove(boxes);

  EXPECT_EQ(rtree.Size(), 0);
  EXPECT_EQ(rtree.Count(Box2_i(0, 0, 1, 1)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(2, 2, 3, 3)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(4, 4, 5, 5)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(10, 10, 11, 11)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(12, 12, 13, 13)), 0);
  EXPECT_EQ(rtree.Count(Box2_i(14, 14, 15, 15)), 0);
}

TEST(RtreeBoxTest, QueryIntersectsPoint1) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::vector<Box2_i> boxes = rtree.QueryIntersects(Point2_i(1, 1));

  EXPECT_THAT(boxes, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
}

TEST(RtreeBoxTest, QueryIntersectsPoint2) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::vector<Box2_i> boxes = rtree.QueryIntersects(Point2_i(2, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(Box2_i(2, 2, 3, 3)));
}

TEST(RtreeBoxTest, QueryIntersectsBox1) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::vector<Box2_i> boxes = rtree.QueryIntersects(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)));
}

TEST(RtreeBoxTest, QueryIntersectsBox2) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::vector<Box2_i> boxes = rtree.QueryIntersects(Box2_i(0, 0, 3, 3));

  EXPECT_THAT(boxes,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3),
                                   Box2_i(2, 2, 3, 3), Box2_i(2, 2, 3, 3)));
}

TEST(RtreeBoxTest, QueryIntersectsBox3) {
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

TEST(RtreeBoxTest, SupportsRangeBasedForLoop1) {
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

TEST(RtreeBoxTest, SupportsRangeBasedForLoop2) {
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

TEST(RtreeBoxTest, SupportsStreamOutput) {
  RtreeBox2_i rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1));
  rtree.Insert(Box2_i(2, 2, 3, 3));
  rtree.Insert(Box2_i(4, 4, 5, 5));

  std::stringstream ss;
  ss << rtree;

  EXPECT_THAT(
      ss.str(),
      StrEq("<Size=3, Values=[((0 0) (1 1)), ((2 2) (3 3)), ((4 4) (5 5))]>"));
}

TEST(RtreeBoxMapTest, InsertKeyValuePair) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  EXPECT_EQ(rtree.Size(), 3);
}

TEST(RtreeBoxMapTest, QueryIntersectsBox1) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert({Box2_i(0, 0, 1, 1), 0});
  rtree.Insert({Box2_i(2, 2, 3, 3), 1});
  rtree.Insert({Box2_i(4, 4, 5, 5), 2});

  std::vector<std::pair<Box2_i, int>> boxes =
      rtree.QueryIntersects(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(Pair(Box2_i(0, 0, 1, 1), 0),
                                          Pair(Box2_i(2, 2, 3, 3), 1)));
}

TEST(RtreeBoxMapTest, QueryIntersectsKeyBox1) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert({Box2_i(0, 0, 1, 1), 0});
  rtree.Insert({Box2_i(2, 2, 3, 3), 1});
  rtree.Insert({Box2_i(4, 4, 5, 5), 2});

  std::vector<Box2_i> boxes = rtree.QueryIntersects<0>(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)));
}

TEST(RtreeBoxMapTest, QueryIntersectsValueBox1) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<int> boxes = rtree.QueryIntersects<1>(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(0, 1));
}

TEST(RtreeBoxMapTest, QueryTouchesValueBox1) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<int> boxes =
      rtree.Query<1>(moab::index::Touches(Box2_i(0, 1, 3, 2)));

  EXPECT_THAT(boxes, UnorderedElementsAre(0, 1));
}

TEST(RtreeBoxMapTest, QueryTouchesValueBox2) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<int> boxes = rtree.QueryTouches<1>(Box2_i(0, 0, 3, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(1));
}

TEST(RtreeBoxMapTest, QueryTouchesKeyBox) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<Box2_i> boxes = rtree.QueryTouches<0>(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)));
}

TEST(RtreeBoxMapTest, QueryStrictlyIntersectsValueBox1) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<int> boxes = rtree.QueryStrictlyIntersects<1>(Box2_i(1, 1, 2, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre());
}

TEST(RtreeBoxMapTest, QueryStrictlyIntersectsValueBox2) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0);
  rtree.Insert(Box2_i(2, 2, 3, 3), 1);
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  std::vector<int> boxes =
      rtree.QueryStrictlyIntersects<1>(Box2_i(0, -1, 3, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(0));
}

TEST(RtreeBoxMapTest, QueryCoversValueBoxSegment) {
  RtreeMapBox2_i<int> rtree;
  rtree.Insert(Box2_i(Point2_i(0, 0), Point2_i(10, 0)), 0);
  rtree.Insert(Box2_i(Point2_i(0, 0), Point2_i(0, 10)), 1);
  rtree.Insert(Box2_i(Point2_i(10, 10), Point2_i(10, 10)), 2);

  std::vector<int> values =
      rtree.QueryCovers<1>(Box2_i(Point2_i(0, 0), Point2_i(10, 0)));
  EXPECT_THAT(values, UnorderedElementsAre(0));

  values = rtree.QueryCovers<1>(Box2_i(Point2_i(1, 0), Point2_i(9, 0)));
  EXPECT_THAT(values, UnorderedElementsAre(0));

  values = rtree.QueryCovers<1>(Box2_i(Point2_i(0, 0), Point2_i(9, 0)));
  EXPECT_THAT(values, UnorderedElementsAre(0));

  values = rtree.QueryCovers<1>(Box2_i(Point2_i(5, 0), Point2_i(10, 0)));
  EXPECT_THAT(values, UnorderedElementsAre(0));

  values = rtree.QueryCovers<1>(Box2_i(Point2_i(5, 0), Point2_i(5, 0)));
  EXPECT_THAT(values, UnorderedElementsAre(0));

  values = rtree.QueryCovers<1>(Box2_i(Point2_i(5, 0), Point2_i(15, 0)));
  EXPECT_THAT(values, UnorderedElementsAre());

  values = rtree.QueryCovers<1>(Box2_i(Point2_i(10, 0), Point2_i(15, 0)));
  EXPECT_THAT(values, UnorderedElementsAre());

  values = rtree.QueryCovers<1>(Box2_i(Point2_i(-5, 0), Point2_i(15, 0)));
  EXPECT_THAT(values, UnorderedElementsAre());

  values = rtree.QueryCovers<1>(Box2_i(Point2_i(10, 10), Point2_i(10, 10)));
  EXPECT_THAT(values, UnorderedElementsAre(2));
}

TEST(RtreePointTest, QueryIntersectsPoint) {
  RtreePoint2_i rtree;
  rtree.Insert(Point2_i(0, 0));
  rtree.Insert(Point2_i(2, 2));
  rtree.Insert(Point2_i(4, 4));

  std::vector<Point2_i> points = rtree.QueryIntersects(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(points, UnorderedElementsAre(Point2_i(0, 0), Point2_i(2, 2)));
}

TEST(RtreeBoxMapTupleTest, QueryIntersectsBox1) {
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

TEST(RtreeBoxMapTupleTest, QueryIntersectsBox2) {
  RtreeMultiMapBox2_i<int, std::string> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0, "zero");
  rtree.Insert(Box2_i(2, 2, 3, 3), 1, "one");
  rtree.Insert(Box2_i(4, 4, 5, 5), 2, "two");

  std::vector<Box2_i> boxes = rtree.QueryIntersects<0>(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(boxes,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(2, 2, 3, 3)));
}

TEST(RtreeBoxMapTupleTest, QueryIntersectsBox3) {
  RtreeMultiMapBox2_i<int, std::string> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0, "zero");
  rtree.Insert(Box2_i(2, 2, 3, 3), 1, "one");
  rtree.Insert(Box2_i(4, 4, 5, 5), 2, "two");

  std::vector<int> boxes = rtree.QueryIntersects<1>(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre(0, 1));
}

TEST(RtreeBoxMapTupleTest, QueryIntersectsBox4) {
  RtreeMultiMapBox2_i<int, std::string> rtree;
  rtree.Insert(Box2_i(0, 0, 1, 1), 0, "zero");
  rtree.Insert(Box2_i(2, 2, 3, 3), 1, "one");
  rtree.Insert(Box2_i(4, 4, 5, 5), 2, "two");

  std::vector<std::string> boxes = rtree.QueryIntersects<2>(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(boxes, UnorderedElementsAre("zero", "one"));
}

TEST(RtreePointMapTest, QueryIntersectsBox) {
  RtreeMapPoint2_i<int> rtree;
  rtree.Insert({Point2_i(0, 0), 0});
  rtree.Insert({Point2_i(2, 2), 1});
  rtree.Insert({Point2_i(4, 4), 2});

  std::vector<std::pair<Point2_i, int>> points =
      rtree.QueryIntersects(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(points, UnorderedElementsAre(Pair(Point2_i(0, 0), 0),
                                           Pair(Point2_i(2, 2), 1)));
}

TEST(RtreePointMapTest, QueryIntersectsKeyBox) {
  std::vector<std::pair<Point2_i, int>> point_values = {
      {Point2_i(0, 0), 0}, {Point2_i(2, 2), 1}, {Point2_i(4, 4), 2}};
  RtreeMapPoint2_i<int> rtree;
  rtree.Insert(point_values);

  std::vector<Point2_i> points = rtree.QueryIntersects<0>(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(points, UnorderedElementsAre(Point2_i(0, 0), Point2_i(2, 2)));
}

TEST(RtreePointMapTest, QueryIntersectsValueBox) {
  RtreeMapPoint2_i<int> rtree;
  rtree.Insert(Point2_i(0, 0), 0);
  rtree.Insert(Point2_i(2, 2), 1);
  rtree.Insert(Point2_i(4, 4), 2);

  std::vector<int> points = rtree.QueryIntersects<1>(Box2_i(0, 0, 2, 2));

  EXPECT_THAT(points, UnorderedElementsAre(0, 1));
}

TEST(RtreeSegmentMapTest, QueryIntersectsValueSegment) {
  RtreeMapSegment2_i<int> rtree;
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 0)), 0);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(0, 10)), 1);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 10)), 2);

  std::vector<int> values =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(0, 5), Point2_i(10, 5)));
  EXPECT_THAT(values, UnorderedElementsAre(1, 2));

  values =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(5, 0), Point2_i(5, 10)));
  EXPECT_THAT(values, UnorderedElementsAre(0, 2));

  values =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(-5, 0), Point2_i(0, 0)));
  EXPECT_THAT(values, UnorderedElementsAre(0, 1, 2));

  values =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(10, 0), Point2_i(20, 0)));
  EXPECT_THAT(values, UnorderedElementsAre(0));

  values =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(5, 0), Point2_i(15, 0)));
  EXPECT_THAT(values, UnorderedElementsAre(0));
}

TEST(RtreeSegmentMapTest, QueryIntersectsValueBox) {
  RtreeMapSegment2_i<int> rtree;
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 0)), 0);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(0, 10)), 1);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 10)), 2);

  std::vector<int> values = rtree.QueryIntersects<1>(Box2_i(0, 5, 10, 10));

  EXPECT_THAT(values, UnorderedElementsAre(1, 2));
}

TEST(RtreeBoxMapTupleTest, InsertTupleLvalue) {
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

TEST(RtreeBoxMapTupleTest, InsertRange) {
  std::vector<std::tuple<Box2_i, int, std::string>> values = {
      {Box2_i(0, 0, 1, 1), 0, "zero"},
      {Box2_i(2, 2, 3, 3), 1, "one"},
      {Box2_i(4, 4, 5, 5), 2, "two"}};
  RtreeMultiMapBox2_i<int, std::string> rtree;

  rtree.Insert(values);

  EXPECT_EQ(rtree.Size(), 3u);
}

// A two-element tuple must not take an iterator pair as (key, value).
TEST(RtreeBoxMapTupleTest, InsertIteratorPairTwoElementTuple) {
  std::vector<std::tuple<Box2_i, int>> values = {{Box2_i(0, 0, 1, 1), 0},
                                                 {Box2_i(2, 2, 3, 3), 1}};
  RtreeMultiMapBox2_i<int> rtree;

  rtree.Insert(values.begin(), values.end());
  rtree.Insert(Box2_i(4, 4, 5, 5), 2);

  EXPECT_EQ(rtree.Size(), 3u);
  EXPECT_THAT(rtree.QueryIntersects<1>(Box2_i(0, 0, 5, 5)),
              UnorderedElementsAre(0, 1, 2));
}

// Brute-force equivalence tests for QueryStrictlyIntersects and QueryTouches.
// Boost.Geometry implements touches only for box-box among the types here,
// so these queries need box keys and box queries.
// The data is grid-aligned so that many shapes abut exactly, which exercises
// the touching cases as well as the overlapping ones.
namespace brute_force {

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

}  // namespace brute_force

TEST(RtreeBruteForceTest, BoxValuesBoxQueries) {
  std::mt19937 rng(1);
  brute_force::ExpectMatchesScan(
      brute_force::RandomGridBoxes(rng, 2000, 2000, 100),
      brute_force::RandomGridBoxes(rng, 300, 2000, 300));
}

TEST(RtreeBruteForceTest, BoxMapValuesBoxQueries) {
  std::mt19937 rng(2);
  std::vector<std::pair<Box2_i, int>> values;
  for (const Box2_i& b : brute_force::RandomGridBoxes(rng, 2000, 2000, 100)) {
    values.emplace_back(b, static_cast<int>(values.size()));
  }
  const std::vector<Box2_i> queries =
      brute_force::RandomGridBoxes(rng, 300, 2000, 300);
  brute_force::ExpectMatchesScan(values, queries);

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
  for (const Box2_i& b : brute_force::RandomGridBoxes(rng, 1000, 1500, 100)) {
    const int id = static_cast<int>(values.size());
    values.emplace_back(b, id, -id);
  }
  brute_force::ExpectMatchesScan(
      values, brute_force::RandomGridBoxes(rng, 200, 1500, 300));
}

}  // namespace moab

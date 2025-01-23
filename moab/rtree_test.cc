#include "rtree.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <utility>

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

  std::vector<int> segments =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(0, 5), Point2_i(10, 5)));
  EXPECT_THAT(segments, UnorderedElementsAre(1, 2));

  segments =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(5, 0), Point2_i(5, 10)));
  EXPECT_THAT(segments, UnorderedElementsAre(0, 2));

  segments =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(-5, 0), Point2_i(0, 0)));
  EXPECT_THAT(segments, UnorderedElementsAre(0, 1, 2));

  segments =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(10, 0), Point2_i(20, 0)));
  EXPECT_THAT(segments, UnorderedElementsAre(0));

  segments =
      rtree.QueryIntersects<1>(Segment2_i(Point2_i(5, 0), Point2_i(15, 0)));
  EXPECT_THAT(segments, UnorderedElementsAre(0));
}

TEST(RtreeSegmentMapTest, QueryIntersectsValueBox) {
  RtreeMapSegment2_i<int> rtree;
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 0)), 0);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(0, 10)), 1);
  rtree.Insert(Segment2_i(Point2_i(0, 0), Point2_i(10, 10)), 2);

  std::vector<int> segments = rtree.QueryIntersects<1>(Box2_i(0, 5, 10, 10));

  EXPECT_THAT(segments, UnorderedElementsAre(1, 2));
}

}  // namespace moab

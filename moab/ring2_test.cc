#include "ring2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"
#include "moab/box2.h"
#include "moab/point2.h"

namespace moab {

using ::testing::ElementsAre;
using ::testing::Pair;
using ::testing::StrEq;
using ::testing::UnorderedElementsAre;

TEST(Constructor, Default) {
  Ring2_i r;

  EXPECT_EQ(r.Size(), 0);
}

TEST(Constructor, VectorCopy) {
  std::vector<Point2_i> points = {Point2_i(0, 0), Point2_i(2, 0),
                                  Point2_i(2, 2), Point2_i(0, 2),
                                  Point2_i(0, 0)};
  Ring2_i r(points);

  EXPECT_EQ(points.size(), 5);
  EXPECT_EQ(r.Size(), 5);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Constructor, VectorMove) {
  std::vector<Point2_i> points = {Point2_i(0, 0), Point2_i(2, 0),
                                  Point2_i(2, 2), Point2_i(0, 2),
                                  Point2_i(0, 0)};
  Ring2_i r(std::move(points));

  EXPECT_EQ(points.size(), 0);
  EXPECT_EQ(r.Size(), 5);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Constructor, InitializerList) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Size(), 5);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Constructors, Box) {
  Box2_i b(0, 0, 2, 2);
  Ring2_i r(b);

  EXPECT_EQ(r.Size(), 5);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Constructor, Copy) {
  Ring2_i r1 = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
                Point2_i(0, 0)};
  Ring2_i r2(r1);

  EXPECT_EQ(r1.Size(), 5);
  EXPECT_THAT(r1.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
  EXPECT_EQ(r2.Size(), 5);
  EXPECT_THAT(r2.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Constructor, Move) {
  Ring2_i r1 = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
                Point2_i(0, 0)};
  Ring2_i r2(std::move(r1));

  EXPECT_EQ(r1.Size(), 0);
  EXPECT_THAT(r1.Points(), ElementsAre());
  EXPECT_EQ(r2.Size(), 5);
  EXPECT_THAT(r2.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Accessors, Points) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Size(), 5);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Accessors, PointsConst) {
  const Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                     Point2_i(0, 2), Point2_i(0, 0)};

  EXPECT_EQ(r.Size(), 5);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Accessors, Size) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Size(), 5);
}

TEST(Accessors, Empty1) {
  Ring2_i r;

  EXPECT_EQ(r.Size(), 0);
  EXPECT_TRUE(r.Empty());
}

TEST(Accessors, Empty2) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Size(), 5);
  EXPECT_FALSE(r.Empty());
}

TEST(Accessors, Area1) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Area(), 2 * 2);
}

TEST(Accessors, Area2) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(4, 0), Point2_i(4, 4), Point2_i(2, 4),
               Point2_i(2, 2), Point2_i(0, 2), Point2_i(0, 0)};

  EXPECT_EQ(r.Area(), 2 * 2 + 2 * 4);
}

TEST(Accessors, Area3) {
  Ring2_i r = {Point2_i(0, 0),   Point2_i(40, 0),  Point2_i(40, 40),
               Point2_i(20, 40), Point2_i(20, 20), Point2_i(0, 20),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Area(), 20 * 20 + 20 * 40);
}

TEST(Accessors, Centroid1) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Centroid(), Point2_i(1, 1));
}

TEST(Accessors, Centroid2) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(4, 0), Point2_i(4, 4), Point2_i(2, 4),
               Point2_i(2, 2), Point2_i(0, 2), Point2_i(0, 0)};

  EXPECT_EQ(r.Centroid(), Point2_i(2, 1));
}

TEST(Accessors, Centroid3) {
  Ring2_i r = {Point2_i(0, 0),   Point2_i(40, 0),  Point2_i(40, 40),
               Point2_i(20, 40), Point2_i(20, 20), Point2_i(0, 20),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Centroid(), Point2_i(23, 16));
}

TEST(Accessors, BoundingBox1) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_EQ(r.BoundingBox(), Box2_i(0, 0, 2, 2));
}

TEST(Accessors, BoundingBox2) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(4, 0), Point2_i(4, 4), Point2_i(2, 4),
               Point2_i(2, 2), Point2_i(0, 2), Point2_i(0, 0)};

  EXPECT_EQ(r.BoundingBox(), Box2_i(0, 0, 4, 4));
}

TEST(Accessors, BoundingBox3) {
  Ring2_i r = {Point2_i(0, 0),   Point2_i(40, 0),  Point2_i(40, 40),
               Point2_i(20, 40), Point2_i(20, 20), Point2_i(0, 20),
               Point2_i(0, 0)};

  EXPECT_EQ(r.BoundingBox(), Box2_i(0, 0, 40, 40));
}

TEST(Operations, MaxBoxes) {
  Ring2_i r = {Point2_i(0, 0),   Point2_i(40, 0),  Point2_i(40, 40),
               Point2_i(20, 40), Point2_i(20, 20), Point2_i(0, 20),
               Point2_i(0, 0)};

  std::vector<Box2_i> boxes;
  boxes = r.MaxBoxes();

  EXPECT_THAT(
      boxes, UnorderedElementsAre(Box2_i(20, 0, 40, 40), Box2_i(0, 0, 40, 20)));
}

TEST(Mutators, Clear) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};
  r.Clear();

  EXPECT_EQ(r.Size(), 0);
  EXPECT_TRUE(r.Empty());
}

TEST(Mutators, Append) {
  Ring2_i r;
  r.Append(Point2_i(0, 0));
  r.Append(Point2_i(2, 0));
  r.Append(Point2_i(2, 2));
  r.Append(Point2_i(0, 2));
  r.Append(Point2_i(0, 0));

  EXPECT_EQ(r.Size(), 5);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Mutators, Resize1) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};
  r.Resize(0);

  EXPECT_EQ(r.Size(), 0);
  EXPECT_TRUE(r.Empty());
  EXPECT_THAT(r.Points(), ElementsAre());
}

TEST(Mutators, Resize2) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};
  r.Resize(10);

  EXPECT_EQ(r.Size(), 10);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0), Point2_i(0, 0),
                          Point2_i(0, 0), Point2_i(0, 0), Point2_i(0, 0),
                          Point2_i(0, 0)));
}

TEST(Mutators, Resize3) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};
  r.Resize(3);

  EXPECT_EQ(r.Size(), 3);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2)));
}

TEST(Mutators, AssignIterator) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(1, 0), Point2_i(1, 1), Point2_i(0, 1),
               Point2_i(0, 0)};
  std::vector<Point2_i> points = {Point2_i(0, 0), Point2_i(1, 0),
                                  Point2_i(2, 0), Point2_i(2, 2),
                                  Point2_i(0, 2), Point2_i(0, 0)};
  r.Assign(points.begin(), points.end());

  EXPECT_EQ(r.Size(), 6);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(1, 0), Point2_i(2, 0),
                          Point2_i(2, 2), Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Mutators, AssignInitializer) {
  Ring2_i r;
  r.Assign({Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
            Point2_i(0, 0)});

  EXPECT_EQ(r.Size(), 5);
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                          Point2_i(0, 2), Point2_i(0, 0)));
}

TEST(Operations, BloatedRings1) {
  Ring2_i r = {Point2_i(0, 0),   Point2_i(40, 0),  Point2_i(40, 40),
               Point2_i(20, 40), Point2_i(20, 20), Point2_i(0, 20),
               Point2_i(0, 0)};

  std::vector<Ring2_i> rings = r.BloatedRings(5, 5);

  EXPECT_EQ(rings.size(), 1);

  EXPECT_EQ(rings[0].Area(), 20 * 30 + 30 * 50);
}

TEST(Operators, Subscript) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_EQ(r.Size(), 5);
  EXPECT_EQ(r[0], Point2_i(0, 0));
  EXPECT_EQ(r[1], Point2_i(2, 0));
  EXPECT_EQ(r[2], Point2_i(2, 2));
  EXPECT_EQ(r[3], Point2_i(0, 2));
  EXPECT_EQ(r[4], Point2_i(0, 0));

  r[0] = Point2_i(1, 1);
  r[1] = Point2_i(2, 1);
  r[2] = Point2_i(2, 2);
  r[3] = Point2_i(1, 2);
  r[4] = Point2_i(1, 1);

  EXPECT_EQ(r[0], Point2_i(1, 1));
  EXPECT_EQ(r[1], Point2_i(2, 1));
  EXPECT_EQ(r[2], Point2_i(2, 2));
  EXPECT_EQ(r[3], Point2_i(1, 2));
  EXPECT_EQ(r[4], Point2_i(1, 1));
}

TEST(Operators, SubscriptConst) {
  const Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2),
                     Point2_i(0, 2), Point2_i(0, 0)};

  EXPECT_EQ(r.Size(), 5);
  EXPECT_EQ(r[0], Point2_i(0, 0));
  EXPECT_EQ(r[1], Point2_i(2, 0));
  EXPECT_EQ(r[2], Point2_i(2, 2));
  EXPECT_EQ(r[3], Point2_i(0, 2));
  EXPECT_EQ(r[4], Point2_i(0, 0));
}

TEST(Operators, Equality) {
  Ring2_i r1 = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
                Point2_i(0, 0)};
  Ring2_i r2 = {Point2_i(0, 0), Point2_i(1, 0), Point2_i(2, 0),
                Point2_i(2, 2), Point2_i(0, 2), Point2_i(0, 0)};
  Ring2_i r3 = {Point2_i(0, 0), Point2_i(1, 0), Point2_i(2, 0), Point2_i(2, 2),
                Point2_i(1, 2), Point2_i(0, 2), Point2_i(0, 0)};

  EXPECT_EQ(r1.Size(), 5);
  EXPECT_EQ(r2.Size(), 6);
  EXPECT_EQ(r3.Size(), 7);
  EXPECT_TRUE(r1 == r2);
  EXPECT_TRUE(r1 == r3);
  EXPECT_TRUE(r2 == r3);
  EXPECT_EQ(r1, r2);
  EXPECT_EQ(r1, r3);
  EXPECT_EQ(r2, r3);
}

TEST(StringConversion, ToString) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};

  EXPECT_THAT(r.ToString(), StrEq("((0 0) (2 0) (2 2) (0 2) (0 0))"));
}

TEST(StringConversion, SupportsAbslStringify) {
  Ring2_i r = {Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)};
  std::string s = absl::StrFormat("%v", r);

  EXPECT_THAT(s, StrEq("((0 0) (2 0) (2 2) (0 2) (0 0))"));
}

TEST(Hash, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Ring2_i(),
      Ring2_i({Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)}),
      Ring2_i({Point2_i(0, 0), Point2_i(2, 0), Point2_i(2, 2), Point2_i(0, 2),
               Point2_i(0, 0)}),
      Ring2_i({Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 20),
               Point2_i(0, 20), Point2_i(0, 0)}),
  }));
}

}  // namespace moab

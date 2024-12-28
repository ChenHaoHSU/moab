#include "operation.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "absl/log/log.h"
#include "moab/box2.h"
#include "moab/point2.h"
#include "moab/ring2.h"

namespace moab {

using ::testing::UnorderedElementsAre;

TEST(PolygonOperators, UnionBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1)};

  UnionSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 1)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1)));
}

TEST(PolygonOperators, UnionBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)};

  UnionSet(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(0, 0, 2, 1), Box2_i(0, 1, 1, 2)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)));
}

TEST(PolygonOperators, UnionBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1), Box2_i(10, 10, 20, 20)};

  UnionSet(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(0, 0, 2, 1), Box2_i(10, 10, 20, 20)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(10, 10, 20, 20)));
}

TEST(PolygonOperators, UnionBoxBox4) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1),
                             Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1)};

  UnionSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1),
                                   Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1)));
}

TEST(PolygonOperators, UnionBoxBox5) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 2)};

  UnionSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
}

TEST(PolygonOperators, UnionRingRing1) {
  std::vector<Ring2_i> lhs = {Ring2_i(Box2_i(0, 0, 1, 1))};
  std::vector<Ring2_i> rhs = {Ring2_i(Box2_i(1, 0, 2, 1))};

  UnionSet(lhs, rhs);

  EXPECT_TRUE(moab::Equivalence(
      lhs, std::vector<Ring2_i>{Ring2_i(Box2_i(0, 0, 2, 1))}));
  EXPECT_THAT(rhs, UnorderedElementsAre(Ring2_i(Box2_i(1, 0, 2, 1))));
}

TEST(PolygonOperators, UnionRingRing2) {
  std::vector<Ring2_i> lhs = {Ring2_i(Box2_i(0, 0, 2, 1))};
  std::vector<Ring2_i> rhs = {Ring2_i(Box2_i(0, 0, 1, 2))};

  UnionSet(lhs, rhs);

  EXPECT_TRUE(moab::Equivalence(
      lhs, std::vector<Ring2_i>{Ring2_i(Box2_i(0, 0, 2, 1)),
                                Ring2_i(Box2_i(0, 0, 1, 2))}));
  EXPECT_THAT(rhs, UnorderedElementsAre(Ring2_i(Box2_i(0, 0, 1, 2))));
}

TEST(PolygonOperators, UnionRingBox1) {
  std::vector<Ring2_i> lhs = {Ring2_i(Box2_i(0, 0, 1, 1))};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1)};

  UnionSet(lhs, rhs);

  EXPECT_TRUE(moab::Equivalence(
      lhs, std::vector<Ring2_i>{Ring2_i(Box2_i(0, 0, 2, 1))}));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1)));
}

TEST(PolygonOperators, IntersectionBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1)};

  IntersectionSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre());
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1)));
}

TEST(PolygonOperators, IntersectionBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)};

  IntersectionSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre());
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)));
}

TEST(PolygonOperators, IntersectionBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1)};

  IntersectionSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
}

TEST(PolygonOperators, DisjointUnionBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 2)};

  DisjointUnionSet(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
}

TEST(PolygonOperators, DisjointUnionBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 2), Box2_i(10, 10, 20, 20)};

  DisjointUnionSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2),
                                        Box2_i(10, 10, 20, 20)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(0, 0, 2, 2), Box2_i(10, 10, 20, 20)));
}

TEST(PolygonOperators, DisjointUnionBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1)};

  DisjointUnionSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 1)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1)));
}

TEST(PolygonOperators, SubtractBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1)};

  SubtractSet(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
}

TEST(PolygonOperators, SubtractBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 2)};

  SubtractSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre());
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
}

TEST(PolygonOperators, SubtractBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)};

  SubtractSet(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)));
}

TEST(PolygonOperators, SubtractBoxBox4) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2), Box2_i(2, 0, 5, 2)};
  std::vector<Box2_i> rhs = {Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)};

  SubtractSet(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(0, 0, 2, 2), Box2_i(4, 0, 5, 2)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)));
}

TEST(PolygonFunctions, AssignBoxBox1) {
  std::vector<Box2_i> lhs;
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1)};

  Assign(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
}

TEST(PolygonFunctions, AssignBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1)};

  Assign(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
}

TEST(PolygonFunctions, AssignBoxBox3) {
  std::vector<Box2_i> lhs;
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2)};

  Assign(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2)));
}

TEST(PolygonFunctions, AssignBoxBox4) {
  std::vector<Box2_i> lhs;
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2),
                             Box2_i(0, 0, 4, 4), Box2_i(10, 10, 20, 20)};

  Assign(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(0, 0, 4, 4), Box2_i(10, 10, 20, 20)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2),
                                   Box2_i(0, 0, 4, 4), Box2_i(10, 10, 20, 20)));
}

TEST(PolygonFunctions, EquivalenceBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1)};

  EXPECT_TRUE(Equivalence(lhs, rhs));
}

TEST(PolygonFunctions, EquivalenceBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 1)};

  EXPECT_FALSE(Equivalence(lhs, rhs));
}

TEST(PolygonFunctions, EquivalenceBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1), Box2_i(1, 0, 2, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 1)};

  EXPECT_TRUE(Equivalence(lhs, rhs));
}

TEST(PolygonFunctions, EquivalenceBoxBox4) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1),
                             Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1),
                             Box2_i(1, 0, 2, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 1), Box2_i(0, 0, 2, 1),
                             Box2_i(0, 0, 2, 1)};

  EXPECT_TRUE(Equivalence(lhs, rhs));
}

TEST(PolygonFunctions, EquivalenceBoxBox5) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 2)};

  EXPECT_FALSE(Equivalence(lhs, rhs));
}

TEST(PolygonFunctions, AreaBox1) {
  std::vector<Box2_i> set = {Box2_i(0, 0, 1, 1)};

  EXPECT_EQ(Area(set), 1);
}

TEST(PolygonFunctions, AreaBox2) {
  std::vector<Box2_i> set = {Box2_i(0, 0, 1, 1), Box2_i(1, 0, 2, 1)};

  EXPECT_EQ(Area(set), 2);
}

TEST(PolygonFunctions, AreaBox3) {
  std::vector<Box2_i> set = {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2)};

  EXPECT_EQ(Area(set), 4);
}

TEST(PolygonFunctions, AreaBox4) {
  std::vector<Box2_i> set = {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2),
                             Box2_i(0, 0, 4, 4), Box2_i(10, 10, 20, 20)};

  EXPECT_EQ(Area(set), 116);
}

TEST(PolygonFunctions, AreaBox5) {
  std::vector<Box2_i> set = {Box2_i(0, 0, 0, 0)};

  EXPECT_EQ(Area(set), 0);
}

TEST(PolygonFunctions, AreaRing1) {
  std::vector<Ring2_i> set = {Ring2_i(Box2_i(0, 0, 10, 10))};

  EXPECT_EQ(Area(set), 100);
}

TEST(PolygonFunctions, AreaRing2) {
  std::vector<Ring2_i> set = {Ring2_i(Box2_i(0, 0, 10, 10)),
                              Ring2_i(Box2_i(0, 0, 10, 10)),
                              Ring2_i(Box2_i(4, 4, 6, 6))};

  EXPECT_EQ(Area(set), 100);
}

TEST(PolygonFunctions, AreaRing3) {
  std::vector<Ring2_i> set = {Ring2_i(Box2_i(0, 0, 10, 10)),
                              Ring2_i(Box2_i(5, 0, 15, 10))};

  EXPECT_EQ(Area(set), 150);
}

TEST(GeometryFunctions, IsCoveredByPointBox) {
  Box2_i b(0, 0, 10, 10);
  Point2_i p1(0, 0);
  Point2_i p2(10, 10);
  Point2_i p3(0, 10);
  Point2_i p4(10, 0);
  Point2_i p5(5, 5);
  Point2_i p6(3, 4);
  Point2_i p7(3, 10);
  Point2_i p8(0, 5);
  Point2_i p9(10, 5);
  Point2_i p10(5, 0);
  Point2_i p11(11, 0);
  Point2_i p12(0, 11);
  Point2_i p13(-1, 0);
  Point2_i p14(0, -1);
  Point2_i p15(100, -1);
  Point2_i p16(0, 20);

  EXPECT_TRUE(IsCoveredBy(p1, b));
  EXPECT_TRUE(IsCoveredBy(p2, b));
  EXPECT_TRUE(IsCoveredBy(p3, b));
  EXPECT_TRUE(IsCoveredBy(p4, b));
  EXPECT_TRUE(IsCoveredBy(p5, b));
  EXPECT_TRUE(IsCoveredBy(p6, b));
  EXPECT_TRUE(IsCoveredBy(p7, b));
  EXPECT_TRUE(IsCoveredBy(p8, b));
  EXPECT_TRUE(IsCoveredBy(p9, b));
  EXPECT_TRUE(IsCoveredBy(p10, b));
  EXPECT_FALSE(IsCoveredBy(p11, b));
  EXPECT_FALSE(IsCoveredBy(p12, b));
  EXPECT_FALSE(IsCoveredBy(p13, b));
  EXPECT_FALSE(IsCoveredBy(p14, b));
  EXPECT_FALSE(IsCoveredBy(p15, b));
  EXPECT_FALSE(IsCoveredBy(p16, b));
}

TEST(GeometryFunctions, IsCoveredByBoxBox) {
  Box2_i b1(0, 0, 10, 10);
  Box2_i b2(0, 0, 1, 1);
  Box2_i b3(1, 1, 2, 2);
  Box2_i b4(-1, 0, 11, 10);
  Box2_i b5(0, 0, 11, 11);
  Box2_i b6(-1, 4, 11, 6);

  EXPECT_TRUE(IsCoveredBy(b1, b1));
  EXPECT_FALSE(IsCoveredBy(b1, b2));
  EXPECT_TRUE(IsCoveredBy(b2, b1));
  EXPECT_FALSE(IsCoveredBy(b1, b3));
  EXPECT_TRUE(IsCoveredBy(b3, b1));
  EXPECT_TRUE(IsCoveredBy(b1, b4));
  EXPECT_FALSE(IsCoveredBy(b4, b1));
  EXPECT_TRUE(IsCoveredBy(b1, b5));
  EXPECT_FALSE(IsCoveredBy(b5, b1));
  EXPECT_FALSE(IsCoveredBy(b1, b6));
  EXPECT_FALSE(IsCoveredBy(b6, b1));
}

TEST(GeometryFunctions, IsIntersectPointBox) {
  Box2_i b(0, 0, 10, 10);
  Point2_i p1(0, 0);
  Point2_i p2(10, 10);
  Point2_i p3(0, 10);
  Point2_i p4(10, 0);
  Point2_i p5(5, 5);
  Point2_i p6(3, 4);
  Point2_i p7(3, 10);
  Point2_i p8(0, 5);
  Point2_i p9(10, 5);
  Point2_i p10(5, 0);
  Point2_i p11(11, 0);
  Point2_i p12(0, 11);
  Point2_i p13(-1, 0);
  Point2_i p14(0, -1);
  Point2_i p15(100, -1);
  Point2_i p16(0, 20);

  EXPECT_TRUE(IsIntersect(p1, b));
  EXPECT_TRUE(IsIntersect(p2, b));
  EXPECT_TRUE(IsIntersect(p3, b));
  EXPECT_TRUE(IsIntersect(p4, b));
  EXPECT_TRUE(IsIntersect(p5, b));
  EXPECT_TRUE(IsIntersect(p6, b));
  EXPECT_TRUE(IsIntersect(p7, b));
  EXPECT_TRUE(IsIntersect(p8, b));
  EXPECT_TRUE(IsIntersect(p9, b));
  EXPECT_TRUE(IsIntersect(p10, b));
  EXPECT_FALSE(IsIntersect(p11, b));
  EXPECT_FALSE(IsIntersect(p12, b));
  EXPECT_FALSE(IsIntersect(p13, b));
  EXPECT_FALSE(IsIntersect(p14, b));
  EXPECT_FALSE(IsIntersect(p15, b));
  EXPECT_FALSE(IsIntersect(p16, b));
}

TEST(GeometryFunctions, IsIntersectBoxBox) {
  Box2_i b1(0, 0, 10, 10);
  Box2_i b2(0, 0, 1, 1);
  Box2_i b3(1, 1, 2, 2);
  Box2_i b4(-1, 0, 11, 10);
  Box2_i b5(0, 0, 11, 11);
  Box2_i b6(-1, 4, 11, 6);
  Box2_i b7(10, 10, 20, 20);
  Box2_i b8(0, 10, 0, 20);
  Box2_i b9(0, 0, 0, 0);
  Box2_i b10(100, 100, 200, 200);

  EXPECT_TRUE(IsIntersect(b1, b1));
  EXPECT_TRUE(IsIntersect(b1, b2));
  EXPECT_TRUE(IsIntersect(b1, b3));
  EXPECT_TRUE(IsIntersect(b1, b4));
  EXPECT_TRUE(IsIntersect(b1, b5));
  EXPECT_TRUE(IsIntersect(b1, b6));
  EXPECT_TRUE(IsIntersect(b1, b7));
  EXPECT_TRUE(IsIntersect(b1, b8));
  EXPECT_TRUE(IsIntersect(b1, b9));
  EXPECT_FALSE(IsIntersect(b1, b10));
}

TEST(GeometryFunctions, IsWithinPointBox) {
  Box2_i b(0, 0, 10, 10);
  Point2_i p1(0, 0);
  Point2_i p2(10, 10);
  Point2_i p3(0, 10);
  Point2_i p4(10, 0);
  Point2_i p5(5, 5);
  Point2_i p6(3, 4);
  Point2_i p7(3, 10);
  Point2_i p8(0, 5);
  Point2_i p9(10, 5);
  Point2_i p10(5, 0);
  Point2_i p11(11, 0);
  Point2_i p12(0, 11);
  Point2_i p13(-1, 0);
  Point2_i p14(0, -1);
  Point2_i p15(100, -1);
  Point2_i p16(0, 20);

  EXPECT_FALSE(IsWithin(p1, b));
  EXPECT_FALSE(IsWithin(p2, b));
  EXPECT_FALSE(IsWithin(p3, b));
  EXPECT_FALSE(IsWithin(p4, b));
  EXPECT_TRUE(IsWithin(p5, b));
  EXPECT_TRUE(IsWithin(p6, b));
  EXPECT_FALSE(IsWithin(p7, b));
  EXPECT_FALSE(IsWithin(p8, b));
  EXPECT_FALSE(IsWithin(p9, b));
  EXPECT_FALSE(IsWithin(p10, b));
  EXPECT_FALSE(IsWithin(p11, b));
  EXPECT_FALSE(IsWithin(p12, b));
  EXPECT_FALSE(IsWithin(p13, b));
  EXPECT_FALSE(IsWithin(p14, b));
  EXPECT_FALSE(IsWithin(p15, b));
  EXPECT_FALSE(IsWithin(p16, b));
}

TEST(GeometryFunctions, IsWithinBoxBox) {
  Box2_i b1(0, 0, 10, 10);
  Box2_i b2(0, 0, 1, 1);
  Box2_i b3(1, 1, 2, 2);
  Box2_i b4(-1, 0, 11, 10);
  Box2_i b5(-1, -1, 11, 11);
  Box2_i b6(-1, 4, 11, 6);

  EXPECT_TRUE(IsWithin(b1, b1));
  EXPECT_FALSE(IsWithin(b1, b2));
  EXPECT_TRUE(IsWithin(b2, b1));
  EXPECT_FALSE(IsWithin(b1, b3));
  EXPECT_TRUE(IsWithin(b3, b1));
  EXPECT_TRUE(IsWithin(b1, b4));
  EXPECT_FALSE(IsWithin(b4, b1));
  EXPECT_TRUE(IsWithin(b1, b5));
  EXPECT_FALSE(IsWithin(b5, b1));
  EXPECT_FALSE(IsWithin(b1, b6));
  EXPECT_FALSE(IsWithin(b6, b1));
}

TEST(GeometryFunctions, IsOverlapBoxBox) {
  Box2_i b1(0, 0, 10, 10);
  Box2_i b2(0, 0, 1, 1);
  Box2_i b3(1, 1, 2, 2);
  Box2_i b4(-1, 0, 11, 10);
  Box2_i b5(0, 0, 11, 11);
  Box2_i b6(-1, 4, 11, 6);
  Box2_i b7(10, 10, 20, 20);
  Box2_i b8(0, 10, 0, 20);
  Box2_i b9(0, 0, 0, 0);
  Box2_i b10(100, 100, 200, 200);

  EXPECT_FALSE(IsOverlap(b1, b1));
  EXPECT_FALSE(IsOverlap(b1, b2));
  EXPECT_FALSE(IsOverlap(b1, b3));
  EXPECT_FALSE(IsOverlap(b1, b4));
  EXPECT_FALSE(IsOverlap(b1, b5));
  EXPECT_TRUE(IsOverlap(b1, b6));
  EXPECT_FALSE(IsOverlap(b1, b7));
  EXPECT_FALSE(IsOverlap(b1, b8));
  EXPECT_FALSE(IsOverlap(b1, b9));
  EXPECT_FALSE(IsOverlap(b1, b10));
}

TEST(GeometryFunctions, IsTouchBoxBox) {
  Box2_i b1(0, 0, 10, 10);
  Box2_i b2(0, 0, 1, 1);
  Box2_i b3(1, 1, 2, 2);
  Box2_i b4(-1, 0, 11, 10);
  Box2_i b5(0, 0, 11, 11);
  Box2_i b6(-1, 4, 11, 6);
  Box2_i b7(10, 10, 20, 20);
  Box2_i b8(0, 10, 0, 20);
  Box2_i b9(0, 0, 0, 0);
  Box2_i b10(100, 100, 200, 200);

  EXPECT_FALSE(IsTouch(b1, b1));
  EXPECT_FALSE(IsTouch(b1, b2));
  EXPECT_FALSE(IsTouch(b1, b3));
  EXPECT_FALSE(IsTouch(b1, b4));
  EXPECT_FALSE(IsTouch(b1, b5));
  EXPECT_FALSE(IsTouch(b1, b6));
  EXPECT_TRUE(IsTouch(b1, b7));
  EXPECT_TRUE(IsTouch(b1, b8));
  EXPECT_TRUE(IsTouch(b1, b9));
  EXPECT_FALSE(IsTouch(b1, b10));
}

TEST(GeometryFunctions, IsDisjointPointBox) {
  Box2_i b(0, 0, 10, 10);
  Point2_i p1(0, 0);
  Point2_i p2(10, 10);
  Point2_i p3(0, 10);
  Point2_i p4(10, 0);
  Point2_i p5(5, 5);
  Point2_i p6(3, 4);
  Point2_i p7(3, 10);
  Point2_i p8(0, 5);
  Point2_i p9(10, 5);
  Point2_i p10(5, 0);
  Point2_i p11(11, 0);
  Point2_i p12(0, 11);
  Point2_i p13(-1, 0);
  Point2_i p14(0, -1);
  Point2_i p15(100, -1);
  Point2_i p16(0, 20);

  EXPECT_FALSE(IsDisjoint(p1, b));
  EXPECT_FALSE(IsDisjoint(p2, b));
  EXPECT_FALSE(IsDisjoint(p3, b));
  EXPECT_FALSE(IsDisjoint(p4, b));
  EXPECT_FALSE(IsDisjoint(p5, b));
  EXPECT_FALSE(IsDisjoint(p6, b));
  EXPECT_FALSE(IsDisjoint(p7, b));
  EXPECT_FALSE(IsDisjoint(p8, b));
  EXPECT_FALSE(IsDisjoint(p9, b));
  EXPECT_FALSE(IsDisjoint(p10, b));
  EXPECT_TRUE(IsDisjoint(p11, b));
  EXPECT_TRUE(IsDisjoint(p12, b));
  EXPECT_TRUE(IsDisjoint(p13, b));
  EXPECT_TRUE(IsDisjoint(p14, b));
  EXPECT_TRUE(IsDisjoint(p15, b));
  EXPECT_TRUE(IsDisjoint(p16, b));
}

TEST(GeometryFunctions, IsDisjointBoxBox) {
  Box2_i b1(0, 0, 10, 10);
  Box2_i b2(0, 0, 1, 1);
  Box2_i b3(1, 1, 2, 2);
  Box2_i b4(-1, 0, 11, 10);
  Box2_i b5(0, 0, 11, 11);
  Box2_i b6(-1, 4, 11, 6);
  Box2_i b7(10, 10, 20, 20);
  Box2_i b8(0, 10, 0, 20);
  Box2_i b9(0, 0, 0, 0);
  Box2_i b10(100, 100, 200, 200);

  EXPECT_FALSE(IsDisjoint(b1, b1));
  EXPECT_FALSE(IsDisjoint(b1, b2));
  EXPECT_FALSE(IsDisjoint(b1, b3));
  EXPECT_FALSE(IsDisjoint(b1, b4));
  EXPECT_FALSE(IsDisjoint(b1, b5));
  EXPECT_FALSE(IsDisjoint(b1, b6));
  EXPECT_FALSE(IsDisjoint(b1, b7));
  EXPECT_FALSE(IsDisjoint(b1, b8));
  EXPECT_FALSE(IsDisjoint(b1, b9));
  EXPECT_TRUE(IsDisjoint(b1, b10));
}

TEST(GeometryFunctions, IsEqualBoxBox) {
  Box2_i b1(0, 0, 10, 10);
  Box2_i b2(0, 0, 1, 1);
  Box2_i b3(1, 1, 2, 2);
  Box2_i b4(-1, 0, 11, 10);
  Box2_i b5(0, 0, 11, 11);
  Box2_i b6(-1, 4, 11, 6);
  Box2_i b7(10, 10, 20, 20);
  Box2_i b8(0, 10, 0, 20);
  Box2_i b9(0, 0, 0, 0);
  Box2_i b10(100, 100, 200, 200);

  EXPECT_TRUE(IsEqual(b1, b1));
  EXPECT_FALSE(IsEqual(b1, b2));
  EXPECT_FALSE(IsEqual(b1, b3));
  EXPECT_FALSE(IsEqual(b1, b4));
  EXPECT_FALSE(IsEqual(b1, b5));
  EXPECT_FALSE(IsEqual(b1, b6));
  EXPECT_FALSE(IsEqual(b1, b7));
  EXPECT_FALSE(IsEqual(b1, b8));
  EXPECT_FALSE(IsEqual(b1, b9));
  EXPECT_FALSE(IsEqual(b1, b10));
}

TEST(GeometryFunctions, IsEqualRingRing) {
  Ring2_i r1 = Ring2_i(Box2_i(0, 0, 10, 10));
  Ring2_i r2 = Ring2_i({Point2_i(0, 0), Point2_i(5, 0), Point2_i(10, 0),
                        Point2_i(10, 10), Point2_i(0, 10), Point2_i(0, 5),
                        Point2_i(0, 0)});
  Ring2_i r3 = Ring2_i({Point2_i(0, 0), Point2_i(5, 0), Point2_i(11, 0),
                        Point2_i(11, 11), Point2_i(0, 11), Point2_i(0, 5),
                        Point2_i(0, 0)});

  EXPECT_TRUE(IsEqual(r1, r1));
  EXPECT_TRUE(IsEqual(r1, r2));
  EXPECT_FALSE(IsEqual(r1, r3));
}

TEST(GeometryFunctions, IsStrictlyIntersectBoxBox) {
  Box2_i b1(0, 0, 10, 10);
  Box2_i b2(0, 0, 1, 1);
  Box2_i b3(1, 1, 2, 2);
  Box2_i b4(-1, 0, 11, 10);
  Box2_i b5(0, 10, 10, 11);
  Box2_i b6(-1, 4, 11, 6);
  Box2_i b7(10, 10, 20, 20);
  Box2_i b8(0, 10, 0, 20);
  Box2_i b9(0, 0, 0, 0);
  Box2_i b10(100, 100, 200, 200);
  Box2_i b11(0, 10, 200, 10);

  EXPECT_TRUE(IsStrictlyIntersect(b1, b1));
  EXPECT_TRUE(IsStrictlyIntersect(b1, b2));
  EXPECT_TRUE(IsStrictlyIntersect(b1, b3));
  EXPECT_TRUE(IsStrictlyIntersect(b1, b4));
  EXPECT_FALSE(IsStrictlyIntersect(b1, b5));
  EXPECT_TRUE(IsStrictlyIntersect(b1, b6));
  EXPECT_FALSE(IsStrictlyIntersect(b1, b7));
  EXPECT_FALSE(IsStrictlyIntersect(b1, b8));
  EXPECT_FALSE(IsStrictlyIntersect(b1, b9));
  EXPECT_FALSE(IsStrictlyIntersect(b1, b10));
  EXPECT_FALSE(IsStrictlyIntersect(b1, b11));
}

}  // namespace moab
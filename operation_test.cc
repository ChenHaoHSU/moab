#include "operation.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "absl/log/log.h"
#include "box2.h"
#include "point2.h"

namespace moab {

using ::testing::UnorderedElementsAre;

TEST(PolygonOperators, UnionBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1)};

  Union(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 1)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1)));
}

TEST(PolygonOperators, UnionBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)};

  Union(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(0, 0, 2, 1), Box2_i(0, 1, 1, 2)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)));
}

TEST(PolygonOperators, UnionBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1), Box2_i(10, 10, 20, 20)};

  Union(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(0, 0, 2, 1), Box2_i(10, 10, 20, 20)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(10, 10, 20, 20)));
}

TEST(PolygonOperators, UnionBoxBox4) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1),
                             Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1)};

  Union(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1),
                                   Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1)));
}

TEST(PolygonOperators, UnionBoxBox5) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 2)};

  Union(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
}

TEST(PolygonOperators, IntersectionBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1)};

  Intersection(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre());
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1)));
}

TEST(PolygonOperators, IntersectionBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)};

  Intersection(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre());
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)));
}

TEST(PolygonOperators, IntersectionBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1)};

  Intersection(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
}

TEST(PolygonOperators, DisjointUnionBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 2)};

  DisjointUnion(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
}

TEST(PolygonOperators, DisjointUnionBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 2), Box2_i(10, 10, 20, 20)};

  DisjointUnion(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2),
                                        Box2_i(10, 10, 20, 20)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(0, 0, 2, 2), Box2_i(10, 10, 20, 20)));
}

TEST(PolygonOperators, DisjointUnionBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 1, 1)};
  std::vector<Box2_i> rhs = {Box2_i(1, 0, 2, 1)};

  DisjointUnion(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 1)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(1, 0, 2, 1)));
}

TEST(PolygonOperators, SubtractBoxBox1) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 1, 1)};

  Subtract(lhs, rhs);

  EXPECT_THAT(lhs,
              UnorderedElementsAre(Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2)));
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 1, 1)));
}

TEST(PolygonOperators, SubtractBoxBox2) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(0, 0, 2, 2)};

  Subtract(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre());
  EXPECT_THAT(rhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
}

TEST(PolygonOperators, SubtractBoxBox3) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2)};
  std::vector<Box2_i> rhs = {Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)};

  Subtract(lhs, rhs);

  EXPECT_THAT(lhs, UnorderedElementsAre(Box2_i(0, 0, 2, 2)));
  EXPECT_THAT(rhs,
              UnorderedElementsAre(Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)));
}

TEST(PolygonOperators, SubtractBoxBox4) {
  std::vector<Box2_i> lhs = {Box2_i(0, 0, 2, 2), Box2_i(2, 0, 5, 2)};
  std::vector<Box2_i> rhs = {Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)};

  Subtract(lhs, rhs);

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

}  // namespace moab
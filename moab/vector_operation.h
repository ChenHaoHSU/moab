#ifndef MOAB_VEC_OPERATION_H_
#define MOAB_VEC_OPERATION_H_

#include "boost/geometry.hpp"
#include "boost/polygon/polygon.hpp"
#include "boost/qvm/vec_operations.hpp"
#include "moab/vector2.h"

namespace moab {

template <typename T>
class Vector2;

// Please refer to
// https://www.boost.org/doc/libs/latest/libs/qvm/doc/html/index.html#_vector_operations
// for more information.

// ==================================
// Boost qvm vector operations
// ==================================

// boost::qvm

// constexpr auto UnionSet = [](auto& lhs, const auto& rhs) constexpr {
//   return boost::polygon::operators::operator|=(lhs, rhs);
// };

// Vector magnitude squared.
constexpr auto MagSqr = [](const auto& v) constexpr {
  Vector2<double> v_double(static_cast<double>(v.dx()),
                           static_cast<double>(v.dy()));

  return boost::qvm::mag_sqr(v_double);
};

// Vector magnitude.
// This is slow and only supports float and double. For integer vectors,
// consider using MagSqr instead.
constexpr auto Mag = [](const auto& v) constexpr {
  Vector2<double> v_double(static_cast<double>(v.dx()),
                           static_cast<double>(v.dy()));
  return boost::qvm::mag(v_double);
};

// Vector normalization.
constexpr auto Normalized = [](const auto& v) constexpr {
  return boost::qvm::normalized(v);
};

// In-place vector normalization.
// input must be a double/float vector
void Normalize(auto& v) { boost::qvm::normalize(v); }

// Vector dot product.
constexpr auto Dot = [](const auto& v1, const auto& v2) constexpr {
  return boost::qvm::dot(v1, v2);
};

// Vector cross product (retuns scalar determinant).
constexpr auto Cross = [](const auto& v1, const auto& v2) constexpr {
  return boost::qvm::cross(v1, v2);
};

constexpr auto Cos = [](const auto& v1, const auto& v2) constexpr {
  const double dot = static_cast<double>(Dot(v1, v2));
  // Use MagSqr to avoid double-sqrt and linker issues for ints
  const double mag_sqr_prod =
      static_cast<double>(MagSqr(v1)) * static_cast<double>(MagSqr(v2));

  if (mag_sqr_prod <= 0) return 0.0;

  return std::clamp(dot / std::sqrt(mag_sqr_prod), -1.0, 1.0);
};

constexpr auto Angle = [](const auto& v1, const auto& v2) constexpr {
  return boost::qvm::acos(Cos(v1, v2));
};

constexpr auto Rotate = [](const auto& v, double angle_rad) constexpr {
  const double cos_angle = std::cos(angle_rad);
  const double sin_angle = std::sin(angle_rad);
  return Vector2<double>{v.dx() * cos_angle - v.dy() * sin_angle,
                         v.dx() * sin_angle + v.dy() * cos_angle};
};

constexpr auto IsParallel = [](const auto& v1, const auto& v2) constexpr {
  return Cross(v1, v2) == 0;
};

constexpr auto IsOrthogonal = [](const auto& v1, const auto& v2) constexpr {
  return Dot(v1, v2) == 0;
};

constexpr auto Project = [](const auto& v1, const auto& v2) constexpr {
  const auto m2 = MagSqr(v2);
  if (m2 <= 0) return v2;  // Return zero vector if v2 is zero

  // Projection formula: (v1 . v2 / |v2|^2) * v2
  double scale = static_cast<double>(Dot(v1, v2)) / static_cast<double>(m2);
  return v2 * scale;
};

}  // namespace moab

#endif  // MOAB_VEC_OPERATION_H_

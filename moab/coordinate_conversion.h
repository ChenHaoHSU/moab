#ifndef MOAB_COORDINATE_CONVERSION_H_
#define MOAB_COORDINATE_CONVERSION_H_

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

namespace moab::internal {

// Returns the name of the coordinate type T, for error messages.
template <typename T>
constexpr absl::string_view CoordinateTypeName() {
  if constexpr (std::is_same_v<T, int32_t>) {
    return "int32";
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return "int64";
  } else if constexpr (std::is_same_v<T, float>) {
    return "float";
  } else if constexpr (std::is_same_v<T, double>) {
    return "double";
  } else {
    return "unknown";
  }
}

// Converts a coordinate read from a proto (type S) to the coordinate type T.
// 'what' names the coordinate in the error message, e.g. "[Box2] min_corner.x".
//
// Returns InvalidArgument if the value is not finite, is outside T's range, or,
// for an integral T, is not a whole number. A floating-point T may lose
// precision, as any conversion to a narrower floating-point type does.
template <typename T, typename S>
absl::StatusOr<T> ConvertCoordinate(S value, absl::string_view what) {
  static_assert(std::is_arithmetic_v<T> && std::is_arithmetic_v<S>,
                "Coordinates must be arithmetic types.");
  if constexpr (std::is_floating_point_v<S>) {
    if (!std::isfinite(value)) {
      return absl::InvalidArgumentError(
          absl::StrFormat("%s: coordinate %v is not finite.", what, value));
    }
  }

  if constexpr (std::is_floating_point_v<T>) {
    if constexpr (std::is_floating_point_v<S> && sizeof(S) > sizeof(T)) {
      if (value < std::numeric_limits<T>::lowest() ||
          value > std::numeric_limits<T>::max()) {
        return absl::InvalidArgumentError(
            absl::StrFormat("%s: coordinate %v does not fit in %s.", what,
                            value, CoordinateTypeName<T>()));
      }
    }
    return static_cast<T>(value);
  } else if constexpr (std::is_floating_point_v<S>) {
    static_assert(std::is_signed_v<T>, "Integral coordinates must be signed.");
    // T's range is [-2^k, 2^k). Both bounds are powers of two, so they are
    // exact in S.
    constexpr S kLow = static_cast<S>(std::numeric_limits<T>::min());
    constexpr S kHigh = -kLow;
    if (value < kLow || value >= kHigh) {
      return absl::InvalidArgumentError(
          absl::StrFormat("%s: coordinate %v does not fit in %s.", what, value,
                          CoordinateTypeName<T>()));
    }
    if (std::trunc(value) != value) {
      return absl::InvalidArgumentError(absl::StrFormat(
          "%s: coordinate %v is not a whole number, so it does not fit in %s.",
          what, value, CoordinateTypeName<T>()));
    }
    return static_cast<T>(value);
  } else {
    const T result = static_cast<T>(value);
    if (static_cast<S>(result) != value || (result < T{}) != (value < S{})) {
      return absl::InvalidArgumentError(
          absl::StrFormat("%s: coordinate %v does not fit in %s.", what, value,
                          CoordinateTypeName<T>()));
    }
    return result;
  }
}

}  // namespace moab::internal

#endif  // MOAB_COORDINATE_CONVERSION_H_

# moab

moab is a header-only C++17 library of 2-D and 3-D geometry types for
integer layout coordinates. It wraps
[Boost.Geometry](https://www.boost.org/libs/geometry) and
[Boost.Polygon](https://www.boost.org/libs/polygon) behind small value types,
adds an R-tree for spatial queries, and converts points, boxes and segments
to and from protocol buffer messages.

## Types

| Header | Type | What it is |
|---|---|---|
| `moab/point2.h` | `Point2<T>` | A 2-D point. |
| `moab/point3.h` | `Point3<T>` | A 3-D point. |
| `moab/interval.h` | `Interval<T>` | A closed 1-D range `[lo hi]`. |
| `moab/box2.h` | `Box2<T>` | An axis-aligned box whose corners are always ordered. |
| `moab/segment2.h` | `Segment2<T>` | A 2-D segment between two points. |
| `moab/segment3.h` | `Segment3<T>` | A 3-D segment between two points. |
| `moab/ring2.h` | `Ring2<T>` | A polygon outline as a list of points, which Boost.Geometry reads as closed and counterclockwise. |
| `moab/rtree.h` | `Rtree<V>` | An R-tree of boxes, points or segments, optionally mapped to values. |
| `moab/operation.h` | functions | Set operations (union, intersection, subtraction) and predicates (`IsIntersect`, `IsWithin`, `IsTouch`, ...). |
| `moab/coordinate_conversion.h` | functions | Checked conversion between coordinate types. |

`moab/moab.h` includes all of them.

Every type is a template on the coordinate type `T`. Aliases exist for the
usual integer types: `Box2_i` (`int`), `Box2_i32` (`int32_t`) and `Box2_i64`
(`int64_t`), and likewise for the other types. `Rtree` has aliases such as
`RtreeBox2_i`, `RtreeMapBox2_i<V>` (box to value) and `RtreeMultiMapBox2_i<V...>`.

A few conventions hold across the library:

- **Areas and squared distances** are returned as `area_type`, which is
  `int64_t` for integer coordinates, so they do not overflow.
- **Value types are `constexpr`** where C++17 allows, so geometry constants
  can be built at compile time.
- **`Ring2` equality means the same shape.** Two rings are equal when they
  cover the same area, even if they start at a different point or have extra
  points along an edge.
- **Debug builds check preconditions** with `DCHECK`, for example index
  bounds and non-negative bloat amounts.

## Example

```cpp
#include "moab/moab.h"

// Boxes keep their corners ordered, and Intersection returns std::nullopt
// when the boxes are apart.
const moab::Box2_i a(0, 0, 10, 10);
const moab::Box2_i b(5, 5, 20, 20);
const std::optional<moab::Box2_i> common = a.Intersection(b);  // (5 5)-(10 10)

// An R-tree map from boxes to ids.
moab::RtreeMapBox2_i<int> rtree;
rtree.Insert(a, 0);
rtree.Insert(b, 1);
const std::vector<int> ids = rtree.QueryIntersects<1>(moab::Box2_i(8, 8, 9, 9));  // 0 and 1
const bool any = rtree.AnyOf(moab::index::Intersects(moab::Point2_i(30, 30)));    // false

// An L-shaped ring and the maximal boxes inside it.
const moab::Ring2_i ring = {
    moab::Point2_i(0, 0),   moab::Point2_i(40, 0), moab::Point2_i(40, 20),
    moab::Point2_i(20, 20), moab::Point2_i(20, 40), moab::Point2_i(0, 40),
    moab::Point2_i(0, 0)};
const std::vector<moab::Box2_i> boxes = ring.MaxBoxes();  // (0 0)-(40 20), (0 0)-(20 40)
```

## Using moab in a Bazel module

moab builds with Bazel modules (Bzlmod). Add it to your `MODULE.bazel`,
pinned to a release tag:

```starlark
bazel_dep(name = "moab")
git_override(
    module_name = "moab",
    remote = "https://github.com/ChenHaoHSU/moab",
    tag = "v2.0",
)
```

Then depend on the whole library, or on one type:

```starlark
cc_library(
    name = "my_lib",
    srcs = ["my_lib.cc"],
    deps = [
        "@moab//moab",       # Everything, through moab/moab.h.
        "@moab//moab:box2",  # Or only Box2 and what it needs.
    ],
)
```

moab needs C++17. Its dependencies, which Bazel fetches, are
[Abseil](https://abseil.io), Boost.Geometry, Boost.Polygon, Boost.Iterator
and [protobuf](https://protobuf.dev); the tests also use
[GoogleTest](https://github.com/google/googletest).

## Building and testing

```sh
bazel build //...
bazel test //...           # Debug checks on.
bazel test -c opt //...    # Optimized, as most users build it.
```

The repository's `.bazelrc` selects C++17 and GCC.

The headers are kept at 100% line and function coverage:

```sh
bazel coverage --combined_report=lcov //moab:all
# The report is bazel-out/_coverage/_coverage_report.dat.
```

C++ files are formatted with `clang-format` using the repository's
`.clang-format`:

```sh
clang-format --dry-run --Werror moab/*.h moab/*.cc
```

## License

moab is licensed under the Apache License, Version 2.0. See
[LICENSE](LICENSE) and [NOTICE](NOTICE).

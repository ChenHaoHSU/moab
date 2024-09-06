workspace(name = "moab")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "bazel_skylib",
    remote = "https://github.com/bazelbuild/bazel-skylib.git",
    tag = "1.5.0",
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

## Bazel rules.
## Abseil-cpp
git_repository(
    name = "com_google_absl",
    remote = "https://github.com/abseil/abseil-cpp.git",
    tag = "20240116.2",
)

## Testing
git_repository(
    name = "com_google_googletest",
    remote = "https://github.com/google/googletest.git",
    tag = "v1.14.0",
)

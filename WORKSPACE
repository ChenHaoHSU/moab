
workspace(name = "third_party_moab")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "bazel_skylib",
    tag = "1.5.0",
    remote = "https://github.com/bazelbuild/bazel-skylib.git",
)
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()

## Bazel rules.
## Abseil-cpp
git_repository(
    name = "third_party_absl",
    tag = "20240116.2",
    #patches = ["//patches:abseil-cpp-20240116.2.patch"],
    #patch_args = ["-p1"],
    remote = "https://github.com/abseil/abseil-cpp.git",
)

## Testing
git_repository(
    name = "third_party_googletest",
    tag = "v1.14.0",
    remote = "https://github.com/google/googletest.git",
)

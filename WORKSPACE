workspace(name = "moab")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

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

## Boost
# Add the rules_boost repository
http_archive(
    name = "com_github_nelhage_rules_boost",
    strip_prefix = "rules_boost-master",
    urls = ["https://github.com/nelhage/rules_boost/archive/refs/heads/master.tar.gz"],
)

# Load the boost rules
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

# Call the boost_deps function to define the dependencies
boost_deps()

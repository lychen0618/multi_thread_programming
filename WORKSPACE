workspace(name = "multi-thread")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_github_google_glog",
    sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
    strip_prefix = "glog-0.5.0",
    urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
)

http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/2a02723b76eae879bafbcee5143d5104a13c3626.zip"],
    strip_prefix = "googletest-2a02723b76eae879bafbcee5143d5104a13c3626",
)
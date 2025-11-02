"""A cc_toolchain_config rule for a local clang toolchain."""

load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
    "tool_path",
)

all_link_actions = [
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

def _impl(ctx):
    tool_paths = [
        tool_path(
            name = "gcc",  # Compiler is referenced by the name "gcc" for historic reasons.
            path = "/usr/bin/clang-18",
        ),
        tool_path(
            name = "ld",
            path = "/usr/bin/ld.lld-18",
        ),
        tool_path(
            name = "ar",
            path = "/usr/bin/llvm-ar-18",
        ),
        tool_path(
            name = "cpp",
            path = "/usr/bin/clang-18",
        ),
        tool_path(
            name = "gcov",
            path = "/usr/bin/llvm-cov-18",
        ),
        tool_path(
            name = "nm",
            path = "/usr/bin/llvm-nm-18",
        ),
        tool_path(
            name = "objdump",
            path = "/usr/bin/llvm-objdump-18",
        ),
        tool_path(
            name = "strip",
            path = "/usr/bin/llvm-strip-18",
        ),
    ]

    # 想使用 LLVM C++ 库，请使用“-lc++”而不是“-lstdc++”。
    features = [
        feature(
            name = "default_linker_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = all_link_actions,
                    flag_groups = ([
                        flag_group(
                            flags = [
                                "-lstdc++",
                            ],
                        ),
                    ]),
                ),
            ],
        ),
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        cxx_builtin_include_directories = [
            "/usr/lib/llvm-18/lib/clang/18/include",
            "/usr/include",
        ],
        toolchain_identifier = "local",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "k8",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)

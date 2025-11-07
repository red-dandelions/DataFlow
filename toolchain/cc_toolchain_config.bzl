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

lto_index_actions = [
    ACTION_NAMES.lto_index_for_executable,
    ACTION_NAMES.lto_index_for_dynamic_library,
    ACTION_NAMES.lto_index_for_nodeps_dynamic_library,
]

def _impl(ctx):
    tool_paths = [
        tool_path(
            name = "gcc",  # Compiler is referenced by the name "gcc" for historic reasons.
            path = "/usr/bin/clang-20",
        ),
        tool_path(
            name = "ld",
            path = "/usr/bin/ld.lld-20",
        ),
        tool_path(
            name = "ar",
            path = "/usr/bin/llvm-ar-20",
        ),
        tool_path(
            name = "cpp",
            path = "/usr/bin/clang-20",
        ),
        tool_path(
            name = "gcov",
            path = "/usr/bin/llvm-cov-20",
        ),
        tool_path(
            name = "nm",
            path = "/usr/bin/llvm-nm-20",
        ),
        tool_path(
            name = "objdump",
            path = "/usr/bin/llvm-objdump-20",
        ),
        tool_path(
            name = "strip",
            path = "/usr/bin/llvm-strip-20",
        ),
    ]

    thinlto_feature = feature(
        name = "thin_lto",
        enabled = False,  # 手动开启
        flag_sets = [
            flag_set(
                actions = [
                    ACTION_NAMES.c_compile,
                    ACTION_NAMES.cpp_compile,
                ] + all_link_actions + lto_index_actions,
                flag_groups = [
                    flag_group(flags = ["-fuse-ld=/usr/bin/ld.lld-20", "-Wno-unused-command-line-argument"]),
                    flag_group(flags = ["-flto=thin"]),
                    flag_group(
                        expand_if_available = "lto_indexing_bitcode_file",
                        flags = [
                            "-Xclang",
                            "-fthin-link-bitcode=%{lto_indexing_bitcode_file}",
                        ],
                    ),
                ],
            ),
            flag_set(
                actions = [ACTION_NAMES.linkstamp_compile],
                flag_groups = [
                    flag_group(flags = ["-DBUILD_LTO_TYPE=thin"]),
                ],
            ),
            flag_set(
                actions = lto_index_actions,
                flag_groups = [
                    flag_group(flags = [
                        "-flto=thin",
                        "-Wl,-plugin-opt,thinlto-index-only%{thinlto_optional_params_file}",
                        "-Wl,-plugin-opt,thinlto-emit-imports-files",
                        "-Wl,-plugin-opt,thinlto-prefix-replace=%{thinlto_prefix_replace}",
                    ]),
                    flag_group(
                        expand_if_available = "thinlto_object_suffix_replace",
                        flags = [
                            "-Wl,-plugin-opt,thinlto-object-suffix-replace=%{thinlto_object_suffix_replace}",
                        ],
                    ),
                    flag_group(
                        expand_if_available = "thinlto_merged_object_file",
                        flags = [
                            "-Wl,-plugin-opt,obj-path=%{thinlto_merged_object_file}",
                        ],
                    ),
                ],
            ),
            flag_set(
                actions = [ACTION_NAMES.lto_backend],
                flag_groups = [
                    flag_group(flags = [
                        "-c",
                        "-fthinlto-index=%{thinlto_index}",
                        "-o",
                        "%{thinlto_output_object_file}",
                        "-x",
                        "ir",
                        "%{thinlto_input_bitcode_file}",
                    ]),
                ],
            ),
        ],
    )

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

    lto_features = [
        thinlto_feature,
        feature(
            name = "supports_start_end_lib",
            enabled = True,
        ),
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        #+ (lto_features if ctx.attr.supports_start_end_lib else []),
        cxx_builtin_include_directories = [
            "/usr/lib/llvm-20/lib/clang/20/include",
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
    #attrs = {
    #    "supports_start_end_lib": attr.bool(),
    #},
    provides = [CcToolchainConfigInfo],
)

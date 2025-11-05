#!/usr/bin/bash

# 格式化所有 C++ 文件
echo $(pwd)
find DataFlow/csrc -type f \( -name "*.cc" -o -name "*.h" \) -exec clang-format-20 -i {} \;
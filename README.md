# DataFlow

DataFlow 是一个高性能的数据处理框架，专门为分布式训练系统设计。它使用 C++ 实现核心功能，并提供 Python 接口，可以无缝集成到 PyTorch 生态系统中。

## 特性

- 高性能 C++ 核心实现
- 友好的 Python 接口
- Pipeline 式数据处理流
- 支持多种数据操作
- 支持多进程数据处理
- 支持本地文件和 HDFS 文件系统
- 自动批处理(Batching)和随机打乱(Shuffling)
- 与 PyTorch 完美集成
- 支持稀疏特征和稠密特征处理
- 内置数据压缩和解压支持

## 快速开始

### 使用 Docker 开发环境

我们提供了开发用的 Docker 环境：

```bash
# 构建开发环境镜像
docker build -f dockerfiles/Dockerfile.develop -t dataflow:develop .

# 启动开发容器
docker compose up -d develop

# 进入开发容器
docker compose exec develop bash

# 启动开发容器
docker compose run --rm develop bash

# 停止所有服务
docker compose down
```

### 基本使用

```python
from DataFlow import DataBatcher
# TODO:
```

## 数据格式

### TXT 格式
支持自定义分隔符的文本格式：

```
sample_id|group_id|sparse_slot@sparse_id:weight,...|dense_slot@value,...|label|timestamp
```

字段说明：
- sample_id: 样本唯一标识
- group_id: 样本分组ID
- sparse_slot: 稀疏特征槽位，格式为 feature_name@feature_id:weight
- dense_slot: 稠密特征槽位，格式为 feature_name@value
- label: 样本标签
- timestamp: 时间戳

### 未来支持的格式
- Protobuf 格式样本 (开发中)
- 消息队列集成 (计划中)
- 特征生成(FG)支持 (计划中)

## 开发指南

### 构建系统

项目使用 Bazel 作为构建系统。

#### 1. 基础命令
```bash
# 清理构建缓存
bazel clean

# 构建指定目标
bazel build //DataFlow/csrc/core:data_pipeline
bazel build //DataFlow/csrc/data_objects:all

# 运行特定测试
bazel test //test:pybind_module_test
bazel test //test/... --test_output=all

# 构建所有目标
bazel build //...

# 运行测试
bazel test //...

# 生成编译命令数据库(用于 IDE 支持)
bazel run @hedron_compile_commands//:refresh_all

# 格式化 BUILD 文件
buildifier $(find . -name BUILD.bazel)

# 生成构建依赖图
bazel query --nohost_deps --noimplicit_deps \
    'deps(//DataFlow/csrc/...)' --output graph > build_graph.dot
dot -Tpng build_graph.dot -o build_graph.png
```

### IDE 配置

推荐使用 Visual Studio Code 进行开发，需要安装以下插件：
- Dev Containers: 容器化开发环境支持
- clangd: C++ 语言支持
- Python: Python 语言支持
- Bazel: Bazel 构建系统支持

VSCode 配置技巧：
1. 使用 vscode 进入容器中
2. 使用 Command + Shift + P (Mac) 或 Ctrl + Shift + P (Windows/Linux) 打开命令面板
3. 输入 "clangd: Restart language server" 可以重新加载 C++ 语言服务

### 代码风格

C++ 代码风格遵循 Google C++ Style Guide，使用 clang-format 进行格式化：
- 缩进使用 2 空格
- 行宽限制为 100 字符
- 使用 C++20 标准

Python 代码风格遵循 PEP 8 规范。

#### 1. C++ 代码格式化
```bash
# 格式化单个文件
clang-format -i DataFlow/csrc/core/*.cc DataFlow/csrc/core/*.h

# 格式化所有 C++ 文件
find DataFlow/csrc -type f \( -name "*.cc" -o -name "*.h" \) -exec clang-format -i {} \;

# 检查格式化问题
clang-format --dry-run -Werror DataFlow/csrc/**/*.{cc,h}
```

#### 2. Python 代码格式化
```bash
# 使用 black 格式化
black DataFlow/

# 使用 isort 整理导入
isort DataFlow/

# 使用 flake8 检查代码质量
flake8 DataFlow/
```

### 调试和分析

#### 日志系统
使用 VLOG 进行日志输出：
```cpp
VLOG(1) << "Debug information";
VLOG(2) << "Verbose debug information";
```

日志级别控制：
```bash
# 设置日志级别
export GLOG_v=2
```

#### 性能分析

##### 1. CPU 分析
```bash
# 使用 perf 记录性能数据
perf record -g ./bazel-bin/test/performance_test

sudo perf record -F 99 --call-graph dwarf ...

# 生成性能报告
perf report --stdio > report.txt

# 使用 gperftools
CPUPROFILE=/tmp/prof.out ./bazel-bin/test/performance_test
google-pprof --pdf ./bazel-bin/test/performance_test /tmp/prof.out > profile.pdf
```

##### 2. 内存分析
```bash
# 使用 Valgrind 检查内存泄漏
valgrind --leak-check=full ./bazel-bin/test/memory_test

# 使用 Massif 分析堆内存使用
valgrind --tool=massif ./bazel-bin/test/memory_test
ms_print massif.out.* > memory_profile.txt
```

##### 3. 监控指标
```bash
# 查看系统资源使用
top -p $(pgrep -f "data_pipeline_test")

# 查看进程内存使用
ps -o pid,rss,command -p $(pgrep -f "data_pipeline_test")

# 查看文件描述符使用
lsof -p $(pgrep -f "data_pipeline_test")
```

#### 开发调试技巧

##### 1. LLDB 调试
```bash
# 启动 LLDB 调试会话
lldb-18 ./bazel-bin/test/debug_test

# 常用 LLDB 命令
breakpoint set --file data_pipeline.cc --line 100  # 设置断点
breakpoint set -n functionName                     # 在函数处设置断点
run                                               # 运行程序
bt                                                # 查看调用栈(backtrace)
frame variable                                    # 打印当前帧的所有变量
p variable                                        # 打印变量值
thread list                                       # 显示所有线程
continue                                          # 继续执行
quit                                              # 退出调试

# 条件断点
breakpoint set -l 100 -c 'counter == 5'          # 当 counter 等于 5 时触发

# 监视变量
watchpoint set variable -w write global_var       # 监视变量写入
```

## 架构设计

### 核心组件

1. 数据对象 (DataObjects)
   - ByteStream: 基础字节流处理
   - InflateStream: 压缩数据流处理
   - String: 字符串处理

2. 数据管道 (DataPipelines)
   - DataReader: 数据读取器
   - DataDecompressor: 数据解压器

3. 工具类 (Utils)
   - API 导出工具
   - 通用工具函数

### 扩展性

如何扩展新的数据格式：
1. 实现新的 DataObject 类
2. 实现对应的 DataPipeline 处理器
3. 注册到 Python 绑定

## 文档

### 生成文档

项目使用 Sphinx 生成文档(开发中)：
```bash
# TODO: 添加文档生成命令
```
#### 1. Sphinx 文档
```bash
# 安装 Sphinx
pip install sphinx sphinx-rtd-theme breathe

# 初始化文档项目
sphinx-quickstart docs

# 生成 API 文档
doxygen Doxyfile
sphinx-build -b html docs/source docs/build
```

### API 参考
- 核心 API 文档
- Python 接口文档
- 配置参数说明

## 贡献指南

### 提交代码
1. Fork 仓库
2. 创建特性分支
3. 提交变更
4. 发起 Pull Request

### 测试
```bash
# 运行所有测试
bazel test //...

# 运行特定测试
bazel test //test:specific_test
```

## 许可证

本项目基于 GNU 通用公共许可证 v3.0 发布。

## 路线图
- [ ] 支持完整的数据操作
- [ ] 支持 Protobuf 格式样本
- [ ] 实现消息队列集成
- [ ] 添加特征生成(FG)支持
- [ ] 完善文档系统
- [ ] 添加更多单元测试
- [ ] 性能优化和基准测试
- [ ] 分布式训练支持增强
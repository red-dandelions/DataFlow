显卡 -> 架构 -> Compute Capability

对应的显卡和架构，需要在编译的时候指定 sm_xx

比如 Compute Capability 为 8.6 时

nvcc -arch=sm_86 xx.cu. xx.cc



调度流程：

```
CPU -> GPU Kernel Launch -> SM 调度 Block -> Thread 执行 -> 结果回 CPU
```



所有 block 到 GPU 队列里等待调度到空闲的 SM。

block 里 32 个 thread 为一个 warp.

所有的 warp 会调度到 SM 的 队列里。

SM 会有多个 warp 调度器，一次性把多个 warp 放到具体的 core 里执行。



# mac 调试 pytorch

```bash
conda create -n test python=3.10
```



```bash
conda activate test
```



```bash
git clone -b v.2.8.0 --recursive https://github.com/pytorch/pytorch.git

cd pytorch

conda install cmake ninja

pip install -r requirements.txt

# linux
export CMAKE_PREFIX_PATH="${CONDA_PREFIX:-'$(dirname $(which conda))/../'}:${CMAKE_PREFIX_PATH}"
DEBUG=1 USE_DISTRIBUTED=0 USE_MKLDNN=0 USE_CUDA=0 BUILD_TEST=0 USE_FBGEMM=0 USE_NNPACK=0 USE_QNNPACK=0 USE_XNNPACK=0 python setup.py develop

# mac
DEBUG=1 USE_DISTRIBUTED=0 USE_MKLDNN=0 USE_CUDA=0 BUILD_TEST=0 USE_FBGEMM=0 USE_NNPACK=0 USE_QNNPACK=0 USE_XNNPACK=0 python setup.py develop


```



mac:

```bash
export DYLD_LIBRARY_PATH=$(pwd)/build/lib:$DYLD_LIBRARY_PATH
export PYTHONPATH=$(pwd):$PYTHONPATH

```

linux:

```bash
export LD_LIBRARY_PATH=$(pwd)/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$(pwd):$PYTHONPATH

```



test_add.py

```bash
import torch
a = torch.tensor([1.0], requires_grad=False)
b = torch.tensor([2.0], requires_grad=False)
c = torch.add(a, b)
print(c)
```



```bash
lldb -- $(which python3) test_add.py
```



```bash
settings set target.run-args test_add.py
```



```bash
breakpoint set -n 'PyInit__C'
```

注意：lldb 需要把 _C.so 加载进来之后才会解析到所有的符号。

所以需要先在 "PyInit__C" 打个断点。然后中断之后再在之后的地方加断点调试。



```bash
run
```



```bash
step
```



```bash
next
```


# yaml-cpp 的安装及使用

yaml-cpp 是一个用于解析 yaml 格式文件的 c++ 库。

## 编译安装流程

0. 到 github 上将源码 clone 到本地（网络环境不好的，可以到 gitee 上找到镜像）
    ``` sh
    git clone https://gitee.com/mirrors/yaml-cpp.git
    ```
0. 进入目录 yaml-cpp/ 编译 CMakeLists.txt 文件
    ``` sh
    cd yaml-cpp
    mkdir build && cd build
    cmake ..
    ```
0. 编译并安装源码
    ``` sh
    make install
    ```

## 基本用法

用 cmake 编译项目时，记得在 CMakeLists.txt 里加上第三方库文件的 include 路径
``` cmake
# 默认是安装在 /usr/local/include 路径下的，如果设置了 make 的安装路径需要修改成相应的路径
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
```
``` c++
/*
 * yaml_test.cc
 */

// 简单的引入头文件就可以开始使用了
#include <yaml-cpp/yaml.h>
```

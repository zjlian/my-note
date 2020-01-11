# Ubuntu 编译安装最新版 CMake

1. 先去官网或者 github 下载最新版 CMake 的源码。
2. 随便什么方法，打开 Ubuntu 的终端，然后按下面顺序执行命令。
    ``` sh
    # 解压下载的源码
    unzip CMake-3.16.2.zip
    cd CMake-3.16.2/
    # 确保已安装 gcc 和 g++ 后，执行下一条命令
    ./bootstrap
    # 这一步在我的阿里云机器上出现了个小插曲，提示无法找到 OpenSSL，输入下面命令安装即可
    apt install libssl-dev
    # 接下来就是编译安装
    make
    make install
    ```
3. 完成后输入 ```cmake -version``` 查看安装是否成功。
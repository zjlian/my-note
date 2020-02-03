# Linux 下文件 I/O 操作笔记

终端命令可以使用 ```man [参数]``` 来查询:

常用到的终端命令有:
``` sh
# 系统只带的开发手册
man
# 基本用法, man 后直接跟要查询的函数、程序或者 shell 命令
man man
man ls
# 限定查询范围
# 1   Executable programs or shell commands
# 2   System calls (functions provided by the kernel)
# 3   Library calls (functions within program libraries)
# 4   Special files (usually found in /dev)
# 5   File formats and conventions eg /etc/passwd
# 例如 umask ,终端命令与系统库函数中都有同名的函数，可以用以下方法区分查询
# 查询终端命令
man 1 open
# 查询系统调用函数
man 2 open


# 查看目录和文件的基本信息
ls
# 查看详细信息
ls -l

# 修改目录或文件的权限等级
chmod [各种参数]

# 修改查看目录的默认权限等级
umask [权限等级]

# 查询或修改用户进程限制
ulimit [参数]

```

系统调用 api 函数都可以使用 ```man 2 [函数名]``` 来查询。

常用到的有:
``` c
// 打开一个文件，返回文件描述符
int open(const char *path, int flag, ...);

// 对已经打开的文件描述符进行写操作，返回成功写入的字节数
ssize_t write(int fd, const void *buf, size_t count);

// 对已打开的文件描述符进行读操作
ssize_t read(int fd, void *buf, size_t count);

// 设置进程创建目录或文件的默认权限级别
mode_t umask(mode_t mask);

// 偏移文件描述符的读写位置
off_t lseek(int fd, off_t offset, int whence);

// 查询或修改文件描述符的属性
int fcntl(int fd, int cmd, ... /* arg */ );
```

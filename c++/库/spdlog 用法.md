> 2021年 8月 4日
- [基本构成](#基本构成)
  - [struct log_msg](#struct-log_msg)
  - [class logger](#class-logger)
  - [class sink](#class-sink)
  - [class formatter](#class-formatter)
- [基本工作流程](#基本工作流程)
- [用法](#用法)
  - [（一）使用默认的日志器进行简单输出](#一使用默认的日志器进行简单输出)
  - [（二）使用宏输出日志](#二使用宏输出日志)
  - [（三）自定义日志格式 和 设置日志有效级别](#三自定义日志格式-和-设置日志有效级别)
  - [（四）输出日志到文件](#四输出日志到文件)
  - [（五）组装自定义的日志器](#五组装自定义的日志器)



## 基本构成
### struct log_msg
**日志消息**，用来存储日志信息的结构体，里面存了日志器名称、日志级别、创建时间、线程id和日志内容等信息。

### class logger
**日志器**，直接面向用户的日志类，调用api输出日志。

### class sink
**输出器**，负责将日志文本输出的类。这个类本身是个抽象类，spdlog基于他派生出了各种类型的输出器，例如输出到控制台的、输出到文件的。

一个logger可以拥有多个sink。也可以多个logger共享一个sink。

### class formatter
**格式化器**，负责将```log_msg```对象转换成最终要输出的字节数组。

每一个sink都拥有一个formatter。

spdlog维护了一个全局注册表，里面存储着全局共享的数据，当创建logger和sink时，默认使用的就是注册表里的formatter。

## 基本工作流程
当用户通过logger实例的info()、wram()等函数输出日志后，logger会使用fmt::format将用户传入的文本中的占位符"{}"替换成对应的参数，随后创建一个log_msg实例并传递给sink，sink输出前会调用formatter，根据指定的日志格式，对log_msg进行格式化，转换成字符数组后输出。

<br/>
<br/>

## 用法

### （一）使用默认的日志器进行简单输出
spdlog提供了默认的全局日志器，使用下面的方法直接调用就可以了。

**例子：**
```C++
spdlog::debug("Hello");
spdlog::debug("Hello {}", "world");
spdlog::info("Hello");
spdlog::info("Hello {}", "world");
// spdlog 提供了7个等级的日志级别，分别为：
// trace, debug, info, warning, error, critical, off
```

<br/>
<br/>

### （二）使用宏输出日志
用宏输出日志可以自动添加当前文件名称和代码行号等信息，方便调试。

spdlog默认情况下关闭了相关的宏，需要在头文件"spdlog.h"引入前，定义指定的宏开启该功能。使用方法如下
```c++
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include "spdlog.h" // SPDLOG_ACTIVE_LEVEL 的定义必须在这之前
```
其中``SPDLOG_LEVEL_INFO``代表最低有效等级为INFO，低于该等级的日志宏将会不生效。spdlog为每一个日志级别都定义了对应的宏，可以按需求设置。这个宏只需要在mian()函数所在的文件定义一次就可以了。
```c++
#define SPDLOG_LEVEL_TRACE 0
#define SPDLOG_LEVEL_DEBUG 1
#define SPDLOG_LEVEL_INFO 2
#define SPDLOG_LEVEL_WARN 3
#define SPDLOG_LEVEL_ERROR 4
#define SPDLOG_LEVEL_CRITICAL 5
#define SPDLOG_LEVEL_OFF 6
```
使用宏输出日志时，需要显式指定用于输出的日志器，可以使用函数``spdlog::default_logger()``获取默认日志器的智能指针，也可以使用自己创建的日志器。

**例子：**
```c++
SPDLOG_LOGGER_INFO(spdlog::default_logger(), "Hello world");
SPDLOG_LOGGER_INFO(spdlog::default_logger(), "Hello {}", "world");
```

<br/>
<br/>

### （三）自定义日志格式 和 设置日志有效级别
spdlog最终输出的日志格式是通过formatter进行格式化的，通过替换不同的formatter，即可实现对日志格式的控制。

spdlog提供了非常简易的方式配置日志格式，不需要我们手动构造formatter实例，只需要调用函数``set_pattern("..")``，并传入需要的格式化标记字符串即可。

例如，设置全局所有日志器的输出格式可以使用下面的方法。
```c++
spdlog::set_pattern("[%H:%M:%S][%n][thr %t][%s:%!():%#] %v");
```
这里摘抄一些比较常用的标记符，其他的可以去github看官方的wiki。
|符号|作用|
|-|-|
|%v|用户的日志文本|
|%t|线程id|
|%P|进程id|
|%n|日志器名称|
|%l|日志级别|
|%x|简略的日期 MM/DD/YY|
|%X|24小时制的时间，显示时分秒|
|%s|显示日志所在的文件的名称。需要使用spdlog提供的宏输出日志才有效果|
|%!|显示日志所在的函数的名称。需要使用spdlog提供的宏输出日志才有效果|
|%#|显示日志所在的行号。需要使用spdlog提供的宏输出日志才有效果|

需要注意的是``spdlog::set_pattern()``函数修改的是全局注册表里登记过的所有日志器。下一节会介绍到相关的东西。

spdlog还提供了设置有效日志级别的接口，低于指定级别的日志信息将会被忽略，不被输出。

**例子：**

```c++
// 设置全局默认日之器的输出级别
spdlog::set_level(spdlog::level::debug);
```

每一个logger和sink都是可以单独设置日志格式和日志级别的，具体做法后面介绍。

<br/>
<br/>

### （四）输出日志到文件
spdlog提供了一些工厂函数可以很方便的创建输出到文件的日志器。

**例子：**
```c++
// 最基本的输出到文件的日志器
auto logger1 = spdlog::basic_logger_mt("[logger_name]", "[file_name]");
logger1->info("文件日志");

// 多文件轮流输出的日志器
// 第三个参数是单个文件最大字节数，第四个参数是最大文件数量
// 当一个文件大小到达单个文件最大字节数后，日志器就会创建新的文件继续输出，
// 直到文件数量到达设定的最大文件数量，这时就回到最早创建的文件，清空后继续输出。
// 输出的文件名称类似: file_name.1 file_name.2 file_name.3 file_name.4 ...
auto logger2 = spdlog::rotating_logger_mt("[logger_name]", "[file_name]", 1024, 10);
logger2->info("文件日志");
```
这些工厂函数都有个"\_mt"后缀，这个后缀是 multi-thread 的缩写，代表该函数创建的日志器是多线程安全的。相反的，spdlog也提供了非线程安全的版本，只需要把后缀"\_mt"改成"\_st"即可，该版本去掉了所有的锁，用于单线程下获得更高的输出效率。

使用spdlog提供的工厂函数创建的日志器，会自动在spdlog维护的全局注册表中进行注册，使用``spdlog::get("[logger_name]")``函数，可以在程序的任何地方获取相同的日志器。
```c++
auto file_logger = spdlog::basic_logger_mt("[logger_name]", "[file_name]");
auto same_logger = spdlog::get("[logger_name]");
// file_logger 和 same_logger 是指向同一个日志器的指针
```

<br/>
<br/>

### （五）组装自定义的日志器
如果想要更加灵活的日志器，例如一个日志器按日志等级输出到终端或者不同的文件、多个日志器输出到同一个文件上、不同的文件使用不同的日志格式等等，都需要自己手动组装日志器来实现。

在命名空间``spdlog::sinks``下提供了大量预设的sink实现，可以直接使用这些预设的实现组合自己的logger。只需要引入对应的头文件即可。
```c++
#include "spdlog/sinks/*.h"
```

创建一个输出到多个文件和控制台的logger。

**例子：**
```c++
// spdlog完全使用c++11标准进行开发，logger、sink、formatter等实例对象均使用std::shared_ptr进行内存管理。
// 创建三个不同的sink，第一个是输出到控制台的，第二和第三个是分别输出到文件file1和file2的
auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
auto file1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/file1");
auto file2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/file2");
std::vector<spdlog::sink_ptr> sinks;
sinks.push_back(console);
sinks.push_back(file1);
sinks.push_back(file2);

// logger构造支持三种sink传递方式，分别是单个sink、传递容器范围、传递初始化列表
// 下面是使用容器范围的构造函数
auto mix_logger = std::make_shared<spdlog::logger>("mix", sinks.begin(), sinks.end());
// 下面是使用初始化列表的构造函数
// auto mix_logger = std::make_shared<spdlog::logger>("mix", {console, file1, file2});
// 下面是单个sink的logger
// auto console_logger = std::make_shared<spdlog::logger>("mix", console);
for (int i = 0; i < 10; i++)
{
    mix_logger->info("12345678");
}
```

创建输出到同一个文件的不同logger

**例子：**
```c++
auto file = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/file");
// 使用同一个sink创建logger
auto file_logger1 = std::make_shared<spdlog::logger>("logger1", file);
auto file_logger2 = std::make_shared<spdlog::logger>("logger2", file);
for (int i = 0; i < 10; i++)
{
    file_logger1->info("12345678");
    file_logger2->info("12345678");
}
```

每一个logger和sink的实例都提供了接口手动设置日志级别和日志格式。

**例子：**
```c++
// logger设置日志格式。
// 需要注意，对一个logger设置日志格式，实际上是对这个logger拥有的全部sink设置日志格式。
logger->set_pattern("%v");
// logger设置日志级别。
logger->set_level(spdlog::level::debug);

// sink 的和上面相同
sink->set_pattern("%v");
sink->set_level(spdlog::level::debug);
```

手动组装的日志器如果想要使用``spdlog::get("[logger_name]")``在任意地方获取，那就需要手动将创建好的日志器加入到spdlog的注册表中，方法也很简单，只需一行代码。
```c++
spdlog::register_logger(mix_logger);
// 注册完成后，在程序的任何地方调用 spdlog::get() 都能拿到全局唯一对象的引用
// spdlog::get() 的参数为日志器构造时指定的名称
auto same_mix_logger = spdlog::get("mix_logger");
```
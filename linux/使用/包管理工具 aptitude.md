# aptitude

aptitude 是 Debian 系 Linux 系统下的一个包管理软件，其功能与 apt-get 相识, 但有所改进。

常用的一些命令：
命令 | 作用
-|-
aptitude update             | 更新可用的包列表
aptitude upgrade            | 升级可用的包
aptitude dist-upgrade       | 将系统升级到新的发行版
aptitude install pkgname    | 安装包
aptitude remove pkgname     | 删除包
aptitude purge pkgname      | 删除包及其配置文件
aptitude search string      | 搜索包
aptitude show pkgname       | 显示包的详细信息
aptitude clean              | 删除下载的包文件
aptitude autoclean          | 仅删除过期的包文件


与 apt-get 的差异点：
功能 | apt-get | aptitude
-|-|-
install     | ✔ | 👍
remove      | ✔ | 👍
reinstall   | ❌| 👍
show        | ❌| 👍
search      | ❌| 👍
hold        | ❌| 👍
unhold      | ❌| 👍
source      | ✔ | ❌
build-deb   | ✔ | ❌
update      | ✔ | ✔
upgrade     | ✔ | ✔

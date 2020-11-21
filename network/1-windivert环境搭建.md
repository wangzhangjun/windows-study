# windivert抓包环境搭建
windivert是一个用户态的抓包工具。

本文记录如何使用vs搭建windivert开发环境。
**最好直接下载windivert提供好的二进制包，官方建议最好不要编译源码**
下载链接：https://reqrypt.org/windivert.html

### 1.新建vs空项目。
### 2.右键属性，在c/c++->Additional include Directories中添加到windivert的include目录。例如：
```
..\dependence\WinDivert-1.4.3-A\include
```

### 3.在Linker->General->Additional include Directories中添加WinDivert.lib的目录。例如：
```
..\dependence\WinDivert-1.4.3-A\x86_64\
```
注意选平台，是64位的。如果是32位的就选的x86那一级的目录。
### 4.在Linker->Input->Additional include Directories中添加WinDivert.lib文件。

以上完后，就可以在我们的工程中使用#include "windivert.h"的头文件了。

### 5.在Linker->Input->Additional include Directories中添加WinDivert.lib文件。

以上完后，就可以在我们的工程中使用#include "windivert.h"的头文件了。

### 4.以上配置好了编译和连接的环境，但是需要注意，dll和sys驱动在加载时，会去当前目录下寻找（与sln文件在同一级的Debug目录或者release目录）。所以还需要将下载好的dll和sys文件拷贝到Debug目录下。另外一种方法是配置到环境变量中。

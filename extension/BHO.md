# windows浏览器插件开发

### 1.概要
首先要说明的是，目前IE和Edge的插件开发方式是不一样的。
从官方文档来看，IE:
```
https://docs.microsoft.com/en-us/previous-versions/windows/internet-explorer/ie-developer/platform-apis/aa753587(v=vs.85)
```
IE上有几种不同的插件开发:
1.Shortcut menu extensions:主要是针对快捷菜单进行定制开发。

2.Toolbars:Custom toolbars complement Internet Explorer's standard toolbars.

3.Explorer Bars:Use Explorer Bars to reserve part of the browser window, either the side or the bottom of the browser window

4.Browser Helper Objects.

Edge官方文档：
```
https://docs.microsoft.com/en-us/microsoft-edge/extensions
```
Edge就和chrome的插件类似了，采用的是js,html,css等。


### 2.BHO
首先，BHO对象依托于浏览器主窗口。实际上，这意味着一旦一个浏览器窗口产生，一个新的BHO对象实例就要生成。任何 BHO对象与浏览器实例的生命周期是一致的。其次， BHO仅存在于Internet Explorer 4.0及以后版本中。

**为了钩住浏览器的事件或者自动化浏览器，BHO需要建立一个私有的基于COM的通讯通道。为此，该BHO应该实现一个称为IObjectWithSite的接口。**

事实上，通过接口IobjectWithSite， Internet Explorer 可以传递它的IUnknown 接口.

> IUnknown接口是一个普通的类或者接口，只是为了接口统一，它规定必须有QueryInterface,AddRef,Relaese方法。

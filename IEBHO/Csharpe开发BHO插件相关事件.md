# Csharpe开发BHO插件相关事件

## c#开发bho中相关的触发事件。

### 1.为什么BHO中的事件触发这么乱呢？
BHO这个东西为什么这么乱呢，比如DocumentComplete，NavigateComplete2，DownloadComplete等，这些并不是我们想象的那样，一个页面加载完就是DownloadComplete，一个页面的加载，DownloadComplete会被加载多次。这是为啥呢？不知道。
**最重要的是绑定事件后，当你刷新后，绑定的事件都不起作用了。**

### 2.NavigateComplete2会被触发多次
因为我需要给dom绑定keyup事件，在测试的时候，发现NavigateComplete2会被触发多次，这个原因在于载入多个框架时，每个框架都会引发自己的NavigateComplete2和DocumentComplete事件，判断是否为主框架（即这个tab）。使用以下代码：
```
void ieInstance_NavigateComplete2(object pDisp, ref object URL)
{
    if (pDisp == ieInstance)
    {
        //Do something.
    }
}
```
其中ieInstance为public对象，而SetSite中将其指定：
```
public InternetExplorer ieInstance;
ieInstance = (InternetExplorer)site;
```
可以认为是该IE窗口（或选项卡）对应的对象。

### 3.鉴于以上两点，要怎么绑定事件呢
充分利用刷新后，失效的特性。
* 1.定义全局事件变量
```
public HTMLElementEvents2_Event rootElementEvents = null;
```
* 2.在NavigateComplete2完成之后,绑定一次
```
  void ieInstance_NavigateComplete2(object pDisp, ref object URL)
        {
            if (pDisp == ieInstance)
            {
                  //绑定事件
            }
        }
```
* 3.在begin事件后，判断是否绑定过,如果没绑定，那可能是刷新引起的
```
if (rootElementEvents != document.documentElement as HTMLElementEvents2_Event)
{
    //重新绑定
}
```
* 4.在DownloadComplete判断是否已经绑定
可以在begin放置一个标志位，如果这个标志位被设置过了，说明已经绑定了，如果没有被绑定，那就在绑定一次
```
void ieInstance_DownloadComplete()
```

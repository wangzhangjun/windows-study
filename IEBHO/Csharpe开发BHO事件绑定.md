# Csharpe开发BHO事件绑定

## 关于BHO中如何给页面中的元素添加事件，也是研究了很久。

```
ieTab = (InternetExplorer)site;
IHTMLDocument3 document = ieTab.Document as IHTMLDocument3;
public HTMLElementEvents2_Event bodyElementEvents = null;
bodyElementEvents = document.documentElement as HTMLElementEvents2_Event;
bodyElementEvents.onclick += new HTMLElementEvents2_onclickEventHandler(events_click);
```
events_click为点击事件之后的回调函数。处理自己的逻辑。

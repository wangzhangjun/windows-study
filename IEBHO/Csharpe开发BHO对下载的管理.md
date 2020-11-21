# Csharpe开发BHO对下载的管理

### 1.BHO中对下载事件的管理
在setsite里面监听filedownload事件
```
ieInstance.FileDownload += new DWebBrowserEvents2_FileDownloadEventHandler(test_FileDownload);
```

```
test_FileDownload(bool ActiveDocument, ref bool Cancel)
{
  Cancel = true;//取消下载事件
}
```

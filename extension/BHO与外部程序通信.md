# BHO如何与外部程序通信

项目中涉及到BHO插件与别的应用程序通信，先是搜索BHO相关的socket编程，心想你一个DLL不能进行socket编程吗，最后的结论好像真是不能。Google一圈后发现，使用最多的是BHO与外部的一个exe创建的pipe进行通信，为什么会这样呢？

### 1.IE的保护模式
windows为了安全考虑，在vista中引入了一个保护模式，引入了一个新的概念：强制完整性级别。由以下四个级别组成：

* 1.系统级：由系统组件使用，通常应用程序不应使用。
* 2.高级：运行于提升至管理员权限的进程所有。
* 3.中级：运行于正常状态的进程所有。
* 4.低级：由IE及Windows Mail使用，以提供保护模式。

所以，windows在运行某个进程时， 通常会包含其完整性级别，一旦进程运行，就不可改变。这个完整性级别会对进程间通信产生以下影响：
* 1.进程所创建的任何安全对象都具有同样的完整性级别
* 2.进程不能访问比自身完整性级别更高的资源。
* 3.进程不能向比自身完整性级别更高的进程发送窗口消息。

**相关链接：**
```
https://www.codeproject.com/Articles/18866/A-Developer-s-Survival-Guide-to-IE-Protected-Mode#enableipc

https://blog.csdn.net/xiliang_pan/article/details/45398099
```

### 2.IE插件的运行级别
所以当写一个BHO框架的时候，因为BHO相当于是IE的子进程，所以BHO的运行级别很低，当BHO想和正常程序通信时，就会出现access denied.解决办法也很简单：把外部程序的访问级别降低，或者和BHO是同一个级别，就可以正常通信了。
降低一个程序的“强制完整性级别”的方法，从MSDN摘抄：
```
LPCWSTR LOW_INTEGRITY_SDDL_SACL_W = L"S:(ML;;NW;;;LW)";
bool SetObjectToLowIntegrity(
	HANDLE hObject, SE_OBJECT_TYPE type = SE_KERNEL_OBJECT)
{
	bool bRet = false;
	DWORD dwErr = ERROR_SUCCESS;
	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pSacl = NULL;
	BOOL fSaclPresent = FALSE;
	BOOL fSaclDefaulted = FALSE;

	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(
		LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL))
	{
		if (GetSecurityDescriptorSacl(
			pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
		{
			dwErr = SetSecurityInfo(
				hObject, type, LABEL_SECURITY_INFORMATION,
				NULL, NULL, NULL, pSacl);

			bRet = (ERROR_SUCCESS == dwErr);
		}

		LocalFree(pSD);
	}

	return bRet;
}
```
这个函数接收一个内核对象的句柄，对这个内核对象降低级别。

### 3.一个完整的demo，建立一个管道的内核对象，并对其降级
server.cpp
```
// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include <iostream>
#include <Sddl.h>
#include <Aclapi.h>
#define BUF_SIZE 4096
// 定义管道名 , 如果是跨网络通信 , 则在圆点处指定服务器端程序所在的主机名
#define EXAMP_PIPE   L"\\\\.\\pipe\\ReadPipe"

using namespace std;

LPCWSTR LOW_INTEGRITY_SDDL_SACL_W = L"S:(ML;;NW;;;LW)";
bool SetObjectToLowIntegrity(
	HANDLE hObject, SE_OBJECT_TYPE type = SE_KERNEL_OBJECT)
{
	bool bRet = false;
	DWORD dwErr = ERROR_SUCCESS;
	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pSacl = NULL;
	BOOL fSaclPresent = FALSE;
	BOOL fSaclDefaulted = FALSE;

	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(
		LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL))
	{
		if (GetSecurityDescriptorSacl(
			pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
		{
			dwErr = SetSecurityInfo(
				hObject, type, LABEL_SECURITY_INFORMATION,
				NULL, NULL, NULL, pSacl);

			bRet = (ERROR_SUCCESS == dwErr);
		}

		LocalFree(pSD);
	}

	return bRet;
}


int main(int argc, char *argv[])
{
	// 创建命名管道
	HANDLE hPipe = NULL;
	hPipe = CreateNamedPipe(
		EXAMP_PIPE,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE |
		PIPE_READMODE_MESSAGE |
		PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		BUF_SIZE,
		BUF_SIZE,
		0,
		NULL);
	//SetObjectToLowIntegrity(hPipe);//当把这个打开的时候，因为降级了，就不能通信了。
	cout << "server send data" << endl;
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		cout << "Create Read Pipe Error" << endl;
		return FALSE;
	}

	// 等待客户端的连接
	if (!ConnectNamedPipe(hPipe, NULL))
	{
		cout << "Connect Failed" << endl;
		return FALSE;
	}
	DWORD dwReturn = 0;
	char szBuffer[BUF_SIZE] = { 0 };

	// 向客户端发送数据
	cin >> szBuffer;
	if (!WriteFile(hPipe, szBuffer, strlen(szBuffer), &dwReturn, NULL))
	{
		cout << "Write Failed" << endl;
	}

	// 读取客户端数据
	memset(szBuffer, 0, BUF_SIZE);
	if (ReadFile(hPipe, szBuffer, BUF_SIZE, &dwReturn, NULL))
	{
		szBuffer[dwReturn] = '\0';
		cout << szBuffer << endl;
	}
	else
	{
		cout << "Read Failed" << endl;
	}

	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	system("pause");
	return 0;
}


```

client.cpp
```
// client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include <iostream>
using namespace std;

#define BUF_SIZE 4096
// 定义管道名 , 如果是跨网络通信 , 则在圆点处指定服务器端程序所在的主机名
#define EXAMP_PIPE   L"\\\\.\\pipe\\ReadPipe"


int main(int argc, char *argv[])
{
	HANDLE hPipe = NULL;
	char  szBuffer[BUF_SIZE] = { 0 };
	DWORD dwReturn = 0;

	cout << "client receive data" << endl;

	// 判断是否有可以利用的命名管道  
	if (!WaitNamedPipe(EXAMP_PIPE, NMPWAIT_USE_DEFAULT_WAIT))
	{
		cout << "No Read Pipe Accessible" << endl;
		return 0;
	}

	// 打开可用的命名管道 , 并与服务器端进程进行通信  
	hPipe = CreateFile(EXAMP_PIPE, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		cout << "Open Read Pipe Error" << endl;
		return 0;
	}

	// 读取服务端发来的数据
	if (ReadFile(hPipe, szBuffer, BUF_SIZE, &dwReturn, NULL))
	{
		szBuffer[dwReturn] = '\0';
		cout << szBuffer << endl;
	}
	else
	{
		cout << "Read Failed" << endl;
	}

	// 向服务端发送数据
	memset(szBuffer, 0, BUF_SIZE);
	cin >> szBuffer;
	if (!WriteFile(hPipe, szBuffer, strlen(szBuffer), &dwReturn, NULL))
	{
		cout << "Write Failed" << endl;
	}

	CloseHandle(hPipe);
	system("pause");
	return 0;
}


```

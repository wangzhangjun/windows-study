# windows内核链表

windows内核中大量使用了链表结构，在内核中，因为是多线程的，所以内核链表一般会和自旋锁配合使用。

### 1.定义一个自旋锁
```
static KSPIN_LOCK testLock;
```

### 2.链表的使用
* 1.定义一个链表
```
LIST_ENTRY testList;
```
* 2.初始化链表
```
InitializeListHead（testList）
```

* 3.插入链表
```
KIRQL irql; //局部变量
KeAcquireSpinLock(&testLock, &irql);
InsertTailList(&testList, &context->entry);//在context这个结构体中包含LIST_ENTRY entry;
KeReleaseSpinLock(&testLock, irql);
```

* 4.删除链表
```
KeAcquireSpinLock(&testLock, &irql);
RemoveEntryList(&context->entry);
KeReleaseSpinLock(&testLock, irql);
```

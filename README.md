# MemLeak

## 1思路

借助自定义的同名同参`malloc/free`函数，从而截取程序调用的过程，在真正调用系统`malloc`后（先申请内存，再记录地址）/ `free`前（先查询地址，再`free`），记录用户申请和释放，从而实现heap使用情况监测。

```
此处malloc/free指代自定义的函数
系统的相应函数地址存到了sys_malloc和sys_free两个指针中。


用户调用malloc/free，使用的是自定义的malloc/free
        |
        |
---------------------

调用sys_malloc和sys_free,同时记录申请和释放痕迹，使用的是双向链表及其头插法,创建新ListNode使用sys_malloc和sys_free。

head-->ptr3-->ptr2-->ptr1-->ptr0-->nullptr
        |
        |
---------------------

系统调用

```

## 2细节
### 使用全局变量
使用全局变量可以保证在第一次用户层面的malloc调用前，已经完成初始化操作，在其生命期内可以记录用户的内存申请释放操作。

### 用户申请和MemLeak类申请heap
MemLeak类拥有`RAII`（资源获取即初始化，借助对象构造函数和析构函数来管理资源）特性，其内部也使用了heap的空间，但是没有去调用自定义的`malloc/free`,而是通过函数地址sys_malloc和sys_free调用系统对应函数,从而避免递归：自己调用malloc,需要再调用malloc申请节点,就递归下去了，所以MemLeak类自身的heap只能通过函数地址调用。

### 线程安全和原子操作

在多线程下，插入（影响头节点）和删除（影响被删节点和其前躯）操作有可能产生竞争，所以在插入和删除操作中加入互斥锁,来保证操作安全性。

使用`atomic<int> m,f;`来记录malloc和free的调用次数。

## 3不足
只能追踪申请和释放是否配对，检测释放的地址是不是从malloc申请来的，以及程序结束时是否全部释放申请的空间，但是不能得到对应操作的文件名和行号。

## 4检测内存泄漏的目标
行号，文件名，进程id，线程id，调用堆栈，已使用大小。

## 5其它做法
### 重载operator new
重载`operator new`为`operator new(文件名,行号,size,其它参数)`,文件名可以使用宏`__FILE__`，行号`__LINE__`。
缺点：不能检测到malloc和free导致的内存泄漏
## 6现有工具，待探索
### vs的`<crtdbg.h>`
### valgrind
### google-perftools中的tcmalloc
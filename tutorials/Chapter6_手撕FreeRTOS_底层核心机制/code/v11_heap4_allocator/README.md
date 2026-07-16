# v11_heap4_allocator

目标：实现简化版 heap allocator，展示空闲链表、块分裂和相邻空闲块合并。

对照源码：`portable/MemMang/heap_4.c`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

这个 demo 只讲 `heap_4` 的心智模型：申请会切分空闲块，释放会让块重新变空闲，相邻空闲块可以合并。中间的 `malloc 48 fails` 用来提醒读者区分总空闲和最大连续块；最后的 `malloc 120 fails` 则是总量本身不够。它不是完整内存分配器。

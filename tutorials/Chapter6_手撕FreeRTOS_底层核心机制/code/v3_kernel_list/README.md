# v3_kernel_list

目标：实现最小内核链表，并让 TCB 嵌入 list item。

对照源码：`list.c`、`include/list.h`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

这个 demo 故意不讲通用链表技巧，只讲任务位置：在 `ready` 里表示可以竞争 CPU，在 `delayed` 里表示等时间，在 `event_wait` 里表示等消息或同步对象。

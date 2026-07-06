# v9_queue

目标：实现最小阻塞队列，展示队列如何连接数据缓冲和任务等待。

对照源码：`queue.c`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

这个 demo 把队列拆成两件事：环形缓冲保存数据，等待发送/等待接收记录任务关系。输出要同时看 `count` 和唤醒谁。

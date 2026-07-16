# v5_priority_scheduler

目标：实现多优先级 ready list、最高优先级选择和同优先级 round robin。

对照源码：`tasks.c:vTaskSwitchContext()`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

读输出时注意两件事：`LED` 和 `SENSOR` 同优先级时轮转；`COMM` 从 blocked 变 ready 后，优先级更高，会持续被选中。这个 demo 只讲“选择谁”，不讲上下文切换。

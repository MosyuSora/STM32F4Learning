# v1_task_stack

目标：用 C 数组模拟任务栈，解释任务入口、参数和独立栈为什么是任务模型的起点。

对照源码：`portable/GCC/ARM_CM4F/port.c:pxPortInitialiseStack()`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

读这个 demo 时只看三件事：每个任务有自己的 `stack[]`，`top_of_stack` 指向可恢复现场，入口函数和参数被预先放进“初始现场”。这不是 Cortex-M 真实栈帧，只是帮助理解 `pxPortInitialiseStack()` 的教学模型。

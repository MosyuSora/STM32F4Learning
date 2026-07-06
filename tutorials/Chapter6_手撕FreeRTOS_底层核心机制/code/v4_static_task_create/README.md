# v4_static_task_create

目标：实现教学版 `tiny_task_create_static()`。

对照源码：`xTaskCreateStatic()`、`prvInitialiseNewTask()`、`prvAddNewTaskToReadyList()`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

读这个 demo 时按顺序看：调用者提供静态栈和 TCB，创建函数填入入口、参数、优先级和栈顶，最后把任务放入 ready list。输出里的 “ready but not necessarily running” 是本节的核心结论。

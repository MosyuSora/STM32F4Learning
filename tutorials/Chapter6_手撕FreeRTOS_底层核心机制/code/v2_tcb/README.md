# v2_tcb

目标：定义最小 TCB，把任务入口、参数、栈顶、优先级和任务名收束成一个对象。

对照源码：`tasks.c:TCB_t`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

读这个 demo 时不要背字段顺序，而要看字段服务的动作：任务名帮助调试，`top_of_stack` 连接上下文恢复，`priority` 服务调度选择，`state_item` 让任务能挂进 ready、delay 或 event list。

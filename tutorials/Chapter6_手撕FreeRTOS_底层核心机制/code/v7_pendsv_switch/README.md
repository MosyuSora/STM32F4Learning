# v7_pendsv_switch

目标：模拟 PendSV 保存旧任务、调用调度器、恢复新任务的过程。

对照源码：`portable/GCC/ARM_CM4F/port.c:xPortPendSVHandler()`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

这是教学模型，不是真实汇编。真实 PendSV 会保存/恢复寄存器并操作 PSP；这里用 `process_stack_pointer` 和 `pxCurrentTCB` 把“保存当前、换当前指针、恢复下一个”三步讲清楚。

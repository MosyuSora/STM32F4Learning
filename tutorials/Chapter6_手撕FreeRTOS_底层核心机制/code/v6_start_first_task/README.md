# v6_start_first_task

目标：用教学版控制流解释 SVC 如何启动第一个任务。

对照源码：`vTaskStartScheduler()`、`xPortStartScheduler()`、`vPortSVCHandler()`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

这是教学模型，不是 Cortex-M 真实异常返回流程。它只表达三件事：`main()` 完成任务创建，调度器选择第一个 ready task，启动入口恢复该任务的上下文后，普通主循环不再拥有 CPU。

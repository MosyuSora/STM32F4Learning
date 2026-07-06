# v8_delay_blocked_list

目标：实现教学版 delay、blocked list 和 tick 唤醒。

对照源码：`vTaskDelay()`、`prvAddCurrentTaskToDelayedList()`、`xTaskIncrementTick()`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

读这个 demo 时看状态变化：调用 delay 后任务从 ready 进入 delayed；Tick 到期后任务回到 ready；回到 ready 不等于马上运行，还要等调度器选择。

# v0_bare_loop

目标：用最小裸机主循环展示 LED、传感器、日志和通信工作挤在一起时，慢日志如何拖住心跳和采样节奏。

对照正文：Chapter6 `## 1 从裸机协作问题走向 RTOS`。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

这个 demo 不是 FreeRTOS 代码，而是 RTOS 出现前的工程痛点。读输出时看 `LOG flush chunk` 后面的时间跳跃：它让 `LED` 和 `SENSOR` 的节奏被推迟。

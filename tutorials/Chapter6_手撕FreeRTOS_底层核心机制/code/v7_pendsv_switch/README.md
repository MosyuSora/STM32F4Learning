# v7_pendsv_switch

目标：模拟 PendSV 保存旧任务、调用调度器、恢复新任务的过程。

对照源码：`portable/GCC/ARM_CM4F/port.c:xPortPendSVHandler()`。

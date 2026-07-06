# v10_mutex_inheritance

目标：实现最小 mutex holder 和优先级继承模型。

对照源码：`queue.c` 中 mutex take/give 及 priority inheritance 路径。

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1
```

这个 demo 的关键不是互斥锁 API，而是现象链：低优先级任务持锁，高优先级任务等待，中优先级任务可能插队，继承让持锁者临时提高优先级并尽快释放锁。

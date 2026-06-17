# 03 GPIO 引脚操作原理

> **一句话总结**：GPIO 通过三个寄存器控制（方向/数据/状态），操作时必须用位运算只改目标引脚，不能影响其他引脚。

**对应 PDF**：第五篇 第3章（第308页）

---

## 前置知识

- 二进制位运算（与、或、非）
- 寄存器的概念（CPU 通过读写特定内存地址来控制硬件）

---

## 1. GPIO 模块结构

```mermaid
flowchart TD
    subgraph “GPIO 控制器”
        GDIR[“GDIR 寄存器<br>方向控制<br>1=输出 / 0=输入”]
        DR[“DR 寄存器<br>输出数据<br>写1=高电平 / 写0=低电平”]
        PSR[“PSR 寄存器<br>输入状态<br>读引脚实时电平”]
    end
    GDIR --> DIR[“引脚方向”]
    DR --> VOL[“引脚输出电压”]
    PIN[“引脚实时状态”] --> PSR
```

> **类比**：把 GPIO 想象成 32 个独立开关：
> - **GDIR**：决定每个开关是"控制别人"（输出）还是"感应外界"（输入）
> - **DR**：输出模式下，决定开关拨到高还是低
> - **PSR**：输入模式下，读取外部实际状态（不受 DR 影响）

---

## 2. 为什么必须用位运算？

一个 GPIO 控制器通常管理 32 个引脚，共用同一个 32-bit 寄存器。

**错误做法**：直接赋值

```c
GDIR = 0x00000008;   // 危险！会把其他31个引脚全清零
```

**正确做法**：位运算只改第 n 位

```c
/* 设置 bit n 为 1（让引脚 n 变为输出模式） */
unsigned int val = GDIR;     // 先读出当前值
val = val | (1 << n);        // 只把第 n 位置 1，其余不变
GDIR = val;                  // 写回

/* 清除 bit n 为 0（让引脚 n 变为输入模式） */
unsigned int val = GDIR;
val = val & ~(1 << n);       // ~(1<<n) 只有第 n 位是0，其余全是1
GDIR = val;
```

---

## 3. 三种寄存器操作详解

### 3.1 设置引脚方向（GDIR）

```c
/* 引脚 n 设为输出 */
static inline void gpio_set_output(volatile unsigned int *GDIR, int n)
{
    *GDIR |= (1 << n);    // 第 n 位置1 = 输出
}

/* 引脚 n 设为输入 */
static inline void gpio_set_input(volatile unsigned int *GDIR, int n)
{
    *GDIR &= ~(1 << n);   // 第 n 位清0 = 输入
}
```

### 3.2 输出高/低电平（DR）

```c
/* 引脚 n 输出高电平 */
static inline void gpio_set_high(volatile unsigned int *DR, int n)
{
    *DR |= (1 << n);
}

/* 引脚 n 输出低电平 */
static inline void gpio_set_low(volatile unsigned int *DR, int n)
{
    *DR &= ~(1 << n);
}
```

### 3.3 读取输入状态（PSR）

```c
/* 读取引脚 n 的当前电平（0 或 1） */
static inline int gpio_get_value(volatile unsigned int *PSR, int n)
{
    return (*PSR >> n) & 1;   // 右移到第0位，取最低位
}
```

---

## 4. 位操作可视化

以第 3 位（bit 3）为例：

```
操作前  GDIR = 0000 0000 0000 0000 0000 0000 0000 0101
                                                    ↑↑
                                                   bit1 bit0 已是输出

置 bit3 为1：
  mask = 1 << 3 = 0000 0000 0000 0000 0000 0000 0000 1000
  GDIR | mask   = 0000 0000 0000 0000 0000 0000 0000 1101
                                                        ↑
                                                     bit3 现在也是输出

清 bit3 为0：
  ~mask         = 1111 1111 1111 1111 1111 1111 1111 0111
  GDIR & ~mask  = 0000 0000 0000 0000 0000 0000 0000 0101
                                                     bit3 回到输入，其余不变
```

---

## 5. Set-and-Clear 协议（部分芯片）

某些芯片（如 STM32）用三个独立寄存器替代读-改-写操作：

| 寄存器 | 作用 |
|--------|------|
| DATA   | 当前输出值 |
| SET    | 写1的位变高，写0无效 |
| CLEAR  | 写1的位变低，写0无效 |

好处：原子操作，不用先读再写，多核环境也安全。

---

## 小结

GPIO 三寄存器：**GDIR**（方向）、**DR**（输出）、**PSR**（输入）。操作规则：**先读-再改-再写**，用位运算（`|=` 置位，`&= ~` 清位，`>> & 1` 读位），绝对不能直接赋值整个寄存器。

内核 GPIO 子系统源码参考：[source/kernel/drivers/gpio/](../source/kernel/drivers/gpio/)

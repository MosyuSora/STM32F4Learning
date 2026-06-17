# ============================================================
#  PCA9555 LED 驱动 + 设备树 统一 Makefile
#
#  目标:
#    make            编译 pca_led_drv.ko + pca_led.dtb
#    make deploy     部署 ko + dtb 到开发板
#    make insmod     开发板上加载 ko
#    make test       快速测试 LED
#    make check      查看板上状态
#    make clean      清理
# ============================================================

# ── 路径 ──
KERNEL_DIR     := /home/mosyu/linux_proj/source/kernel
DTS_DIR        := $(KERNEL_DIR)/arch/arm/boot/dts
INCLUDE_DIR    := $(KERNEL_DIR)/include
WORK_DIR       := $(shell pwd)

# ── 交叉编译 ──
ARCH           := arm
CROSS_COMPILE  := arm-linux-gnueabihf-
MAKE_ARCH      := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

# ── 内核模块目标（必须在这里声明 obj-m，内核构建系统才能识别） ──
obj-m          := pca_led_drv.o

# ── 强制 ARMv7 架构（用于 gcc 9+ 搭配旧内核 headers 的场景） ──
ccflags-y      := -march=armv7-a -mtune=cortex-a7

# ── 文件名 ──
MODULE_NAME    := pca_led_drv
DTS_NAME       := pca_led
DTS_FILE       := $(DTS_NAME).dts
DTB_FILE       := $(DTS_NAME).dtb
BOARD_DTB      := 100ask_imx6ull-14x14.dtb   # 开发板上加载的名字

# ── 开发板 ──
BOARD_IP       := 192.168.0.172
BOARD_USER     := root
BOARD_PASS     := 1
SSH            := sshpass -p '$(BOARD_PASS)' ssh -o StrictHostKeyChecking=no $(BOARD_USER)@$(BOARD_IP)
SCP            := sshpass -p '$(BOARD_PASS)' scp

# ── 工具 ──
CPP            := cpp
DTC            := dtc

# ============================================================
.PHONY: all deploy insmod rmmod test check clean dtb ko

all: ko dtb
	@echo "=== BUILD DONE ==="
	@ls -lh $(MODULE_NAME).ko $(DTB_FILE)

# ── 编译内核模块 ──
ko:
	@echo "=== Building $(MODULE_NAME).ko ==="
	$(MAKE) -C $(KERNEL_DIR) $(MAKE_ARCH) M=$(WORK_DIR) modules

# ── 编译设备树 ──
dtb:
	@echo "=== Building $(DTB_FILE) ==="
	$(CPP) -nostdinc -I $(INCLUDE_DIR) -I $(DTS_DIR) \
		-undef -x assembler-with-cpp $(DTS_FILE) | \
	$(DTC) -I dts -O dtb -o $(DTB_FILE) -

# ── 验证 dtb ──
verify: dtb
	@$(DTC) -I dtb -O dts $(DTB_FILE) 2>/dev/null | \
	 grep -A5 "mosyu,pca-led\|nxp,pca9555" || echo "WARNING: nodes not found"

# ── 部署 ko + dtb ──
deploy: all
	@echo "=== Deploying to $(BOARD_IP) ==="
	$(SCP) $(MODULE_NAME).ko $(BOARD_USER)@$(BOARD_IP):/root/
	$(SCP) $(DTB_FILE) $(BOARD_USER)@$(BOARD_IP):/boot/$(BOARD_DTB)
	@echo "=== Done. Run: make reboot && make insmod ==="

# ── 重启开发板 ──
reboot:
	$(SSH) "reboot" 2>/dev/null || true
	@echo "Waiting 25s for board to come back..."
	@sleep 25
	@$(SSH) "echo READY" && echo "=== Board is up ==="

# ── 加载模块 ──
insmod:
	$(SSH) "insmod /root/$(MODULE_NAME).ko"
	@sleep 1
	@$(SSH) "ls -la /dev/pca_led && echo '=== DEVICE READY ==='"

# ── 卸载模块 ──
rmmod:
	$(SSH) "rmmod $(MODULE_NAME) 2>/dev/null; echo '=== UNLOADED ==='"

# ── 快速测试 ──
test:
	$(SSH) "echo 1 > /dev/pca_led && echo 'ON ' && sleep 0.5 && echo 0 > /dev/pca_led && echo 'OFF'"

# ── 状态检查 ──
check:
	@echo "=== module ==="
	$(SSH) "lsmod | grep pca_led || echo 'not loaded'"
	@echo "=== device ==="
	$(SSH) "ls -la /dev/pca_led 2>/dev/null || echo 'not found'"
	@echo "=== PCA9555 ==="
	$(SSH) "cat /sys/class/gpio/gpiochip496/label 2>/dev/null || echo 'not found'"
	@echo "=== dmesg (last 5) ==="
	$(SSH) "dmesg | tail -5"

# ── 清理 ──
clean:
	$(MAKE) -C $(KERNEL_DIR) $(MAKE_ARCH) M=$(WORK_DIR) clean
	rm -f $(DTB_FILE) *.o .*.cmd *.mod.c *.order *.symvers

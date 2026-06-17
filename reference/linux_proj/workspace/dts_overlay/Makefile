# ============================================================
#  IMX6ULL 设备树编译 Makefile
#
#  用法:
#    make            编译 dtb
#    make deploy     编译并部署到开发板 (scp)
#    make verify     编译并反编译检查 PCA9555 节点
#    make clean      清理
# ============================================================

# --- 路径配置 ---
KERNEL_SRC    := /home/mosyu/linux_proj/source/kernel
DTS_DIR       := $(KERNEL_SRC)/arch/arm/boot/dts
INCLUDE_DIR   := $(KERNEL_SRC)/include

# --- 文件名 ---
DTS_NAME      := 100ask_imx6ull-14x14-mosyu
DTS_FILE      := $(DTS_NAME).dts
DTB_FILE      := $(DTS_NAME).dtb
BOARD_DTB     := 100ask_imx6ull-14x14.dtb       # 开发板上实际加载的文件名

# --- 开发板 ---
BOARD_IP      := 192.168.0.172
BOARD_USER    := root
BOARD_PASS    := 1
SSH_CMD       := sshpass -p '$(BOARD_PASS)' ssh -o StrictHostKeyChecking=no $(BOARD_USER)@$(BOARD_IP)
SCP_CMD       := sshpass -p '$(BOARD_PASS)' scp

# --- 工具 ---
CPP           := cpp
DTC           := dtc

CPP_FLAGS     := -nostdinc \
                 -I $(INCLUDE_DIR) \
                 -I $(DTS_DIR) \
                 -undef \
                 -x assembler-with-cpp

# ============================================================
#  编译目标
# ============================================================

.PHONY: all deploy verify clean reboot

all: $(DTB_FILE)
	@echo "=== DTB compiled: $(DTB_FILE) ==="
	@ls -lh $(DTB_FILE)

$(DTB_FILE): $(DTS_FILE)
	@echo "CPP (preprocessing)..."
	$(CPP) $(CPP_FLAGS) $(DTS_FILE) | \
	$(DTC) -I dts -O dtb -o $@ -

# --- 部署到开发板 ---
deploy: $(DTB_FILE)
	@echo "Deploying to $(BOARD_IP):/boot/$(BOARD_DTB) ..."
	$(SCP_CMD) $(DTB_FILE) $(BOARD_USER)@$(BOARD_IP):/boot/$(BOARD_DTB)
	@echo "=== Done. Reboot board to take effect:  make reboot ==="

# --- 反编译验证 ---
verify: $(DTB_FILE)
	@$(DTC) -I dtb -O dts $(DTB_FILE) 2>/dev/null | \
	 grep -A5 "pca9555\|nxp,pca9555" || echo "WARNING: pca9555 NOT FOUND"
	@ls -lh $(DTB_FILE)

# --- 重启开发板 ---
reboot:
	@echo "Rebooting board..."
	$(SSH_CMD) "reboot" 2>/dev/null || true

# --- 检查开发板上 PCA9555 是否在线 ---
check:
	$(SSH_CMD) "cat /sys/class/gpio/gpiochip496/label 2>/dev/null && echo 'PCA9555 ONLINE' || echo 'PCA9555 OFFLINE'"

# --- 清理 ---
clean:
	rm -f $(DTB_FILE) *.preprocessed

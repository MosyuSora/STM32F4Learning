TARGET := ASM_LED

TOOLROOT := $(USERPROFILE)/AppData/Local/stm32cube/bundles/gnu-tools-for-stm32/14.3.1+st.2/bin
PROGROOT := $(USERPROFILE)/AppData/Local/stm32cube/bundles/programmer/2.22.0+st.1/bin

CC := $(TOOLROOT)/arm-none-eabi-gcc.exe
OBJCOPY := $(TOOLROOT)/arm-none-eabi-objcopy.exe
SIZE := $(TOOLROOT)/arm-none-eabi-size.exe
PROGRAMMER := $(PROGROOT)/STM32_Programmer_CLI.exe

CPU_FLAGS := -mcpu=cortex-m4 -mthumb
ASFLAGS := $(CPU_FLAGS)
LDFLAGS := $(CPU_FLAGS) -nostdlib -T STM32F407XX_FLASH.ld -Wl,-Map=$(TARGET).map -Wl,--gc-sections -Wl,--print-memory-usage
FLASH_ADDR := 0x08000000

.PHONY: all flash rebuild clean

all: $(TARGET).elf $(TARGET).bin

$(TARGET).elf: led.s STM32F407XX_FLASH.ld
	$(CC) $(ASFLAGS) $(LDFLAGS) $< -o $@
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	$(PROGRAMMER) -c port=SWD -w $(TARGET).bin $(FLASH_ADDR) -v -rst

rebuild: clean all

clean:
	powershell -NoProfile -Command "Remove-Item -Force -ErrorAction SilentlyContinue '$(TARGET).elf','$(TARGET).bin','$(TARGET).map'"
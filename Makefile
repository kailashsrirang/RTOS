TARGET = rtos
TOOLCHAIN = arm-none-eabi

CC      = $(TOOLCHAIN)-gcc
OBJCOPY = $(TOOLCHAIN)-objcopy
SIZE    = $(TOOLCHAIN)-size

BUILD_DIR = build

# CPU flags for STM32F407: Cortex-M4 with single-precision FPU
CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

# Include paths
INCLUDES = \
	-Ikernel/include \
	-Idrivers/stm32f4/include \
	-Ilib/include

WARNING_FLAGS = \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wconversion \
	-Wsign-conversion \
	-Wshadow \
	-Wundef \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Werror

# Compiler flags
CFLAGS = $(CPU_FLAGS)
CFLAGS += -std=c11
CFLAGS += $(WARNING_FLAGS)
CFLAGS += -O0 -g
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(INCLUDES)

# Linker flags
LDFLAGS = $(CPU_FLAGS)
LDFLAGS += -T bsp/stm32f407g-disc1/linker/stm32f407.ld
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map
LDFLAGS += -nostdlib

# Source files
C_SRCS = \
	bsp/stm32f407g-disc1/startup/startup.c \
	$(wildcard kernel/src/*.c) \
	$(wildcard drivers/stm32f4/src/*.c) \
	$(wildcard lib/src/*.c) \
	app/demo/main.c

ASM_SRCS = \
	$(wildcard kernel/arch/arm/cortex-m4/*.s)

# Object files
C_OBJS   = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRCS))
ASM_OBJS = $(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SRCS))
OBJS     = $(C_OBJS) $(ASM_OBJS)

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin
	$(SIZE) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

flash: $(BUILD_DIR)/$(TARGET).elf
	openocd -f interface/stlink.cfg \
	        -f target/stm32f4x.cfg \
	        -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

clean:
	rm -rf $(BUILD_DIR)
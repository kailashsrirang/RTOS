TARGET = rtos
TOOLCHAIN = arm-none-eabi

CC      = $(TOOLCHAIN)-gcc
OBJCOPY = $(TOOLCHAIN)-objcopy
SIZE    = $(TOOLCHAIN)-size

# CPU flags for Cortex-M4 with FPU
CFLAGS  = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -Wall -Wextra -O0 -g
CFLAGS += -ffreestanding -nostdlib

LDFLAGS = -T stm32f407.ld -Wl,--gc-sections

C_SRCS = Core/startup.c \
         Core/mutex.c \
         Core/uart.c \
         Core/queue.c \
         Core/semaphore.c \
         Core/systick.c \
         Core/os_kernel.c \
         Core/main.c \
		Core/mem.c

ASM_SRCS = Core/os_context.s

OBJS = $(C_SRCS:.c=.o) $(ASM_SRCS:.s=.o)

all: $(TARGET).elf
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

flash: $(TARGET).elf
	openocd -f interface/stlink.cfg \
	        -f target/stm32f4x.cfg \
	        -c "program $(TARGET).elf verify reset exit"

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin
CC=gcc
CFLAGS=-Wall -Wextra -std=c11
TARGET=demo
SRCS=main.c led.c platform_pc.c
OBJS=$(SRCS:.c=.o)
all:$(TARGET)
$(TARGET):$(OBJS);$(CC) $(CFLAGS) -o $@ $^
%.o:%.c;$(CC) $(CFLAGS) -c -o $@ $<
run:$(TARGET);./$(TARGET)
clean:;rm -f $(TARGET) $(OBJS)
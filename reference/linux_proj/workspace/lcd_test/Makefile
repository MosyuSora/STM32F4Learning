# Makefile for lcd_test
CROSS_COMPILE ?= arm-linux-gnueabihf-
CC      = $(CROSS_COMPILE)gcc
TARGET  = lcd_test

all:
	$(CC) lcd_test.c -o $(TARGET) -Wall -O2 -static

clean:
	rm -f $(TARGET)

push: all
	sshpass -p '1' scp $(TARGET) root@192.168.0.172:/usr/bin/
	sshpass -p '1' scp frame.raw root@192.168.0.172:/usr/bin/
	sshpass -p '1' ssh root@192.168.0.172 "chmod +x /usr/bin/$(TARGET)"

run: push
	sshpass -p '1' ssh root@192.168.0.172 "killall $(TARGET) 2>/dev/null; sleep 0.3; /usr/bin/$(TARGET) &"

CROSS=

all: exe

exe:dnw.c
	$(CROSS)gcc -Wall -o dnw dnw.c libusb-1.0.a -lpthread -static
cpy:
	cp dnw /usr/local/bin -f
clean:
	@rm -f dnw *.o *~

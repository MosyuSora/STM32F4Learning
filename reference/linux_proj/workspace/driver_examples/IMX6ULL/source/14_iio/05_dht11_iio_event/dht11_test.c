#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/types.h>
#include <linux/iio/events.h>

int main(int argc, char **argv)
{
	int fd;
	int ret;
	int event_fd;
	struct iio_event_data event;
	
	if (argc != 2)
	{
		printf("Usage: %s /dev/iio\\:deviceX\n", argv[0]);
		return 0;
	}

	fd = open(argv[1], O_RDWR);  // /dev/iio:device2
	if (fd < 0) {
		printf("can not open %s\n", argv[1]);
		return -1;
	}

	ret = ioctl(fd, IIO_GET_EVENT_FD_IOCTL, &event_fd);
	if (ret < 0) {
		printf("can not get event fd\n");
		return -1;
	}

	while (1)
	{
		ret = read(event_fd, &event, sizeof(event));
		if (ret == sizeof(event))
		{
			printf("Get event: 0x%lx\n", event.id);
		}
	}

	return 0;
}


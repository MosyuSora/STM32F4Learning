#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define MAP_SIZE 4096

/* ./uio_led_test /dev/uio0 on
 * ./uio_led_test /dev/uio0 off
 */
int main(int argc, char *argv[]) {
    int fd;
    void *map_base1, *map_base2;
    off_t offset1 = 0*4096, offset2 = 1*4096; 
    int data;
    unsigned int *reg;

    if (argc != 3) {
        printf("Usage: %s <dev> <on|off>\n", argv[0]);
        printf("eg. ./uio_led_test /dev/uio0 on\n");
        return EXIT_FAILURE;
    }

    // 打开UIO设备文件
    fd = open(argv[1], O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("Failed to open UIO device");
        return EXIT_FAILURE;
    }

    // 映射第一段UIO设备的内存
    map_base1 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset1);
    if (map_base1 == (void *) -1) {
        perror("Failed to mmap first segment of UIO device");
        close(fd);
        return EXIT_FAILURE;
    }

    // 映射第二段UIO设备的内存
    map_base2 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset2);
    if (map_base2 == (void *) -1) {
        perror("Failed to mmap second segment of UIO device");
        munmap(map_base1, MAP_SIZE);
        close(fd);
        return EXIT_FAILURE;
    }

    
	/* enable gpio5
	 * configure gpio5_io3 as gpio
	 * configure gpio5_io3 as output 
	 */
    reg = (unsigned int *)(map_base1 + 0x14);
	*reg &= ~0xf;
	*reg |= 0x5;

    //	configure gpio5_io3 as output 
    reg = (unsigned int *)(map_base2 + 4);
    *reg |= (1<<3);

    reg = (unsigned int *)(map_base2 + 0);

    if (!strcmp(argv[2], "on")) {
        *reg &= ~(1<<3);
    } else {
        *reg |= (1<<3);
    }
    

    // 解除内存映射
    if (munmap(map_base1, MAP_SIZE) == -1) {
        perror("Failed to unmap first segment of UIO device");
    }
    if (munmap(map_base2, MAP_SIZE) == -1) {
        perror("Failed to unmap second segment of UIO device");
    }

    // 关闭设备文件
    close(fd);

    return EXIT_SUCCESS;
}
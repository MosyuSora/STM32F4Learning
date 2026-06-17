/**
 * lcd_test.c  —  LCD 图片显示程序
 * 从 frame.raw 读取 1024x600x4 的 BGR 像素数据，刷到 /dev/fb0
 * 交叉编译: make
 *
 * 生成 frame.raw: python3 convert.py test.png
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <signal.h>

#define FB_DEV      "/dev/fb0"
#define WIDTH       1024
#define HEIGHT      600

static int fb_fd = -1;
static unsigned int *fb_mem = NULL;
static unsigned int  fb_size;

/* 整屏填充 */
static void fill_screen(unsigned char r, unsigned char g, unsigned char b) {
    unsigned int pixel = (r << 16) | (g << 8) | b;
    for (int i = 0; i < WIDTH * HEIGHT; i++)
        fb_mem[i] = pixel;
}

static void cleanup(void) {
    if (fb_mem) munmap(fb_mem, fb_size);
    if (fb_fd >= 0) close(fb_fd);
}

static void sig_handler(int sig) {
    /* 退出前恢复黑色 */
    fill_screen(0, 0, 0);
    cleanup();
    exit(0);
}

int main(void) {
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;

    signal(SIGINT,  sig_handler);
    signal(SIGTERM, sig_handler);

    /* 1. 打开 framebuffer */
    fb_fd = open(FB_DEV, O_RDWR);
    if (fb_fd < 0) { perror("open " FB_DEV); return 1; }

    /* 2. 获取参数 */
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &var);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix);
    printf("fb: %dx%d, bpp=%d, line_len=%d\n",
           var.xres, var.yres, var.bits_per_pixel, fix.line_length);

    /* 3. mmap 映射显存 */
    fb_size = fix.smem_len;
    fb_mem = mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_mem == MAP_FAILED) { perror("mmap"); close(fb_fd); return 1; }

    /* ===== 加载图片并显示 ===== */

    /* 先清黑屏 */
    fill_screen(0, 0, 0);

    /* 打开 frame.raw 读取原始像素 (1024x600x4 BGRx) */
    int img_fd = open("/usr/bin/frame.raw", O_RDONLY);
    if (img_fd < 0) {
        /* 没有图片：画个默认的色块 */
        printf("frame.raw not found, drawing default pattern.\n");
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                unsigned char r = (unsigned char)(x * 255 / WIDTH);
                unsigned char g = (unsigned char)(y * 255 / HEIGHT);
                unsigned char b = 128;
                fb_mem[y * WIDTH + x] = (r << 16) | (g << 8) | b;
            }
        }
    } else {
        /* 直接把 raw 文件映射到显存 */
        unsigned char buf[4096];
        ssize_t n;
        int off = 0;
        while ((n = read(img_fd, buf, sizeof(buf))) > 0) {
            /* 每4字节一个像素 直接拷贝到显存 */
            for (ssize_t i = 0; i < n; i += 4)
                fb_mem[off++] = *(unsigned int *)(buf + i);
        }
        close(img_fd);
        printf("Image loaded from /usr/bin/frame.raw\n");
    }

    printf("LCD test pattern on screen. Press Ctrl+C to exit.\n");

    /* 阻塞住，让画面一直显示 */
    while (1) pause();

    return 0;
}

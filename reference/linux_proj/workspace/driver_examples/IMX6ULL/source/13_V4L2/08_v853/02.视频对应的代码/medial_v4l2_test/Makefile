# 使用GLIBC的工具链
#export PATH=$PATH:/home/book/tina-v853-open/prebuilt/rootfsbuilt/arm/toolchain-sunxi-glibc-gcc-830/toolchain/bin
#export STAGING_DIR=/home/book/tina-v853-open/prebuilt/rootfsbuilt/arm/toolchain-sunxi-glibc-gcc-830/toolchain/arm-openwrt-linux-gnueabi

CFLAGS= -I include

all:
	arm-openwrt-linux-gnueabi-gcc ${CFLAGS} -o medial_v4l2_test  medial_v4l2_test.c libmediactl.c

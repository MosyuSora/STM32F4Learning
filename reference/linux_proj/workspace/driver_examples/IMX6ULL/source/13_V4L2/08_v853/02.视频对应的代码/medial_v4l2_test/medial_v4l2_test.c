#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#include <linux/media.h>
#include <linux/types.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>

#include "mediactl.h"
#include "sunxi_camera_v2.h"
#include "mediactl-priv.h"
#include "options.h"
#include "tools.h"
#include "v4l2subdev.h"


#define SUNXI_VIDEO "vin_video"
#define SUNXI_ISP   "sunxi_isp"
#define SUNXI_H3A   "sunxi_h3a"

#define CAP_WIDTH  640
#define CAP_HEIGHt 640

struct video_plane {
	unsigned int size;
	int dma_fd;
	void *mem;
	unsigned int  mem_phy;
};

struct video_buffer {
	unsigned int index;
	unsigned int bytesused;
	unsigned int frame_cnt;
	unsigned int exp_time;
	struct timeval timestamp;
	bool error;
	bool allocated;
	unsigned int nplanes;
	struct video_plane *planes;
};

struct my_video_dev {
    int id;
	struct media_device *media;
    struct media_entity *video_entity;
    int video_entity_fd;

    struct media_entity *isp_entity;
    int isp_entity_fd;

    struct media_entity *sensor_entity;
    int sensor_entity_fd;

	struct v4l2_pix_format_mplane format;
	unsigned int nbufs;
	unsigned int nplanes;    
	unsigned int capturemode;

    struct video_buffer buffers[32];
};

static struct my_video_dev g_my_video_dev;

struct media_entity *media_pipeline_get_head(struct media_entity *me)
{
	struct media_pad *pad = &me->pads[0];

	while (!(pad->flags & MEDIA_PAD_FL_SOURCE)) {
		pad = media_entity_remote_source(pad);
		if (!pad)
			break;
		me = pad->entity;
		pad = &me->pads[0];
	}
	if (me->info.type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR)
		return me;
	else
		return NULL;
}

int video_set_fmt(struct my_video_dev *video_dev)
{
	struct v4l2_format fmt;
	struct v4l2_input inp;
	struct v4l2_streamparm parms;
	struct sensor_isp_cfg sensor_isp_cfg;
	int frame_mode;
	struct csi_ve_online_cfg ve_online_cfg;
    
    int fd = video_dev->video_entity_fd;

	memset(&inp, 0, sizeof inp);
	inp.index = 0; // vfmt->index;
	if (-1 == ioctl(fd, VIDIOC_S_INPUT, &inp)) {
		printf("VIDIOC_S_INPUT %d error!\n", 0);
		return -1;
	}

	// only vipp0 support online
	if (0 == video_dev->id)
	{
		memset(&ve_online_cfg, 0, sizeof ve_online_cfg);
		ve_online_cfg.ve_online_en = 0; //vfmt->ve_online_en;
		ve_online_cfg.dma_buf_num  = 0; //vfmt->dma_buf_num;

		/* must set after VIDIOC_S_INPUT and before VIDIOC_S_PARM */
		if (-1 == ioctl(fd, VIDIOC_SET_VE_ONLINE, &ve_online_cfg)) {
			printf("video fd[%d] VIDIOC_SET_VE_ONLINE error\n", fd);
			return -1;
		}
	}

	memset(&parms, 0, sizeof parms);
	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator = 30; // 30;
	parms.parm.capture.capturemode = V4L2_MODE_VIDEO ; //vfmt->capturemode;
	parms.parm.capture.reserved[0] = 0; /*when different video have the same sensor source, 1:use sensor current win, 0:find the nearest win*/
	parms.parm.capture.reserved[1] = 0; /*2:comanding, 1: wdr, 0: normal*/

	if (-1 == ioctl(fd, VIDIOC_S_PARM, &parms)) {
		printf("VIDIOC_S_PARM error\n");
		return -1;
	}

	memset(&sensor_isp_cfg, 0, sizeof sensor_isp_cfg);
	sensor_isp_cfg.isp_wdr_mode = 0; /*2:command, 1: wdr, 0: normal*/
	if (-1 == ioctl(fd, VIDIOC_SET_SENSOR_ISP_CFG, &sensor_isp_cfg)) {
		printf("VIDIOC_SET_SENSOR_ISP_CFG error\n");
	}
	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV21;
    fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    fmt.fmt.pix_mp.colorspace = V4L2_COLORSPACE_JPEG;
    fmt.fmt.pix_mp.width  = CAP_WIDTH;
    fmt.fmt.pix_mp.height = CAP_HEIGHt;

	if (-1 == ioctl(fd, VIDIOC_S_FMT, &fmt)) {
		printf("VIDIOC_S_FMT error!\n");
		//return -1;
	}

	if (-1 == ioctl(fd, VIDIOC_G_FMT, &fmt)) {
		printf("VIDIOC_G_FMT error!\n");
		//return -1;
	} 

    video_dev->nplanes = fmt.fmt.pix_mp.num_planes;
    video_dev->format = fmt.fmt.pix_mp;

	if (-1 == ioctl(fd, VIDIOC_G_PARM, &parms)) {
		printf("VIDIOC_G_PARM error\n");
		//return -1;
	}

	video_dev->capturemode = parms.parm.capture.capturemode;


	struct tdm_speeddn_cfg speeddn_cfg;
	memset(&speeddn_cfg, 0, sizeof speeddn_cfg);
	speeddn_cfg.pix_num = 0;
	speeddn_cfg.tdm_speed_down_en = 0;
	speeddn_cfg.tdm_tx_valid_num = 1;
	speeddn_cfg.tdm_tx_invalid_num = 0;
	if (-1 == ioctl(fd, VIDIOC_SET_TDM_SPEEDDN_CFG, &speeddn_cfg)) {
		printf("VIDIOC_SET_TDM_SPEEDDN_CFG error!\n");
		//return -1;
	}

	return 0;
}



int capture_loop(struct my_video_dev *video_dev)
{
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_frmsizeenum fsenum;
    int fmt_index = 0;
    int frame_index = 0;
    int i;
    void *bufs[32];
    int buf_cnt;
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    struct pollfd fds[1];
    char filename[32];
    int file_cnt = 0;
    int fd = video_dev->video_entity_fd;
	struct v4l2_buffer buf;

    /*
     * 申请buffer
     */
    struct v4l2_requestbuffers rb;
    memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
    rb.count = 32;
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    rb.memory = V4L2_MEMORY_MMAP;

    if (0 == ioctl(fd, VIDIOC_REQBUFS, &rb))
    {
        /* 申请成功后, mmap这些buffer */
        buf_cnt = rb.count;
        for(i = 0; i < rb.count; i++) {

    		struct v4l2_plane planes[VIDEO_MAX_PLANES];
            
    		memset(&buf, 0, sizeof buf);
    		memset(planes, 0, sizeof planes);

    		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    		buf.memory = V4L2_MEMORY_MMAP;
    		buf.index = i;
    		buf.length = video_dev->nplanes;
    		buf.m.planes = planes;

            if (0 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
            {
                struct video_plane *video_planes;
                video_planes = calloc(video_dev->nplanes, sizeof *planes);
                if (video_planes == NULL) {
                    printf("planes struct calloc failed!\n");
                    return -1;
                }
                video_dev->buffers[i].index = i;
                video_dev->buffers[i].planes = video_planes;
                video_dev->buffers[i].nplanes = video_dev->nplanes;
                video_dev->buffers[i].allocated = true;
                
                /* mmap */
    			for (int j = 0; j < video_dev->nplanes; j++) {
    				video_dev->buffers[i].planes[j].size = buf.m.planes[j].length;
    				video_dev->buffers[i].planes[j].mem =
    				    mmap(NULL /* start anywhere */ ,
    					 buf.m.planes[j].length,
    					 PROT_READ | PROT_WRITE /* required */ ,
    					 MAP_SHARED /* recommended */ ,
    					 fd, buf.m.planes[j].m.mem_offset);

    				if (MAP_FAILED == video_dev->buffers[i].planes[j].mem) {
    					printf("unable to map buffer %u (%d)\n",
    					       i, errno);
    					return -1;
    				}
    			}
            }
            else
            {
                printf("can not query buffer\n");
                return -1;
            }            
        }

        printf("map %d buffers ok\n", buf_cnt);
        
    }
    else
    {
        printf("can not request buffers\n");
        return -1;
    }

    /* 把所有buffer放入"空闲链表" */
    for(i = 0; i < buf_cnt; ++i) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        
        memset(&buf, 0, sizeof buf);
        memset(planes, 0, sizeof planes);
        
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.length = video_dev->nplanes;
        buf.m.planes = planes;
        
        if (0 != ioctl(fd, VIDIOC_QBUF, &buf))
        {
            perror("Unable to queue buffer");
            return -1;
        }
    }
    printf("queue buffers ok\n");

    /* 启动摄像头 */
    if (0 != ioctl(fd, VIDIOC_STREAMON, &type))
    {
        perror("Unable to start capture");
        return -1;
    }
    printf("start capture ok\n");

    while (1)
    {
        /* poll */
        memset(fds, 0, sizeof(fds));
        fds[0].fd = fd;
        fds[0].events = POLLIN;
        if (1 == poll(fds, 1, -1))
        {
            /* 把buffer取出队列 */
            struct v4l2_buffer buf;
            struct v4l2_plane planes[VIDEO_MAX_PLANES];
            
            memset(&buf, 0, sizeof buf);
            memset(planes, 0, sizeof planes);
            
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.length = video_dev->nplanes;
            buf.m.planes = planes;
            
            if (0 != ioctl(fd, VIDIOC_DQBUF, &buf))
            {
                perror("Unable to dequeue buffer");
                return -1;
            }
            
            /* 把buffer的数据存为文件 */
            sprintf(filename, "video_raw_data_%04d.yuv", file_cnt++);
            int fd_file = open(filename, O_RDWR | O_CREAT, 0666);
            if (fd_file < 0)
            {
                printf("can not create file : %s\n", filename);
                return -1;
            }
            printf("capture to %s\n", filename);
            //write(fd_file, bufs[buf.index], buf.bytesused);

            int index = buf.index;
            for (int j = 0; j < video_dev->nplanes; j++)
            {
                if (buf.m.planes[j].bytesused)
                    write(fd_file, video_dev->buffers[index].planes[j].mem, buf.m.planes[j].bytesused);
            }
            
            close(fd_file);

            /* 把buffer放入队列 */
            memset(&buf, 0, sizeof buf);
            memset(planes, 0, sizeof planes);
            buf.index = index;
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.length = video_dev->nplanes;
            buf.m.planes = planes;
            
            if (0 != ioctl(fd, VIDIOC_QBUF, &buf))
            {
                perror("Unable to queue buffer");
                return -1;
            }
        }
    }

    if (0 != ioctl(fd, VIDIOC_STREAMOFF, &type))
    {
        perror("Unable to stop capture");
        return -1;
    }
    printf("stop capture ok\n");
    close(fd);

    return 0;
}


int main(int argc, char **argv)
{
	struct media_entity *entity = NULL;
	int ret = -1;
    char name[20];
    int id = 0;


    if (argc != 2 && argc != 3)
    {
        printf("Usage: %s <media_dev> [video_id]\n", argv[0]);
        printf("eg: %s /dev/media0\n", argv[0]);
        printf("    %s /dev/media0 0\n", argv[0]);  /* 通过/dev/media0找到、操作vin_video0设备 */
        return -1;
    }

    if (argc == 3)
    {
        id = (int)strtoul(argv[2], NULL, 0);
        g_my_video_dev.id = id;
    }

	g_my_video_dev.media = media_device_new(argv[1]);
	if (g_my_video_dev.media == NULL) {
		printf("Failed to create media device %s\n", argv[1]);
		goto out;
	}

	/* Enumerate entities, pads and links. */
	ret = media_device_enumerate(g_my_video_dev.media);
	if (ret < 0) {
		printf("Failed to enumerate %s (%d)\n", argv[1], ret);
		goto out;
	}

    /* 根据名字找到video entity */
    sprintf(name, "%s%d", SUNXI_VIDEO, id);
    
	g_my_video_dev.video_entity = media_get_entity_by_name(g_my_video_dev.media, name);
	if (g_my_video_dev.video_entity == NULL) {
		printf("can not get entity by name %s\n", name);
		goto out;
	}

    printf("video entity %s's device node is %s\n", name, g_my_video_dev.video_entity->devname);

    /* 打开video设备 */
	g_my_video_dev.video_entity_fd = open(g_my_video_dev.video_entity->devname, O_RDWR|O_NONBLOCK|O_CLOEXEC, 0);
	if (g_my_video_dev.video_entity_fd == -1) {
		printf("%s: Failed to open subdev device node %s\n", __func__,
			g_my_video_dev.video_entity->devname);
		goto out;
	}

    video_set_fmt(&g_my_video_dev);

    /* 根据名字找到isp entity */
    sprintf(name, "%s.%d", SUNXI_ISP, id);
    
	g_my_video_dev.isp_entity = media_get_entity_by_name(g_my_video_dev.media, name);
	if (g_my_video_dev.isp_entity == NULL) {
		printf("can not get entity by name %s\n", name);
		goto out;
	}

    printf("isp entity %s is %s\n", name, g_my_video_dev.isp_entity->devname);

    /* 打开isp subdev */
	g_my_video_dev.isp_entity_fd = open(g_my_video_dev.isp_entity->devname, O_RDWR | O_NONBLOCK, 0);
	if (g_my_video_dev.isp_entity_fd == -1) {
		printf("%s: Failed to open isp subdev device node %s\n", __func__,
			g_my_video_dev.isp_entity->devname);
		goto out;
	}

    /* 如果熟悉ISP操作的话,可以通过ioctl操作文件句柄g_my_video_dev.isp_entity_fd */

    /* 根据pipe找到sensor entity */
	g_my_video_dev.sensor_entity = media_pipeline_get_head(g_my_video_dev.isp_entity);
	if (g_my_video_dev.sensor_entity == NULL) {
        printf("can not get sensor entity for %s\n", name);
        goto out;
    }

    printf("sensor entity is %s\n", g_my_video_dev.sensor_entity->devname);

    /* 打开sensor subdev */
	g_my_video_dev.sensor_entity_fd = open(g_my_video_dev.sensor_entity->devname, O_RDWR | O_NONBLOCK, 0);
	if (g_my_video_dev.sensor_entity_fd == -1) {
		printf("%s: Failed to open isp subdev device node %s\n", __func__,
			g_my_video_dev.sensor_entity->devname);
		//goto out;
	}

    /* 如果熟悉sensor操作的话,可以通过ioctl操作文件句柄g_my_video_dev.sensor_entity_fd */

    capture_loop(&g_my_video_dev);
    
out:
    if (g_my_video_dev.media)
        media_device_unref(g_my_video_dev.media);    
    if (g_my_video_dev.media)
        media_device_unref(g_my_video_dev.media);    
    return -1;
}


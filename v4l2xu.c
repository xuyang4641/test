#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <time.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <asm/types.h>

//#define CAMDEVNAME "/dev/video0"
#define JPG "xuimage%d.yuv" 
int fd;
int ret;
v4l2_std_id std;
struct v4l2_requestbuffers req;
typedef struct {
                void * start;
                size_t length;
        } vbuf;
        vbuf * buffers;

int init_mmap(int fd){
	int num = 0;
	memset(&req,0,sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	printf("in mmap!\n");
	sleep(1);
	if(-1 == ioctl(fd,VIDIOC_REQBUFS,&req)){
		printf("Fail to ioctl 'VIDIOC_REQBUFS'");
		return -1;
	}
	buffers = (vbuf *)calloc(req.count,sizeof(* buffers));
	struct v4l2_buffer buf;
	for(num = 0;num < req.count;++num){
		memset(&buf,0,sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;  
        	buf.index = num;
		printf("begin VIDIOC_QUERYBUF!\n");
		if(-1 == ioctl(fd,VIDIOC_QUERYBUF,&buf)) {
			return -1;
		}
		buffers[num].length = buf.length;
		buffers[num].start = mmap(NULL,buf.length,PROT_READ|PROT_WRITE,
		MAP_SHARED,fd,buf.m.offset);
		if(MAP_FAILED == buffers[num].start){
			return -1;
		}
	}
}

int init_camera_device(int fd){
	struct v4l2_capability cap;
	struct v4l2_format tv_fmt;
	int ret;
	memset(&tv_fmt,0,sizeof(tv_fmt));
	printf("now initcam!\n");
	sleep(1);
	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	sleep(2);
	printf("VIDIOC_QUERYCAP success!\n");
	if(ret < 0){
		printf("Fail to ioctl VIDEO_QUERYCAP");
		return -1;
	}
	if(!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE)){
		printf("The Current device is not a video capture device\n");
		return -1;
	}
	if(!(cap.capabilities & V4L2_CAP_STREAMING)){
		printf("The Current device does not support streaming i/o\n");
		return -1;
	}
	struct v4l2_input input;
   	memset(&input, 0, sizeof(struct v4l2_input));
    	input.index = 0;
    	int rtn = ioctl(fd, VIDIOC_S_INPUT, &input);
	tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    	tv_fmt.fmt.pix.width = 640;  
    	tv_fmt.fmt.pix.height = 480;  
    	tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
    	//tv_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	sleep(1);
	printf("begin VIDIOC_S_FMT!\n");
	sleep(1);
	ret = ioctl(fd, VIDIOC_S_FMT, &tv_fmt);
	tv_fmt.type = V4L2_BUF_TYPE_PRIVATE;
    	ioctl(fd, VIDIOC_S_FMT, &tv_fmt);
	tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	sleep(1);
	printf("begin map!\n");
	init_mmap(fd);
	printf("init cam sucsses!\n");
	return 0;
}

int open_camera_device(){	
	fd = open("/dev/video0", O_RDWR | O_NONBLOCK, 0);
	sleep(1);
	return 1;
}

int start_capture(int fd){
	enum v4l2_buf_type type;
	int i;
	int a;
	for (i = 0; i < 4; ++i) {
        	struct v4l2_buffer buf;
        	memset(&buf, 0, sizeof(struct v4l2_buffer));
        	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        	buf.memory = V4L2_MEMORY_MMAP;
        	buf.index = i;
		//printf("start_capture for!\n");
        	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
		{
			printf("************** %s, line = %d\n", __FUNCTION__, __LINE__);
            		return -1;
		}
        }
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("begin VIDIOC_STREAMON!\n");
//	a = ioctl(fd, VIDIOC_STREAMON, &type);
	if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
   	{
		printf("************** %s, line = %d\n", __FUNCTION__, __LINE__);
        return -1;
        }
	printf("************** %s, line = %d\n", __FUNCTION__, __LINE__);
	return 0;
}

int processimg(void * addr,int length){
	static int num = 0;
	FILE * fp;
	//char * imgname;
	//sprintf(imgname,JPG,num++);
	printf("************** %s, line = %d\n", __FUNCTION__, __LINE__);
	if((fp = fopen("xuimage.yuv","wb"))==NULL){
		printf("************** %s, line = %dfaild!\n", __FUNCTION__, __LINE__);

		return -1;
	}
	fwrite(addr,length,1,fp);
	usleep(500);
	fclose(fp);
	return 0;
}

int read_frame(int fd){
	printf("************** %s, line = %d\n", __FUNCTION__, __LINE__);
	sleep(1);
	struct v4l2_buffer buf;
	//struct v4l2_format fmt;
	memset(&buf,0,sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	//ioctl(fd, VIDIOC_G_FMT, &fmt);
	if(-1 == ioctl(fd,VIDIOC_DQBUF,&buf)){
		printf("Fail to ioctl 'VIDIOC_DQBUF'");
		return -1;	
	}
	printf("************** %s, line = %d,buf.index = %d,buf.length = %d\n", __FUNCTION__, __LINE__,buf.index,buffers[buf.index].length);
	processimg(buffers[buf.index].start,buffers[buf.index].length);
	if(-1 == ioctl(fd,VIDIOC_QBUF,&buf)){
		printf("Fail to ioctl 'VIDIOC_QBUF'");
                return -1;
	}
	return 1;
}

void stop_capture(int fd){
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("************** %s, line = %d\n", __FUNCTION__, __LINE__);
	if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &type)){
                printf("VIDIOC_STREAMOFF faild!\n");
		exit(EXIT_FAILURE);
	}
}

void close_camera_device(int fd){
	unsigned int i;
	printf("************** %s, line = %d\n", __FUNCTION__, __LINE__);
	for(i = 0;i<4;i++){
		if(-1 == munmap(buffers[i].start,buffers[i].length)){
			exit(-1);
		}
	}
	free(buffers);
	close(fd);
}

int mainloop(int fd){
	int count = 10;
	while(count-- > 0){
		/*for(;;){
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd,&fds);
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			r = select(fd + 1,&fds,NULL,NULL,&tv);
			printf("select r= %d\n",r);
			if(-1 == r){
				if(EINTR == errno)
					continue;
				perror("Fail to select");
				exit(EXIT_FAILURE);
			}
			
			if(0 == r){
				fprintf(stderr,"select Timeout\n");
				exit(-1);
			}
			
			if(read_frame(fd))
				break;
		}*/
		read_frame(fd);
	}
	return 0;
}

int main(void)  
{   
    int aa;
    printf("begin opened!\n");	
    sleep(1);
    if(open_camera_device()){
 	printf("open sucsses!\n");	
    }
    sleep(1);
    init_camera_device(fd);  
    start_capture(fd);  
    //mainloop(fd);  
    read_frame(fd);
    stop_capture(fd);  
    close_camera_device(fd);  
   return 0;  
} 

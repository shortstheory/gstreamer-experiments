#include <dirent.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
int main() {
       int fd = open("/dev/video0", O_RDWR, 0);
enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    struct v4l2_fmtdesc fmt;
    struct v4l2_frmivalenum frmsize;
    struct v4l2_frmivalenum frmival;

    fmt.index = 0;
    fmt.type = type;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
        frmsize.pixel_format = fmt.pixelformat;
        frmsize.index = 0;
        // while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                // printf("%dx%d\n", frmsize.discrete.width, frmsize.discrete.height);
                while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmsize) != -1) {
				    printf("%f fps", (float)frmsize.discrete.denominator/frmsize.discrete.numerator);
				    frmsize.index += 1;
			    }
            // } else 
            // if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
            //     printf("%dx%d\n", 
            //                       frmsize.stepwise.max_width,
            //                       frmsize.stepwise.max_height);
            // }
            //     frmsize.index++;
            // }
            }
            fmt.index++;
    // }
    }
}
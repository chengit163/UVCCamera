#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h>  /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h> /* for videodev2.h */
#include <linux/videodev2.h>

#include "V4L2Capture.h"



/*  */
#define CLEAR(x) memset(&(x), 0, sizeof(x))

static int xioctl(int fd, int request, void *arg)
{
    int r;
    do
        r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

static int errno_exit(const char *s)
{
    LOGW("%s error %d, %s", s, errno, strerror(errno));
    return CALL_ERROR;
}

/*  */

V4L2Capture::V4L2Capture
        (process_image_callback_t *callback, void *ptr, const char *name, int fd)
        : AbstractCapture(callback, ptr, name, fd)
{
    io = IO_METHOD_MMAP;
    buffers = NULL;
    n_buffers = 0;
}

V4L2Capture::~V4L2Capture()
{
    stopCapture();
    Close();
}

//====================================================================================================

bool V4L2Capture::Open(void)
{
    int r = open_device();
    if (CALL_SUCCESS == r)
        r = init_device();
    mIsOpened = (CALL_SUCCESS == r ? true : false);
    return mIsOpened;
}

void V4L2Capture::Close(void)
{
    uninit_device();
    close_device();
    mIsOpened = false;
}

void V4L2Capture::formatChange()
{
    LOGI("{pixelformat: %d, fps: %d, width: %d, height: %d}", mPixelformat, mFps, mWidth, mHeight);
    uninit_device();
    close_device();
    int r = open_device();
    if (CALL_SUCCESS == r)
        r = init_device();
    mIsOpened = (CALL_SUCCESS == r ? true : false);
}

void V4L2Capture::startCapture()
{
    if (!isRunning())
    {
        start_capturing();
        AbstractCapture::startCapture();
    }
}

void V4L2Capture::stopCapture()
{
    if (isCancelable())
    {
        if (isRunning())
        {
            AbstractCapture::stopCapture();
            stop_capturing();
        }
    }
}

void V4L2Capture::handleMainLooper(JNIEnv *env)
{
    while (isRunning())
    {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(mFd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(mFd + 1, &fds, NULL, NULL, &tv);

        if (r > 0)
        {
            int n = read_frame(env);
            // LOGW("select: %d; read_frame: %d", r, n);
        } else if (r == 0)
        {
            LOGW("select timeout");
        } else if (r < 0)
        {
            if (EINTR == errno)
                continue;
            LOGW("select error: %d", r);
        }
        /* EAGAIN - continue select loop. */
    }
}

//====================================================================================================
//private
int V4L2Capture::read_frame(JNIEnv *env)
{
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io)
    {
        case IO_METHOD_READ:
            if (-1 == read(mFd, buffers[0].start, buffers[0].length))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        return errno_exit("read");
                }
            }

            // process_image(buffers[0].start);

            break;

        case IO_METHOD_MMAP:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl(mFd, VIDIOC_DQBUF, &buf))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        return errno_exit("VIDIOC_DQBUF");
                }
            }

            assert(buf.index < n_buffers);

            // process_image(buffers[buf.index].start);
            processImageCallback(env, buffers[buf.index].start, buf.bytesused);

            if (-1 == xioctl(mFd, VIDIOC_QBUF, &buf))
                return errno_exit("VIDIOC_QBUF");

            break;

        case IO_METHOD_USERPTR:
            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == xioctl(mFd, VIDIOC_DQBUF, &buf))
            {
                switch (errno)
                {
                    case EAGAIN:
                        return 0;

                    case EIO:
                        /* Could ignore EIO, see spec. */

                        /* fall through */

                    default:
                        return errno_exit("VIDIOC_DQBUF");
                }
            }

            for (i = 0; i < n_buffers; ++i)
                if (buf.m.userptr == (unsigned long) buffers[i].start &&
                    buf.length == buffers[i].length)
                    break;

            assert(i < n_buffers);

            // process_image((void *)buf.m.userptr);

            if (-1 == xioctl(mFd, VIDIOC_QBUF, &buf))
                return errno_exit("VIDIOC_QBUF");

            break;
    }

    return 1;
}

int V4L2Capture::open_device(void)
{
    if ((NULL == mName) || (0 == strlen(mName)))
    {
        LOGW("no device");
        return CALL_ERROR;
    }

    struct stat st;

    if (-1 == stat(mName, &st))
    {
        LOGW("Cannot identify '%s': %d, %s", mName, errno, strerror(errno));
        return CALL_ERROR;
    }

    if (!S_ISCHR(st.st_mode))
    {
        LOGW("%s is no device", mName);
        return CALL_ERROR;
    }

    // fd = open(mFilename, O_RDWR /* required */ | O_NONBLOCK, 0);
    mFd = open(mName, O_RDWR, 0);

    if (-1 == mFd)
    {
        LOGW("Cannot open '%s': %d, %s", mName, errno, strerror(errno));
        return CALL_ERROR;
    }

    return CALL_SUCCESS;
}

int V4L2Capture::close_device(void)
{
    if (-1 == close(mFd))
    {
        mFd = -1;
        return errno_exit("close");
    }
    mFd = -1;
    return CALL_SUCCESS;
}

int V4L2Capture::init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(mFd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno)
        {
            LOGW("%s is no V4L2 device", mName);
            return CALL_ERROR;
        } else
        {
            return errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        LOGW("%s is no video capture device", mName);
        return CALL_ERROR;
    }

    switch (io)
    {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE))
            {
                LOGW("%s does not support read i/o", mName);
                return CALL_ERROR;
            }

            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING))
            {
                LOGW("%s does not support streaming i/o", mName);
                return CALL_ERROR;
            }

            break;
    }

    /* Select video input, video standard and tune here. */

    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(mFd, VIDIOC_CROPCAP, &cropcap))
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(mFd, VIDIOC_S_CROP, &crop))
        {
            switch (errno)
            {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    } else
    {
        /* Errors ignored. */
    }

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = mWidth;
    fmt.fmt.pix.height = mHeight;
    // V4L2_PIX_FMT_MJPEG
    // V4L2_PIX_FMT_YUYV
    // #define V4L2_PIX_FMT_H264 v4l2_fourcc('H', '2', '6', '4')
    switch (mPixelformat)
    {
        case PIXEL_FORMAT_MJPEG:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
            break;
        case PIXEL_FORMAT_H264:
            fmt.fmt.pix.pixelformat = v4l2_fourcc('H', '2', '6', '4');
            break;
        default:
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
            break;
    }
    // V4L2_FIELD_ANY
    // V4L2_FIELD_INTERLACED
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (-1 == xioctl(mFd, VIDIOC_S_FMT, &fmt))
        return errno_exit("VIDIOC_S_FMT");

    /* Note VIDIOC_S_FMT may change width and height. */

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    int r = CALL_ERROR;
    switch (io)
    {
        case IO_METHOD_READ:
            r = init_read(fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD_MMAP:
            r = init_mmap();
            break;

        case IO_METHOD_USERPTR:
            r = init_userp(fmt.fmt.pix.sizeimage);
            break;
    }

    return r;
}

int V4L2Capture::uninit_device(void)
{
    unsigned int i;

    switch (io)
    {
        case IO_METHOD_READ:
            free(buffers[0].start);
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i)
                if (-1 == munmap(buffers[i].start, buffers[i].length))
                    return errno_exit("munmap");
            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i)
                free(buffers[i].start);
            break;
    }

    free(buffers);

    return CALL_SUCCESS;
}

int V4L2Capture::start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    switch (io)
    {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
            for (i = 0; i < n_buffers; ++i)
            {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (-1 == xioctl(mFd, VIDIOC_QBUF, &buf))
                    return errno_exit("VIDIOC_QBUF");
            }

            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (-1 == xioctl(mFd, VIDIOC_STREAMON, &type))
                return errno_exit("VIDIOC_STREAMON");

            break;

        case IO_METHOD_USERPTR:
            for (i = 0; i < n_buffers; ++i)
            {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;
                buf.index = i;
                buf.m.userptr = (unsigned long) buffers[i].start;
                buf.length = buffers[i].length;

                if (-1 == xioctl(mFd, VIDIOC_QBUF, &buf))
                    return errno_exit("VIDIOC_QBUF");
            }

            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (-1 == xioctl(mFd, VIDIOC_STREAMON, &type))
                return errno_exit("VIDIOC_STREAMON");

            break;
    }

    return CALL_SUCCESS;
}

int V4L2Capture::stop_capturing(void)
{
    enum v4l2_buf_type type;

    switch (io)
    {
        case IO_METHOD_READ:
            /* Nothing to do. */
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (-1 == xioctl(mFd, VIDIOC_STREAMOFF, &type))
                return errno_exit("VIDIOC_STREAMOFF");

            break;
    }

    return CALL_SUCCESS;
}

int V4L2Capture::init_read(unsigned int buffer_size)
{
    // buffers = calloc(1, sizeof(*buffers));
    buffers = (struct buffer *) calloc(1, sizeof(*buffers));

    if (!buffers)
    {
        LOGW("Out of memory");
        return CALL_ERROR;
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);

    if (!buffers[0].start)
    {
        LOGW("Out of memory");
        return CALL_ERROR;
    }

    return CALL_SUCCESS;
}

int V4L2Capture::init_mmap()
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(mFd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            LOGW("%s does not support memory mapping", mName);
            return CALL_ERROR;
        } else
        {
            return errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2)
    {
        LOGW("Insufficient buffer memory on %s", mName);
        return CALL_ERROR;
    }

    // buffers = calloc(req.count, sizeof(*buffers));
    buffers = (struct buffer *) calloc(req.count, sizeof(*buffers));

    if (!buffers)
    {
        LOGW("Out of memory");
        return CALL_ERROR;
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl(mFd, VIDIOC_QUERYBUF, &buf))
            return errno_exit("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     mFd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            return errno_exit("mmap");
    }

    return CALL_SUCCESS;
}

int V4L2Capture::init_userp(unsigned int buffer_size)
{
    struct v4l2_requestbuffers req;
    unsigned int page_size;

    page_size = getpagesize();
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(mFd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            LOGW("%s does not support user pointer i/o", mName);
            return CALL_ERROR;
        } else
        {
            return errno_exit("VIDIOC_REQBUFS");
        }
    }

    // buffers = calloc(4, sizeof(*buffers));
    buffers = (struct buffer *) calloc(4, sizeof(*buffers));

    if (!buffers)
    {
        LOGW("Out of memory");
        return CALL_ERROR;
    }

    for (n_buffers = 0; n_buffers < 4; ++n_buffers)
    {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = memalign(/* boundary */ page_size,
                                                           buffer_size);

        if (!buffers[n_buffers].start)
        {
            LOGW("Out of memory");
            return CALL_ERROR;
        }
    }

    return CALL_SUCCESS;
}
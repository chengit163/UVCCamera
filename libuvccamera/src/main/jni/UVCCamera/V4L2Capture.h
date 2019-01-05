#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H

#include "AbstractCapture.h"

typedef enum
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method;

struct buffer
{
    void *start;
    size_t length;
};

class V4L2Capture : public AbstractCapture
{
public:
    V4L2Capture(process_image_callback_t *callback, void *ptr, const char *name, int fd);
    virtual ~V4L2Capture();

private:
    io_method io;
    struct buffer *buffers;
    unsigned int n_buffers;

public:
    virtual bool Open(void);
    virtual void Close(void);
    virtual void formatChange();
    virtual void startCapture();
    virtual void stopCapture();
    virtual void handleMainLooper(JNIEnv *env);

private:
    int read_frame(JNIEnv *env);
    int open_device(void);
    int close_device(void);
    int init_device(void);
    int uninit_device(void);
    int start_capturing(void);
    int stop_capturing(void);
    int init_read(unsigned int buffer_size);
    int init_mmap();
    int init_userp(unsigned int buffer_size);
};


#endif //V4L2CAPTURE_H

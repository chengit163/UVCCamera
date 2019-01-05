#ifndef OBJCET_FRAME_STREAM_H
#define OBJCET_FRAME_STREAM_H

#define MAX_FRAME_SZ 2
#define MAX_CACHE_SZ MAX_FRAME_SZ + 2

typedef struct frame_stream
{
    int subsample;  /** 类型 */
    int width;      /** 宽 */
    int height;     /** 高 */
    int data_max;   /** data最大大小 */
    int data_bytes; /** data有效大小 */
    void *data;     /** 预览流数据 */
} frame_stream_t;

frame_stream_t *frame_stream_create(int size);                 /* 创建预览流对象 */
void frame_stream_recycle(frame_stream_t *frame);              /* 销毁销毁预览流对象 */
int frame_stream_save(frame_stream_t *frame, char *file_name); /* 保存 */
frame_stream_t *frame_stream_read(const char *file_name);      /* 读取 */

#endif //OBJCET_FRAME_STREAM_H

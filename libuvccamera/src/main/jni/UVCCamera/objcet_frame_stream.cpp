#include <stdio.h>
#include <stdlib.h>
#include "_utilbase.h"
#include "objcet_frame_stream.h"

frame_stream_t *frame_stream_create(int size)
{
    frame_stream_t *frame = new frame_stream_t;
    frame->width = 0;
    frame->height = 0;
    frame->data_max = size;
    frame->data_bytes = 0;
    frame->data = new unsigned char[size];
    return frame;
}

void frame_stream_recycle(frame_stream_t *frame)
{
    if (NULL != frame)
    {
        if (NULL != frame->data)
        {
            delete[](unsigned char *) frame->data;
            frame->data = NULL;
        }
        delete frame;
        frame = NULL;
    }
}

int frame_stream_save(frame_stream_t *frame, char *file_name)
{
    if (LIKELY(frame))
    {
        FILE *file = fopen(file_name, "wb");
        if (LIKELY(file))
        {
            fwrite(frame->data, frame->data_bytes, 1, file);
            fclose(file);
            file = NULL;
            return CALL_SUCCESS;
        }
    }
    return CALL_ERROR;
}

frame_stream_t *frame_stream_read(const char *file_name)
{
    frame_stream_t *frame = NULL;
    FILE *frame_file = fopen(file_name, "rb");
    if (LIKELY(frame_file))
    {
        fseek(frame_file, 0, SEEK_END);
        int size = ftell(frame_file);
        fseek(frame_file, 0, SEEK_SET);
        frame = frame_stream_create(size);
        frame->data_bytes = size;
        fread(frame->data, 1, frame->data_bytes, frame_file);
        fclose(frame_file);
        frame_file = NULL;
    }
    return frame;
}


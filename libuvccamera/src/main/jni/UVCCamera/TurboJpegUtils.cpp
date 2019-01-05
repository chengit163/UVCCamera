#include "TurboJpegUtils.h"

TurboJpegUtils *TurboJpegUtils::m_pInstance = NULL;
Mutex TurboJpegUtils::m_Mutex;

TurboJpegUtils::TurboJpegUtils()
{
    m_count = 0;
    m_tjhandle = NULL;
}

TurboJpegUtils::~TurboJpegUtils()
{
}

TurboJpegUtils *TurboJpegUtils::instance()
{
    if (NULL == m_pInstance)
    {
        m_Mutex.lock();
        if (NULL == m_pInstance)
        {
            m_pInstance = new TurboJpegUtils();
        }
        m_Mutex.unlock();
    }
    return m_pInstance;
}

void TurboJpegUtils::destroy()
{
    if (m_pInstance)
    {
        m_Mutex.lock();
        if (m_pInstance)
        {
            delete m_pInstance;
            m_pInstance = NULL;
        }
        m_Mutex.unlock();
    }
}

void TurboJpegUtils::init()
{
    m_Mutex.lock();
    if (NULL == m_tjhandle)
    {
        m_tjhandle = tjInitDecompress();
    }
    m_count++;
    m_Mutex.unlock();
}

void TurboJpegUtils::reinit()
{
    m_Mutex.lock();
    tjDestroy(m_tjhandle);
    m_tjhandle = tjInitDecompress();
    m_Mutex.unlock();
}

void TurboJpegUtils::uninit()
{
    m_Mutex.lock();
    if (--m_count <= 0)
    {
        if (NULL != m_tjhandle)
        {
            tjDestroy(m_tjhandle);
            m_tjhandle = NULL;
        }
    }
    m_Mutex.unlock();
}

int TurboJpegUtils::mjpeg2rgbx(frame_stream_t *in, frame_stream_t *out)
{
    int r = CALL_SUCCESS;

    m_Mutex.lock();
    if (m_tjhandle != NULL)
    {
        unsigned char *jpegBuf = (unsigned char *)in->data;
        unsigned long jpegSize = in->data_bytes;

        unsigned char *colors = (unsigned char *)out->data;
        int *width = &out->width;
        int *height = &out->height;

        int jpegSubsamp = 0;
        if (0 != tjDecompressHeader2(m_tjhandle, jpegBuf, jpegSize, width, height, &jpegSubsamp))
        {
            LOGE("tjDecompressHeader2 failed: %s; (w=%d, h=%d)", tjGetErrorStr(), *width, *height);
            r = CALL_ERROR;
        }

        int pitch = tjPixelSize[TJPF_RGBA] * (*width);
        if (0 != tjDecompress2(m_tjhandle, jpegBuf, jpegSize, colors, *width, pitch, *height, TJPF_RGBA, 0))
        {
            LOGE("tjDecompress2 failed: %s; (w=%d, h=%d)", tjGetErrorStr(), *width, *height);
            r = CALL_ERROR;
        }
    }
    m_Mutex.unlock();

    return r;
}

int TurboJpegUtils::mjpeg2yuvx(frame_stream_t *in, frame_stream_t *out)
{
    int r = CALL_SUCCESS;

    m_Mutex.lock();
    if (m_tjhandle != NULL)
    {
        unsigned char *jpegBuf = (unsigned char *)in->data;
        unsigned long jpegSize = in->data_bytes;

        unsigned char *colors = (unsigned char *)out->data;
        int *width = &out->width;
        int *height = &out->height;

        int jpegSubsamp = 0, jpegColorspace = 0;
        if (0 != tjDecompressHeader3(m_tjhandle, jpegBuf, jpegSize, width, height, &jpegSubsamp, &jpegColorspace))
        {
            LOGE("tjDecompressHeader3 failed: %s; (w=%d, h=%d)", tjGetErrorStr(), *width, *height);
            r = CALL_ERROR;
        }

        int padding = 1;
        int yuv_size = tjBufSizeYUV2(*width, padding, *height, jpegSubsamp);
        out->subsample = jpegSubsamp;
        out->data_bytes = yuv_size;
        if (0 != tjDecompressToYUV2(m_tjhandle, jpegBuf, jpegSize, colors, *width, padding, *height, 0))
        {
            LOGE("tjDecompressToYUV2 failed: %s; (w=%d, h=%d)", tjGetErrorStr(), *width, *height);
            r = CALL_ERROR;
        }
    }
    m_Mutex.unlock();

    return r;
}

int TurboJpegUtils::yuvx2rgbx(frame_stream_t *in, frame_stream_t *out)
{
    int r = CALL_SUCCESS;

    m_Mutex.lock();
    if (m_tjhandle != NULL)
    {
        int width = in->width;
        int height = in->height;
        int yuv_size = in->data_bytes;
        int subsample = in->subsample;

        out->width = width;
        out->height = height;

        int padding = 1;
        int need_size = tjBufSizeYUV2(width, padding, height, subsample);
        if (need_size != yuv_size)
        {
            LOGE("(need_size = %d) (yuv_size = %d) (w=%d, h=%d)", need_size, yuv_size, width, height);
            r = CALL_ERROR;
        }

        unsigned char *srcBuf = (unsigned char *)in->data;
        unsigned char *dstBuf = (unsigned char *)out->data;
        if (0 != tjDecodeYUV(m_tjhandle, srcBuf, padding, subsample, dstBuf, width, 0, height, TJPF_RGBA, 0))
        {
            LOGE("tjDecodeYUV failed: %s; (w=%d, h=%d)", tjGetErrorStr(), width, height);
            r = CALL_ERROR;
        }
    }
    m_Mutex.unlock();

    return r;
}
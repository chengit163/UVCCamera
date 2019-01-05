#ifndef TURBOJPEGUTILS_H
#define TURBOJPEGUTILS_H

#include "_utilbase.h"
#include "objcet_frame_stream.h"
#include "Mutex.h"

#if 1
#include "../libjpeg-turbo-1.5.3/turbojpeg.h"
#else
#include "turbojpeg.h"
#endif

class TurboJpegUtils
{

public:
    static TurboJpegUtils *instance();

    static void destroy();

private:
    TurboJpegUtils();

    virtual ~TurboJpegUtils();

    TurboJpegUtils(const TurboJpegUtils &)
    {};

    TurboJpegUtils &operator=(const TurboJpegUtils &)
    {};

private:
    static TurboJpegUtils *m_pInstance;
    static Mutex m_Mutex;

private:
    int m_count;
    tjhandle m_tjhandle;

public:
    void init();

    void reinit();

    void uninit();

    int mjpeg2rgbx(frame_stream_t *in, frame_stream_t *out);

    int mjpeg2yuvx(frame_stream_t *in, frame_stream_t *out);

    int yuvx2rgbx(frame_stream_t *in, frame_stream_t *out);
};

#endif //TURBOJPEGUTILS_H
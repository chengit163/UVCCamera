#ifndef UVCCAMERA_MJPEGPREVIEW_H
#define UVCCAMERA_MJPEGPREVIEW_H


#include "AbstractPreview.h"

class MjpegPreview : public AbstractPreview
{
public:
    MjpegPreview();
    virtual ~MjpegPreview();

private:
    virtual void handleMainLooper(JNIEnv *env);

};


#endif //UVCCAMERA_MJPEGPREVIEW_H

#include "MjpegPreview.h"
#include "TurboJpegUtils.h"

MjpegPreview::MjpegPreview()
{}

MjpegPreview::~MjpegPreview()
{

}

//====================================================================================================
static void copyFrame(const uint8_t *src, uint8_t *dest, const int width, int height,
                      const int stride_src, const int stride_dest)
{
    const int h8 = height % 8;
    for (int i = 0; i < h8; i++)
    {
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
    }
    for (int i = 0; i < height; i += 8)
    {
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
    }
}

static int copyToSurface(uint8_t *data, ANativeWindow **window, int Width, int Height)
{
    int result = -1;
    if (LIKELY(*window))
    {
        ANativeWindow_Buffer buffer;
        if ((result = ANativeWindow_lock(*window, &buffer, NULL)) == 0)
        {
            // source = frame data
            const uint8_t *src = data;
            const int src_w = Width * 4;
            const int src_step = Width * 4;
            // destination = Surface(ANativeWindow)
            uint8_t *dest = (uint8_t *) buffer.bits;
            const int dest_w = buffer.width * 4;
            const int dest_step = buffer.stride * 4;
            // use lower transfer bytes
            const int w = src_w < dest_w ? src_w : dest_w;
            // use lower height
            const int h = Height < buffer.height ? Height : buffer.height;
            // transfer from frame data to the Surface
            copyFrame(src, dest, w, h, src_step, dest_step);
            ANativeWindow_unlockAndPost(*window);
        }
    }
    return result;
}

//====================================================================================================
#define TEST_OPENGL_ES 1 //OpenGL ES测试

#if TEST_OPENGL_ES

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#define GET_STR(x) #x

//顶点着色器
static const char *vertexShader = GET_STR(
        attribute vec4 aPosition; //顶点坐标
        attribute vec2 aTexCoord; //材质顶点坐标
        varying vec2 vTexCoord;   //输出的材质坐标
        void main()
        {
            vTexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
            gl_Position = aPosition;
        }
);

//片元着色器
static const char *fragYUV420P = GET_STR(
        precision mediump float;    //精度
        varying vec2 vTexCoord;     //顶点着色器传递的坐标
        uniform sampler2D yTexture; //输入的材质（不透明灰度，单像素）
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;
        void main()
        {
            vec3 yuv;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uTexture, vTexCoord).r - 0.5;
            yuv.b = texture2D(vTexture, vTexCoord).r - 0.5;
            rgb = mat3(1.0, 1.0, 1.0,
                       0.0, -0.39465, 2.03211,
                       1.13983, -0.58060, 0.0) * yuv;
            //输出像素颜色
            gl_FragColor = vec4(rgb, 1.0);
        }
);

static int window_id = 0;

static EGLDisplay eglDisplay = NULL;
static EGLSurface eglSurface = NULL;
static GLuint eglTexts[3] = {0};

static bool init(int id, ANativeWindow *window)
{
    if (window_id != id)
    {
        eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (EGL_NO_DISPLAY == eglDisplay)
        {
            LOGW("eglGetDisplay failed!");
            return false;
        }
        if (EGL_TRUE != eglInitialize(eglDisplay, 0, 0))
        {
            LOGW("eglInitialize failed!");
            return false;
        }

        EGLConfig config;
        EGLint configNum;
        EGLint configSpec[] = {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
        };
        if (EGL_TRUE != eglChooseConfig(eglDisplay, configSpec, &config, 1, &configNum))
        {
            LOGW("eglChooseConfig failed!");
            return false;
        }
        eglSurface = eglCreateWindowSurface(eglDisplay, config, window, 0);
        if (EGL_NO_SURFACE == eglSurface)
        {
            LOGW("eglCreateWindowSurface failed!");
            return false;
        }

        const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        EGLContext context = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxAttr);
        if (EGL_NO_CONTEXT == context)
        {
            LOGW("eglCreateContext failed!");
            return false;
        }
        if (EGL_TRUE != eglMakeCurrent(eglDisplay, eglSurface, eglSurface, context))
        {
            LOGW("eglMakeCurrent failed!");
            return false;
        }

        window_id = id;
        return true;
    }
    return false;
}

static GLint InitShader(const char *code, GLint type)
{
    //创建shader
    GLint sh = glCreateShader(type);
    if (sh == 0)
    {
        LOGW("glCreateShader %d failed!", type);
        return 0;
    }
    //加载shader(,shader数量,shader代码,代码长度)
    glShaderSource(sh, 1, &code, 0);
    //编译shader
    glCompileShader(sh);
    //获取编译情况
    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        LOGW("glCompileShader failed!");
        return 0;
    }
    LOGW("glCompileShader success!");
    return sh;
}

static void initTexts(int width, int height)
{
    //顶点着色器vertexShader初始化
    GLint vsh = InitShader(vertexShader, GL_VERTEX_SHADER);
    //片元着色器fragYUV420P初始化
    GLint fsh = InitShader(fragYUV420P, GL_FRAGMENT_SHADER);

    //创建渲染程序
    GLint program = glCreateProgram();
    if (program == 0)
    {
        LOGW("glCreateProgram failed!");
        return;
    }
    //渲染程序中加入着色器代码
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    //链接程序
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        LOGW("glLinkProgram failed!");
        return;
    }
    glUseProgram(program);
    LOGW("glLinkProgram success!");

    //加入三维顶点数据 两个三角形组成正方形
    static float vers[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
    };
    GLuint apos = (GLuint) glGetAttribLocation(program, "aPosition");
    glEnableVertexAttribArray(apos);
    //传递顶点
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 12, vers);

    //加入材质坐标数据
    static float txts[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0, 1.0
    };
    GLuint atex = (GLuint) glGetAttribLocation(program, "aTexCoord");
    glEnableVertexAttribArray(atex);
    glVertexAttribPointer(atex, 2, GL_FLOAT, GL_FALSE, 8, txts);

    //材质纹理初始化
    //设置纹理层
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0); //对于纹理第1层
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1); //对于纹理第2层
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2); //对于纹理第3层

    //创建opengl纹理
    // GLuint texts[3] = {0};
    //创建三个纹理
    glGenTextures(3, eglTexts);

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, eglTexts[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width, height, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, eglTexts[1]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width / 2, height / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, eglTexts[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width / 2, height / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );
}


static int renderToSurface(uint8_t *data, int Width, int Height)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, eglTexts[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_LUMINANCE,
                    GL_UNSIGNED_BYTE, data);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, eglTexts[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width / 2, Height / 2, GL_LUMINANCE,
                    GL_UNSIGNED_BYTE, data + (Width * Height));


    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, eglTexts[2]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width / 2, Height / 2, GL_LUMINANCE,
                    GL_UNSIGNED_BYTE, data + (Width * Height * 5 / 4));

    //三维绘制
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //窗口显示
    EGLBoolean flag = eglSwapBuffers(eglDisplay, eglSurface);
    return flag ? 0 : -1;
}

#endif

//====================================================================================================

void MjpegPreview::handleMainLooper(JNIEnv *env)
{
    TurboJpegUtils::instance()->init();
    frame_stream_t *colors = frame_stream_create(mWidth * mHeight * 4);
    frame_stream_t *yuv = frame_stream_create(mWidth * mHeight * 3 / 2);
    ObjectArray<int> lostIndexs;

    clock_t start, end;
    while (isRunning())
    {
        frame_stream_t *frame = mFramePool->waitFrame();
        if (LIKELY(frame))
        {
            if (CALL_SUCCESS == TurboJpegUtils::instance()->mjpeg2yuvx(frame, yuv))
            {
                int size = 0; // 预览数
                threadLock();
                onShared(env, yuv->data, yuv->data_bytes);
                size = mDisplayWindows.size();// 初次获取预览数
                threadUnlock();
                if (size > 0)
                {
                    start = clock();//记录起始时间
#if TEST_OPENGL_ES
                    threadLock();
                    size = mDisplayWindows.size();//再次获取预览数
                    //只渲染最后一个
                    int i = size - 1;
                    display_window_t *display = mDisplayWindows.get(i);
                    if (LIKELY(display))
                    {
                        if (init(display->id, display->window))
                        {
                            initTexts(mWidth, mHeight);
                        }
                        int result = renderToSurface((uint8_t *) yuv->data, mWidth, mHeight);
                        if (0 != result)
                        {
                            int index = i + 1;//避免 if LIKELY(0) (+1非零)
                            lostIndexs.put(index);//上屏失败，认为该Surface已失效
                        }
                    }
                    while (!lostIndexs.isEmpty())
                    {
                        int index = lostIndexs.last() - 1;//避免 if LIKELY(0) (-1还原)
                        display_window_t *display = mDisplayWindows.remove(index);
                        lostDisplayWindow(display);
                    }
                    threadUnlock();
#else
                    if (CALL_SUCCESS == TurboJpegUtils::instance()->yuvx2rgbx(yuv, colors))
                    {
                        threadLock();
                        size = mDisplayWindows.size();//再次获取预览数
                        for (int i = 0; i < size; i++)
                        {
                            display_window_t *display = mDisplayWindows.get(i);
                            if (LIKELY(display))
                            {
                                int result = copyToSurface((uint8_t *) colors->data,
                                                           &display->window,
                                                           mWidth, mHeight);
                                if (0 != result)
                                {
                                    int index = i + 1;//避免 if LIKELY(0) (+1非零)
                                    lostIndexs.put(index);//上屏失败，认为该Surface已失效
                                }
                            }
                        }
                        while (!lostIndexs.isEmpty())
                        {
                            int index = lostIndexs.last() - 1;//避免 if LIKELY(0) (-1还原)
                            display_window_t *display = mDisplayWindows.remove(index);
                            lostDisplayWindow(display);
                        }
                        threadUnlock();
                    }
#endif
                    end = clock();//记录结束时间
                    LOGI("time: %.8fs", (double) (end - start) / CLOCKS_PER_SEC);
                }
            }
            mFramePool->pushCacheFrame(frame);
        }
    }

    lostIndexs.clear();
    frame_stream_recycle(colors);
    TurboJpegUtils::instance()->uninit();
}

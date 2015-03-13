#ifndef MYGLAPP_HPP
#define MYGLAPP_HPP

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <android/native_window.h> // requires ndk r5 or newer

#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

//#include <GLES/gl.h>

#include "logger.h"

#define LOG_TAG "MyGlApp"

struct MyGlApp {
    enum RenderThreadMessage {
        MSG_NONE = 0,
        MSG_WINDOW_SET,
        MSG_RENDER_LOOP_EXIT
    };

    pthread_t _threadId;
    pthread_mutex_t _mutex;
    enum RenderThreadMessage _msg;

    // android window, supported by NDK r5 and newer
    ANativeWindow* _window;

    EGLDisplay _display;
    EGLSurface _surface;
    EGLContext _context;

    MyGlApp() : _msg(MSG_NONE), _display(0), _surface(0), _context(0), _window(0)
    {
        LOG_INFO("Renderer instance created");
        pthread_mutex_init(&_mutex, 0);
        return;
    }
    virtual ~MyGlApp() {}

    ANativeWindow *window() {
    	return _window;
    }

    void start() {
        LOG_INFO("Creating renderer thread");
        pthread_create(&_threadId, 0, threadStartCallback, this);
        return;

    }

    void stop() {
        LOG_INFO("Stopping renderer thread");

        // send message to render thread to stop rendering
        pthread_mutex_lock(&_mutex);
        _msg = MSG_RENDER_LOOP_EXIT;
        pthread_mutex_unlock(&_mutex);

        pthread_join(_threadId, 0);
        LOG_INFO("Renderer thread stopped");

        return;
    }

    void destroy() {
        LOG_INFO("Destroying context");

        eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(_display, _context);
        eglDestroySurface(_display, _surface);
        eglTerminate(_display);

        _display = EGL_NO_DISPLAY;
        _surface = EGL_NO_SURFACE;
        _context = EGL_NO_CONTEXT;

        if (_window != NULL) {
        	ANativeWindow_release(_window);
        }

        return;
    }

    void setWindow(ANativeWindow *window)
    {
        // notify render thread that window has changed
        pthread_mutex_lock(&_mutex);
        _msg = MSG_WINDOW_SET;
        _window = window;

        ANativeWindow_acquire(window);

        pthread_mutex_unlock(&_mutex);

        return;
    }

    static void* threadStartCallback(void *myself)
    {
    	MyGlApp *app = (MyGlApp*)myself;

        app->renderLoop();
        pthread_exit(0);

        return 0;
    }

    void renderLoop()
    {
        bool renderingEnabled = true;

        LOG_INFO("entering renderLoop()");

        while (renderingEnabled) {

            pthread_mutex_lock(&_mutex);

            // process incoming messages
            switch (_msg) {

                case MSG_WINDOW_SET:
                    initialize();
                    break;

                case MSG_RENDER_LOOP_EXIT:
                    renderingEnabled = false;
                    destroy();
                    break;

                default:
                    break;
            }
            _msg = MSG_NONE;

            if (_display) {
//                setViewport();

                onDraw();
                if (!eglSwapBuffers(_display, _surface)) {
                    LOG_ERROR("eglSwapBuffers() returned error %d", eglGetError());
                }
            }

            pthread_mutex_unlock(&_mutex);
        }

        LOG_INFO("Render loop exits");

        return;
    }

    void setViewport() {
        EGLint width;
        EGLint height;

        if (!eglQuerySurface(_display, _surface, EGL_WIDTH, &width) ||
            !eglQuerySurface(_display, _surface, EGL_HEIGHT, &height)) {
            LOG_ERROR("eglQuerySurface() returned error %d", eglGetError());
            destroy();
        }
        glViewport(0, 0, width, height);
    }

    virtual bool initialize()
    {
        EGLDisplay display;
        EGLConfig config;
        EGLint numConfigs;
        EGLint format;
        EGLSurface surface;
        EGLContext context;
        EGLint width;
        EGLint height;
        GLfloat ratio;

        LOG_INFO("Initializing context");

        if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
            LOG_ERROR("eglGetDisplay() returned error %d", eglGetError());
            return false;
        }
        if (!eglInitialize(display, 0, 0)) {
            LOG_ERROR("eglInitialize() returned error %d", eglGetError());
            return false;
        }

        const EGLint RGBX_8888_ATTRIBS[] =
        {
        		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_BLUE_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_RED_SIZE, 8,
				EGL_DEPTH_SIZE, 8,
				EGL_NONE
        };

        const EGLint RGB_565_ATTRIBS[] =
        {
        		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_BLUE_SIZE, 5,
				EGL_GREEN_SIZE, 6,
				EGL_RED_SIZE, 5,
				EGL_DEPTH_SIZE, 8, EGL_NONE
        };

        const EGLint* attribList;
        int windowFormat = ANativeWindow_getFormat(_window);
        if (true || windowFormat == WINDOW_FORMAT_RGBA_8888 || windowFormat == WINDOW_FORMAT_RGBX_8888) {
        	LOG_INFO("setting window format to WINDOW_FORMAT_RGBA_8888");
        	attribList = RGBX_8888_ATTRIBS;
        }
        else {
        	LOG_INFO("setting window format to WINDOW_FORMAT_RGB_565");
        	attribList = RGB_565_ATTRIBS;
        }

        if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs)) {
            LOG_ERROR("eglChooseConfig() returned error %d", eglGetError());
            destroy();
            return false;
        }

        if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
            LOG_ERROR("eglGetConfigAttrib() returned error %d", eglGetError());
            destroy();
            return false;
        }

        ANativeWindow_setBuffersGeometry(_window, 0, 0, format);

        if ((surface = eglCreateWindowSurface(display, config, _window, 0)) == EGL_NO_SURFACE) {
            LOG_ERROR("eglCreateWindowSurface() returned error %d", eglGetError());
            destroy();
            return false;
        }

        EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
//        EGLint *contextAttribs = NULL;
        if ((context = eglCreateContext(display, config, 0, contextAttribs)) == EGL_NO_CONTEXT) {
            LOG_ERROR("eglCreateContext() returned error %d", eglGetError());
            destroy();
            return false;
        }

        if (!eglMakeCurrent(display, surface, surface, context)) {
            LOG_ERROR("eglMakeCurrent() returned error %d", eglGetError());
            destroy();
            return false;
        }

        if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
            !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
            LOG_ERROR("eglQuerySurface() returned error %d", eglGetError());
            destroy();
            return false;
        }

        _display = display;
        _surface = surface;
        _context = context;

//        glDisable(GL_DITHER);
//        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//        glEnable(GL_CULL_FACE);
//        glShadeModel(GL_SMOOTH);
//        glEnable(GL_DEPTH_TEST);

        LOG_INFO("set viewport width %d height %d", width, height);
        glViewport(0, 0, width, height);

//        ratio = (GLfloat) width / height;
//        glMatrixMode(GL_PROJECTION);
//        glLoadIdentity();
//        glFrustumf(-ratio, ratio, -1, 1, 2, 10);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        LOG_INFO("Version: %s GLSL: %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

        LOG_INFO("base init() done.");

        return true;
    }

    virtual void onDraw() = 0;

};

#endif

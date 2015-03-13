
#include <stdint.h>
#include <jni.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer

#include "jniapi.h"
#include "logger.h"

#include "gl_app.hpp"
#include "examples/Model.hpp"
#include "examples/NullApp.hpp"

#define LOG_TAG "jniapi"


static ANativeWindow *window = 0;
static MyGlApp *myapp = 0;

JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeOnStart(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnStart");
    myapp = new ModelApp();
//    myapp = new NullApp();
    return;
}

JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeOnResume(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnResume");
    myapp->start();
    return;
}

JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeOnPause(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnPause");
    myapp->stop();
    return;
}

JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeOnStop(JNIEnv* jenv, jobject obj)
{
    LOG_INFO("nativeOnStop");
    delete myapp;
    myapp = 0;
    return;
}

JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface)
{
    if (surface != 0) {
        window = ANativeWindow_fromSurface(jenv, surface);
        LOG_INFO("Got window %p", window);
        myapp->setWindow(window);
    } else {
        LOG_INFO("Releasing window");
        ANativeWindow_release(window);
    }

    return;
}


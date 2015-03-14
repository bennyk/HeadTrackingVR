#ifndef JNIAPI_H
#define JNIAPI_H

extern "C" {
    JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeOnStart(JNIEnv* jenv, jobject obj);
    JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeOnResume(JNIEnv* jenv, jobject obj);
    JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeOnPause(JNIEnv* jenv, jobject obj);
    JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeOnStop(JNIEnv* jenv, jobject obj);
    JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface);
    JNIEXPORT void JNICALL Java_com_bennykhoo_vr_headtrackingvr_MainActivity_nativeSetLookAtAngles(JNIEnv* jenv, jobject obj, jfloat azimuth, jfloat pitch, jfloat roll);
};

#endif // JNIAPI_H


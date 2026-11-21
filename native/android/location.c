// <location> on Android: the externs Eskiu calls, into EskiuLocation (Java) through JNI.
#include <jni.h>
#include "ui_native.h"

static void call(const char *method, const char *signature, int precise) {
    JNIEnv *env = ui_jni_env();
    jclass type = ui_find_class("org.eskiu.location.EskiuLocation");
    if (!type) return;
    jmethodID id = (*env)->GetStaticMethodID(env, type, method, signature);
    if (signature[1] == 'Z') (*env)->CallStaticVoidMethod(env, type, id, (jboolean)(precise != 0));
    else (*env)->CallStaticVoidMethod(env, type, id);
    (*env)->DeleteLocalRef(env, type);
}

int location_native_supported(void) {
    JNIEnv *env = ui_jni_env();
    jclass type = ui_find_class("org.eskiu.location.EskiuLocation");
    if (!type) return 0;
    jboolean has = (*env)->CallStaticBooleanMethod(env, type, (*env)->GetStaticMethodID(env, type, "supported", "()Z"));
    (*env)->DeleteLocalRef(env, type);
    return has ? 1 : 0;
}

void location_start(int precise) { call("start", "(Z)V", precise); }

void location_stop(void) { call("stop", "()V", 0); }

﻿/**********************************
 Java Native Interface library
**********************************/
#include <jni.h>
#include "AnimatedModelTest.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/AndroidMountPath.h"

AnimatedModelTest* g_app = new AnimatedModelTest("animated model Test App");

/** This is the C++ implementation of the Java native method.
@param env Pointer to JVM environment
@param thiz Reference to Java this object
*/
extern "C"
JNIEXPORT void JNICALL
Java_com_AnimatedModelTestApp_AnimatedModelTestApp_initBridgeCallback(JNIEnv * env, jobject thiz)
{
    g_app->initBridgeCallback();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_AnimatedModelTestApp_AnimatedModelTestApp_initAdditionalMountPath(JNIEnv * env, jobject thiz)
{
}

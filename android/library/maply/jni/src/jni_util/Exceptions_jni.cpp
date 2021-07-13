/*  Exceptions_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 3/8/2021
 *  Copyright 2021-2021 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#import "Maply_jni.h"

namespace WhirlyKit {

// https://stackoverflow.com/a/10410117/135138
static void appendExceptionTraceMessages(
        JNIEnv*      env,
        std::ostringstream& msg,
        jthrowable   ex,
        jmethodID    throwable_getCause,
        jmethodID    throwable_getStackTrace,
        jmethodID    throwable_toString,
        jmethodID    frame_toString)
{
    if (!ex)
    {
        return;
    }

    // Get the array of StackTraceElements.
    const auto frames = (jobjectArray)env->CallObjectMethod(ex, throwable_getStackTrace);

    // Add Throwable.toString() before descending
    // stack trace messages.
    if (frames)
    {
        if (const auto msg_obj = (jstring)env->CallObjectMethod(ex, throwable_toString))
        {
            if (const char* msg_str = env->GetStringUTFChars(msg_obj, nullptr))
            {
                msg << msg_str;
                env->ReleaseStringUTFChars(msg_obj, msg_str);
            }
            env->DeleteLocalRef(msg_obj);
        }
    }

    // Append stack trace messages if there are any.
    const jsize frames_length = frames ? env->GetArrayLength(frames) : 0;
    for (jsize i = 0; i < frames_length; i++)
    {
        // Get the string returned from the 'toString()' method of the next frame and append it to the error message.
        if (const auto frame = env->GetObjectArrayElement(frames, i))
        {
            if (const auto msg_obj = (jstring) env->CallObjectMethod(frame, frame_toString))
            {
                if (const char* msg_str = env->GetStringUTFChars(msg_obj, nullptr))
                {
                    msg << "\n    ";
                    msg << msg_str;
                    env->ReleaseStringUTFChars(msg_obj, msg_str);
                }
                env->DeleteLocalRef(msg_obj);
            }
            env->DeleteLocalRef(frame);
        }
    }

    // If ot has a cause then append the stack trace messages from the cause.
    if (frames)
    {
        if (auto cause = (jthrowable)env->CallObjectMethod(ex, throwable_getCause))
        {
            appendExceptionTraceMessages(env, msg, cause, throwable_getCause,
                                         throwable_getStackTrace, throwable_toString,
                                         frame_toString);
        }
    }
}
static jmethodID mid_throwable_getCause = nullptr;
static jmethodID mid_throwable_getStackTrace = nullptr;
static jmethodID mid_throwable_toString = nullptr;
static jmethodID mid_frame_toString = nullptr;

static jthrowable makeThrowable(JNIEnv* env)
{
    auto tc = env->FindClass("java/lang/Throwable");
    auto ctor = env->GetMethodID(tc, "<init>", "()V");
    return (jthrowable)env->NewObject(tc,ctor);
}

void appendExceptionTraceMessages(JNIEnv* env, std::ostringstream& msg, jthrowable ex)
{
    if (!mid_throwable_getCause)
    {
        jclass throwable_class = env->FindClass("java/lang/Throwable");
        jclass frame_class = env->FindClass("java/lang/StackTraceElement");

        mid_throwable_getCause = env->GetMethodID(throwable_class, "getCause", "()Ljava/lang/Throwable;");
        mid_throwable_getStackTrace = env->GetMethodID(throwable_class, "getStackTrace", "()[Ljava/lang/StackTraceElement;");
        mid_throwable_toString = env->GetMethodID(throwable_class, "toString", "()Ljava/lang/String;");
        mid_frame_toString = env->GetMethodID(frame_class, "toString", "()Ljava/lang/String;");
    }
    appendExceptionTraceMessages(env, msg, ex, mid_throwable_getCause,
                                 mid_throwable_getStackTrace, mid_throwable_toString, mid_frame_toString);
}

std::string getExceptionTraceMessages(JNIEnv* env, jthrowable ex)
{
    std::ostringstream ss;
    appendExceptionTraceMessages(env, ss, ex);
    ss.flush();
    return ss.str();
}

void logJVMException(JNIEnv* env, jthrowable throwable, const char* where, android_LogPriority priority)
{
    const auto trace = getExceptionTraceMessages(env, throwable);
    __android_log_print(priority, "Maply", where ? "Exception in %s:\n%s" : "%s%s",
                        where ? where : "", trace.c_str());
}

bool logAndClearJVMException(JNIEnv* env, const char* where, android_LogPriority priority)
{
    if (auto ex = env->ExceptionOccurred())
    {
        env->ExceptionClear();  // exception must be cleared before making any more calls
        logJVMException(env,ex,where,priority);
        return true;
    }
    return false;
}

void logStackTrace(JNIEnv* env, const char* where, android_LogPriority priority)
{
    logJVMException(env,makeThrowable(env),where,priority);
}

std::string getStackTrace(JNIEnv* env)
{
    return getExceptionTraceMessages(env, makeThrowable(env));
}

}
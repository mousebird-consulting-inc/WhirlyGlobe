/*  Exceptions_jni.h
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

#import <jni.h>
#import <android/log.h>
#import <iosfwd>

namespace WhirlyKit {

/// Build a string from the throwable stack trace info, from ExceptionOccurred().
/// The exception must be cleared with env.ExceptionClear() first.
extern void appendExceptionTraceMessages(JNIEnv* env, std::ostringstream& msg, jthrowable ex);

/// Build a string from the throwable stack trace info, from ExceptionOccurred().
/// The exception must be cleared with env.ExceptionClear() first.
extern std::string getExceptionTraceMessages(JNIEnv* env, jthrowable ex);

extern void logJVMException(JNIEnv* env,
                            jthrowable throwable,
                            const char* where = nullptr,
                            android_LogPriority priority = ANDROID_LOG_ERROR);

extern bool logAndClearJVMException(JNIEnv* env,
                                    const char* where = nullptr,
                                    android_LogPriority = ANDROID_LOG_ERROR);

extern bool logStackTrace(JNIEnv* env,
                          const char* where = nullptr,
                          android_LogPriority = ANDROID_LOG_ERROR);

extern std::string getStackTrace(JNIEnv* env);

}
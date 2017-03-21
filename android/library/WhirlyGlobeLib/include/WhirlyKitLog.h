/*
 *  WhirlyKitLogs.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2013 mousebird consulting
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
 *
 */
#ifndef WhirlyKitLog_h
#define WhirlyKitLog_h

#define  LOG_TAG    "WhirlyKit"

#ifdef ANDROID
	#include <android/log.h>
	#define WHIRLYKIT_LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__)
	#define WHIRLYKIT_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG,__VA_ARGS__)
	#define WHIRLYKIT_LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__)
	#define WHIRLYKIT_LOGW(...) __android_log_print(ANDROID_LOG_WARN   , LOG_TAG,__VA_ARGS__)
	#define WHIRLYKIT_LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG,__VA_ARGS__)
	#define WHIRLYKIT_LOGSIMPLE LOGSIMPLE
#else
	#include <stdio.h>
	#define WHIRLYKIT_LOGV(...) do { printf("  ");printf(__VA_ARGS__); printf("\t -  <%s> \n", LOG_TAG); } while (0)
	#define WHIRLYKIT_LOGD(...) do { printf("  ");printf(__VA_ARGS__); printf("\t -  <%s> \n", LOG_TAG); } while (0)
	#define WHIRLYKIT_LOGI(...) do { printf("  ");printf(__VA_ARGS__); printf("\t -  <%s> \n", LOG_TAG); } while (0)
	#define WHIRLYKIT_LOGW(...) do { printf("  * Warning: "); printf(__VA_ARGS__); printf("\t -  <%s> \n", LOG_TAG); } while (0)
	#define WHIRLYKIT_LOGE(...) do { printf("  *** Error:  ");printf(__VA_ARGS__); printf("\t -  <%s> \n", LOG_TAG); } while (0)
	#define WHIRLYKIT_LOG(...) do { printf(" ");printf(__VA_ARGS__); } while (0)
#endif


#endif

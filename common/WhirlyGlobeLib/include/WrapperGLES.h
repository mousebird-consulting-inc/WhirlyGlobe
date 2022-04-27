/*  WrapperGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/18/13.
 *  Copyright 2011-2022 mousebird consulting
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

#ifdef __ANDROID__ 
# import <jni.h>
# import <android/log.h>
# if __ANDROID_API__ >= 24
#  import <GLES3/gl32.h>
# elif __ANDROID_API__ >= 21
#  import <GLES3/gl31.h>
# else
#  import <GLES3/gl3.h>
# endif
# import <EGL/egl.h>
#elif defined(CUSTOM_GL_WRAPPER)
# if !defined(STRINGIFY) && !defined(_STRINGIFY)
#  define STRINGIFY_(x) #x
#  define STRINGIFY(x) STRINGIFY_(x)
# endif
# import STRINGIFY(CUSTOM_GL_WRAPPER)
#else
# error "Unsupported environment"
#endif

extern const bool hasVertexArraySupport;
extern const bool hasSharedBufferSupport;
extern const bool hasMapBufferSupport;
extern const bool invalidateGLDepth;

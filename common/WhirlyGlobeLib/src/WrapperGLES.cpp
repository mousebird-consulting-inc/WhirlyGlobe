/*  WrapperGLES.cpp
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

#import "WrapperGLES.h"

#if defined(HAS_GL_VERT_ARRAY) && \
    defined(HAS_GL_MAP_BUFFER) && \
    defined(INVALIDATE_GL_DEPTH)
    const bool hasVertexArraySupport = HAS_GL_VERT_ARRAY;
    const bool hasSharedBufferSupport = HAS_GL_SHARED_BUFFER;
    const bool hasMapBufferSupport = HAS_GL_MAP_BUFFER;
    const bool invalidateGLDepth = INVALIDATE_GL_DEPTH;
#elif __ANDROID__
    const bool hasVertexArraySupport = false;
    const bool hasSharedBufferSupport = true;
    const bool hasMapBufferSupport = false;
    const bool invalidateGLDepth = false;
#elif defined(iOS)
    const bool hasVertexArraySupport = true;
    const bool hasSharedBufferSupport = true;
    const bool hasMapBufferSupport = true;
    const bool invalidateGLDepth = true;
#else
# error Unsupported Platform
#endif

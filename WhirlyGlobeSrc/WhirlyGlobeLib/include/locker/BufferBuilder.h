/*
 *  BufferBuilder.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/21/12.
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

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <set>
#import "Drawable.h"

namespace WhirlyKit
{
    
// This is as many vertices as we can address, if it's just vertices
#define MaxBuilderBufferSize (28*65536)

/// Used to build up shared GL buffers.
/// This version can only add and builds buffers as it goes.
class BufferBuilder
{
public:
    BufferBuilder(unsigned int maxBufferSize=MaxBuilderBufferSize);
    
    /// If we're counting size, add this in
    void addToTotal(GLuint size);
    
    /// Add in the size for this drawable
    void addToTotal(BasicDrawable *drawable);
    
    /// Tack the given drawable on to our buffer(s)
    void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager,BasicDrawable *drawable);
    
    /// Return the buffers allocated thus far
    std::vector<GLuint> getBuffers() { return buffers; }
    
protected:
    /// Total amount of data we're expecting (if provided)
    unsigned int totalSize;
    /// Maximum size of a single buffer
    unsigned int maxBufferSize;
    // Current buffer
    GLuint curBuf;
    // Location in current buffer
    int curBufLoc;
    // Space allocated so far
    int allocSoFar;
    // Buffers allocated so far
    std::vector<GLuint> buffers;
};

}

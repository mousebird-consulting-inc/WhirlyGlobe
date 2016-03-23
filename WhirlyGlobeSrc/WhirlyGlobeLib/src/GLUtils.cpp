/*
 *  GLUtils.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/21/11.
 *  Copyright 2011-2016 mousebird consulting
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

#import <stdio.h>
#import "GLUtils.h"

// Turn this off for a little performance gain
#if DEBUG || __ANDROID__
static bool ErrorsOn = true;
#else
static bool ErrorsOn = false;
#endif
bool CheckGLError(const char *msg)
{
    if (!ErrorsOn)
        return true;
    GLenum theError = glGetError();
    if (theError != GL_NO_ERROR)
    {
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_ERROR, "Maply", "GL Error: %d - %s",theError,msg);
#else
        fprintf(stderr,"GL Error: %d - %s",theError,msg);
#endif
        // Note: Porting
//        NSLog(@"GL Error: %d - %s",theError,msg);
        return false;
    }
    
    return true;
}

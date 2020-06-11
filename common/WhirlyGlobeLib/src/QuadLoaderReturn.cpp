/*
 *  QuadDisplayControllerNew.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/14/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "QuadLoaderReturn.h"

namespace WhirlyKit
{

QuadFrameInfo::QuadFrameInfo()
: frameIndex(-1)
{
}
    
QuadLoaderReturn::QuadLoaderReturn(int generation)
    : ident(0,0,0), hasError(false), generation(generation), frame(new QuadFrameInfo())
{
    frame->frameIndex = -1;
}

QuadLoaderReturn::~QuadLoaderReturn()
{
}
    
void QuadLoaderReturn::clear()
{
    images.clear();
    compObjs.clear();
    ovlCompObjs.clear();
}
    
}


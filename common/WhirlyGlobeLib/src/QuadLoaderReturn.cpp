/*  QuadDisplayControllerNew.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/14/19.
 *  Copyright 2011-2023 mousebird consulting
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

#import "QuadLoaderReturn.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

QuadLoaderReturn::QuadLoaderReturn(int generation) :
    ident(0,0,0),
    frame(std::make_shared<QuadFrameInfo>()),
    generation(generation)
{
}

QuadLoaderReturn::~QuadLoaderReturn()
{
    if (!changes.empty())
    {
        wkLogLevel(Warn, "LoaderReturn destroyed with %lld pending changes", changes.size());
    }
}

void QuadLoaderReturn::clear()
{
    cancel = true;
    images.clear();
    compObjs.clear();
    ovlCompObjs.clear();
    
    // Note: changes are not cleared, they have to be deleted and should be handled elsewhere
}

}


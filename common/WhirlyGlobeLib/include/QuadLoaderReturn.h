/*
 *  QuadDisplayControllerNew.h
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

#import "WhirlyVector.h"
#import "Scene.h"
#import "GlobeMath.h"
#import "ComponentManager.h"
#import "ImageTile.h"
#import "QuadTreeNew.h"

namespace WhirlyKit
{

// Base class for the FrameInfo, which we don't make strong use of in the base
class QuadFrameInfo : public WhirlyKit::Identifiable
{
public:
    QuadFrameInfo();
    
    // Either the position in the frame list or -1 if there's just one frame
    int frameIndex;
};
typedef std::shared_ptr<QuadFrameInfo> QuadFrameInfoRef;

/** Quad Loader Return wrapper for data coming back
  */
class QuadLoaderReturn
{
public:
    QuadLoaderReturn(int generation);
    virtual ~QuadLoaderReturn();
    
    // Which node this is in the quad tree
    QuadTreeIdentifier ident;
    
    // Frame identifier
    QuadFrameInfoRef frame;
        
    // Any images that may have been added, in whatever state
    std::vector<ImageTileRef> images;

    // Regular component objects added for a tile
    std::vector<ComponentObjectRef> compObjs;

    // Overlay component objects added for a tile
    std::vector<ComponentObjectRef> ovlCompObjs;
    
    // If we make changes directly with the managers, they are reflected in this change set
    ChangeSet changes;
    
    // The generation associated with the loader.
    // We use this to catch lagging loads after a reload
    int generation;
    
    // Set if something went wrong with loading
    bool hasError;
    
    // Clean out references to everything
    virtual void clear();
};
    
typedef std::shared_ptr<QuadLoaderReturn> QuadLoaderReturnRef;
    
}

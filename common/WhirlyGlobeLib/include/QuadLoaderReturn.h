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

/** Quad Loader Return wrapper for data coming back
  */
class QuadLoaderReturn
{
public:
    QuadLoaderReturn();
    virtual ~QuadLoaderReturn();
    
    // Which node this is in the quad tree
    QuadTreeIdentifier ident;
    
    // Frame, or -1 if there are no frames
    int frame;
    
    // Unparsed data coming back from the
    std::vector<RawDataRef> tileData;
    
    // Any images that my have been added, in whatever state
    std::vector<ImageTileRef> images;

    // Regular component objects added for a tile
    std::vector<ComponentObjectRef> compObjs;

    // Overlay component objects added for a tile
    std::vector<ComponentObjectRef> ovlCompObjs;
    
    // Set if something went wrong with loading
    bool hasError;
    
    // Clean out references to everything
    void clear();
};
    
typedef std::shared_ptr<QuadLoaderReturn> QuadLoaderReturnRef;
    
}

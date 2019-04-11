/*
 *  ImageTile.h
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

#import <math.h>
#import "WhirlyVector.h"
#import "Texture.h"

namespace WhirlyKit
{

/** Image Tile representation.
 
    Used to wrap data we're going to turn into a Texture.
    Subclass does most of the work.
  */
class ImageTile
{
public:
    ImageTile();
    ImageTile(const std::string &name);
    virtual ~ImageTile();
    
    /// Construct and return a texture, if possible.
    virtual Texture *buildTexture() = 0;
    
    /// Stop keeping track of texture if you were
    virtual void clearTexture() = 0;
    
public:
    // Optional name.  Not always set.
    std::string name;
    int borderSize;
    int width,height,components;
    int targetWidth,targetHeight;
};

typedef std::shared_ptr<ImageTile> ImageTileRef;

}

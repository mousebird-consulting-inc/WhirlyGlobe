/*
 *  ImageTile_iOS.h
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

#import "WhirlyGlobe.h"
#import "SceneRenderer.h"

namespace WhirlyKit
{

typedef enum {MaplyImgTypeImage,MaplyImgTypeDataUIKitRecognized,MaplyImgTypeDataPKM,MaplyImgTypeDataPVRTC4,MaplyImgTypeRawImage} MaplyImgType;

/** ImageTile (iOS) Version
    This bridges the gap between ImageTile (and texture construction)
    and the various iOS specific data formats.
  */
class ImageTile_iOS : public ImageTile
{
public:
    ImageTile_iOS(SceneRenderer::Type renderType);
    virtual ~ImageTile_iOS();
    
    /// Construct and return a texture, if possible.
    virtual Texture *buildTexture();
    
    /// Generate the texture and then store it
    Texture *prebuildTexture();
    
    /// Stop keeping track of texture if you were
    virtual void clearTexture();
    
public:
    SceneRenderer::Type renderType;
    MaplyImgType type;
    
    // The NSData or UIImage or whatever
    id imageStuff;
    
    Texture *tex;
};
    
typedef std::shared_ptr<ImageTile_iOS> ImageTile_iOSRef;
    
}

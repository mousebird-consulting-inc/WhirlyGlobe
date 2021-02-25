/*
 *  ImageTile_Android.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/19.
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

#ifdef __ANDROID__
#import <jni.h>
#endif
#import "Maply_jni.h"
#import "WhirlyGlobe.h"

namespace WhirlyKit
{

typedef enum {MaplyImgTypeNone,MaplyImgTypeDataPKM,MaplyImgTypeDataPVRTC4,MaplyImgTypeRawImage} MaplyImgType;

/**
    Android version of the Image Tile.
    Some specific Android stuff, mostly involving Bitmap.
 **/
class ImageTile_Android : public ImageTile
{
public:
    ImageTile_Android();
    virtual ~ImageTile_Android();

    /// Scoop the contents out of a Bitmap
    void setBitmap(JNIEnv *env,jobject bitmapObj);

    /// Construct and return a texture suitable for the renderer
    virtual Texture *buildTexture();

    // Build and cache the texture for later
    Texture *prebuildTexture();

    /// Stop keeping track of texture if you were
    virtual void clearTexture();

public:
    MaplyImgType type;
    Texture *tex;
    RawDataRef rawData;
};

typedef std::shared_ptr<ImageTile_Android> ImageTile_AndroidRef;

}

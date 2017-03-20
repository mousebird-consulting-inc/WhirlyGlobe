/*
 *  ImageTexture_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2015 mousebird consulting
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

#import <WhirlyGlobe.h>
#import "MaplyTexture_private.h"

// Used to map UIImages to Texture IDs
#if 0
class MaplyImageTexture
{
public:
    MaplyImageTexture() : image(nil), maplyTexPtr(nil), refCount(0) { }
    MaplyImageTexture(id image) : image(image), maplyTexPtr(nil), refCount(0) { }
    MaplyImageTexture(id image,MaplyTexture *maplyTex) : image(image), maplyTexPtr(maplyTex), refCount(0) {  }
    MaplyImageTexture(const MaplyImageTexture &that) : image(that.image), maplyTexPtr(that.maplyTexPtr), refCount(that.refCount) { }
    bool operator < (const MaplyImageTexture &that) const
    {
        if (!image && !that.image)
            return maplyTexAddr < that.maplyTexAddr;
        return image < that.image;
    }

    id __weak image;
    long imageAddr;
    MaplyTexture * __weak maplyTexPtr;
    int refCount;
};
#endif

typedef std::list<MaplyTexture __weak *> MaplyImageTextureList;

/*
 *  ImageTexture_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2012 mousebird consulting
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

// Used to map UIImages to Texture IDs
class MaplyImageTexture
{
public:
    MaplyImageTexture(UIImage *image) : image(image), texID(WhirlyKit::EmptyIdentity), refCount(0) { }
    MaplyImageTexture(UIImage *image,WhirlyKit::SimpleIdentity texID) : image(image), texID(texID), refCount(0) { }
    MaplyImageTexture(const MaplyImageTexture &that) : image(that.image), texID(that.texID), refCount(that.refCount) { }
    bool operator < (const MaplyImageTexture &that) const { return image < that.image; }

    UIImage *image;
    WhirlyKit::SimpleIdentity texID;
    int refCount;
};
typedef std::set<MaplyImageTexture> MaplyImageTextureSet;

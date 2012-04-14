/*
 *  GlobeScene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
 *  Copyright 2011 mousebird consulting
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

#import "GlobeScene.h"
#import "GlobeView.h"
#import "GlobeMath.h"
#import "TextureAtlas.h"

namespace WhirlyGlobe
{
    
using namespace WhirlyKit;
    
GlobeScene::GlobeScene(unsigned int numX,unsigned int numY,WhirlyKit::CoordSystem *coordSystem)
    : Scene(numX,numY,coordSystem)
{
    
}

// Return a list of overlapping cullables, given the geo MBR
// Note: This could be a lot smarter
void GlobeScene::overlapping(GeoMbr geoMbr,std::vector<Cullable *> &foundCullables)
{
    foundCullables.clear();
    for (unsigned int ii=0;ii<numX*numY;ii++)
    {
        Cullable *cullable = &cullables[ii];
        if (geoMbr.overlaps(cullable->geoMbr))
            foundCullables.push_back(cullable);
    }
}
    
void GlobeScene::addDrawable(Drawable *drawable)
{
    drawables.insert(drawable);

    // Sort into cullables
    // Note: Need a more selective MBR check.  We're going to catch edge overlaps
    std::vector<Cullable *> foundCullables;
    overlapping(drawable->getGeoMbr(),foundCullables);
    for (unsigned int ci=0;ci<foundCullables.size();ci++)
        foundCullables[ci]->addDrawable(drawable);
}


}
/*
 *  GlobeScene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
 *  Copyright 2011-2017 mousebird consulting
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

using namespace WhirlyKit;

namespace WhirlyGlobe
{
        
GlobeScene::GlobeScene(CoordSystemDisplayAdapter *inCoordAdapter)
{
    Init(inCoordAdapter,GeoMbr(GeoCoord::CoordFromDegrees(-180,-90),GeoCoord::CoordFromDegrees(180,90)));
}

GlobeScene::~GlobeScene()
{
}
    
void GlobeScene::addDrawable(DrawableRef draw)
{
    drawables[draw->getId()] = draw;
}

void GlobeScene::remDrawable(DrawableRef draw)
{
    auto it = drawables.find(draw->getId());
    if (it != drawables.end())
        drawables.erase(it);
}

}

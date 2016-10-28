/*
 *  MaplyScene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/11/12.
 *  Copyright 2011-2013 mousebird consulting
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

#import "WhirlyKitLog.h"
#import "MaplyScene.h"

using namespace WhirlyKit;

namespace Maply
{
    
MapScene::MapScene(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter)
{
    Init(coordAdapter,GeoMbr(GeoCoord::CoordFromDegrees(-180,-90),GeoCoord::CoordFromDegrees(180,90)),1);
}
    
void MapScene::addDrawable(DrawableRef draw)
{
    drawables.insert(draw);
    
    // Dump it in the top level for now
    Mbr localMbr = draw->getLocalMbr();
    cullTree->getTopCullable()->addDrawable(cullTree, localMbr, draw);
}

void MapScene::remDrawable(DrawableRef draw)
{
    // We're expecting it to just be at the top level
    Mbr localMbr = draw->getLocalMbr();
    cullTree->getTopCullable()->remDrawable(cullTree, localMbr, draw);
    
    drawables.erase(draw);
}
    
}

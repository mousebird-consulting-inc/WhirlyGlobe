/*
 *  GlobeScene.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
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

#import "GlobeScene.h"
#import "GlobeView.h"
#import "GlobeMath.h"
// Note: Porting
//#import "TextureAtlas.h"

using namespace WhirlyKit;

namespace WhirlyGlobe
{
        
GlobeScene::GlobeScene(CoordSystemDisplayAdapter *inCoordAdapter,int depth)
{
    Init(inCoordAdapter,GeoMbr(GeoCoord::CoordFromDegrees(-180,-90),GeoCoord::CoordFromDegrees(180,90)),depth-1);
}

GlobeScene::~GlobeScene()
{
}
    
void GlobeScene::addDrawable(DrawableRef draw)
{
    drawables.insert(draw);

    // Account for the geo coordinate wrapping
    Mbr localMbr = draw->getLocalMbr();
    if (localMbr.valid())
    {
        GeoMbr geoMbr(GeoCoord(localMbr.ll().x(),localMbr.ll().y()),GeoCoord(localMbr.ur().x(),localMbr.ur().y()));
        std::vector<Mbr> localMbrs;
        geoMbr.splitIntoMbrs(localMbrs);
        
        for (unsigned int ii=0;ii<localMbrs.size();ii++)
            cullTree->getTopCullable()->addDrawable(cullTree,localMbrs[ii],draw);
    } else
        cullTree->getTopCullable()->addDrawable(cullTree, localMbr, draw);
}

void GlobeScene::remDrawable(DrawableRef draw)
{
    Mbr localMbr = draw->getLocalMbr();
    GeoMbr geoMbr(GeoCoord(localMbr.ll().x(),localMbr.ll().y()),GeoCoord(localMbr.ur().x(),localMbr.ur().y()));
    std::vector<Mbr> localMbrs;
    geoMbr.splitIntoMbrs(localMbrs);
    
    for (unsigned int ii=0;ii<localMbrs.size();ii++)
        cullTree->getTopCullable()->remDrawable(cullTree,localMbrs[ii],draw);

    drawables.erase(draw);
}

}

/*  QuadSamplingParams.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/14/19.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "QuadSamplingParams.h"

namespace WhirlyKit
{

void SamplingParams::setCoordSys(CoordSystemRef newSys)
{
    coordSys = std::move(newSys);

    if (coordSys)
    {
        // Bounds are in projected coordinates
        coordBounds = MbrD(coordSys->getBoundsLocal());
    }
    else
    {
        coordBounds.reset();
    }
}

bool SamplingParams::operator == (const SamplingParams &that) const
{
    if (!coordSys && !that.coordSys)
        return true;
    if ((!coordSys && that.coordSys) ||
        (coordSys && !that.coordSys))
        return false;
    
    if (!coordSys->isSameAs(that.coordSys.get()))
        return false;
    
    return minZoom == that.minZoom && maxZoom == that.maxZoom && reportedMaxZoom == that.reportedMaxZoom &&
        maxTiles == that.maxTiles &&
        minImportance == that.minImportance && minImportanceTop == that.minImportanceTop &&
        coverPoles == that.coverPoles && edgeMatching == that.edgeMatching &&
        tessX == that.tessX && tessY == that.tessY &&
        singleLevel == that.singleLevel &&
        boundsScale == that.boundsScale &&
        forceMinLevel == that.forceMinLevel &&
        forceMinLevelHeight == that.forceMinLevelHeight &&
        clipBounds == that.clipBounds &&
        useClipBoundsForImportance == that.useClipBoundsForImportance &&
        generateGeom == that.generateGeom &&
        levelLoads == that.levelLoads &&
        importancePerLevel == that.importancePerLevel;
}
    
void SamplingParams::setImportanceLevel(double theMinImportance,int level)
{
    if (level >= importancePerLevel.size()) {
        importancePerLevel.resize(level+1,-2.0);
    }
    importancePerLevel[level] = theMinImportance;
}
    
}

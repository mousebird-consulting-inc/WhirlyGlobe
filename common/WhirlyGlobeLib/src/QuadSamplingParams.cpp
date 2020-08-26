/*
 *  QuadSamplingParams.cpp
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

#import "QuadSamplingParams.h"

namespace WhirlyKit
{

SamplingParams::SamplingParams()
    : coordSys(NULL),
    minZoom(0), maxZoom(0), reportedMaxZoom(-1),
    maxTiles(128),
    minImportance(256*256), minImportanceTop(0.0),
    coverPoles(true), edgeMatching(true),
    tessX(10), tessY(10),
    singleLevel(false),
    forceMinLevel(true),
    forceMinLevelHeight(0.0),
    generateGeom(true)
{
}

SamplingParams::~SamplingParams()
{
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
        forceMinLevel == that.forceMinLevel &&
        forceMinLevelHeight == that.forceMinLevelHeight &&
        clipBounds == that.clipBounds &&
        generateGeom == that.generateGeom &&
        levelLoads == that.levelLoads &&
        importancePerLevel == that.importancePerLevel;
}
    
void SamplingParams::setImportanceLevel(double minImportance,int level)
{
    if (level >= importancePerLevel.size()) {
        importancePerLevel.resize(level+1,-2.0);
    }
    importancePerLevel[level] = minImportance;
}
    
}

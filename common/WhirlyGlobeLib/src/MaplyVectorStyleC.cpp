/*
*  MaplyVectorStyleC.cpp
*  WhirlyGlobe-MaplyComponent
*
*  Created by Steve Gifford on 4/9/20.
*  Copyright 2011-2020 mousebird consulting
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

#import "MaplyVectorStyleC.h"

namespace WhirlyKit
{

VectorStyleSettingsImpl::VectorStyleSettingsImpl(double scale)
{
    lineScale = scale;
    textScale = scale;
    markerScale = scale;
    markerImportance = 2.0;
    markerSize = 10.0;
    labelImportance = 1.5;
    useZoomLevels = false;
    baseDrawPriority = 0;
    drawPriorityPerLevel = 0;
    mapScaleScale = 1.0;
    dashPatternScale = 1.0;
    useWideVectors = false;
    oldVecWidthScale = 1.0;
    wideVecCuttoff = 0.0;
    selectable = false;
    settingsArealShaderID = EmptyIdentity;
}

}

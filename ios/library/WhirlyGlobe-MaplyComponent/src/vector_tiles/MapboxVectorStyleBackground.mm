/*
 *  MapboxVectorStyleBackground.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
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

#import "private/MapboxVectorStyleBackground_private.h"
#import "MapboxVectorStyleSet_private.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

bool MapboxVectorBackgroundPaint::parse(MapboxVectorStyleSetImplRef styleSet,DictionaryRef styleEntry)
{
    color = styleSet->transColor("background-color",styleEntry,RGBAColor::black());
    if (styleEntry->hasField("background-image"))
        wkLogLevel(Warn,"MapboxStyleSet: Ignoring background image");

    opacity = styleSet->transDouble("background-opacity",styleEntry,1.0);
    
    return true;
}

bool MapboxVectorLayerBackground::parse(MapboxVectorStyleSetImplRef styleSet,
                                        DictionaryRef styleEntry,
                                        MapboxVectorStyleLayerRef refLayer,
                                        int drawPriority)
{
    if (!MapboxVectorStyleLayer::parse(styleSet,styleEntry,refLayer,drawPriority)) {
        return false;
    }
    
    if (styleEntry->hasField("layout"))
        wkLogLevel(Warn,"MapboxStyleSet: Ignoring background layout");
    
    if (!paint.parse(styleSet,styleEntry->getDict("paint"))) {
        return false;
    }
    
    // Mess directly with the opacity because we're using it for other purposes
    if (styleEntry->hasField("alphaoverride")) {
        paint.color->setAlphaOverride(styleEntry->getDouble("alphaoverride"));
    }
    
    return true;
}

void MapboxVectorLayerBackground::buildObjects(std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo)
{
    // Nothing to build
}

}

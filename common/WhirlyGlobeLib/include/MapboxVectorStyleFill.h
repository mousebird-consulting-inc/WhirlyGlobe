/*
 *  MapboxVectorStyleFill.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
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

#import "MapboxVectorStyleSetC.h"
#import "MapboxVectorStyleLayer.h"

namespace WhirlyKit
{

// Paint for the vector layer fill
class MapboxVectorFillPaint
{
public:
    bool parse(PlatformThreadInfo *inst,
               MapboxVectorStyleSetImpl *styleSet,
               DictionaryRef styleEntry);

    MapboxTransDoubleRef opacity;
    MapboxTransColorRef color;
    MapboxTransColorRef outlineColor;
};

// Polygon fill layer
class MapboxVectorLayerFill : public MapboxVectorStyleLayer
{
public:
    MapboxVectorLayerFill(MapboxVectorStyleSetImpl *styleSet) : MapboxVectorStyleLayer(styleSet) { }
    
    virtual bool parse(PlatformThreadInfo *inst,
                       DictionaryRef styleEntry,
                       MapboxVectorStyleLayerRef refLayer,
                       int drawPriority);
    
    virtual void buildObjects(PlatformThreadInfo *inst,
                              std::vector<VectorObjectRef> &vecObjs,
                              VectorTileDataRef tileInfo);
    
    virtual void cleanup(PlatformThreadInfo *inst,ChangeSet &changes);
    
public:
    MapboxVectorFillPaint paint;
    SimpleIdentity arealShaderID;
};

}

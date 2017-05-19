/*
 *  MaplyVectorStyle.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
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

#import "MapboxVectorTiles.h"
#import "MaplyVectorStyle.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@implementation MaplyVectorStyleSettings

- (instancetype)init
{
    self = [super init];
    _lineScale = 1.0;
    _textScale = 1.0;
    _markerScale = 1.0;
    _markerImportance = 2.0;
    _markerSize = 10.0;
    _mapScaleScale = 1.0;
    _dashPatternScale = 1.0;
    _useWideVectors = false;
    _wideVecCuttoff = 0.0;
    _oldVecWidthScale = 1.0;
    _selectable = false;
  
    return self;
}

- (NSString*)description
{
  return [NSString stringWithFormat:@"%@: lineScale:%f textScale:%f markerScale:%f mapScaleScale:%f",
          [[self class] description], _lineScale, _textScale, _markerScale, _mapScaleScale];
}

@end

NSArray * _Nonnull AddMaplyVectorsUsingStyle(NSArray * _Nonnull vecObjs,NSObject<MaplyVectorStyleDelegate> * _Nonnull styleDelegate,MaplyBaseViewController * _Nonnull viewC,MaplyThreadMode threadMode)
{
    NSMutableArray *compObjs = [NSMutableArray array];
    MaplyTileID fakeTileID;
    fakeTileID.x = 0;  fakeTileID.y = 0;  fakeTileID.level = 0;
    NSMutableDictionary *featureStyles = [[NSMutableDictionary alloc] init];
    
    // First we sort by the styles that each feature uses.
    int whichLayer = 0;
    for (MaplyVectorObject *thisVecObj in vecObjs)
    {
        for (MaplyVectorObject *vecObj in [thisVecObj splitVectors])
        {
            NSString *layer = vecObj.attributes[@"layer"];
            if (!layer)
                layer = vecObj.attributes[@"layer_name"];
            if (layer && ![styleDelegate layerShouldDisplay:layer tile:fakeTileID])
                continue;
            
            if (!layer)
                layer = [NSString stringWithFormat:@"layer%d",whichLayer];
            
            // Need to set a geometry type
            MapnikGeometryType geomType = GeomTypeUnknown;
            switch ([vecObj vectorType])
            {
                case MaplyVectorPointType:
                    geomType = GeomTypePoint;
                    break;
                case MaplyVectorLinearType:
                case MaplyVectorLinear3dType:
                    geomType = GeomTypeLineString;
                    break;
                case MaplyVectorArealType:
                    geomType = GeomTypePolygon;
                    break;
                case MaplyVectorMultiType:
                    break;
                default:
                    break;
            }
            vecObj.attributes[@"geometry_type"] = @(geomType);
            
            NSArray *styles = [styleDelegate stylesForFeatureWithAttributes:vecObj.attributes onTile:fakeTileID inLayer:layer viewC:viewC];
            if (styles.count == 0)
                continue;
            
            for (NSObject<MaplyVectorStyle> *style in styles)
            {
                NSMutableArray *featuresForStyle = featureStyles[style.uuid];
                if(!featuresForStyle) {
                    featuresForStyle = [NSMutableArray new];
                    featureStyles[style.uuid] = featuresForStyle;
                }
                [featuresForStyle addObject:vecObj];
            }
        }
        
        whichLayer++;
    }

    // Then we add each of those styles as a group for efficiency
    NSArray *symbolizerKeys = [featureStyles.allKeys sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"self" ascending:YES]]];
    for(id key in symbolizerKeys) {
        NSObject<MaplyVectorStyle> *symbolizer = [styleDelegate styleForUUID:key viewC:viewC];
        NSArray *features = featureStyles[key];
        NSArray *newCompObjs = [symbolizer buildObjects:features forTile:fakeTileID viewC:viewC];
        if (newCompObjs.count > 0)
            [compObjs addObjectsFromArray:newCompObjs];
    }

    return compObjs;
}

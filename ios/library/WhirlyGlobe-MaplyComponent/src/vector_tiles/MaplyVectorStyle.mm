/*
 *  MaplyVectorStyle.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
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

#import "vector_tiles/MapboxVectorTiles.h"
#import "vector_styles/MaplyVectorStyle.h"
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
    _labelImportance = 1.5;
    _markerSize = 10.0;
    _mapScaleScale = 1.0;
    _dashPatternScale = 1.0;
    _useWideVectors = false;
    _wideVecCuttoff = 0.0;
    _oldVecWidthScale = 1.0;
    _selectable = false;
    _baseDrawPriority = kMaplyVectorDrawPriorityDefault;
    _drawPriorityPerLevel = 0;
  
    return self;
}

- (instancetype)initWithScale:(CGFloat)scale
{
    self = [super init];
    _lineScale = scale;
    _textScale = scale;
    _markerScale = scale;
    _markerImportance = 2.0;
    _labelImportance = 1.5;
    _markerSize = 10.0;
    _mapScaleScale = 1.0;
    _dashPatternScale = 1.0;
    _useWideVectors = false;
    _wideVecCuttoff = 0.0;
    _oldVecWidthScale = 1.0;
    _selectable = false;
    _baseDrawPriority = kMaplyVectorDrawPriorityDefault;
    _drawPriorityPerLevel = 0;

    return self;
}

- (NSString*)description
{
  return [NSString stringWithFormat:@"%@: lineScale:%f textScale:%f markerScale:%f mapScaleScale:%f",
          [[self class] description], _lineScale, _textScale, _markerScale, _mapScaleScale];
}

@end

NSArray * _Nonnull AddMaplyVectorsUsingStyle(NSArray * _Nonnull vecObjs,NSObject<MaplyVectorStyleDelegate> * _Nonnull styleDelegate,NSObject<MaplyRenderControllerProtocol> * _Nonnull viewC,MaplyThreadMode threadMode)
{
    MaplyTileID tileID = {0, 0, 0};
    MaplyBoundingBoxD geoBBox;
    geoBBox.ll.x = -M_PI;  geoBBox.ll.y = -M_PI/2.0;
    geoBBox.ur.x = M_PI; geoBBox.ur.y = M_PI/2.0;
    MaplyVectorTileData *tileData = [[MaplyVectorTileData alloc] initWithID:tileID bbox:geoBBox geoBBox:geoBBox];
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
            if (layer && ![styleDelegate layerShouldDisplay:layer tile:tileData.tileID])
                continue;
            
            if (!layer)
                layer = [NSString stringWithFormat:@"layer%d",whichLayer];
            
            // Need to set a geometry type
            MapboxGeometryType geomType = MapboxGeometryType::GeomTypeUnknown;
            switch ([vecObj vectorType])
            {
                case MaplyVectorPointType:
                    geomType = MapboxGeometryType::GeomTypePoint;
                    break;
                case MaplyVectorLinearType:
                case MaplyVectorLinear3dType:
                    geomType = MapboxGeometryType::GeomTypeLineString;
                    break;
                case MaplyVectorArealType:
                    geomType = MapboxGeometryType::GeomTypePolygon;
                    break;
                case MaplyVectorMultiType:
                    break;
                default:
                    break;
            }
            vecObj.attributes[@"geometry_type"] = @(geomType);
            
            NSArray *styles = [styleDelegate stylesForFeatureWithAttributes:vecObj.attributes onTile:tileData.tileID inLayer:layer viewC:viewC];
            if (styles.count == 0)
                continue;
            
            for (NSObject<MaplyVectorStyle> *style in styles)
            {
                NSMutableArray *featuresForStyle = featureStyles[@(style.uuid)];
                if(!featuresForStyle) {
                    featuresForStyle = [NSMutableArray new];
                    featureStyles[@(style.uuid)] = featuresForStyle;
                }
                [featuresForStyle addObject:vecObj];
            }
        }
        
        whichLayer++;
    }

    // Then we add each of those styles as a group for efficiency
    NSArray *symbolizerKeys = [featureStyles.allKeys sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"self" ascending:YES]]];
    for(id key in symbolizerKeys) {
        NSObject<MaplyVectorStyle> *symbolizer = [styleDelegate styleForUUID:[key longLongValue] viewC:viewC];
        NSArray *features = featureStyles[key];
        [symbolizer buildObjects:features forTile:tileData viewC:viewC];
    }

    return [tileData componentObjects];
}

@implementation MaplyVectorTileInfo
@end

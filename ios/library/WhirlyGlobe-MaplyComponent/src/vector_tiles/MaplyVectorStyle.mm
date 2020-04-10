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
#import "private/MaplyVectorStyle_private.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@implementation MaplyVectorStyleSettings

- (instancetype)init
{
    return [self initWithScale:[UIScreen mainScreen].scale];
}

- (instancetype)initWithScale:(CGFloat)scale
{
    self = [super init];

    impl = VectorStyleSettingsImplRef(new VectorStyleSettingsImpl(scale));
    impl->baseDrawPriority = kMaplyVectorDrawPriorityDefault;

    return self;
}

- (void)setLineScale:(float)lineScale
{
    impl->lineScale = lineScale;
}

- (float)lineScale
{
    return impl->lineScale;
}

- (void)setTextScale:(float)textScale
{
    impl->textScale = textScale;
}

- (float)textScale
{
    return impl->textScale;
}

- (void)setMarkerScale:(float)markerScale
{
    impl->markerScale = markerScale;
}

- (float)markerScale
{
    return impl->markerScale;
}

- (void)setMarkerImportance:(float)markerImportance
{
    impl->markerImportance = markerImportance;
}

- (float)markerImportance
{
    return impl->markerImportance;
}

- (void)setMarkerSize:(float)markerSize
{
    impl->markerSize = markerSize;
}

- (float)markerSize
{
    return impl->markerSize;
}

- (void)setLabelImportance:(float)labelImportance
{
    impl->labelImportance = labelImportance;
}

- (float)labelImportance
{
    return impl->labelImportance;
}

- (void)setUseZoomLevels:(bool)useZoomLevels
{
    impl->useZoomLevels = useZoomLevels;
}

- (bool)useZoomLevels
{
    return impl->useZoomLevels;
}

- (void)setUuidField:(NSString *)uuidField
{
    if (uuidField)
        impl->uuidField = [uuidField cStringUsingEncoding:NSASCIIStringEncoding];
    else
        impl->uuidField.clear();
}

- (NSString *)uuidField
{
    if (impl->uuidField.empty())
        return nil;
    return [NSString stringWithCString:impl->uuidField.c_str() encoding:NSASCIIStringEncoding];
}

- (void)setBaseDrawPriority:(int)baseDrawPriority
{
    impl->baseDrawPriority = baseDrawPriority;
}

- (int)baseDrawPriority
{
    return impl->baseDrawPriority;
}

- (void)setDrawPriorityPerLevel:(int)drawPriorityPerLevel
{
    impl->drawPriorityPerLevel = drawPriorityPerLevel;
}

- (int)drawPriorityPerLevel
{
    return impl->drawPriorityPerLevel;
}

- (void)setMapScaleScale:(float)mapScaleScale
{
    impl->mapScaleScale = mapScaleScale;
}

- (float)mapScaleScale
{
    return impl->mapScaleScale;
}

- (void)setDashPatternScale:(float)dashPatternScale
{
    impl->dashPatternScale = dashPatternScale;
}

- (float)dashPatternScale
{
    return impl->dashPatternScale;
}

- (void)setUseWideVectors:(bool)useWideVectors
{
    impl->useWideVectors = useWideVectors;
}

- (bool)useWideVectors
{
    return impl->useWideVectors;
}

- (void)setOldVecWidthScale:(float)oldVecWidthScale
{
    impl->oldVecWidthScale = oldVecWidthScale;
}

- (float)oldVecWidthScale
{
    return impl->oldVecWidthScale;
}

- (void)setWideVecCuttoff:(float)wideVecCuttoff
{
    impl->wideVecCuttoff = wideVecCuttoff;
}

- (float)wideVecCuttoff
{
    return impl->wideVecCuttoff;
}

- (void)setArealShaderName:(NSString *)arealShaderName
{
    if (arealShaderName)
        impl->arealShaderName = [arealShaderName cStringUsingEncoding:NSASCIIStringEncoding];
    else
        impl->arealShaderName.clear();
}

- (NSString *)arealShaderName
{
    if (impl->arealShaderName.empty())
        return nil;
    return [NSString stringWithCString:impl->arealShaderName.c_str() encoding:NSASCIIStringEncoding];
}

- (void)setSelectable:(bool)selectable
{
    impl->selectable = selectable;
}

- (bool)selectable
{
    return impl->selectable;
}

- (void)setIconDirectory:(NSString *)iconDirectory
{
    if (iconDirectory)
        impl->iconDirectory = [iconDirectory cStringUsingEncoding:NSASCIIStringEncoding];
    else
        impl->iconDirectory.clear();
}

- (NSString *)iconDirectory
{
    if (impl->iconDirectory.empty())
        return nil;
    return [NSString stringWithCString:impl->iconDirectory.c_str() encoding:NSASCIIStringEncoding];
}

- (void)setFontName:(NSString *)fontName
{
    if (fontName)
        impl->fontName = [fontName cStringUsingEncoding:NSASCIIStringEncoding];
    else
        impl->fontName.clear();
}

- (NSString *)fontName
{
    if (impl->fontName.empty())
        return nil;
    return [NSString stringWithCString:impl->fontName.c_str() encoding:NSASCIIStringEncoding];
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
    
    // Turn all the objects on
    NSArray *compObjs = [tileData componentObjects];
    [viewC enableObjects:compObjs mode:threadMode];

    return compObjs;
}

// TODO: These need to be turned into appropriate wrappers
//MapboxVectorTileParser_iOS::MapboxVectorTileParser_iOS(NSObject<MaplyVectorStyleDelegate> * styleDelegate,NSObject<MaplyRenderControllerProtocol> *viewC)
//    : styleDelegate(styleDelegate), viewC(viewC), debugLabel(false), debugOutline(false)
//{
//    // Index all the categories ahead of time.  Once.
//    NSArray *allStyles = [styleDelegate allStyles];
//    for (NSObject<MaplyVectorStyle> *style in allStyles) {
//        NSString *category = [style getCategory];
//        if (category) {
//            long long styleID = style.uuid;
//            std::string categoryStr = [category cStringUsingEncoding:NSUTF8StringEncoding];
//            addCategory(categoryStr, styleID);
//        }
//    }
//}
//
//MapboxVectorTileParser_iOS::~MapboxVectorTileParser_iOS()
//{
//}
//    
//VectorTileDataRef MapboxVectorTileParser_iOS::makeTileDataCopy(VectorTileData *inTileData)
//{
//    return VectorTileDataRef(new VectorTileData(*inTileData));
//}
//    
//bool MapboxVectorTileParser_iOS::layerShouldParse(const std::string &layerName,VectorTileData *tileData)
//{
//    NSString *layerNameStr = [NSString stringWithUTF8String:layerName.c_str()];
//    MaplyTileID tileID;
//    tileID.x = tileData->ident.x;  tileID.y = tileData->ident.y;  tileID.level = tileData->ident.level;
//
//    return [styleDelegate layerShouldDisplay:layerNameStr tile:tileID];
//}
//
//// Return a set of styles that will parse the given feature
//SimpleIDSet MapboxVectorTileParser_iOS::stylesForFeature(MutableDictionaryRef attributes,const std::string &layerName,VectorTileData *tileData)
//{
//    iosMutableDictionaryRef dict = std::dynamic_pointer_cast<iosMutableDictionary>(attributes);
//    NSString *layerNameStr = [NSString stringWithUTF8String:layerName.c_str()];
//    MaplyTileID tileID;
//    tileID.x = tileData->ident.x;  tileID.y = tileData->ident.y;  tileID.level = tileData->ident.level;
//
//    NSArray *styles = [styleDelegate stylesForFeatureWithAttributes:dict->dict onTile:tileID inLayer:layerNameStr viewC:viewC];
//    SimpleIDSet styleIDs;
//    for (NSObject<MaplyVectorStyle> *style in styles) {
//        styleIDs.insert(style.uuid);
//    }
//    
//    return styleIDs;
//}
//    
//void MapboxVectorTileParser_iOS::buildForStyle(long long styleID,std::vector<VectorObjectRef> &vecs,VectorTileDataRef data)
//{
//    // Make up an NSArray for this since it's outward facing
//    NSMutableArray *vecObjs = [[NSMutableArray alloc] init];
//    for (auto vec : vecs) {
//        MaplyVectorObject *wrapObj = [[MaplyVectorObject alloc] initWithRef:vec];
//        [vecObjs addObject:wrapObj];
//    }
//    
//    NSObject<MaplyVectorStyle> *style = [styleDelegate styleForUUID:styleID viewC:viewC];
//    if (!style)
//        return;
//    MaplyVectorTileData *tileDataWrap = [[MaplyVectorTileData alloc] initWithTileData:data];
//    [style buildObjects:vecObjs forTile:tileDataWrap viewC:viewC];
//}

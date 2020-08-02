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
#import "private/MapboxVectorTiles_private.h"
#import "private/MaplyVectorObject_private.h"
#import "helpers/MaplyTextureBuilder.h"
#import "WhirlyGlobe.h"
#import "MaplyTexture_private.h"

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

namespace WhirlyKit
{

MapboxVectorStyleSetImpl_iOS::MapboxVectorStyleSetImpl_iOS(Scene *scene,CoordSystem *coordSys,VectorStyleSettingsImplRef settings)
: MapboxVectorStyleSetImpl(scene,coordSys,settings)
{
}

MapboxVectorStyleSetImpl_iOS::~MapboxVectorStyleSetImpl_iOS()
{
}

SimpleIdentity MapboxVectorStyleSetImpl_iOS::makeCircleTexture(PlatformThreadInfo *inst,
                                                               double inRadius,
                                                               const RGBAColor &fillColor,
                                                               const RGBAColor &strokeColor,
                                                               float inStrokeWidth,
                                                               Point2f *circleSize)
{
    // We want the texture a bit bigger than specified
    float scale = tileStyleSettings->markerScale * 2;

    // Build an image for the circle
    float buffer = 1.0 * scale;
    float radius = inRadius*scale;
    float strokeWidth = inStrokeWidth*scale;
    float size = ceil(buffer + radius + strokeWidth)*2;
    circleSize->x() = size / 2;
    circleSize->y() = size / 2;
    UIGraphicsBeginImageContext(CGSizeMake(size, size));
    // TODO: Use the opacity
    [[UIColor clearColor] setFill];
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextFillRect(ctx, CGRectMake(0.0, 0.0, size, size));

    // Outer stroke
    if (strokeWidth > 0.0) {
        CGContextBeginPath(ctx);
        CGContextAddEllipseInRect(ctx, CGRectMake(size/2.0-radius-strokeWidth, size/2.0-radius-strokeWidth, 2*(radius+strokeWidth), 2*(radius+strokeWidth)));
        [[UIColor colorFromRGBA:strokeColor] setFill];
        CGContextDrawPath(ctx, kCGPathFill);
    }

    // Inner circle
    CGContextBeginPath(ctx);
    CGContextAddEllipseInRect(ctx, CGRectMake(size/2.0-radius, size/2.0-radius, 2*radius, 2*radius));
    [[UIColor colorFromRGBA:fillColor] setFill];
    CGContextDrawPath(ctx, kCGPathFill);

    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    MaplyTexture *tex = [viewC addTexture:image desc:nil mode:MaplyThreadCurrent];
    textures.push_back(tex);
    return tex.texID;
}

SimpleIdentity MapboxVectorStyleSetImpl_iOS::makeLineTexture(PlatformThreadInfo *inst,const std::vector<double> &inComp)
{
    NSMutableArray *dashComp = [NSMutableArray array];
    for (double comp: inComp)
        [dashComp addObject:[NSNumber numberWithDouble:comp]];
    
    MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
    [lineTexBuilder setPattern:dashComp];
    UIImage *lineImage = [lineTexBuilder makeImage];
    MaplyTexture *tex = [viewC addTexture:lineImage
                                               desc:@{kMaplyTexFormat: @(MaplyImageIntRGBA),
                                                      kMaplyTexWrapY: @(MaplyImageWrapY)
                                                      }
                                               mode:MaplyThreadCurrent];
    textures.push_back(tex);
    
    return tex.texID;
}

LabelInfoRef MapboxVectorStyleSetImpl_iOS::makeLabelInfo(PlatformThreadInfo *inst,const std::string &fontName,float fontSize)
{
    // TODO: Do we need the size here?
    NSString *fontNameStr = [NSString stringWithFormat:@"%s",fontName.c_str()];
    UIFont *font = [UIFont fontWithName:fontNameStr size:fontSize];
    if (!font) {
//        NSLog(@"Failed to find font %@",fontNameStr);
        font = [UIFont systemFontOfSize:fontSize];
    }
    
    LabelInfoRef labelInfo(new LabelInfo_iOS(font,true));
    labelInfo->programID = screenMarkerProgramID;
    
    return labelInfo;
}

SingleLabelRef MapboxVectorStyleSetImpl_iOS::makeSingleLabel(PlatformThreadInfo *inst,const std::string &text)
{
    NSString *textStr = [NSString stringWithUTF8String:text.c_str()];
    
    SingleLabel_iOS *label = new SingleLabel_iOS();
    label->text = textStr;
    
    return SingleLabelRef(label);
}

ComponentObjectRef MapboxVectorStyleSetImpl_iOS::makeComponentObject(PlatformThreadInfo *inst)
{
    return ComponentObjectRef(new ComponentObject_iOS());
}

void MapboxVectorStyleSetImpl_iOS::addSelectionObject(SimpleIdentity selectID,VectorObjectRef vecObj,ComponentObjectRef compObj)
{
    if (!compManage)
        return;
    
    ComponentManager_iOS *compManage_iOS = (ComponentManager_iOS *)compManage;
    
    MaplyVectorObject *vectorObj = [[MaplyVectorObject alloc] initWithRef:vecObj];
    compManage_iOS->addSelectObject(selectID, vectorObj);
}


double MapboxVectorStyleSetImpl_iOS::calculateTextWidth(PlatformThreadInfo *inst,LabelInfoRef inLabelInfo,const std::string &testStr)
{
    LabelInfo_iOSRef labelInfo = std::dynamic_pointer_cast<LabelInfo_iOS>(inLabelInfo);
    if (!labelInfo)
        return 0.0;
    
    NSAttributedString *testAttrStr = [[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:testStr.c_str()] attributes:@{NSFontAttributeName:labelInfo->font}];
    CGSize size = [testAttrStr size];
    
    return size.width;
}

VectorStyleDelegateWrapper::VectorStyleDelegateWrapper(NSObject<MaplyRenderControllerProtocol> *viewC,NSObject<MaplyVectorStyleDelegate> *delegate)
: viewC(viewC), delegate(delegate)
{
}

std::vector<VectorStyleImplRef>
VectorStyleDelegateWrapper::stylesForFeature(DictionaryRef attrs,
                                              const QuadTreeIdentifier &tileID,
                                              const std::string &layerName)
{
    iosDictionaryRef dictRef = std::dynamic_pointer_cast<iosDictionary>(attrs);
    NSDictionary *dict = dictRef->dict;
    
    MaplyTileID theTileID;
    theTileID.x = tileID.x;  theTileID.y = tileID.y; theTileID.level = tileID.level;
    NSString *layerStr = [NSString stringWithFormat:@"%s",layerName.c_str()];
    NSArray *styles = [delegate stylesForFeatureWithAttributes:dict onTile:theTileID inLayer:layerStr viewC:viewC];
    
    std::vector<VectorStyleImplRef> retStyles;
    for (NSObject<MaplyVectorStyle> *style : styles) {
        VectorStyleWrapperRef wrap(new VectorStyleWrapper(viewC,style));
        retStyles.push_back(wrap);
    }
    
    return retStyles;
}

bool VectorStyleDelegateWrapper::layerShouldDisplay(const std::string &name,
                                const QuadTreeNew::Node &tileID)
{
    MaplyTileID theTileID;
    theTileID.x = tileID.x;  theTileID.y = tileID.y; theTileID.level = tileID.level;
    NSString *layerStr = [NSString stringWithFormat:@"%s",name.c_str()];
    return [delegate layerShouldDisplay:layerStr tile:theTileID];
}

VectorStyleImplRef VectorStyleDelegateWrapper::styleForUUID(long long uuid)
{
    NSObject<MaplyVectorStyle> *style = [delegate styleForUUID:uuid viewC:viewC];
    return VectorStyleImplRef(new VectorStyleWrapper(viewC,style));
}

std::vector<VectorStyleImplRef> VectorStyleDelegateWrapper::allStyles()
{
    NSArray *styles = [delegate allStyles];

    std::vector<VectorStyleImplRef> retStyles;
    for (NSObject<MaplyVectorStyle> *style : styles) {
        VectorStyleWrapperRef wrap(new VectorStyleWrapper(viewC,style));
        retStyles.push_back(wrap);
    }
    
    return retStyles;
}

RGBAColorRef VectorStyleDelegateWrapper::backgroundColor(double zoom)
{
    return RGBAColorRef(new RGBAColor(RGBAColor::black()));
}

VectorStyleWrapper::VectorStyleWrapper(NSObject<MaplyRenderControllerProtocol> *viewC,NSObject<MaplyVectorStyle> *style)
: viewC(viewC),style(style)
{
}

long long VectorStyleWrapper::VectorStyleWrapper::getUuid()
{
    return [style uuid];
}

std::string VectorStyleWrapper::getCategory()
{
    std::string category;
    NSString *catStr = [style getCategory];
    category = [catStr cStringUsingEncoding:NSUTF8StringEncoding];
    
    return category;
}

bool VectorStyleWrapper::geomAdditive()
{
    return [style geomAdditive];
}

void VectorStyleWrapper::buildObjects(PlatformThreadInfo *inst,
                                      std::vector<VectorObjectRef> &vecObjs,
                                      VectorTileDataRef tileInfo)
{
    MaplyVectorTileData *tileData = [[MaplyVectorTileData alloc] init];
    tileData->data = tileInfo;
    
    NSMutableArray *vecArray = [NSMutableArray array];
    for (VectorObjectRef vecObj: vecObjs) {
        MaplyVectorObject *mVecObj = [[MaplyVectorObject alloc] init];
        mVecObj->vObj = vecObj;
        [vecArray addObject:mVecObj];
    }
        
    [style buildObjects:vecArray forTile:tileData viewC:viewC];
}

}

/*
 *  MapboxVectorStyleSet.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/16/15.
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

#import <WhirlyGlobe.h>
#import "private/MapboxVectorStyleSet_private.h"
#import "private/MaplyVectorStyle_private.h"
#import "MaplyRenderController_private.h"
#import <map>

using namespace WhirlyKit;

@implementation MapboxVectorStyleSet

- (id __nullable)initWithDict:(NSDictionary * __nonnull)styleDict
                    settings:(MaplyVectorStyleSettings * __nonnull)settings
                       viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC
{
    self = [super init];
    if (!self)
        return nil;

    _viewC = viewC;
    VectorStyleSettingsImplRef styleSettings;
    if (settings)
        styleSettings = settings->impl;
    else
        styleSettings = VectorStyleSettingsImplRef(new VectorStyleSettingsImpl([UIScreen mainScreen].scale));
    
    style = MapboxVectorStyleSetImplRef(new MapboxVectorStyleSetImpl_iOS([viewC getRenderControl]->scene,[viewC getRenderControl]->visualView->coordAdapter->getCoordSystem(),styleSettings));

    iosDictionaryRef dictWrap(new iosDictionary(styleDict));
    if (!style->parse(NULL,dictWrap))
        return nil;
    
    _spriteURL = styleDict[@"sprite"];
    
    // Sources tell us where to get tiles
    NSDictionary *sourceStyles = styleDict[@"sources"];
    NSMutableArray *sources = [NSMutableArray array];
    for (NSString *sourceName in sourceStyles.allKeys) {
        NSDictionary *styleEntry = sourceStyles[sourceName];
        MaplyMapboxVectorStyleSource *source = [[MaplyMapboxVectorStyleSource alloc] initWithName:sourceName styleEntry:styleEntry styleSet:self viewC:_viewC];
        if (source)
            [sources addObject:source];
    }
    
    _sources = sources;
    
    return self;
}

- (id)initWithJSON:(NSData *)styleJSON
          settings:(MaplyVectorStyleSettings *)settings
             viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
        
    NSError *error = nil;
    NSDictionary *styleDict = [NSJSONSerialization JSONObjectWithData:styleJSON options:NULL error:&error];
    if (!styleDict)
        return nil;
    
    return [self initWithDict:styleDict settings:settings viewC:viewC];
}

- (UIColor * __nullable)backgroundColorForZoom:(double)zoom
{
    RGBAColorRef color = style->backgroundColor(zoom);
    if (!color)
        return [UIColor blackColor];
    return [UIColor colorFromRGBA:*color];
}

// These are here just to satisfy the compiler.  We use the underlying C++ calls instead

- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                                              onTile:(MaplyTileID)tileID
                                             inLayer:(NSString *__nonnull)layer
                                               viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC
{
    return nil;
}

- (BOOL)layerShouldDisplay:(NSString *__nonnull)layer tile:(MaplyTileID)tileID
{
    return false;
}

- (nullable NSObject<MaplyVectorStyle> *)styleForUUID:(long long)uiid viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC
{
    return nil;
}

- (NSArray * __nonnull)allStyles
{
    return [NSMutableArray array];
}

// Returns the C++ class that does the work
- (WhirlyKit::VectorStyleDelegateImplRef) getVectorStyleImpl
{
    return style;
}

@end

@implementation MaplyMapboxVectorStyleSource

- (id __nullable)initWithName:(NSString *)name styleEntry:(NSDictionary * __nonnull)styleEntry styleSet:(MapboxVectorStyleSet * __nonnull)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC
{
    self = [super init];
    
    _name = name;
    
    NSString *typeStr = styleEntry[@"type"];
    if ([typeStr isEqualToString:@"vector"]) {
        _type = MapboxSourceVector;
    } else if ([typeStr isEqualToString:@"raster"]) {
        _type = MapboxSourceRaster;
    } else {
        NSLog(@"Unsupport source type %@",typeStr);
        return nil;
    }
    
    _url = styleEntry[@"url"];
    _tileSpec = styleEntry[@"tiles"];
    
    if (!_url && !_tileSpec) {
        NSLog(@"Expecting either URL or tileSpec in source %@",_name);
    }
    
    return self;
}

@end

/*
 *  MaplyVectorMarkerStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "MaplyVectorMarkerStyle.h"
#import "MaplyIconManager.h"

@interface MaplyVectorTileSubStyleMarker : NSObject
{
@public
    NSMutableDictionary *desc;
    UIImage *markerImage;
    UIColor *fillColor;
    UIColor *strokeColor;
    float width;
    float height;
    float strokeWidth;
    bool allowOverlap;
    NSString *markerImageTemplate;
}

@end

@implementation MaplyVectorTileSubStyleMarker
@end

// Marker placement style
@implementation MaplyVectorTileStyleMarker
{
    MaplyVectorTileStyleSettings *settings;
    NSMutableArray *subStyles;
}

- (id)initWithStyleEntry:(NSDictionary *)styles settings:(MaplyVectorTileStyleSettings *)inSettings viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithStyleEntry:styles viewC:viewC];
    settings = inSettings;
    
    NSArray *stylesArray = styles[@"substyles"];
    subStyles = [NSMutableArray array];
    for (NSDictionary *styleEntry in stylesArray)
    {
        MaplyVectorTileSubStyleMarker *subStyle = [[MaplyVectorTileSubStyleMarker alloc] init];
        subStyle->fillColor = [UIColor whiteColor];
        if (styleEntry[@"fill"])
            subStyle->fillColor = [MaplyVectorTiles ParseColor:styleEntry[@"fill"]];
        subStyle->strokeColor = nil;
        if (styleEntry[@"stroke"])
            subStyle->strokeColor = [MaplyVectorTiles ParseColor:styleEntry[@"stroke"]];
        subStyle->width = inSettings.markerSize;
        if (styleEntry[@"width"])
            subStyle->width = [styleEntry[@"width"] floatValue];
        subStyle->height = subStyle->width;
        if (styleEntry[@"height"])
            subStyle->height = [styleEntry[@"height"] floatValue];
        subStyle->allowOverlap = false;
        if (styleEntry[@"allow-overlap"])
            subStyle->allowOverlap = [styleEntry[@"allow-overlap"] boolValue];
        subStyle->strokeWidth = 1.0;
        NSString *fileName = nil;
        if (styleEntry[@"file"])
            fileName = styleEntry[@"file"];
        if (subStyle->fillColor && !subStyle->strokeColor)
            subStyle->strokeColor = [UIColor blackColor];
        
        subStyle->desc = [NSMutableDictionary dictionary];
        subStyle->desc[kMaplyEnable] = @NO;
        [self resolveVisibility:styleEntry settings:settings desc:subStyle->desc];
      
        if(!fileName || [fileName rangeOfString:@"["].location == NSNotFound)
        {
            subStyle->markerImage = [MaplyIconManager iconForName:fileName
                                                             size:CGSizeMake(settings.markerScale*subStyle->width+2,
                                                                             settings.markerScale*subStyle->height+2)
                                                            color:[UIColor blackColor]
                                                      circleColor:subStyle->fillColor
                                                       strokeSize:settings.markerScale*subStyle->strokeWidth
                                                      strokeColor:subStyle->strokeColor];
            if ([subStyle->markerImage isKindOfClass:[NSNull class]])
                subStyle->markerImage = nil;
        } else
            subStyle->markerImageTemplate = fileName;

        [subStyles addObject:subStyle];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID layer:(MaplyQuadPagingLayer *)layer viewC:(MaplyBaseViewController *)viewC;
{    
    bool isRetina = [UIScreen mainScreen].scale > 1.0;

    // One marker per object
    NSMutableArray *compObjs = [NSMutableArray array];
    for (MaplyVectorTileSubStyleMarker *subStyle in subStyles)
    {
        NSMutableArray *markers = [NSMutableArray array];
        for (MaplyVectorObject *vec in vecObjs)
        {
            MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
            marker.selectable = self.selectable;
            if(subStyle->markerImage)
                marker.image = subStyle->markerImage;
            else {
                NSString *markerName = [self formatText:subStyle->markerImageTemplate forObject:vec];
                marker.image =  [MaplyIconManager iconForName:markerName
                                                       size:CGSizeMake(settings.markerScale*subStyle->width+2,
                                                                       settings.markerScale*subStyle->height+2)
                                                      color:[UIColor blackColor]
                                                circleColor:subStyle->fillColor
                                                 strokeSize:settings.markerScale*subStyle->strokeWidth
                                                  strokeColor:subStyle->strokeColor];
                if ([marker.image isKindOfClass:[NSNull class]])
                    marker.image = nil;
            }

            if (marker.image) {
                marker.loc = [vec center];
                marker.layoutImportance = settings.markerImportance;
                if (marker.image)
                {
                    marker.size = ((UIImage *)marker.image).size;
                    // The markers will be scaled up on a retina display, so compensate
                    if (isRetina)
                        marker.size = CGSizeMake(marker.size.width/2.0, marker.size.height/2.0);
                } else
                    marker.size = CGSizeMake(settings.markerScale*subStyle->width, settings.markerScale*subStyle->height);
                [markers addObject:marker];
            }
        }

        MaplyComponentObject *compObj = [viewC addScreenMarkers:markers desc:subStyle->desc mode:MaplyThreadCurrent];
        if (compObj)
            [compObjs addObject:compObj];
    }
    
    return compObjs;
}

@end

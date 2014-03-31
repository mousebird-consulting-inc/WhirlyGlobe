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
    float width;
    bool allowOverlap;
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
        UIColor *fillColor = [UIColor whiteColor];
        if (styleEntry[@"fill"])
            fillColor = [MaplyVectorTiles ParseColor:styleEntry[@"fill"]];
        UIColor *strokeColor = [UIColor blackColor];
        if (styleEntry[@"stroke"])
            strokeColor = [MaplyVectorTiles  ParseColor:styleEntry[@"stroke"]];
        subStyle->width = 10.0;
        if (styleEntry[@"width"])
            subStyle->width = [styleEntry[@"width"] floatValue];
        subStyle->allowOverlap = false;
        if (styleEntry[@"allow-overlap"])
            subStyle->allowOverlap = [styleEntry[@"allow-overlap"] boolValue];
        float strokeWidth = 2.0;
        NSString *fileName = nil;;
        if (styleEntry[@"file"])
            fileName = styleEntry[@"file"];
        
        subStyle->desc = [NSMutableDictionary dictionary];
        [self resolveVisibility:styleEntry settings:settings desc:subStyle->desc];
        
        subStyle->markerImage = [MaplyIconManager iconForName:fileName size:CGSizeMake(4*settings.markerScale*subStyle->width, 4*settings.markerScale*subStyle->width) color:fillColor strokeSize:2*settings.markerScale*strokeWidth strokeColor:strokeColor];
        
        [subStyles addObject:subStyle];
    }
    
    return self;
}

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;
{
    // One marker per object
    NSMutableArray *markers = [NSMutableArray array];
    for (MaplyVectorTileSubStyleMarker *subStyle in subStyles)
    {
        for (MaplyVectorObject *vec in vecObjs)
        {
            MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
            marker.selectable = false;
            marker.image = subStyle->markerImage;
            marker.loc = [vec center];
            if (subStyle->allowOverlap)
                marker.layoutImportance = MAXFLOAT;
            else
                marker.layoutImportance = 1.0;
            marker.size = CGSizeMake(settings.markerScale*subStyle->width, settings.markerScale*subStyle->width);
            [markers addObject:marker];
        }
    }
    
    MaplyComponentObject *compObj = [viewC addScreenMarkers:markers desc:nil];
    if (compObj)
        return @[compObj];
    
    return nil;
}

@end

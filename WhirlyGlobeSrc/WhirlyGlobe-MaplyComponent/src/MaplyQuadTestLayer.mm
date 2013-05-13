/*
 *  MaplyQuadTestLayer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 3/19/13.
 *  Copyright 2011-2012 mousebird consulting
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

#import "MaplyQuadTestLayer_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyQuadTestLayer 
{
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    Scene *scene;
    GeoCoordSystem coordSys;
    int maxZoom;
    Mbr extents;
}

- (id)initWithMaxZoom:(int)inMaxZoom
{
    self = [super init];
    
    if (!self)
        return nil;
    
    maxZoom = inMaxZoom;
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)renderer;
{
    scene = inScene;

    // Cover the whole earth
    GeoCoord ll = GeoCoord::CoordFromDegrees(-180, -90);
    GeoCoord ur = GeoCoord::CoordFromDegrees(180, 90);
    extents.addPoint(Point2f(ll.x(),ll.y()));
    extents.addPoint(Point2f(ur.x(),ur.y()));

    // Set up tile and and quad layer with us as the data source
    tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:self];
    tileLoader.ignoreEdgeMatching = true;
    tileLoader.coverPoles = true;
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:tileLoader renderer:renderer];
    [layerThread addLayer:quadLayer];
    
    return true;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene
{
    [layerThread removeLayer:quadLayer];
}

/// Return the coordinate system we're working in
- (WhirlyKit::CoordSystem *)coordSystem
{
    return &coordSys;
}

/// Bounding box used to calculate quad tree nodes.  In local coordinate system.
- (WhirlyKit::Mbr)totalExtents
{
    return extents;
}

/// Bounding box of data you actually want to display.  In local coordinate system.
/// Unless you're being clever, make this the same as totalExtents.
- (WhirlyKit::Mbr)validExtents
{
    return extents;
}

/// Return the minimum quad tree zoom level (usually 0)
- (int)minZoom
{
    return 0;
}

/// Return the maximum quad tree zoom level.  Must be at least minZoom
- (int)maxZoom
{
    return maxZoom;
}

/// Return an importance value for the given tile
- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *) viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    if (ident.level == 0)
        return MAXFLOAT;
    
    float import = ScreenImportance(viewState, frameSize, viewState->eyeVec, 128, &coordSys, scene->getCoordAdapter(), mbr, ident, attrs);
    return import;
}

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown
{
}

/// Number of simultaneous fetches this data source can support.
/// You can change this on the fly, but it won't cancel outstanding fetches.
- (int)maxSimultaneousFetches
{
    return 1;
}

static const int MaxDebugColors = 10;
static const int debugColors[MaxDebugColors] = {0x86812D, 0x5EB9C9, 0x2A7E3E, 0x4F256F, 0xD89CDE, 0x773B28, 0x333D99, 0x862D52, 0xC2C653, 0xB8583D};

/// This version of the load method passes in a mutable dictionary.
/// Store your expensive to generate key/value pairs here.
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(NSMutableDictionary *)attrs
{
    CGSize size;  size = CGSizeMake(128,128);
    UIGraphicsBeginImageContext(size);
    
    // Draw into the image context
    UIColor *backColor = [UIColor colorFromHexRGB:debugColors[level % MaxDebugColors]];
    [backColor setFill];
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextFillRect(ctx, CGRectMake(0,0,size.width,size.height));
    
    CGContextSetTextDrawingMode(ctx, kCGTextFill);
    [[UIColor whiteColor] setStroke];
    [[UIColor whiteColor] setFill];
    NSString *textStr = [NSString stringWithFormat:@"%d: (%d,%d)",level,col,row];
    [textStr drawInRect:CGRectMake(0,0,size.width,size.height) withFont:[UIFont systemFontOfSize:24.0]];
    
    // Grab the image and shut things down
    UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
    NSData *imgData = UIImagePNGRepresentation(retImage);
    UIGraphicsEndImageContext();
    
    [quadLoader dataSource: self loadedImage: [WhirlyKitLoadedImage LoadedImageWithNSDataAsPNGorJPG:imgData] forLevel: level col: col row: row];    
}

@end

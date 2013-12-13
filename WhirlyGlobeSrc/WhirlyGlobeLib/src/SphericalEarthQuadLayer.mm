/*
 *  SphericalEarthQuadLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/6/12.
 *  Copyright 2012 mousebird consulting
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

#import "SphericalEarthQuadLayer.h"
#import "GlobeLayerViewWatcher.h"
#import "TileQuadLoader.h"

using namespace Eigen;
using namespace WhirlyKit;

// Describes the structure of the image database
@interface WholeEarthStructure : NSObject<WhirlyKitQuadDataStructure>
{
    GeoCoordSystem coordSystem;
    int maxZoom;
    int pixelsSquare;
}
@end

@implementation WholeEarthStructure

- (id)initWithPixelsSquare:(int)inPixelsSquare maxZoom:(int)inMaxZoom
{
    self = [super init];
    if (self)
    {
        pixelsSquare = inPixelsSquare;
        maxZoom = inMaxZoom;
    }
    
    return self;
}

/// Return the coordinate system we're working in
- (WhirlyKit::CoordSystem *)coordSystem
{
    return &coordSystem;
}

/// Bounding box used to calculate quad tree nodes.  In local coordinate system.
- (WhirlyKit::Mbr)totalExtents
{
    return GeoMbr(GeoCoord::CoordFromDegrees(-180, -90),GeoCoord::CoordFromDegrees(180, 90));
}

/// Bounding box of data you actually want to display.  In local coordinate system.
/// Unless you're being clever, make this the same as totalExtents.
- (WhirlyKit::Mbr)validExtents
{
    return [self totalExtents];
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
- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)tileMbr viewInfo:(WhirlyKit::ViewState *) viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    if (ident.level == [self minZoom])
        return MAXFLOAT;
    
    return ScreenImportance(viewState, frameSize, viewState->eyeVec, pixelsSquare, &coordSystem, viewState->coordAdapter, tileMbr, ident, attrs);
}

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown
{
}

@end

// Data source that serves individual images as requested
@interface ImageDataSource : NSObject<WhirlyKitQuadTileImageDataSource>

@property(nonatomic) NSString *basePath,*ext,*baseName;
@property(nonatomic,assign) int maxZoom,pixelsSquare,borderPixels;

@end

@implementation ImageDataSource

- (id)initWithInfo:(NSString *)infoName
{
    self = [super init];

    if (self)
    {
        // This should be the info plist.  That has everything
        NSDictionary *dict = [NSDictionary dictionaryWithContentsOfFile:infoName];
        if (!dict)
        {
            return nil;
        }
        // If the user specified a real path, as opposed to just
        //  the file, we'll hang on to that
        _basePath=[infoName stringByDeletingLastPathComponent];
        _ext = [dict objectForKey:@"format"];
        _baseName = [dict objectForKey:@"baseName"];
        _maxZoom = [[dict objectForKey:@"maxLevel"] intValue];
        _pixelsSquare = [[dict objectForKey:@"pixelsSquare"] intValue];
        _borderPixels = [[dict objectForKey:@"borderSize"] intValue];    
    }
    
    return self;
}

- (int)maxSimultaneousFetches
{
    return 1;
}

- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(NSMutableDictionary *)attrs
{
    NSString *name = [NSString stringWithFormat:@"%@_%dx%dx%d.%@",_baseName,level,col,row,_ext];
	if (self.basePath)
		name = [self.basePath stringByAppendingPathComponent:name];
    
    NSData *imageData = [NSData dataWithContentsOfFile:name];
    
    bool isPvrtc = ![_ext compare:@"pvrtc"];
    
    WhirlyKitLoadedImage *loadImage = nil;
    if (isPvrtc)
        loadImage = [WhirlyKitLoadedImage LoadedImageWithPVRTC:imageData size:_pixelsSquare];
    else
        loadImage = [WhirlyKitLoadedImage LoadedImageWithNSDataAsPNGorJPG:imageData];
    [quadLoader dataSource:self loadedImage:loadImage forLevel:level col:col row:row];
}

@end


@interface WhirlyKitSphericalEarthQuadLayer()
{
    WholeEarthStructure *earthDataStructure;
    ImageDataSource *imageDataSource;
    WhirlyKitQuadTileLoader *quadTileLoader;
}
@end

@implementation WhirlyKitSphericalEarthQuadLayer

- (int)drawPriority
{
    return quadTileLoader.drawPriority;
}

- (void)setDrawPriority:(int)drawPriority
{
    quadTileLoader.drawPriority = drawPriority;
}

- (int)drawOffset
{
    return quadTileLoader.drawOffset;
}

- (void)setDrawOffset:(int)drawOffset
{
    quadTileLoader.drawOffset = drawOffset;
}

- (bool)ignoreEdgeMatching
{
    return quadTileLoader.ignoreEdgeMatching;
}

- (void)setIgnoreEdgeMatching:(bool)ignoreEdgeMatching
{
    quadTileLoader.ignoreEdgeMatching = ignoreEdgeMatching;
}

- (id) initWithInfo:(NSString *)infoName renderer:(WhirlyKit::SceneRendererES *)inRenderer
{
    return [self initWithInfo:infoName imageType:WKTileIntRGBA renderer:inRenderer];
}

- (id) initWithInfo:(NSString *)infoName imageType:(WhirlyKitTileImageType)imageType renderer:(WhirlyKit::SceneRendererES *)inRenderer;
{
    // Data source serves the tiles
    ImageDataSource *theDataSource = [[ImageDataSource alloc] initWithInfo:infoName];
    if (!theDataSource)
        return nil;
    
    // This describes the quad tree and extents
    WholeEarthStructure *theStructure = [[WholeEarthStructure alloc] initWithPixelsSquare:theDataSource.pixelsSquare maxZoom:theDataSource.maxZoom];
    
    // This handles the geometry and loading
    WhirlyKitQuadTileLoader *theLoader = [[WhirlyKitQuadTileLoader alloc] initWithName:@"SphericalEarthQuadLayer" dataSource:theDataSource];
    if (![theDataSource.ext compare:@"pvrtc"])
        [theLoader setImageType:WKTilePVRTC4];
    // On non-retina displays we're loading fewer tiles
    if ([UIScreen mainScreen].scale == 1.0)
        theLoader.textureAtlasSize = 1024;
    theLoader.imageType = imageType;
    
    self = [super initWithDataSource:theStructure loader:theLoader renderer:inRenderer];
    if (self)
    {
        earthDataStructure = theStructure;
        imageDataSource = theDataSource;
        quadTileLoader = theLoader;
    }
	
	return self;    
}

- (void)shutdown
{
    [super shutdown];
    
    earthDataStructure = nil;
    imageDataSource = nil;
    quadTileLoader = nil;
}

@end

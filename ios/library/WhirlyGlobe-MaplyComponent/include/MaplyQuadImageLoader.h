/*
 *  MaplyQuadImageLoader.h
 *
 *  Created by Steve Gifford on 4/10/18.
 *  Copyright 2012-2018 Saildrone Inc
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

#import "MaplyViewControllerLayer.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyTileSource.h"

/**
    Data return for a loading request.
 
    The tile source fills this in and passes it back to the QuadImageLoader.
  */
@interface MaplyQuadImageLoaderReturn : NSObject

// Tile this is the image for
@property (nonatomic,assign) MaplyTileID tileID;

// If set, the frame.  -1 by default
@property (nonatomic,assign) int frame;

// Can be a UIImage or an NSData containing an image or a MaplyImageTile
@property (nonatomic) id __nullable image;

// If any component objects are associated with the tile, these are them.
// They need to start disabled.  The system will enable and delete them when it is time.
@property (nonatomic) NSArray * __nullable compObjs;

@end

/**
 The Maply Quad Image Loader is for paging image pyramids local or remote.
 
 This layer pages image pyramids.  They can be local or remote, in any coordinate system Maply supports and you provide a MaplyTileSource conformant object to do the actual image tile fetching.
 
 You probably don't have to implement your own tile source.  Go look at the MaplyRemoteTileSource and MaplyMBTileSource objects, as well as MaplyMultiplexTileSource.  Those will do remote, local, and sources for animation respectively.  There's also MaplyWMSTileSource, but I wouldn't expect to use that.
 @see MaplyRemoteTileSource
 @see MaplyMBTileSource
 @see MaplyMultiplexTileSource
 @see MaplyWMSTileSource
 */
@interface MaplyQuadImageLoader : NSObject

/**
 Initialize with a tile source object.
 
 The initialize expects a tile source.  The tile source can be one of the standard ones listed above, or it can be one of your own that conforms to the MaplyTileSource protocol. The tile source's coordinate system will be used.
 
 @param tileSource This is an object conforming to the MaplyTileSource protocol.  There are several you can pass in, or you can write your own.
 */
- (nullable instancetype)initWithTileSource:(NSObject<MaplyTileSource> *__nonnull)tileSource;

/**
 The number of simultaneous fetches the layer will attempt at once.
 
 The toolkit loves its dispatch queues and threads.  By default this number is set to 8 or 16, but if you need to constrain it, you can set it lower (or higher!).  If your tile source can't handle multi-thread access, set this to 1.
 */
@property (nonatomic,assign) int numSimultaneousFetches;

/** Called by the tile source when a tile had loaded (or failed to load).
    The caller is responsible for filling out the loadReturn completely.
  */
- (void)loadedReturn:(MaplyQuadImageLoaderReturn * __nullable)loadReturn error:(NSError * __nullable)error;

@end

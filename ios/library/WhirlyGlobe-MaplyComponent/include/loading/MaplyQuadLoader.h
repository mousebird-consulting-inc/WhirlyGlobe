/*
 *  MaplyQuadLoader.h
 *
 *  Created by Steve Gifford on 2/12/19.
 *  Copyright 2012-2019 Saildrone Inc
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
#import "MaplyTileSourceNew.h"
#import "MaplyRenderController.h"
#import "MaplyQuadSampler.h"
#import "MaplyRemoteTileFetcher.h"

/**
 Passed in to and returned by the Loader Interpreter.
 
 We pass this into the interpreter with the unparsed data.  It parses it and passes that
 data back, possibly with an error.
 */
@interface MaplyLoaderReturn : NSObject

// Tile this is the image for
@property (nonatomic,assign) MaplyTileID tileID;

// If set, the frame.  -1 by default
@property (nonatomic,assign) int frame;

// Data returned from a tile request.  Unparsed.
@property (nonatomic,strong) NSData * __nullable tileData;

// If you have more than one tileInfo, you'll get your data back here unparsed.
@property (nonatomic,strong) NSArray * __nullable multiTileData;

// Can be zero or more UIImage or an NSData containing an image or a MaplyImageTile
@property (nonatomic,strong) NSArray *__nullable images;

// If any component objects are associated with the tile, these are them.
// They need to start disabled.  The system will enable and delete them when it is time.
@property (nonatomic,strong) NSArray * __nullable compObjs;

// These component objects are assumed to be overlaid and so only one
// set will be displayed at a time.
@property (nonatomic,strong) NSArray * __nullable ovlCompObjs;

// If this is set, the tile failed to parse
@property (nonatomic,strong) NSError * __nullable error;

@end

/**
 Loader Interpreter converts raw data into images and objects.
 
 Converts data returned from a remote source (or cache) into images and/or
 MaplyComponentObjects that have already been added to the view (disabled).
 */
@protocol MaplyLoaderInterpreter<NSObject>

/**
 Parse the data coming back from a remote request and turn it into something we can use.
 
 Convert the NSData passed in to image and component objects (e.g. add stuff to the view controller).
 Everything added should be disabled to start.
 */
- (void)dataForTile:(MaplyLoaderReturn * __nonnull)loadReturn;

@end

/**
 Image loader intrepreter turns NSData objects into MaplyImageTiles.
 
 This is the default interpreter used byt the MaplyQuadImageLoader.
 */
@interface MaplyImageLoaderInterpreter : NSObject<MaplyLoaderInterpreter>
@end

@class MaplyQuadImageLoaderBase;

/**
 This loader interpreter sticks a designator in the middle of tiles
 and a line around the edge.  Nice for debugging.
 */
@interface MaplyOvlDebugImageLoaderInterpreter : MaplyImageLoaderInterpreter

// Intialize with the loader we're using.  Need this for extents of tiles
- (instancetype __nonnull)initWithLoader:(MaplyQuadImageLoaderBase * __nonnull)inLoader viewC:(MaplyBaseViewController * __nonnull)viewC;

@end

/**
 This loader interpreter makes up an image for the given frame/tile
 and returns that.  It doesn't use any returned data.
 */
@interface MaplyDebugImageLoaderInterpreter : MaplyImageLoaderInterpreter

- (instancetype __nonnull)initWithLoader:(MaplyQuadImageLoaderBase *__nonnull)inLoader viewC:(MaplyBaseViewController * __nonnull)viewC;

@end

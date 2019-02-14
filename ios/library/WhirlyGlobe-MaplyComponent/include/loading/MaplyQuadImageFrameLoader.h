/*
 *  MaplyQuadImageFrameLoader.h
 *
 *  Created by Steve Gifford on 9/13/18.
 *  Copyright 2012-2018 mousebird consulting inc
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

#import "MaplyQuadImageLoader.h"
#import "MaplyActiveObject.h"

/**
 This version of the loader return is used by the MaplyImageLoaderInterpreter.
 
 When image tiles load, the interpeter fill in these contents, which can
 include any sort of ComponentObject and, of course, images.
 */
@interface MaplyImageLoaderReturn : MaplyLoaderReturn

/// Add an image to this loaded return.
/// You can multiple, but the interpreter should be expecting that
- (void)addImageTile:(MaplyImageTile * __nonnull)image;

/// Add a UIImage to the loader return
/// This just adds a MaplyImageTile wrapper around the UIImage.
- (void)addImage:(UIImage * __nonnull)image;

/// Return an array of Images
- (NSArray<MaplyImageTile *> * __nonnull)getImages;

/// If any component objects are associated with the tile, these are them.
/// They need to start disabled.  The system will enable and delete them when it is time.
- (void)addCompObjs:(NSArray<MaplyComponentObject *> * __nonnull)compObjs;

/// Return the component objects added for this loader return
- (NSArray<MaplyComponentObject *> *__nonnull)getCompObjs;

/// These component objects are assumed to be overlaid and so only one
/// set will be displayed at a time.
- (void)addOvlCompObjs:(NSArray<MaplyComponentObject *> * __nonnull)compObjs;

/// Return the overlay component objects added for this loader return
- (NSArray<MaplyComponentObject *> *__nonnull)getOvlCompObjs;

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

@class MaplyQuadImageFrameLoader;

/**
    Quad Image FrameAnimation runs through the frames in a Quad Image Frame loader over time.
 
    Set this up with a MaplyQuadImageFrameLoader and it'll run through the available frames from start to finish.
    At the end it will snap back to the beginning.
  */
@interface MaplyQuadImageFrameAnimator : MaplyActiveObject

/// Initialize with the image frame loader and view controller
- (nonnull instancetype)initWithFrameLoader:(MaplyQuadImageFrameLoader * __nonnull)loader viewC:(MaplyBaseViewController * __nonnull)viewC;

/// How long to animate from start to finish.
@property (nonatomic,assign) NSTimeInterval period;

/// How long to pause at the end of the sequence before starting back
@property (nonatomic,assign) NSTimeInterval pauseLength;

/// Remove the animator and stop animating
- (void)shutdown;

@end

/**
 The Maply Quad Image Loader is for paging image pyramids local or remote.
 
 This layer pages image pyramids.  They can be local or remote, in any coordinate system Maply supports and you provide a MaplyTileInfoNew conformant object to do the actual image tile fetching.
 
 You probably don't have to implement your own tile source.  Go look at the MaplyRemoteTileInfoNew and MaplyMBTileFetcher objects.  Those will do remote and local fetching.
 */
@interface MaplyQuadImageFrameLoader : MaplyQuadImageLoaderBase

/**
 Initialize with multiple tile sources (one per frame).
 
 @param params The sampling parameters describing how to break down the data for projection onto a globe or map.
 @param tileInfos A list of tile info objects to fetch for each frame.
 @param viewC the View controller (or renderer) to add objects to.
 */
- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)params tileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *__nonnull)tileInfos viewC:(MaplyBaseViewController * __nonnull)viewC;

/**
  Set the interpolated location within the array of frames.
 
  Each set of frames can be accessed from [0.0,numFrames].  Images will be interpolated between
  those values and may be snapped if data has not yet loaded.
 
  This value is used once per frame, so feel free to call this as much as you'd like.
  */
- (void)setCurrentImage:(double)where;

/** Turn off the image loader and shut things down.
 This unregisters us with the sampling layer and shuts down the various objects we created.
 */
- (void)shutdown;

@end

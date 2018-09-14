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

/**
  Shader to use for rendering the image frames.
 
  If not, set we'll pick the default visual shader.
  */
- (void)setShader:(MaplyShader *)shader;

/** Turn off the image loader and shut things down.
 This unregisters us with the sampling layer and shuts down the various objects we created.
 */
- (void)shutdown;

@end

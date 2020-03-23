/*
 *  MaplyQuadPagingLoader.h
 *
 *  Created by Steve Gifford on 2/21/91.
 *  Copyright 2012-2019 mousebird consulting
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

#import "loading/MaplyQuadLoader.h"

/**
 This version of the loader return is used by the MaplyQuadPagingLoader.
 
 The Object pager is only expecting Component Objects and will manage
 those as things are loaded in and out.
 */
@interface MaplyObjectLoaderReturn : MaplyLoaderReturn

/// Initialize with the loader this will be attached to
- (id __nonnull)initWithLoader:(MaplyQuadLoaderBase *__nonnull)loader;

/// If any component objects are associated with the tile, these are them.
/// They need to start disabled.  The system will enable and delete them when it is time.
- (void)addCompObjs:(NSArray<MaplyComponentObject *> * __nonnull)compObjs;

/// Add a component object to the collection if it's non null
- (void)addCompObj:(MaplyComponentObject * __nullable)compObj;

/// Return an array of component objects that were added to this loader return
- (NSArray<MaplyComponentObject *> * __nonnull)getCompObjs;

@end

/** General purpose quad paging loader.
 
    This quadtree based paging loader is for fetching and load general geometry.
    There are other loaders that handle images and image animations.  This one is
    purely for geometry.
 
    You need to fill in at least a MaplyLoaderInterpreter, which is probably your own
    implementation.
 
    This replaces the QuadPagingLayer from WhirlyGlobe-Maply 2.x.
  */
@interface MaplyQuadPagingLoader : MaplyQuadLoaderBase

/**
 Initialize with a single tile info object, the interpreter and the sampling parameters.
 
 @param params The sampling parameters describing how to break down the data for projection onto a globe or map.
 @param tileInfo A optional tile info object describing where the data is and how to get it.
 @param loadInterp The interpreter makes geometry from the input data.  Or just makes it up if there is no input.
 @param viewC the View controller (or renderer) to add objects to.
 */
- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)params
                               tileInfo:(NSObject<MaplyTileInfoNew> *__nullable)tileInfo
                             loadInterp:(NSObject<MaplyLoaderInterpreter> *__nullable)loadInterp
                                  viewC:(MaplyBaseViewController * __nonnull)viewC;

/**
  Force a reload of the data.
  <br>
  All the current loads will be cancelled, any in flight will be ignored and the loader will ask for a whole new set of data.
  */
- (void)reload;

@end

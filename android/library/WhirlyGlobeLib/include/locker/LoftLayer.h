/*
 *  LoftLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/11.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "Scene.h"
#import "Drawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "TextureAtlas.h"
#import "SelectionManager.h"
#import "ShapeReader.h"
#import "DataLayer.h"
#import "LoftManager.h"

/** The WhirlyGlobe Loft Layer creates a polygon lofted above the globe
    with sides.  These are typically drawn transparently 
    Represents a set of lofted polygons.

     When adding a set of lofted polys, you can pass in an optional dictionary
     describing how they'll look.  That can have any of these key/value pairs:
     <list type="bullet">
     <item>color       [UIColor]
     <item>height      [NSNumber float]
     <item>base        [NSNumber float]
     <item>drawPriority [NSNumber int]
     <item>minVis      [NSNumber float]
     <item>maxVis      [NSNumber float]
     <item>fade        [NSNumber float]
     <item>top         [NSNumber bool]
     <item>side        [NSNumber bool]
     <item>layered     [NSNumber bool]
     <item>outline     [NSNumber bool]
     <item>outlineColor [UIColor]
     <item>outlineWidth [NSNumber float]
     <item>enable       [NSNumber bool]
     </list>
 */
@interface WhirlyKitLoftLayer : NSObject<WhirlyKitLayer>

/// Shapes are clipped against a grid before lofting.
/// This is the grid size, in radians
@property (nonatomic,assign) float gridSize;

/// Called in layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

/** Create one or more lofted polygons from the set of shapes given.
    The given dictionary defines how the will look.
    If the cache name is specified, we'll look for the given cache file
    or create it.
    Returns the ID used to identify the group.
 */
- (WhirlyKit::SimpleIdentity) addLoftedPolys:(WhirlyKit::ShapeSet *)shape desc:(NSDictionary *)desc cacheName:(NSString *)name cacheHandler:(NSObject<WhirlyKitLoftedPolyCache> *)cacheHandler;

/** Create a single lofted polygon from the given shape.
    Visual characteristics are defined by the dictionary.
    If the cache name is specified, we'll look for the given cache file
    or create it.
  */
- (WhirlyKit::SimpleIdentity) addLoftedPoly:(WhirlyKit::VectorShapeRef)shape desc:(NSDictionary *)desc cacheName:(NSString *)name cacheHandler:(NSObject<WhirlyKitLoftedPolyCache> *)cacheHandler;

/// Remove a group of lofted polygons as specified by the ID.
- (void) removeLoftedPoly:(WhirlyKit::SimpleIdentity)polyID;

@end

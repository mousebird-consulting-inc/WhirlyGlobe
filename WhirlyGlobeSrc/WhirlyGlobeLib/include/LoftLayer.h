/*
 *  LoftLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/11.
 *  Copyright 2011-2012 mousebird consulting. All rights reserved.
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
#import "DrawCost.h"
#import "SelectionLayer.h"
#import "ShapeReader.h"
#import "DataLayer.h"

/** This is the protocol for an object that can handle caching for the lofted poly
    layer.  Generating the lofted polys is expensive, so we try to cache them
    somewhere.  It's up to the object implementing this protocol to store and retrieve
    the cached data.
  */
@protocol WhirlyKitLoftedPolyCache<NSObject>
/// Return the NSData for the given cache name (e.g. individual object).
- (NSData *)readLoftedPolyData:(NSString *)cacheName;
/// Write the given NSData ...somewhere... for the given cache name (e.g. individual object)
- (bool)writeLoftedPolyData:(NSData *)data cacheName:(NSString *)cacheName;
@end

namespace WhirlyKit
{
    
/** Representation of one or more lofted polygons.
    Used to keep track of the assets we create.
  */
class LoftedPolySceneRep : public WhirlyKit::Identifiable
{
public:
    LoftedPolySceneRep() { }
    ~LoftedPolySceneRep() { }
    
    // If we're keeping a cache of the meshes, read and write
    bool readFromCache(NSObject<WhirlyKitLoftedPolyCache> *cache,NSString *key);
    bool writeToCache(NSObject<WhirlyKitLoftedPolyCache> *cache,NSString *key);
        
    WhirlyKit::SimpleIDSet drawIDs;  // Drawables created for this
    WhirlyKit::ShapeSet shapes;    // The shapes for the outlines
    WhirlyKit::GeoMbr shapeMbr;       // Overall bounding box
    float fade;            // Fade out, used for delete
    std::vector<WhirlyKit::VectorRing> triMesh;  // The post-clip triangle mesh, pre-loft
};
typedef std::map<WhirlyKit::SimpleIdentity,LoftedPolySceneRep *> LoftedPolySceneRepMap;
    
}

/** The WhirlyGlobe Loft Layer creates a polygon lofted above the globe
    with sides.  These are typically drawn transparently 
    Represents a set of lofted polygons.

     When adding a set of lofted polys, you can pass in an optional dictionary
     describing how they'll look.  That can have any of these key/value pairs:
     <list type="bullet">
     <item>color       [UIColor]
     <item>height      [NSNumber float]
     <item>priority    [NSNumber int]
     <item>minVis      [NSNumber float]
     <item>maxVis      [NSNumber float]
     <item>fade        [NSNumber float]
     <item>top         [NSNumber bool]
     <item>side        [NSNumber bool]
     </list>
 */
@interface WhirlyKitLoftLayer : NSObject<WhirlyKitLayer>
{
    WhirlyKitLayerThread * __weak layerThread;
    WhirlyKit::Scene *scene;
    
    /// Used to keep track of the lofted polygons
    WhirlyKit::LoftedPolySceneRepMap polyReps;

    /// Shapes are clipped against a grid before lofting.
    /// This is the grid size, in radians
    float gridSize;
}

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

/// Change a lofted poly group as defined by the dictionary.
- (void) changeLoftedPoly:(WhirlyKit::SimpleIdentity)polyID desc:(NSDictionary *)desc;

@end

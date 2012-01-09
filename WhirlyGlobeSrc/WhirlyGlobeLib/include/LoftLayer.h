/*
 *  LoftLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/11.
 *  Copyright 2011 mousebird consulting. All rights reserved.
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
#import "Drawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "TextureAtlas.h"
#import "DrawCost.h"
#import "SelectionLayer.h"
#import "ShapeReader.h"
#import "DataLayer.h"

namespace WhirlyGlobe
{
    
/** Representation of one or more lofted polygons.
    Used to keep track of the assets we create.
  */
class LoftedPolySceneRep : public Identifiable
{
public:
    LoftedPolySceneRep() { }
    ~LoftedPolySceneRep() { }
    
    // If we're keeping a cache of the meshes, read and write
    bool readFromCache(NSString *key);
    bool writeToCache(NSString *key);
        
    SimpleIDSet drawIDs;  // Drawables created for this
    ShapeSet shapes;    // The shapes for the outlines
    GeoMbr shapeMbr;       // Overall bounding box
    float fade;            // Fade out, used for delete
    std::vector<VectorRing> triMesh;  // The post-clip triangle mesh, pre-loft
};
typedef std::map<SimpleIdentity,LoftedPolySceneRep *> LoftedPolySceneRepMap;
    
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
     </list>
 */
@interface WhirlyGlobeLoftLayer : NSObject<WhirlyGlobeLayer>
{
    WhirlyGlobeLayerThread *layerThread;
    WhirlyGlobe::GlobeScene *scene;
    
    /// Used to keep track of the lofted polygons
    WhirlyGlobe::LoftedPolySceneRepMap polyReps;

    /// Shapes are clipped against a grid before lofting.
    /// This is the grid size, in radians
    float gridSize;
}

@property (nonatomic,assign) float gridSize;
@property (nonatomic,assign) BOOL useCache;

/// Called in layer thread
- (void)startWithThread:(WhirlyGlobeLayerThread *)layerThread scene:(WhirlyGlobe::GlobeScene *)scene;

/** Create one or more lofted polygons from the set of shapes given.
    The given dictionary defines how the will look.
    If the cache name is specified, we'll look for the given cache file
    or create it.
    Returns the ID used to identify the group.
 */
- (WhirlyGlobe::SimpleIdentity) addLoftedPolys:(WhirlyGlobe::ShapeSet *)shape desc:(NSDictionary *)desc cacheName:(NSString *)cacheName;

/** Create a single lofted polygon from the given shape.
    Visual characteristics are defined by the dictionary.
    If the cache name is specified, we'll look for the given cache file
    or create it.
  */
- (WhirlyGlobe::SimpleIdentity) addLoftedPoly:(WhirlyGlobe::VectorShapeRef)shape desc:(NSDictionary *)desc cacheName:(NSString *)cacheName;

/// Remove a group of lofted polygons as specified by the ID.
- (void) removeLoftedPoly:(WhirlyGlobe::SimpleIdentity)polyID;

/// Change a lofted poly group as defined by the dictionary.
- (void) changeLoftedPoly:(WhirlyGlobe::SimpleIdentity)polyID desc:(NSDictionary *)desc;

@end

/*
 *  LoftManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/30/13.
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
#import "VectorDatabase.h"

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

@class WhirlyKitLoftedPolyInfo;

namespace WhirlyKit
{
    
/** Representation of one or more lofted polygons.
 Used to keep track of the assets we create.
 */
class LoftedPolySceneRep : public WhirlyKit::Identifiable
{
public:
    LoftedPolySceneRep() : triMesh(VectorTriangles::createTriangles()) { }
    LoftedPolySceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    ~LoftedPolySceneRep() { }
    
    // If we're keeping a cache of the meshes, read and write
    bool readFromCache(NSObject<WhirlyKitLoftedPolyCache> *cache,NSString *key);
    bool writeToCache(NSObject<WhirlyKitLoftedPolyCache> *cache,NSString *key);
    
    WhirlyKit::SimpleIDSet drawIDs;  // Drawables created for this
    WhirlyKit::ShapeSet shapes;    // The shapes for the outlines
    std::vector<WhirlyKit::VectorRing>  outlines;  // If we're displaying outlines, the shapes for that
    WhirlyKit::GeoMbr shapeMbr;       // Overall bounding box
    float fade;            // Fade out, used for delete
    VectorTrianglesRef triMesh;  // The post-clip triangle mesh, pre-loft
};
typedef std::set<LoftedPolySceneRep *,IdentifiableSorter> LoftedPolySceneRepSet;

#define kWKLoftedPolyManager "kWKLoftedPolyManager"
    
/** The Loft Manager handles the geometr associated with lofted polygons.
    It's entirely thread safe (except for destruction).
  */
class LoftManager : public SceneManager
{
public:
    LoftManager();
    virtual ~LoftManager();

    /// Add lofted polygons
    SimpleIdentity addLoftedPolys(WhirlyKit::ShapeSet *shapes,NSDictionary *desc,NSString *cacheName,NSObject<WhirlyKitLoftedPolyCache> *cacheHandler,float gridSize,ChangeSet &changes);

    /// Enable/disable lofted polys
    void enableLoftedPolys(SimpleIDSet &polyIDs,bool enable,ChangeSet &changes);
    
    /// Remove lofted polygons
    void removeLoftedPolys(SimpleIDSet &polyIDs,ChangeSet &changes);
        
protected:
    void addGeometryToBuilder(LoftedPolySceneRep *sceneRep,WhirlyKitLoftedPolyInfo *polyInfo,GeoMbr &drawMbr,ChangeSet &changes);
    
    pthread_mutex_t loftLock;
    LoftedPolySceneRepSet loftReps;
};

}

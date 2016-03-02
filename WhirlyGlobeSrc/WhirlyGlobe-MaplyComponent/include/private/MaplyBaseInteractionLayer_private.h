/*
 *  MaplyBaseInteractionLayer_private.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012-2015 mousebird consulting
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

#import <Foundation/Foundation.h>
#import <set>
#import <WhirlyGlobe.h>
#import "MaplyComponentObject_private.h"
#import "SelectObject_private.h"
#import "ImageTexture_private.h"
#import "MaplyBaseViewController.h"
#import "MaplyQuadImageTilesLayer.h"
#import "MaplyTextureAtlas_private.h"

@interface MaplyBaseInteractionLayer : NSObject<WhirlyKitLayer>
{
@public
    WhirlyKitView * __weak visualView;
    WhirlyKitGLSetupInfo *glSetupInfo;

    pthread_mutex_t selectLock;
    // Use to map IDs in the selection layer to objects the user passed in
    SelectObjectSet selectObjectSet;

    // Layer thread we're part of
    WhirlyKitLayerThread * __weak layerThread;

    // Scene we're using
    WhirlyKit::Scene *scene;
    
    pthread_mutex_t imageLock;
    // Used to track textures
    MaplyImageTextureList imageTextures;

    // Component objects created for the user
    NSMutableSet *userObjects;
    
    // Texture atlas manager
    MaplyTextureAtlasGroup *atlasGroup;
    
    pthread_mutex_t tempContextLock;
    // We keep a set of temporary OpenGL ES contexts around for threads that don't have them
    std::set<EAGLContext *> tempContexts;
    
    // Set when we're trying to shut things down
    bool isShuttingDown;
}

// Note: Not a great idea to be passing this in
@property (nonatomic,weak) UIView * glView;

// Offset for draw priorities on screen objects
@property (nonatomic,assign) int screenObjectDrawPriorityOffset;

// Initialize with the view we'll be using
- (id)initWithView:(WhirlyKitView *)visualView;

// Add screen space (2D) markers
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add 3D markers
- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add screen space (2D) labels
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add 3D labels
- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add vectors
- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Instance vectors
- (MaplyComponentObject *)instanceVectors:(MaplyComponentObject *)baseObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add widened vectors
- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add vectors that we'll only use for selection
- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors desc:(NSDictionary *)desc;

// Change vector representation
- (void)changeVectors:(MaplyComponentObject *)vecObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add shapes
- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add model instances
- (MaplyComponentObject *)addModelInstances:(NSArray *)modelInstances desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add raw geometry
- (MaplyComponentObject *)addGeometry:(NSArray *)geom desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add stickers
- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Modify stickers
- (void)changeSticker:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add lofted polys
- (MaplyComponentObject *)addLoftedPolys:(NSArray *)vectors desc:(NSDictionary *)desc key:(NSString *)key cache:(NSObject<WhirlyKitLoftedPolyCache> *)cache mode:(MaplyThreadMode)threadMode;

// Add billboards
- (MaplyComponentObject *)addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add a particle system
- (MaplyComponentObject *)addParticleSystem:(MaplyParticleSystem *)partSys desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Add a particle system batch
- (void)addParticleBatch:(MaplyParticleBatch *)batch mode:(MaplyThreadMode)threadMode;

// Remove objects associated with the user objects
- (void)removeObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode;

// Enable objects
- (void)enableObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode;

// Disable objects
- (void)disableObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode;

// Explicitly add a texture
- (MaplyTexture *)addTexture:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Explicitly remove a texture
- (void)removeTextures:(NSArray *)textures mode:(MaplyThreadMode)threadMode;

// Add a texture to an atlas
- (MaplyTexture *)addTextureToAtlas:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;

// Start collecting changes for this thread
- (void)startChanges;

// Flush out outstanding changes for this thread
- (void)endChanges;

///// Internal routines.  Don't ever call these outside of the layer thread.

// An internal routine to add an image to our local UIImage/ID cache
- (MaplyTexture *)addImage:(id)image imageFormat:(MaplyQuadImageFormat)imageFormat wrapFlags:(int)wrapFlags interpType:(GLenum)interpType mode:(MaplyThreadMode)threadMode;

// This version defaults the wrap flags
- (MaplyTexture *)addImage:(id)image imageFormat:(MaplyQuadImageFormat)imageFormat mode:(MaplyThreadMode)threadMode;

// Remove the texture associated with an image  or just decrement its reference count
- (void)removeImageTexture:(MaplyTexture *)tex changes:(WhirlyKit::ChangeSet &)changes;

// Do a point in poly check for vectors we're representing
- (NSArray *)findVectorsInPoint:(WhirlyKit::Point2f)pt;
- (NSArray *)findVectorsInPoint:(WhirlyKit::Point2f)pt inView:(MaplyBaseViewController*)vc multi:(bool)multi;

// Find the Maply object corresponding to the given ID (from the selection manager).
// Thread-safe
- (NSObject *)getSelectableObject:(WhirlyKit::SimpleIdentity)objId;

// Called right before asking us to do some work
- (bool)startOfWork;

// Called right after asking for some work
- (void)endOfWork;

// Shutdown that waits for absolutely everything to end
- (void)lockingShutdown;

// Clean up a given texture
- (void)clearTexture:(MaplyTexture *)tex;

@end

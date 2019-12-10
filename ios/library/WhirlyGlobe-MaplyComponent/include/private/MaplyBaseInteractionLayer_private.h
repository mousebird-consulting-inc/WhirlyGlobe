/*
 *  MaplyBaseInteractionLayer_private.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
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

#import <Foundation/Foundation.h>
#import <set>
#import <WhirlyGlobe_iOS.h>
#import "MaplyComponentObject_private.h"
#import "ImageTexture_private.h"
#import "control/MaplyBaseViewController.h"
#import "MaplyTextureAtlas_private.h"
#import "ComponentManager_iOS.h"
#import "MemManagerGLES.h"

@interface MaplyBaseInteractionLayer : NSObject<WhirlyKitLayer>
{
@public
    WhirlyKit::ViewRef visualView;
    const WhirlyKit::RenderSetupInfo *setupInfo;
    
    // Layer we were started on (probably the regular MainThread)
    NSThread * __weak mainThread;

    // Layer thread we're part of
    WhirlyKitLayerThread * __weak layerThread;

    // Scene we're using
    WhirlyKit::Scene *scene;
    
    WhirlyKit::SceneRenderer *sceneRender;
    
    // Pointer to the layerThreads we're using in the base view controller
    NSArray *layerThreads;

    // Used to track groups of low level objects and vectors
    WhirlyKit::ComponentManager_iOS *compManager;

    std::mutex imageLock;
    // Used to track textures
    MaplyImageTextureList imageTextures;
        
    // Texture atlas manager
    MaplyTextureAtlasGroup *atlasGroup;
    
    /// Active shaders
    NSMutableArray *shaders;
        
    std::mutex tempContextLock;
    // We keep a set of temporary OpenGL ES contexts around for threads that don't have them
    std::set<EAGLContext *> tempContexts;
    
    // Set when we're trying to shut things down
    bool isShuttingDown;
}

// Offset for draw priorities on screen objects
@property (nonatomic,assign) int screenObjectDrawPriorityOffset;

// Initialize with the view we'll be using
- (instancetype __nonnull)initWithView:(WhirlyKit::ViewRef)visualView;

// Add screen space (2D) markers
- (MaplyComponentObject *__nullable)addScreenMarkers:(NSArray * __nonnull)markers desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Add a marker cluster generator
- (void)addClusterGenerator:(NSObject <MaplyClusterGenerator> *__nonnull)clusterGen;

// Add 3D markers
- (MaplyComponentObject *__nullable)addMarkers:(NSArray *__nonnull)markers desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Add screen space (2D) labels
- (MaplyComponentObject *__nullable)addScreenLabels:(NSArray *__nonnull)labels desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Add 3D labels
- (MaplyComponentObject *__nullable)addLabels:(NSArray *__nonnull)labels desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Add vectors
- (MaplyComponentObject *__nullable)addVectors:(NSArray *__nonnull)vectors desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Instance vectors
- (MaplyComponentObject *__nullable)instanceVectors:(MaplyComponentObject *__nonnull)baseObj desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Add widened vectors
- (MaplyComponentObject *__nullable)addWideVectors:(NSArray *__nonnull)vectors desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Add vectors that we'll only use for selection
- (MaplyComponentObject *__nullable)addSelectionVectors:(NSArray *__nonnull)vectors desc:(NSDictionary * __nullable)desc;

// Change vector representation
- (void)changeVectors:(MaplyComponentObject *__nonnull)vecObj desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Add shapes
- (MaplyComponentObject *__nullable)addShapes:(NSArray *__nonnull)shapes desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Add model instances
- (MaplyComponentObject *__nullable)addModelInstances:(NSArray *__nonnull)modelInstances desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Add raw geometry
- (MaplyComponentObject *__nullable)addGeometry:(NSArray *__nonnull)geom desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Add stickers
- (MaplyComponentObject *__nullable)addStickers:(NSArray *__nonnull)stickers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Modify stickers
- (void)changeSticker:(MaplyComponentObject *__nonnull)compObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Add lofted polys
- (MaplyComponentObject *__nullable)addLoftedPolys:(NSArray *__nonnull)vectors desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Add billboards
- (MaplyComponentObject *__nullable)addBillboards:(NSArray *__nonnull)billboards desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Add a particle system
- (MaplyComponentObject *__nullable)addParticleSystem:(MaplyParticleSystem *__nonnull)partSys desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Change the render target for a particle system
- (void)changeParticleSystem:(MaplyComponentObject *__nonnull)compObj renderTarget:(MaplyRenderTarget * __nullable)target;

// Add a particle system batch
- (void)addParticleBatch:(MaplyParticleBatch *__nonnull)batch mode:(MaplyThreadMode)threadMode;

// Add a group of points
- (MaplyComponentObject *__nullable)addPoints:(NSArray *__nonnull)points desc:(NSDictionary * __nullable)desc mode:(MaplyThreadMode)threadMode;

// Remove objects associated with the user objects, but just generate the changes don't flush them
- (void)removeObjects:(NSArray *__nonnull)userObjs mode:(MaplyThreadMode)threadMode;

// Remove objects associated with the user objects
- (void)removeObjects:(NSArray *__nonnull)userObjs changes:(WhirlyKit::ChangeSet &)changes;

// Enable objects
- (void)enableObjects:(NSArray *__nonnull)userObjs mode:(MaplyThreadMode)threadMode;

// Enable objects, but just generate the changes don't flush them
- (void)enableObjects:(NSArray *__nonnull)userObjs changes:(WhirlyKit::ChangeSet &)changes;

// Disable objects
- (void)disableObjects:(NSArray *__nonnull)userObjs mode:(MaplyThreadMode)threadMode;

// Disable objects, but just generate the changes don't flush them
- (void)disableObjects:(NSArray *__nonnull)userObjs changes:(WhirlyKit::ChangeSet &)changes;

// Pass through a uniform block
- (void)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID forObjects:(NSArray<MaplyComponentObject *> *__nonnull)compObjs mode:(MaplyThreadMode)threadMode;

// Explicitly add a texture
- (MaplyTexture *__nullable)addTexture:(UIImage *__nullable)image desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Explicitly remove a texture
- (void)removeTextures:(NSArray *__nonnull)textures mode:(MaplyThreadMode)threadMode;

// Add a texture to an atlas
- (MaplyTexture *__nullable)addTextureToAtlas:(UIImage *__nonnull)image desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode;

// Add a subtexture that references and existing texture
- (MaplyTexture *__nullable)addSubTexture:(MaplyTexture *__nonnull)tex xOffset:(int)x yOffset:(int)y width:(int)width height:(int)height mode:(MaplyThreadMode)threadMode;

// Start collecting changes for this thread
- (void)startChanges;

// Flush out outstanding changes for this thread
- (void)endChanges;

// Add a render target to the renderer
- (void)addRenderTarget:(MaplyRenderTarget *__nonnull)renderTarget;

// Change the texture being used by a render target
- (void)changeRenderTarget:(MaplyRenderTarget *__nonnull)renderTarget tex:(MaplyTexture * __nullable)tex;

// Ask for a one time clear on the render target in the coming frame
- (void)clearRenderTarget:(MaplyRenderTarget *__nonnull)renderTarget mode:(MaplyThreadMode)threadMode;

// Stop rendering to a given render target
- (void)removeRenderTarget:(MaplyRenderTarget *__nonnull)renderTarget;

// Remove the texture associated with an image  or just decrement its reference count
- (void)removeImageTexture:(MaplyTexture *__nonnull)tex changes:(WhirlyKit::ChangeSet &)changes;

// Do a point in poly check for vectors we're representing
- (NSArray *__nullable)findVectorsInPoint:(WhirlyKit::Point2f)pt;
- (NSArray *__nullable)findVectorsInPoint:(WhirlyKit::Point2f)pt inView:(MaplyBaseViewController*__nullable)vc multi:(bool)multi;

- (NSObject*__nullable)selectLabelsAndMarkerForScreenPoint:(CGPoint)screenPoint;

// Find the Maply object corresponding to the given ID (from the selection manager).
// Thread-safe
- (NSObject *__nullable)getSelectableObject:(WhirlyKit::SimpleIdentity)objId;

// Called right before asking us to do some work
- (bool)startOfWork;

// Called right after asking for some work
- (void)endOfWork;

// Shutdown that waits for absolutely everything to end
- (void)lockingShutdown;

// Clean up a given texture
- (void)clearTexture:(MaplyTexture *__nonnull)tex when:(NSTimeInterval)when;

// Write out usage stats
- (void)dumpStats;

///// Internal routines.  Don't ever call these outside of the layer thread.

// An internal routine to add an image to our local UIImage/ID cache
- (MaplyTexture *__nullable)addImage:(id __nonnull)image imageFormat:(MaplyQuadImageFormat)imageFormat wrapFlags:(int)wrapFlags interpType:(GLenum)interpType mode:(MaplyThreadMode)threadMode;

// This version defaults the wrap flags
- (MaplyTexture *__nullable)addImage:(id __nonnull)image imageFormat:(MaplyQuadImageFormat)imageFormat mode:(MaplyThreadMode)threadMode;

@end

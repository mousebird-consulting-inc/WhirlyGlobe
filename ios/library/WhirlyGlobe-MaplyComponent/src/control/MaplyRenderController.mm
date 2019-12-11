/*
 *  MaplyRenderController.h
 *  WhirlyGlobeMaplyComponent
 *
 *  Created by Stephen Gifford on 1/19/18.
 *  Copyright 2012-2018 Saildrone Inc.
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

#import "MaplyRenderController_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "UIKit/NSData+Zlib.h"

#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"
#import "UIKit/NSDictionary+StyleRules.h"

#import "SceneGLES.h"
#import "SceneMTL.h"
#import "SceneRendererMTL.h"
#import "MaplyActiveObject_private.h"

using namespace WhirlyKit;
using namespace Eigen;


@implementation MaplyRenderController
{
    // This view is used when we're not provided with a real view
    PassThroughCoordSystemRef coordSys;
    GeneralCoordSystemDisplayAdapterRef genCoordAdapter;
    Maply::FlatViewRef flatView;

    bool offlineMode;
    NSData *snapshotData;
    CGSize initialFramebufferSize;
}

- (instancetype __nullable)init
{
    self = [super init];
    mainThread = [NSThread currentThread];

    tileFetcherConnections = 16;
    userLayers = [NSMutableArray array];

    return self;
}

- (instancetype __nullable)initWithSize:(CGSize)size
{
    return [self initWithSize:size mode:MaplyRenderMetal];
}

- (instancetype)initWithSize:(CGSize)size mode:(MaplyRenderType)inRenderType
{
    self = [self init];

    mainThread = [NSThread currentThread];
    offlineMode = true;
    initialFramebufferSize = size;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (inRenderType == MaplyRenderGLES) {
        renderType = SceneRenderer::RenderGLES;
    } else {
        renderType = SceneRenderer::RenderMetal;
    }
    
    // If the coordinate system hasn't been set up, we'll do a flat one
    if (!coordAdapter) {
        // Coordinate system and view that just pass coordinates through
        coordSys = PassThroughCoordSystemRef(new PassThroughCoordSystem());
        Point3d ll(0.0,0.0,0.0),ur(size.width,size.height,0.0);
        Point3d scale(1.0,1.0,1.0);
        Point3d center = (ll+ur)/2.0;
        genCoordAdapter = GeneralCoordSystemDisplayAdapterRef(new GeneralCoordSystemDisplayAdapter(coordSys.get(),ll,ur,center,scale));
        coordAdapter = genCoordAdapter.get();
        flatView = Maply::FlatViewRef(new Maply::FlatView(coordAdapter));
        visualView = flatView;
        Mbr extents;
        extents.addPoint(Point2f(ll.x(),ll.y()));
        extents.addPoint(Point2f(ur.x(),ur.y()));
        flatView->setExtents(extents);
        flatView->setWindow(Point2d(size.width,size.height),Point2d(0.0,0.0));
    }
    
    [self loadSetup];
    
    [self loadSetup_scene:[[MaplyBaseInteractionLayer alloc] initWithView:visualView]];
    [self setupShaders];
    
    if (inRenderType == MaplyRenderGLES) {
        if (oldContext)
            [EAGLContext setCurrentContext:oldContext];
    }
    
    return self;
}

- (void)teardown
{
    SceneRendererGLES_iOSRef sceneRendererGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);
    
    for (auto tileFetcher : tileFetchers)
        [tileFetcher shutdown];
    tileFetchers.clear();
    
    defaultClusterGenerator = nil;

    if (baseLayerThread)
    {
        // Kill off all the other layers first
        for (unsigned int ii=1;ii<[layerThreads count];ii++)
        {
            WhirlyKitLayerThread *layerThread = [layerThreads objectAtIndex:ii];
            if (layerThread != baseLayerThread)
                [baseLayerThread addThreadToShutdown:layerThread];
        }

        [baseLayerThread addThingToDelete:scene];
        [baseLayerThread addThingToRelease:baseLayerThread];
        [baseLayerThread cancel];
        
        // Wait for the base layer thread to finish
        baseLayerThread->existenceLock.lock();
    }

    EAGLContext *oldContext = nil;
    if (sceneRendererGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRendererGLES->useContext();
    }
    // This stuff is our responsibility if we created it
    if (offlineMode)
    {
        if (interactLayer)
            [interactLayer teardown];
        interactLayer = nil;
        flatView = NULL;
        genCoordAdapter = NULL;
        visualView = NULL;
        coordAdapter = NULL;
        coordSys = NULL;
        scene = NULL;
    }
    sceneRenderer = nil;
    if (sceneRendererGLES && oldContext)
        [EAGLContext setCurrentContext:oldContext];
    
        layerThreads = nil;
    //    NSLog(@"BaseViewController: Layers shut down");
    fontTexManager = NULL;
    baseLayerThread = nil;
    layoutLayer = nil;

    activeObjects = nil;
    
    while ([userLayers count] > 0)
    {
        MaplyControllerLayer *layer = [userLayers objectAtIndex:0];
        [userLayers removeObject:layer];
    }
    userLayers = nil;
}

- (void)clear
{
    scene = nil;
    if (sceneRenderer)
        sceneRenderer->setScene(NULL);
    interactLayer = nil;
    theClearColor = nil;
}

- (void)loadSetup
{    
    screenDrawPriorityOffset = 1000000;
    
    if (renderType == WhirlyKit::SceneRendererGLES_iOS::RenderGLES) {
        // Set up the OpenGL ES renderer
        SceneRendererGLES_iOSRef sceneRendererGLES =
            SceneRendererGLES_iOSRef(
            offlineMode ?
        new SceneRendererGLES_iOS((int)initialFramebufferSize.width,(int)initialFramebufferSize.height,1.0)
            :
            new SceneRendererGLES_iOS(1.0)
                                     );
            
        sceneRendererGLES->useContext();
        sceneRenderer = sceneRendererGLES;
    } else {
        SceneRendererMTLRef sceneRendererMTL = SceneRendererMTLRef(new SceneRendererMTL(MTLCreateSystemDefaultDevice(),1.0));
        if (offlineMode)
            sceneRendererMTL->setup((int)initialFramebufferSize.width,(int)initialFramebufferSize.height, true);
        sceneRenderer = sceneRendererMTL;
    }

    sceneRenderer->setZBufferMode(zBufferOffDefault);
    sceneRenderer->setClearColor([[UIColor blackColor] asRGBAColor]);
    
    // Turn on the model matrix optimization for drawing
    sceneRenderer->setUseViewChanged(true);
}

- (void)loadSetup_view:(WhirlyKit::ViewRef)view
{
    visualView = view;
}

- (void)loadSetup_scene:(MaplyBaseInteractionLayer *)newInteractLayer
{
    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);

    if (sceneRenderGLES) {
        scene = new SceneGLES(visualView->coordAdapter);
    } else {
        scene = new SceneMTL(visualView->coordAdapter);
    }
    sceneRenderer->setScene(scene);

    // Set up a Font Texture Manager
    fontTexManager = FontTextureManager_iOSRef(new FontTextureManager_iOS(sceneRenderer.get(),scene));
    scene->setFontTextureManager(fontTexManager);
    
    layerThreads = [NSMutableArray array];
    
    // Need a layer thread to manage the layers
    baseLayerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView.get() renderer:sceneRenderer.get() mainLayerThread:true];
    [layerThreads addObject:baseLayerThread];
    
    // Layout still needs a layer to kick it off
    layoutLayer = [[WhirlyKitLayoutLayer alloc] initWithRenderer:sceneRenderer.get()];
    [baseLayerThread addLayer:layoutLayer];
    
    if (newInteractLayer) {
        interactLayer = newInteractLayer;
        interactLayer.screenObjectDrawPriorityOffset = [self screenObjectDrawPriorityOffset];
    }
    
    // Give the renderer what it needs
    sceneRenderer->setView(visualView.get());
    
    [self setupShaders];
    
    // Kick off the layer thread
    // This will start loading things
    [baseLayerThread start];
    
    // Default cluster generator
    defaultClusterGenerator = [[MaplyBasicClusterGenerator alloc] initWithColors:@[[UIColor orangeColor]] clusterNumber:0 size:CGSizeMake(32,32) viewC:self];
    [self addClusterGenerator:defaultClusterGenerator];

    interactLayer->layerThreads = layerThreads;
    [baseLayerThread addLayer:interactLayer];
}

- (void)setScreenObjectDrawPriorityOffset:(int)drawPriorityOffset
{
    screenDrawPriorityOffset = drawPriorityOffset;
    if (interactLayer)
        interactLayer.screenObjectDrawPriorityOffset = screenDrawPriorityOffset;
}

- (int)screenObjectDrawPriorityOffset
{
    return screenDrawPriorityOffset;
}

- (UIImage *)renderToImage
{
    if (!sceneRenderer)
        return nil;

    UIImage *toRet = nil;
    if (sceneRenderer->getType() == SceneRenderer::RenderGLES) {
        SceneRendererGLES_iOSRef sceneRendererGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);
        
        // TODO: Implement for Metal
        
        EAGLContext *oldContext = [EAGLContext currentContext];

        [self useGLContext];

        sceneRendererGLES->addSnapshotDelegate(self);
        sceneRendererGLES->forceDrawNextFrame();
        sceneRendererGLES->render(0.0);

        // TODO: Convert to image

        snapshotData = nil;
        sceneRendererGLES->removeSnapshotDelegate(self);

        [EAGLContext setCurrentContext:oldContext];
    } else {
        // TODO: Convert to image

    }

        // Note: Convert to image
        
    
    return toRet;
}

- (NSData *)renderToImageData
{
    if (!sceneRenderer)
        return nil;

    NSData *toRet = nil;
    if (sceneRenderer->getType() == SceneRenderer::RenderGLES) {
        SceneRendererGLES_iOSRef sceneRendererGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);
        
        EAGLContext *oldContext = [EAGLContext currentContext];
        
        [self useGLContext];

        sceneRendererGLES->addSnapshotDelegate(self);
        sceneRendererGLES->forceDrawNextFrame();
        sceneRendererGLES->render(0.0);
        
        toRet = snapshotData;
        snapshotData = nil;
        sceneRendererGLES->removeSnapshotDelegate(self);

        [EAGLContext setCurrentContext:oldContext];
    } else {
        SceneRendererMTLRef sceneRendererMTL = std::dynamic_pointer_cast<SceneRendererMTL>(sceneRenderer);
        
        sceneRendererMTL->addSnapshotDelegate(self);
        sceneRendererMTL->forceDrawNextFrame();
        sceneRendererMTL->render(1.0/60.0,nil,nil);
        toRet = snapshotData;
        snapshotData = nil;
        sceneRendererMTL->removeSnapshotDelegate(self);
    }
    
    return toRet;
}

- (void) useGLContext
{
    SceneRendererGLES_iOSRef sceneRendererGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);
    if (!sceneRendererGLES)
        return;

    sceneRendererGLES->useContext();
}

- (void)clearLights
{
    lights = nil;
    [self updateLights];
}

- (void)resetLights
{
    [self clearLights];
    
    NSString *lightingType = hints[kWGRendererLightingMode];
    int lightingRegular = true;
    if ([lightingType respondsToSelector:@selector(compare:)])
        lightingRegular = [lightingType compare:@"none"];
    
    // Regular lighting is on by default
    if (!lightingRegular)
    {
        // Note: Porting
//        SimpleIdentity triNoLighting = scene->getProgramIDByName(kToolkitDefaultTriangleNoLightingProgram);
//        if (triNoLighting != EmptyIdentity)
//            scene->setSceneProgram(kSceneDefaultTriShader, triNoLighting);
    } else {
        // Add a default light
        MaplyLight *light = [[MaplyLight alloc] init];
        light.pos = MaplyCoordinate3dMake(0.75, 0.5, -1.0);
        light.ambient = [UIColor colorWithRed:0.6 green:0.6 blue:0.6 alpha:1.0];
        light.diffuse = [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
        light.viewDependent = false;
        [self addLight:light];
    }
}

- (void)addLight:(MaplyLight *__nonnull)light
{
    if (!lights)
        lights = [NSMutableArray array];
    [lights addObject:light];
    [self updateLights];
}

- (void)removeLight:(MaplyLight *__nonnull)light
{
    [lights removeObject:light];
    [self updateLights];
}

- (void)updateLights
{
    std::vector<DirectionalLight> newLights;
    for (MaplyLight *light in lights)
    {
        DirectionalLight theLight;
        theLight.pos = Vector3f(light.pos.x,light.pos.y,light.pos.z);
        theLight.ambient = [light.ambient asVec4];
        theLight.diffuse = [light.diffuse asVec4];
        theLight.viewDependent = light.viewDependent;
        newLights.push_back(theLight);
    }
    sceneRenderer->replaceLights(newLights);
}

- (void)addShaderProgram:(MaplyShader *__nonnull)shader
{
    if (!shader)
        return;
    if (!interactLayer)
        return;
    
    if (!shader.program) {
        NSLog(@"Shader %@ didn't compile.  Not adding to scene.",shader.name);
        return;
    }
    
    @synchronized (interactLayer->shaders) {
        if (![interactLayer->shaders containsObject:shader])
            [interactLayer->shaders addObject:shader];
    }
}

- (void)removeShaderProgram:(MaplyShader *__nonnull)shaderToRemove
{
    if (!interactLayer)
        return;

    @synchronized (interactLayer->shaders) {
        bool found = false;

        for (MaplyShader *shader in interactLayer->shaders) {
            if (shader == shaderToRemove) {
                found = true;
                break;
            }
        }

        if (!found)
            return;
        [interactLayer->shaders removeObject:shaderToRemove];
    }
    
    if (shaderToRemove.program)
        scene->removeProgram(shaderToRemove.program->getId());
}

- (MaplyShader *__nullable)getShaderByName:(NSString *__nonnull)name
{
    if (!interactLayer)
        return nil;

    @synchronized (interactLayer->shaders) {
        for (MaplyShader *shader in interactLayer->shaders)
            if (![shader.name compare:name])
                return shader;
    }
    
    return nil;
}

// Merge the two dictionaries, add taking precidence into account, and then look for NSNulls
- (NSDictionary *)mergeAndCheck:(NSDictionary *)baseDict changeDict:(NSDictionary *)addDict
{
    if (!addDict)
        return baseDict;
    
    NSMutableDictionary *newDict = [NSMutableDictionary dictionaryWithDictionary:baseDict];
    [newDict addEntriesFromDictionary:addDict];
    
    // Now look for NSNulls, which we use to remove things
    NSArray *keys = [newDict allKeys];
    for (NSString *key in keys)
    {
        NSObject *obj = [newDict objectForKey:key];
        if (obj && [obj isKindOfClass:[NSNull class]])
            [newDict removeObjectForKey:key];
    }
    
    return newDict;
}

- (void)setHints:(NSDictionary *__nonnull)hintsDict
{
    hints = [self mergeAndCheck:hints changeDict:hintsDict];
    
    // Settings we store in the hints
    BOOL zBuffer = [hints boolForKey:kWGRenderHintZBuffer default:false];
    sceneRenderer->setZBufferMode(zBuffer ? zBufferOn : zBufferOffDefault);
}

- (bool)startOfWork
{
    return [interactLayer startOfWork];
}

- (void)endOfWork
{
    [interactLayer endOfWork];
}

- (MaplyComponentObject *__nullable)addScreenMarkers:(NSArray *__nonnull)markers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addScreenMarkers:markers desc:desc mode:threadMode];
    
    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *__nullable)addMarkers:(NSArray *__nonnull)markers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addMarkers:markers desc:desc mode:threadMode];
    
    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *__nullable)addScreenLabels:(NSArray *__nonnull)labels desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addScreenLabels:labels desc:desc mode:threadMode];
    
    [self endOfWork];

    return compObj;
}

- (MaplyComponentObject *__nullable)addLabels:(NSArray *__nonnull)labels desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addLabels:labels desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *__nullable)addVectors:(NSArray *__nonnull)vectors desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addVectors:vectors desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *__nullable)instanceVectors:(MaplyComponentObject *__nonnull)baseObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer instanceVectors:baseObj desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *__nullable)addWideVectors:(NSArray *__nonnull)vectors desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addWideVectors:vectors desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *__nullable)addModelInstances:(NSArray *__nonnull)modelInstances desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addModelInstances:modelInstances desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *__nullable)addGeometry:(NSArray *__nonnull)geom desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addGeometry:geom desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (void)changeVector:(MaplyComponentObject *__nonnull)compObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return;

    [interactLayer changeVectors:compObj desc:desc mode:threadMode];

    [self endOfWork];
}

- (MaplyComponentObject *__nullable)addShapes:(NSArray *__nonnull)shapes desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addShapes:shapes desc:desc mode:threadMode];

    [self endOfWork];

    return compObj;
}

- (MaplyComponentObject *__nullable)addStickers:(NSArray *__nonnull)stickers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addStickers:stickers desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (void)changeSticker:(MaplyComponentObject *__nonnull)compObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return;

    [interactLayer changeSticker:compObj desc:desc mode:threadMode];

    [self endOfWork];
}

- (MaplyComponentObject *__nullable)addBillboards:(NSArray *__nonnull)billboards desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addBillboards:billboards desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addParticleSystem:(MaplyParticleSystem *)partSys desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addParticleSystem:partSys desc:desc mode:threadMode];

    [self endOfWork];

    return compObj;
}

- (void)changeParticleSystem:(MaplyComponentObject *__nonnull)compObj renderTarget:(MaplyRenderTarget *__nullable)target
{
    if ([NSThread currentThread] != mainThread) {
        NSLog(@"MaplyBaseViewController: changeParticleSystem:renderTarget: must be called on main thread");
        return;
    }
    
    if (![self startOfWork])
        return;
    
    [interactLayer changeParticleSystem:compObj renderTarget:target];

    [self endOfWork];
}

- (void)addParticleBatch:(MaplyParticleBatch *)batch mode:(MaplyThreadMode)threadMode
{
    if (![batch isValid])
        return;
    
    if (![self startOfWork])
        return;
    
    [interactLayer addParticleBatch:batch mode:threadMode];

    [self endOfWork];
}

- (MaplyComponentObject *__nullable)addLoftedPolys:(NSArray *__nonnull)polys desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addLoftedPolys:polys desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *__nullable)addPoints:(NSArray * __nonnull)points desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyComponentObject *compObj = [interactLayer addPoints:points desc:desc mode:threadMode];

    [self endOfWork];
    
    return compObj;
}

- (MaplyTexture *__nullable)addTexture:(UIImage *__nonnull)image desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyTexture *tex = [interactLayer addTexture:image desc:desc mode:threadMode];

    [self endOfWork];
    
    return tex;
}

- (MaplyTexture *__nullable)addSubTexture:(MaplyTexture *__nonnull)tex xOffset:(int)x yOffset:(int)y width:(int)width height:(int)height mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;

    MaplyTexture *maplyTex = [interactLayer addSubTexture:tex xOffset:x yOffset:y width:width height:height mode:threadMode];

    [self endOfWork];
    
    return maplyTex;
}


- (MaplyTexture *__nullable)createTexture:(NSDictionary * _Nullable)spec sizeX:(int)sizeX sizeY:(int)sizeY mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return nil;
    
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:spec];
    desc[kMaplyTexSizeX] = @(sizeX);
    desc[kMaplyTexSizeY] = @(sizeY);
    MaplyTexture *maplyTex = [interactLayer addTexture:nil desc:desc mode:threadMode];

    [self endOfWork];
    
    return maplyTex;
}

- (void)removeTextures:(NSArray *__nonnull)textures mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return;

    [interactLayer removeTextures:textures mode:threadMode];
    
    [self endOfWork];
}

- (void)addRenderTarget:(MaplyRenderTarget * _Nonnull)renderTarget
{
    if (![self startOfWork])
        return;

    [interactLayer addRenderTarget:renderTarget];

    [self endOfWork];
}

- (void)changeRenderTarget:(MaplyRenderTarget * __nonnull)renderTarget tex:(MaplyTexture * __nullable)tex
{
    if (![self startOfWork])
        return;

    [interactLayer changeRenderTarget:renderTarget tex:tex];

    [self endOfWork];
}

- (void)clearRenderTarget:(MaplyRenderTarget *)renderTarget mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return;
    
    [interactLayer clearRenderTarget:renderTarget mode:threadMode];

    [self endOfWork];
}

- (void)removeRenderTarget:(MaplyRenderTarget * _Nonnull)renderTarget
{
    if (![self startOfWork])
        return;

    [interactLayer removeRenderTarget:renderTarget];

    [self endOfWork];
}

- (void)removeObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return;

    [interactLayer removeObjects:[NSArray arrayWithArray:theObjs] mode:threadMode];

    [self endOfWork];
}

- (void)disableObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return;

    [interactLayer disableObjects:theObjs mode:threadMode];

    [self endOfWork];
}

- (void)enableObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return;

    [interactLayer enableObjects:theObjs mode:threadMode];

    [self endOfWork];
}

- (void)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID forObjects:(NSArray<MaplyComponentObject *> *__nonnull)compObjs mode:(MaplyThreadMode)threadMode
{
    if (![self startOfWork])
        return;

    [interactLayer setUniformBlock:uniBlock buffer:bufferID forObjects:compObjs mode:threadMode];

    [self endOfWork];
}

- (MaplyCoordinateSystem *__nullable)coordSystem
{
    // Note: Hack.  Should wrap the real coordinate system
    MaplyCoordinateSystem *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    return coordSys;
}

- (void)setClearColor:(UIColor *)clearColor
{
    theClearColor = clearColor;
    sceneRenderer->setClearColor([clearColor asRGBAColor]);
}

- (MaplyRenderController * __nullable)getRenderControl
{
    return self;
}

- (void)startChanges
{
}

- (void)endChanges
{
}

- (CGSize)getFramebufferSize
{
    if (!sceneRenderer)
        return CGSizeZero;
    
    return CGSizeMake(sceneRenderer->framebufferWidth,sceneRenderer->framebufferHeight);
}

// MARK: Snapshot protocol

- (WhirlyKit::SimpleIdentity)renderTargetID {
    return EmptyIdentity;
}

- (void)snapshotData:(NSData *)data {
    snapshotData = data;
}

- (bool)needSnapshot:(NSTimeInterval)now {
    return true;
}

- (CGRect)snapshotRect {
    return CGRectZero;
}


- (void)addShader:(NSString *)inName program:(ProgramRef)program
{
    if (!interactLayer)
        return;
    
    if (!program) {
        NSLog(@"Default shader setup:  Failed to create %@",inName);
        return;
    }
    
    std::string name = [inName cStringUsingEncoding:NSASCIIStringEncoding];
    MaplyShader *shader = [[MaplyShader alloc] initWithProgram:program viewC:self];
    shader.name = inName;
    @synchronized (interactLayer->shaders) {
        [interactLayer->shaders addObject:shader];
    }
}

- (MaplyRenderType)getRenderType
{
    switch (sceneRenderer->getType())
    {
        case SceneRenderer::RenderGLES:
            return MaplyRenderGLES;
        case SceneRenderer::RenderMetal:
            return MaplyRenderMetal;
        default:
            return MaplyRenderUnknown;
    }
}

- (void)setupShaders
{
    if (sceneRenderer->getType() == SceneRenderer::RenderGLES)
        [self setupShadersGL];
    else
        [self setupShadersMTL];
}

- (void)setupShadersMTL
{
    if (!interactLayer)
        return;
    
    bool isGlobe = !scene->getCoordAdapter()->isFlat();

    // Get the default library.  This should be bundled with WhirlyGlobe-Maply
    SceneRendererMTL *sceneRenderMTL = (SceneRendererMTL *)sceneRenderer.get();
    id<MTLDevice> mtlDevice = ((RenderSetupInfoMTL *)sceneRenderMTL->getRenderSetupInfo())->mtlDevice;
    NSError *err = nil;
    id<MTLLibrary> mtlLib = [mtlDevice newDefaultLibraryWithBundle:[NSBundle bundleForClass:[MaplyRenderController class]] error:&err];
    
    ProgramRef defaultLineShader = ProgramRef(new ProgramMTL([kMaplyShaderDefaultLine cStringUsingEncoding:NSASCIIStringEncoding],
                                                             [mtlLib newFunctionWithName:@"vertexLineOnly_globe"],
                                                             [mtlLib newFunctionWithName:@"framentLineOnly_globe"]));
    ProgramRef defaultLineShaderNoBack = ProgramRef(new ProgramMTL([kMaplyShaderDefaultLineNoBackface cStringUsingEncoding:NSASCIIStringEncoding],
                                                             [mtlLib newFunctionWithName:@"vertexLineOnly_flat"],
                                                             [mtlLib newFunctionWithName:@"fragmentLineOnly_flat"]));
    if (isGlobe)
        [self addShader:kMaplyShaderDefaultLine program:defaultLineShader];
    else
        [self addShader:kMaplyShaderDefaultLine program:defaultLineShaderNoBack];
    [self addShader:kMaplyShaderDefaultLineNoBackface program:defaultLineShaderNoBack];

    // Default triangle shaders
    [self addShader:kMaplyShaderDefaultTri
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultTri cStringUsingEncoding:NSASCIIStringEncoding],
                                              [mtlLib newFunctionWithName:@"vertexTri_light"],
                                              [mtlLib newFunctionWithName:@"fragmentTri_basic"]))];
    [self addShader:kMaplyShaderDefaultTriNoLighting
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultTriNoLighting cStringUsingEncoding:NSASCIIStringEncoding],
                                              [mtlLib newFunctionWithName:@"vertexTri_noLight"],
                                              [mtlLib newFunctionWithName:@"fragmentTri_basic"]))];
    
    // TODO: Screen Space Texture application

    // Multitexture shader - Used for animation
    [self addShader:kMaplyShaderDefaultTriMultiTex
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultTriMultiTex cStringUsingEncoding:NSASCIIStringEncoding],
                                              [mtlLib newFunctionWithName:@"vertexTri_multiTex"],
                                              [mtlLib newFunctionWithName:@"fragmentTri_multiTex"]))];
    
    // Multitexture ramp shader - Very simple implementation of animated color lookup
    [self addShader:kMaplyShaderDefaultTriMultiTexRamp
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultTriMultiTexRamp cStringUsingEncoding:NSASCIIStringEncoding],
                                              [mtlLib newFunctionWithName:@"vertexTri_multiTex"],
                                              [mtlLib newFunctionWithName:@"fragmentTri_multiTexRamp"]))];
    
    // MultiTexture for Markers
    [self addShader:kMaplyShaderDefaultMarker
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultTriMultiTex cStringUsingEncoding:NSASCIIStringEncoding],
                                              [mtlLib newFunctionWithName:@"vertexTri_multiTex"],
                                              [mtlLib newFunctionWithName:@"fragmentTri_multiTex"]))];

    // Model Instancing
    [self addShader:kMaplyShaderDefaultModelTri
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultModelTri cStringUsingEncoding:NSASCIIStringEncoding],
                                              [mtlLib newFunctionWithName:@"vertexTri_model"],
                                              [mtlLib newFunctionWithName:@"fragmentTri_multiTex"]))];

    // TODO: Night/Day Shader

    // Billboards
    ProgramRef billboardProg(new ProgramMTL([kMaplyShaderBillboardGround cStringUsingEncoding:NSASCIIStringEncoding],
                                            [mtlLib newFunctionWithName:@"vertexTri_billboard"],
                                            [mtlLib newFunctionWithName:@"fragmentTri_basic"]));
    [self addShader:kMaplyShaderBillboardGround program:billboardProg];
    [self addShader:kMaplyShaderBillboardEye program:billboardProg];

    // Wide vectors
    [self addShader:kMaplyShaderDefaultWideVector
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultWideVector cStringUsingEncoding:NSASCIIStringEncoding],
                                        [mtlLib newFunctionWithName:@"vertexTri_wideVec"],
                                        [mtlLib newFunctionWithName:@"fragmentTri_wideVec"]))];

    // Screen Space (motion and regular are the same)
    ProgramRef screenSpace = ProgramRef(new
            ProgramMTL([kMaplyScreenSpaceDefaultProgram cStringUsingEncoding:NSASCIIStringEncoding],
                       [mtlLib newFunctionWithName:@"vertexTri_screenSpace"],
                       [mtlLib newFunctionWithName:@"fragmentTri_basic"]));
    [self addShader:kMaplyScreenSpaceDefaultProgram program:screenSpace];
    [self addShader:kMaplyScreenSpaceDefaultMotionProgram program:screenSpace];

    // TODO: Particles
}

// Install the various shaders we expect to be running
- (void)setupShadersGL
{
    if (!interactLayer)
        return;
    
    [self useGLContext];
    
    bool isGlobe = !scene->getCoordAdapter()->isFlat();

    // Default line shaders
    ProgramGLESRef defaultLineShader(BuildDefaultLineShaderCullingGLES([kMaplyShaderDefaultLine cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()));
    ProgramGLESRef defaultLineShaderNoBack(BuildDefaultLineShaderNoCullingGLES([kMaplyShaderDefaultLineNoBackface cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()));
    if (isGlobe)
        [self addShader:kMaplyShaderDefaultLine program:defaultLineShader];
    else
        [self addShader:kMaplyShaderDefaultLine program:defaultLineShaderNoBack];
    [self addShader:kMaplyShaderDefaultLineNoBackface program:defaultLineShaderNoBack];
    
    // Default triangle shaders
    [self addShader:kMaplyShaderDefaultTri
            program:ProgramGLESRef(BuildDefaultTriShaderLightingGLES([kMaplyShaderDefaultTri cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    [self addShader:kMaplyShaderDefaultTriNoLighting
            program:ProgramGLESRef(BuildDefaultTriShaderNoLightingGLES([kMaplyShaderDefaultTriNoLighting cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    
    // Model instancing
    [self addShader:kMaplyShaderDefaultModelTri
            program:ProgramGLESRef(BuildDefaultTriShaderModelGLES([kMaplyShaderDefaultModelTri cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    
    // Screen space texture application
    [self addShader:kMaplyShaderDefaultTriScreenTex
            program:ProgramGLESRef(BuildDefaultTriShaderScreenTextureGLES([kMaplyShaderDefaultTriScreenTex cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    
    // Multi-texture support
    [self addShader:kMaplyShaderDefaultTriMultiTex
            program:ProgramGLESRef(BuildDefaultTriShaderMultitexGLES([kMaplyShaderDefaultTriMultiTex cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    // Same one, but for markers.  This fixes any weird conflicts
    [self addShader:kMaplyShaderDefaultMarker program:ProgramGLESRef(BuildDefaultTriShaderMultitexGLES([kMaplyShaderDefaultMarker cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    
    // Ramp texture support
    [self addShader:kMaplyShaderDefaultTriMultiTexRamp
            program:ProgramGLESRef(BuildDefaultTriShaderRamptexGLES([kMaplyShaderDefaultTriMultiTexRamp cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];

    // Night/day shading for globe
    [self addShader:kMaplyShaderDefaultTriNightDay
            program:ProgramGLESRef(BuildDefaultTriShaderNightDayGLES([kMaplyShaderDefaultTriNightDay cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];

    // Billboards
    [self addShader:kMaplyShaderBillboardGround program:ProgramGLESRef(BuildBillboardGroundProgramGLES([kMaplyShaderBillboardGround cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    [self addShader:kMaplyShaderBillboardEye program:ProgramGLESRef(BuildBillboardEyeProgramGLES([kMaplyShaderBillboardEye cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    // Wide vectors
    if (isGlobe)
        [self addShader:kMaplyShaderDefaultWideVector program:ProgramGLESRef(BuildWideVectorGlobeProgramGLES([kMaplyShaderDefaultWideVector cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    else
        [self addShader:kMaplyShaderDefaultWideVector program:ProgramGLESRef(BuildWideVectorProgramGLES([kMaplyShaderDefaultWideVector cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    // Screen space
    [self addShader:kMaplyScreenSpaceDefaultMotionProgram program:ProgramGLESRef(BuildScreenSpaceProgramGLES([kMaplyScreenSpaceDefaultMotionProgram cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    [self addShader:kMaplyScreenSpaceDefaultProgram program:ProgramGLESRef(BuildScreenSpaceMotionProgramGLES([kMaplyScreenSpaceDefaultProgram cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
    // Particles
    [self addShader:kMaplyShaderParticleSystemPointDefault program:ProgramGLESRef(BuildParticleSystemProgramGLES([kMaplyShaderParticleSystemPointDefault cStringUsingEncoding:NSASCIIStringEncoding],sceneRenderer.get()))];
}

- (void)addActiveObject:(MaplyActiveObject *)theObj
{
    if ([NSThread currentThread] != mainThread)
    {
        NSLog(@"Must call addActiveObject: on the main thread.");
        return;
    }
        
    if (!activeObjects)
        activeObjects = [NSMutableArray array];

    if (![activeObjects containsObject:theObj])
    {
        theObj->scene = scene;
        [theObj registerWithScene];
        [activeObjects addObject:theObj];
    }
}

- (void)removeActiveObject:(MaplyActiveObject *)theObj
{
    if ([NSThread currentThread] != mainThread)
    {
        NSLog(@"Must call removeActiveObject: on the main thread.");
        return;
    }
    
    if ([activeObjects containsObject:theObj])
    {
        [theObj removeFromScene];
        [activeObjects removeObject:theObj];
    }
}

- (void)removeActiveObjects:(NSArray *)theObjs
{
    if ([NSThread currentThread] != mainThread)
    {
        NSLog(@"Must call removeActiveObject: on the main thread.");
        return;
    }

    for (MaplyActiveObject *theObj in theObjs)
        [self removeActiveObject:theObj];
}

- (bool)addLayer:(MaplyControllerLayer *)newLayer
{
    if (newLayer && ![userLayers containsObject:newLayer])
    {
        WhirlyKitLayerThread *layerThread = baseLayerThread;
        if ([newLayer isKindOfClass:[MaplyQuadSamplingLayer class]])
        {
            layerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView.get() renderer:sceneRenderer.get() mainLayerThread:false];
            [layerThreads addObject:layerThread];
            [layerThread start];
        }
        
        if ([newLayer startLayer:layerThread scene:scene renderer:sceneRenderer.get() viewC:self])
        {
            if (!newLayer.drawPriorityWasSet)
            {
                newLayer.drawPriority = 100*(layerDrawPriority++) + kMaplyImageLayerDrawPriorityDefault;
            }
            [userLayers addObject:newLayer];
            return true;
        }
    }
    
    return false;
}

- (void)removeLayer:(MaplyControllerLayer *)layer wait:(bool)wait
{
    bool found = false;
    MaplyControllerLayer *theLayer = nil;
    for (theLayer in userLayers)
    {
        if (theLayer == layer)
        {
            found = true;
            break;
        }
    }
    if (!found)
        return;
    
    WhirlyKitLayerThread *layerThread = layer.layerThread;
    [layer cleanupLayers:layerThread scene:scene];
    [userLayers removeObject:layer];
    
    // Need to shut down the layer thread too
    if (layerThread != baseLayerThread)
    {
        if ([layerThreads containsObject:layerThread])
        {
            [layerThreads removeObject:layerThread];
            [layerThread addThingToRelease:theLayer];
            [layerThread cancel];

            if (wait) {
                // We also have to make sure it actually does finish
                bool finished = true;
                do {
                    finished = [layerThread isFinished];
                    if (!finished)
                        [NSThread sleepForTimeInterval:0.0001];
                } while (!finished);
            }
        }
    }
}

- (void)removeLayer:(MaplyControllerLayer *)layer
{
    [self removeLayer:layer wait:false];
}

- (void)removeLayers:(NSArray *)layers
{
    if ([NSThread currentThread] != mainThread)
    {
        [self performSelector:@selector(removeLayers:) withObject:layers];
        return;
    }

    for (MaplyControllerLayer *layer in layers)
        [self removeLayer:layer];
}

- (void)removeAllLayers
{
    if ([NSThread currentThread] != mainThread)
    {
        [self performSelector:@selector(removeAllLayers) withObject:nil];
        return;
    }

    NSArray *allLayers = [NSArray arrayWithArray:userLayers];
    
    for (MaplyControllerLayer *theLayer in allLayers)
        [self removeLayer:theLayer];
}

- (MaplyQuadSamplingLayer *)findSamplingLayer:(const WhirlyKit::SamplingParams &)params forUser:(QuadTileBuilderDelegateRef)userObj
{
    if ([NSThread currentThread] != mainThread)
    {
        NSLog(@"Caller called findSamplerLayer:forUser: off of main thread.");
        return nil;
    }

    // Look for a matching sampler
    for (auto layer : samplingLayers) {
        if (layer.params == params) {
            [layer addBuilderDelegate:userObj];
            return layer;
        }
    }
    
    // Create a new sampler
    MaplyQuadSamplingLayer *layer = [[MaplyQuadSamplingLayer alloc] initWithParams:params];
    [layer addBuilderDelegate:userObj];
    [self addLayer:layer];
    samplingLayers.push_back(layer);
    
    return layer;
}

- (void)releaseSamplingLayer:(MaplyQuadSamplingLayer *)layer forUser:(QuadTileBuilderDelegateRef)userObj
{
    if ([NSThread currentThread] != mainThread)
    {
        NSLog(@"Caller called findSamplerLayer:forUser: off of main thread.");
        return;
    }

    [layer removeBuilderDelegate:userObj];
    
    if (layer.numClients == 0) {
        [self removeLayer:layer wait:false];
        auto it = std::find(samplingLayers.begin(),samplingLayers.end(),layer);
        if (it != samplingLayers.end())
            samplingLayers.erase(it);
    }
}

- (MaplyRemoteTileFetcher *)addTileFetcher:(NSString *)name
{
    for (auto tileFetcher : tileFetchers)
        if ([tileFetcher.name isEqualToString:name])
            return tileFetcher;
    
    MaplyRemoteTileFetcher *tileFetcher = [[MaplyRemoteTileFetcher alloc] initWithName:name connections:tileFetcherConnections];
    tileFetchers.push_back(tileFetcher);
    
    return tileFetcher;
}

- (void)addClusterGenerator:(NSObject <MaplyClusterGenerator> *)clusterGen
{
    [interactLayer addClusterGenerator:clusterGen];
}

- (void)runLayout
{
    [layoutLayer scheduleUpdateNow];
}

- (id<MTLDevice>)getMetalDevice
{
    if (sceneRenderer->getType() != SceneRenderer::RenderMetal)
        return nil;
    
    SceneRendererMTL *renderMTL = (SceneRendererMTL *)sceneRenderer.get();
    return renderMTL->setupInfo.mtlDevice;
}

- (id<MTLLibrary>)getMetalLibrary
{
    if (sceneRenderer->getType() != SceneRenderer::RenderMetal)
        return nil;

    SceneRendererMTL *renderMTL = (SceneRendererMTL *)sceneRenderer.get();
    NSError *err = nil;
    id<MTLLibrary> mtlLib = [renderMTL->setupInfo.mtlDevice newDefaultLibraryWithBundle:[NSBundle bundleForClass:[MaplyRenderController class]] error:&err];
    return mtlLib;

}

@end

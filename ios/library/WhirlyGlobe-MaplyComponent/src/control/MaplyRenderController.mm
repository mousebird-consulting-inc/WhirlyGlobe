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

using namespace WhirlyKit;
using namespace Eigen;


@implementation MaplyRenderController
{
    // This view is used when we're doing an offline renderer
    PassThroughCoordSystemRef coordSys;
    GeneralCoordSystemDisplayAdapterRef coordAdapter;
    Maply::FlatViewRef flatView;
    bool offlineMode;
    UIImage *snapshotImage;
    NSData *snapshotData;
}

- (instancetype __nullable)initWithSize:(CGSize)size
{
    return [self initWithSize:size mode:MaplyRenderMetal];
}

- (instancetype)initWithSize:(CGSize)size mode:(MaplyRenderType)renderType
{
    self = [super init];
    
    offlineMode = true;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    
    // Coordinate system and view that just pass coordinates through
    coordSys = PassThroughCoordSystemRef(new PassThroughCoordSystem());
    Point3d ll(0.0,0.0,0.0),ur(size.width,size.height,0.0);
    Point3d scale(1.0,1.0,1.0);
    Point3d center = (ll+ur)/2.0;
    coordAdapter = GeneralCoordSystemDisplayAdapterRef(new GeneralCoordSystemDisplayAdapter(coordSys.get(),ll,ur,center,scale));
    flatView = Maply::FlatViewRef(new Maply::FlatView(coordAdapter.get()));
    Mbr extents;
    extents.addPoint(Point2f(ll.x(),ll.y()));
    extents.addPoint(Point2f(ur.x(),ur.y()));
    flatView->setExtents(extents);
    flatView->setWindow(Point2d(size.width,size.height),Point2d(0.0,0.0));

    // Set up the renderer with a target size
    if (renderType == MaplyRenderGLES) {
        scene = new SceneGLES(coordAdapter.get());
        SceneRendererGLES_iOSRef theSceneRenderer = SceneRendererGLES_iOSRef(new SceneRendererGLES_iOS((int)size.width,(int)size.height));
        sceneRenderer = theSceneRenderer;
    } else {
        scene = new SceneMTL(coordAdapter.get());
        SceneRendererMTLRef theSceneRenderer = SceneRendererMTLRef(new SceneRendererMTL( MTLCreateSystemDefaultDevice()));
        sceneRenderer = theSceneRenderer;
    }
    sceneRenderer->setZBufferMode(zBufferOffDefault);
    sceneRenderer->setScene(scene);
    sceneRenderer->setView(flatView.get());
    sceneRenderer->setClearColor([[UIColor blackColor] asRGBAColor]);
    
    // Turn on the model matrix optimization for drawing
    sceneRenderer->setUseViewChanged(true);
    
    interactLayer = [[MaplyBaseInteractionLayer alloc] initWithView:flatView.get()];
    [interactLayer startWithThread:nil scene:scene];

    [self setupShaders];
    
    if (renderType == MaplyRenderGLES) {
        if (oldContext)
            [EAGLContext setCurrentContext:oldContext];
    }
    
    return self;
}

- (void)teardown
{
    SceneRendererGLES_iOSRef sceneRendererGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);
    
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
        if (flatView)
            flatView = nil;
        coordAdapter = NULL;
        coordSys = NULL;
        scene = NULL;
    }
    sceneRenderer = nil;
    if (sceneRendererGLES && oldContext)
        [EAGLContext setCurrentContext:oldContext];
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
        SceneRendererGLES_iOSRef sceneRendererGLES = SceneRendererGLES_iOSRef(new SceneRendererGLES_iOS());
        sceneRendererGLES->useContext();
        sceneRenderer = sceneRendererGLES;
    } else {
        SceneRendererMTLRef sceneRendererMTL = SceneRendererMTLRef(new SceneRendererMTL(MTLCreateSystemDefaultDevice()));
        sceneRenderer = sceneRendererMTL;
    }

    sceneRenderer->setZBufferMode(zBufferOffDefault);
    sceneRenderer->setClearColor([[UIColor blackColor] asRGBAColor]);
    
    // Turn on the model matrix optimization for drawing
    sceneRenderer->setUseViewChanged(true);
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
    SceneRendererGLES_iOSRef sceneRendererGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);
    if (!sceneRendererGLES)
        return nil;
    
    // TODO: Implement for Metal
    
    EAGLContext *oldContext = [EAGLContext currentContext];

    [self useGLContext];

    sceneRendererGLES->addSnapshotDelegate(self);
    sceneRendererGLES->render(0.0);
    
    UIImage *toRet = snapshotImage;
    snapshotImage = nil;
    snapshotData = nil;
    sceneRendererGLES->removeSnapshotDelegate(self);

    [EAGLContext setCurrentContext:oldContext];
    
    return toRet;
}

- (NSData *)renderToImageData
{
    SceneRendererGLES_iOSRef sceneRendererGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);
    if (!sceneRendererGLES)
        return nil;
    
    // TODO: Implement for Metal

    EAGLContext *oldContext = [EAGLContext currentContext];

    [self useGLContext];

    sceneRendererGLES->addSnapshotDelegate(self);
    sceneRendererGLES->render(0.0);
    
    NSData *toRet = snapshotData;
    snapshotImage = nil;
    snapshotData = nil;
    sceneRendererGLES->removeSnapshotDelegate(self);

    [EAGLContext setCurrentContext:oldContext];
    
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
    return [interactLayer addScreenMarkers:markers desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addMarkers:(NSArray *__nonnull)markers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addMarkers:markers desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addScreenLabels:(NSArray *__nonnull)labels desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addScreenLabels:labels desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addLabels:(NSArray *__nonnull)labels desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addLabels:labels desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addVectors:(NSArray *__nonnull)vectors desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addVectors:vectors desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)instanceVectors:(MaplyComponentObject *__nonnull)baseObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer instanceVectors:baseObj desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addWideVectors:(NSArray *__nonnull)vectors desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addWideVectors:vectors desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addModelInstances:(NSArray *__nonnull)modelInstances desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addModelInstances:modelInstances desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addGeometry:(NSArray *__nonnull)geom desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addGeometry:geom desc:desc mode:threadMode];
}

- (void)changeVector:(MaplyComponentObject *__nonnull)compObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    [interactLayer changeVectors:compObj desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addShapes:(NSArray *__nonnull)shapes desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addShapes:shapes desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addStickers:(NSArray *__nonnull)stickers desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addStickers:stickers desc:desc mode:threadMode];
}

- (void)changeSticker:(MaplyComponentObject *__nonnull)compObj desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    [interactLayer changeSticker:compObj desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addBillboards:(NSArray *__nonnull)billboards desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addBillboards:billboards desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addLoftedPolys:(NSArray *__nonnull)polys desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addLoftedPolys:polys desc:desc mode:threadMode];
}

- (MaplyComponentObject *__nullable)addPoints:(NSArray * __nonnull)points desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addPoints:points desc:desc mode:threadMode];
}

- (MaplyTexture *__nullable)addTexture:(UIImage *__nonnull)image desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addTexture:image desc:desc mode:threadMode];
}

- (MaplyTexture *__nullable)addSubTexture:(MaplyTexture *__nonnull)tex xOffset:(int)x yOffset:(int)y width:(int)width height:(int)height mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addSubTexture:tex xOffset:x yOffset:y width:width height:height mode:threadMode];
}


- (MaplyTexture *__nullable)createTexture:(NSDictionary * _Nullable)spec sizeX:(int)sizeX sizeY:(int)sizeY mode:(MaplyThreadMode)threadMode
{
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:spec];
    desc[kMaplyTexSizeX] = @(sizeX);
    desc[kMaplyTexSizeY] = @(sizeY);
    MaplyTexture *maplyTex = [interactLayer addTexture:nil desc:desc mode:threadMode];
    
    return maplyTex;
}

- (void)removeTextures:(NSArray *__nonnull)textures mode:(MaplyThreadMode)threadMode
{
    [interactLayer removeTextures:textures mode:threadMode];
}

- (void)addRenderTarget:(MaplyRenderTarget * _Nonnull)renderTarget
{
    [interactLayer addRenderTarget:renderTarget];
}

- (void)changeRenderTarget:(MaplyRenderTarget * __nonnull)renderTarget tex:(MaplyTexture * __nullable)tex
{
    [interactLayer changeRenderTarget:renderTarget tex:tex];
}

- (void)removeRenderTarget:(MaplyRenderTarget * _Nonnull)renderTarget
{
    [interactLayer removeRenderTarget:renderTarget];
}

- (void)removeObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode
{
    [interactLayer removeObjects:[NSArray arrayWithArray:theObjs] mode:threadMode];
}

- (void)disableObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode
{
    [interactLayer disableObjects:theObjs mode:threadMode];
}

- (void)enableObjects:(NSArray *__nonnull)theObjs mode:(MaplyThreadMode)threadMode
{
    [interactLayer enableObjects:theObjs mode:threadMode];
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

- (void)snapshotImage:(UIImage *)image {
    snapshotImage = image;
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
    ProgramRef defaultLineShaderNoBack = ProgramRef(new ProgramMTL([kMaplyShaderDefaultLine cStringUsingEncoding:NSASCIIStringEncoding],
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
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultTri cStringUsingEncoding:NSASCIIStringEncoding],
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
    
    // TODO: MultiTexture for Markers

    // TODO: Model Instancing

    // TODO: Night/Day Shader
    
    // TODO: Billboards
    
    // Wide vectors
    [self addShader:kMaplyShaderDefaultWideVector
            program:ProgramRef(new ProgramMTL([kMaplyShaderDefaultWideVector cStringUsingEncoding:NSASCIIStringEncoding],
                                        [mtlLib newFunctionWithName:@"vertexTri_wideVec"],
                                        [mtlLib newFunctionWithName:@"fragmentTri_wideVec"]))];

    // Screen Space
    // TODO: Motion Screen Space
    [self addShader:kMaplyScreenSpaceDefaultProgram
            program:ProgramRef(new ProgramMTL([kMaplyScreenSpaceDefaultProgram cStringUsingEncoding:NSASCIIStringEncoding],
                                      [mtlLib newFunctionWithName:@"vertexTri_screenSpace"],
                                      [mtlLib newFunctionWithName:@"fragmentTri_basic"]))];

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

@end

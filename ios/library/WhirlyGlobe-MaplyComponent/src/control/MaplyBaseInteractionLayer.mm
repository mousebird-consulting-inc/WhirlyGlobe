/*
 *  MaplyBaseInteractionLayer.mm
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

#import "MaplyBaseInteractionLayer_private.h"
#import "visual_objects/MaplyScreenMarker.h"
#import "visual_objects/MaplyMarker.h"
#import "visual_objects/MaplyScreenLabel.h"
#import "visual_objects/MaplyLabel.h"
#import "MaplyVectorObject_private.h"
#import "visual_objects/MaplyShape.h"
#import "visual_objects/MaplySticker.h"
#import "visual_objects/MaplyBillboard.h"
#import "math/MaplyCoordinate.h"
#import "ImageTexture_private.h"
#import "MaplySharedAttributes.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyTexture_private.h"
#import "MaplyMatrix_private.h"
#import "MaplyGeomModel_private.h"
#import "MaplyScreenObject_private.h"
#import "MaplyVertexAttribute_private.h"
#import "MaplyParticleSystem_private.h"
#import "MaplyShape_private.h"
#import "MaplyPoints_private.h"
#import "MaplyRenderTarget_private.h"
#import "Dictionary_NSDictionary.h"
#import "SingleLabel_iOS.h"
#import "FontTextureManager_iOS.h"
#import "TextureGLES_iOS.h"
#import "ComponentManager_iOS.h"
#import "SphericalEarthChunkManager.h"
#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"
#import "MaplyBaseViewController_private.h"

using namespace Eigen;
using namespace WhirlyKit;

// We store per thread changes we may be journaling here
class ThreadChanges
{
public:
    ThreadChanges() : thread(NULL) { }
    ThreadChanges(NSThread *thread) : thread(thread) { }
    
    // Comparison operator for set
    bool operator < (const ThreadChanges &that) const
    {
        return (thread < that.thread);
    }
    
    // Which thread this belongs to
    NSThread *thread;
    // Outstanding changes
    ChangeSet changes;
};
typedef std::set<ThreadChanges> ThreadChangeSet;

typedef std::map<int,NSObject <MaplyClusterGenerator> *> ClusterGenMap;

@interface MaplyBaseInteractionLayer()
- (void) startLayoutObjects;
- (void) makeLayoutObject:(int)clusterID layoutObjects:(const std::vector<LayoutObjectEntry *> &)layoutObjects retObj:(LayoutObject &)retObj;
- (void) endLayoutObjects;
- (void) clusterID:(SimpleIdentity)clusterID params:(ClusterGenerator::ClusterClassParams &)params;
@end

// Interface between the layout manager and the cluster generators
class OurClusterGenerator : public ClusterGenerator
{
public:
    MaplyBaseInteractionLayer * __weak layer;
    
    // Called right before we start generating layout objects
    void startLayoutObjects()
    {
        [layer startLayoutObjects];
    }

    // Figure out
    void makeLayoutObject(int clusterID,const std::vector<LayoutObjectEntry *> &layoutObjects,LayoutObject &retObj)
    {
        [layer makeLayoutObject:clusterID layoutObjects:layoutObjects retObj:retObj];
    }

    // Called right after all the layout objects are generated
    virtual void endLayoutObjects()
    {
        [layer endLayoutObjects];
    }
    
    void paramsForClusterClass(int clusterID,ClusterClassParams &clusterParams)
    {
        return [layer clusterID:clusterID params:clusterParams];
    }
};

@implementation MaplyBaseInteractionLayer
{
    std::mutex changeLock;
    ThreadChangeSet perThreadChanges;
    std::mutex workLock;
    std::condition_variable workWait;
    int numActiveWorkers;
    ClusterGenMap clusterGens;
    OurClusterGenerator ourClusterGen;
    // Last frame (layout frame, not screen frame)
    std::vector<MaplyTexture *> currentClusterTex,oldClusterTex;
    bool offlineMode;
        
    // Pre-fetched IDs for the various programs
    SimpleIdentity screenSpaceMotionProgram,screenSpaceDefaultProgram;
}

- (instancetype)initWithView:(WhirlyKit::ViewRef)inVisualView
{
    self = [super init];
    if (!self)
        return nil;
    
//    NSLog(@"Creating interactLayer %lx",(long)self);
    
    visualView = inVisualView;
    mainThread = [NSThread currentThread];
    numActiveWorkers = 0;
    
    // Grab everything to force people to wait, hopefully
    imageLock.lock();
    changeLock.lock();
    tempContextLock.lock();
    workLock.lock();
    
    compManager = NULL;
    shaders = [NSMutableArray array];
    
    return self;
}

- (void)dealloc
{
//    NSLog(@"Deallocing interactLayer %lx",(long)self);
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    offlineMode = layerThread == nil;
    scene = inScene;
    sceneRender = layerThread.renderer;
    
    compManager = (ComponentManager_iOS *)scene->getManager(kWKComponentManager);
    
    atlasGroup = [[MaplyTextureAtlasGroup alloc] initWithScene:scene sceneRender:sceneRender];
    
    if (layerThread)
        setupInfo = layerThread.renderer->getRenderSetupInfo();
    
    if (layerThread)
    {
        ourClusterGen.layer = self;
        compManager->layoutManager->addClusterGenerator(&ourClusterGen);
    }
    
    // We locked these in hopes of slowing down anyone trying to race us.  Unlock 'em.
    imageLock.unlock();
    changeLock.unlock();
    tempContextLock.unlock();
    workLock.unlock();
}

- (void)teardown
{
    layerThread = nil;
    scene = NULL;
    imageTextures.clear();
    atlasGroup = nil;
    layerThreads = nil;
    ourClusterGen.layer = nil;
    clusterGens.clear();
    tempContexts.clear();
    compManager->clear();
    
    for (MaplyShader *shader in shaders)
        [shader teardown];

    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    for (ThreadChangeSet::iterator it = perThreadChanges.begin();
         it != perThreadChanges.end();++it)
    {
        ThreadChanges threadChanges = *it;
        for (unsigned int ii=0;ii<threadChanges.changes.size();ii++)
            delete threadChanges.changes[ii];
    }
    perThreadChanges.clear();
}

- (void)lockingShutdown
{
    // This shouldn't happen
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;
    
    if ([NSThread currentThread] != layerThread)
    {
        [self performSelector:@selector(lockingShutdown) onThread:layerThread withObject:nil waitUntilDone:YES];
        return;
    }

//    NSLog(@"Shutting down interactLayer %lx",(long)self);

    std::unique_lock<std::mutex> lk(workLock);
    isShuttingDown = true;
    while (numActiveWorkers > 0) {
        workWait.wait(lk);
        lk.unlock();
    }

    [self teardown];
}

- (bool)startOfWork
{
    if (isShuttingDown)
        return false;
    
    bool ret = true;
    
    std::lock_guard<std::mutex> guardLock(workLock);
    ret = !isShuttingDown;
    if (ret)
        numActiveWorkers++;
    
    return ret;
}

- (void)endOfWork
{
    std::lock_guard<std::mutex> guardLock(workLock);
    numActiveWorkers--;
    workWait.notify_one();
}

// If we're not running in our own thread, we're part of an offline render.
// In that case, render everything now.
- (MaplyThreadMode)resolveThreadMode:(MaplyThreadMode)threadMode
{
    if (!layerThread)
        return MaplyThreadCurrent;
    
    return threadMode;
}

- (Texture *)createTexture:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];
    
    int imageFormat = [desc intForKey:kMaplyTexFormat default:MaplyImageIntRGBA];
    bool wrapX = [desc boolForKey:kMaplyTexWrapX default:false];
    bool wrapY = [desc boolForKey:kMaplyTexWrapY default:false];
    int magFilter = [desc enumForKey:kMaplyTexMagFilter values:@[kMaplyMinFilterNearest,kMaplyMinFilterLinear] default:0];
    bool mipmap = [desc boolForKey:kMaplyTexMipmap default:false];
    
    int imgWidth,imgHeight;
    if (image)
    {
        imgWidth = image.size.width * image.scale;
        imgHeight = image.size.height * image.scale;
        imgWidth = NextPowOf2(imgWidth);
        imgHeight = NextPowOf2(imgHeight);
    } else {
        imgWidth = [desc intForKey:kMaplyTexSizeX default:0];
        imgHeight = [desc intForKey:kMaplyTexSizeY default:0];
    }
    
    
    // Add it and download it
    Texture *tex;
    if (sceneRender->getType() == WhirlyKit::SceneRendererGLES_iOS::RenderGLES) {
        if (image)
            tex = new TextureGLES_iOS("MaplyBaseInteraction",image,imgWidth,imgHeight);
        else {
            tex = new TextureGLES_iOS("MaplyBaseInteraction");
            tex->setWidth(imgWidth);
            tex->setHeight(imgHeight);
            tex->setIsEmptyTexture(true);
        }
    } else {
        // Metal
        if (image)
            tex = new TextureMTL("MaplyBaseInteraction",image,imgWidth,imgHeight);
        else {
            tex = new TextureMTL("MaplyBaseInteraction");
            tex->setWidth(imgWidth);
            tex->setHeight(imgHeight);
            tex->setIsEmptyTexture(true);
        }
    }
    tex->setWrap(wrapX, wrapY);
    tex->setUsesMipmaps(mipmap);
    tex->setInterpType(magFilter == 0 ? TexInterpNearest : TexInterpLinear);
    switch (imageFormat)
    {
        case MaplyImageIntRGBA:
        case MaplyImage4Layer8Bit:
        default:
            tex->setFormat(TexTypeUnsignedByte);
            break;
        case MaplyImageUShort565:
            tex->setFormat(TexTypeShort565);
            break;
        case MaplyImageUShort4444:
            tex->setFormat(TexTypeShort4444);
            break;
        case MaplyImageUShort5551:
            tex->setFormat(TexTypeShort5551);
            break;
        case MaplyImageUByteRed:
            tex->setFormat(TexTypeSingleChannel);
            tex->setSingleByteSource(WKSingleRed);
            break;
        case MaplyImageUByteGreen:
            tex->setFormat(TexTypeSingleChannel);
            tex->setSingleByteSource(WKSingleGreen);
            break;
        case MaplyImageUByteBlue:
            tex->setFormat(TexTypeSingleChannel);
            tex->setSingleByteSource(WKSingleBlue);
            break;
        case MaplyImageUByteAlpha:
            tex->setFormat(TexTypeSingleChannel);
            tex->setSingleByteSource(WKSingleAlpha);
            break;
        case MaplyImageUByteRGB:
            tex->setFormat(TexTypeSingleChannel);
            tex->setSingleByteSource(WKSingleRGB);
            break;
        case MaplyImageSingleFloat16:
            tex->setFormat(TexTypeSingleFloat16);
            break;
        case MaplyImageSingleFloat32:
            tex->setFormat(TexTypeSingleFloat32);
            break;
        case MaplyImageDoubleFloat16:
            tex->setFormat(TexTypeDoubleFloat16);
            break;
        case MaplyImageDoubleFloat32:
            tex->setFormat(TexTypeDoubleFloat32);
            break;
        case MaplyImageQuadFloat16:
            tex->setFormat(TexTypeQuadFloat16);
            break;
        case MaplyImageQuadFloat32:
            tex->setFormat(TexTypeQuadFloat32);
            break;
        case MaplyImageUInt32:
            tex->setFormat(TexTypeSingleUInt32);
            break;
        case MaplyImageDoubleUInt32:
            tex->setFormat(TexTypeDoubleUInt32);
            break;
        case MaplyImageQuadUInt32:
            tex->setFormat(TexTypeQuadUInt32);
            break;
    }

    return tex;
}

// Explicitly add a texture
- (MaplyTexture *)addTexture:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];
    ChangeSet changes;
    MaplyTexture *maplyTex = nil;

    // Look for an image texture that's already representing our UIImage
    if (image != nil)
    {
        std::lock_guard<std::mutex> guardLock(imageLock);

        std::vector<MaplyImageTextureList::iterator> toRemove;
        for (MaplyImageTextureList::iterator theImageTex = imageTextures.begin();
             theImageTex != imageTextures.end(); ++theImageTex)
        {
            if (*theImageTex)
            {
                if ((*theImageTex).image == image && !(*theImageTex).isBeingRemoved)
                {
                    maplyTex = *theImageTex;
                    break;
                }
            } else
                toRemove.push_back(theImageTex);
        }
        for (auto rem : toRemove)
            imageTextures.erase(rem);
    }

    // Takes the altas path instead
    if (!maplyTex && image && [desc boolForKey:kMaplyTexAtlas default:false])
    {
        return [self addTextureToAtlas:image desc:desc mode:threadMode];
    }
    
    if (!maplyTex)
    {
        std::lock_guard<std::mutex> guardLock(imageLock);

        maplyTex = [[MaplyTexture alloc] init];
        
        Texture *tex = [self createTexture:image desc:desc mode:threadMode];
        maplyTex.texID = tex->getId();
        maplyTex.interactLayer = self;
        maplyTex.image = image;
        maplyTex.width = tex->getWidth();
        maplyTex.height = tex->getHeight();
        
        changes.push_back(new AddTextureReq(tex));
        imageTextures.push_back(maplyTex);
    }
    
    if (!changes.empty())
        [self flushChanges:changes mode:threadMode];

    return maplyTex;
}

- (MaplyTexture *)addTextureToAtlas:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    ChangeSet changes;

    // May need a temporary context when setting up textures
    EAGLContext *tmpContext = [self setupTempContext:threadMode];

    // Convert to a texture
    Texture *tex = [self createTexture:image desc:desc mode:threadMode];
    if (!tex)
        return nil;
    
    // Add to a texture atlas
    MaplyTexture *maplyTex = nil;
    SubTexture subTex;
    if ([atlasGroup addTexture:tex subTex:subTex changes:changes])
    {
        maplyTex = [[MaplyTexture alloc] init];
        maplyTex.image = image;
        maplyTex.texID = subTex.getId();
        maplyTex.isSubTex = true;
        maplyTex.interactLayer = self;
        {
            std::lock_guard<std::mutex> guardLock(imageLock);
            imageTextures.push_back(maplyTex);
        }
    }
    delete tex;

    // If we're making changes in this thread, do the flushes
    if (threadMode == MaplyThreadCurrent)
    {
        bool requiresFlush = false;
        
        // Set up anything that needs to be set up
        ChangeSet changesToAdd;
        for (unsigned int ii=0;ii<changes.size();ii++)
        {
            ChangeRequest *change = changes[ii];
            if (change)
            {
                requiresFlush |= change->needsFlush();
                change->setupForRenderer(sceneRender->getRenderSetupInfo());
                changesToAdd.push_back(change);
            } else
                // A NULL change request is just a flush request
                requiresFlush = true;
        }
        
        // If anything needed a flush after that, let's do it
        if (requiresFlush)
        {
            glFlush();
            
            // If there were no changes to add we probably still want to poke the scene
            // Otherwise texture changes don't show up
            if (changesToAdd.empty())
                changesToAdd.push_back(NULL);
        }
        
        scene->addChangeRequests(changesToAdd);
    } else
        scene->addChangeRequests(changes);
    
    [self clearTempContext:tmpContext];
    
    return maplyTex;
}

- (MaplyTexture *)addSubTexture:(MaplyTexture *)tex xOffset:(int)x yOffset:(int)y width:(int)width height:(int)height mode:(MaplyThreadMode)threadMode
{
    MaplyTexture *newTex = [[MaplyTexture alloc] init];
    if (tex.width == 0 || tex.height == 0 || tex.isSubTex)
        return nil;
    
    // Set up the subtexture reference
    SubTexture subTex;
    subTex.setFromTex(TexCoord(x / (double)tex.width, y / (double)tex.height),
                      TexCoord((x+width) / (double)tex.width, (y+height) / (double)tex.height));
    scene->addSubTexture(subTex);
    newTex.isSubTex = true;
    newTex.texID = subTex.getId();
    newTex.width = width;
    newTex.height = height;

    return newTex;
}

- (void)removeTextures:(NSArray *)textures mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    for (MaplyTexture *texture in textures)
        [texture clear];
}

// Called by the texture dealloc
- (void)clearTexture:(MaplyTexture *)tex when:(TimeInterval)when
{
    if (!layerThread || isShuttingDown)
        return;
    
    ChangeSet changes;

    if (tex.isSubTex)
    {
        if (atlasGroup)
        {
            [atlasGroup removeTexture:tex.texID changes:changes when:when];
            scene->removeSubTexture(tex.texID);
        }
    } else {
        if (scene)
            changes.push_back(new RemTextureReq(tex.texID));
    }
    tex.texID = EmptyIdentity;

    [self flushChanges:changes mode:MaplyThreadAny];
}

- (MaplyTexture *)addImage:(id)image imageFormat:(MaplyQuadImageFormat)imageFormat mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    return [self addImage:image imageFormat:imageFormat wrapFlags:MaplyImageWrapNone interpType:GL_NEAREST mode:threadMode];
}

// Add an image to the cache, or find an existing one
// Called in the layer thread
- (MaplyTexture *)addImage:(id)image imageFormat:(MaplyQuadImageFormat)imageFormat wrapFlags:(int)wrapFlags interpType:(GLenum)interpType mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyTexture *maplyTex = [self addTexture:image desc:@{kMaplyTexFormat: @(imageFormat),
                               kMaplyTexWrapX: @(wrapFlags & MaplyImageWrapX),
                               kMaplyTexWrapY: @(wrapFlags & MaplyImageWrapY),
                            kMaplyTexMagFilter: (interpType == GL_NEAREST ? kMaplyMinFilterNearest : kMaplyMinFilterLinear)}
                                         mode:threadMode];
    
    return maplyTex;
}

// Remove an image for the cache, or just decrement its reference count
- (void)removeImageTexture:(MaplyTexture *)tex changes:(ChangeSet &)changes
{
    std::lock_guard<std::mutex> guardLock(imageLock);

    // Clear up any textures that may have vanished
    std::vector<MaplyImageTextureList::iterator> toRemove;
    for (MaplyImageTextureList::iterator it = imageTextures.begin();
         it != imageTextures.end(); ++it)
        if (!(*it).image)
            toRemove.push_back(it);
    for (auto rem : toRemove)
        imageTextures.erase(rem);
    
    // Atlas textures take care of themselves via the MaplyTexture dealloc
    
    // If it's associated with the view controller, it exists outside us, so we just let it clean itself up
    //  when it gets dealloc'ed.
    // Note: This time is a hack.  Should look at the fade out.
    if (tex.interactLayer) {
        tex.isBeingRemoved = true;
        [self performSelector:@selector(delayedRemoveTexture:) withObject:tex afterDelay:2.0];
    } else {
        // If we created it in this object, we'll clean it up
        if (tex.texID != EmptyIdentity)
        {
            changes.push_back(new RemTextureReq(tex.texID));
            tex.texID = EmptyIdentity;
        }
    }
}

// Remove the given Texture ID after a delay
// Note: This is a hack to work around fade problems
- (void)delayedRemoveTexture:(MaplyTexture *)maplyTex
{
    // Holding the object until there delays the deletion
}

- (void)delayedRemoveTextures:(NSArray *)texs
{
    // Holding the objects until there delays the deletion
}

// We flush out changes in different ways depending on the thread mode
- (void)flushChanges:(ChangeSet &)changes mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    if (changes.empty())
        return;
    // This means we beat the layer thread setup, so we'll put this in orbit
    if (!scene)
        threadMode = MaplyThreadAny;

    switch (threadMode)
    {
        case MaplyThreadCurrent:
        {
            std::lock_guard<std::mutex> guardLock(changeLock);

            // We might be journaling changes, so let's check
            NSThread *currentThread = [NSThread currentThread];
            ThreadChanges threadChanges(currentThread);
            ThreadChangeSet::iterator it = perThreadChanges.find(threadChanges);
            if (it != perThreadChanges.end())
            {
                // We are, so just toss these changes on to the end
                ThreadChanges theChanges = *it;
                theChanges.changes.insert(theChanges.changes.end(), changes.begin(), changes.end());
                perThreadChanges.erase(it);
                perThreadChanges.insert(theChanges);
            } else {
                // If we're on a layer thread, we want to flush on that thread
                auto thisLayerThread = [NSThread currentThread];
                bool flushHere = thisLayerThread == mainThread;
                if (!flushHere) {
                    flushHere = true;
                    for (WhirlyKitLayerThread *layerThread in layerThreads) {
                        if (layerThread == thisLayerThread) {
                            flushHere = false;
                            [layerThread addChangeRequests:changes];
                            break;
                        }
                    }
                }
                
                // We're not, so execute the changes
                if (flushHere)
                    scene->addChangeRequests(changes);
            }
        }
            break;
        case MaplyThreadAny:
            [layerThread addChangeRequests:changes];
            break;
    }
}

- (void)startChanges
{
    std::lock_guard<std::mutex> guardLock(changeLock);

    // Look for changes in the current thread
    NSThread *currentThread = [NSThread currentThread];
    ThreadChanges changes(currentThread);
    ThreadChangeSet::iterator it = perThreadChanges.find(changes);
    // If there isn't one, we add it.  That's how we know we're doing this.
    if (it == perThreadChanges.end())
        perThreadChanges.insert(changes);
}

- (void)endChanges
{
    std::lock_guard<std::mutex> guardLock(changeLock);

    // Look for outstanding changes
    NSThread *currentThread = [NSThread currentThread];
    ThreadChanges changes(currentThread);
    ThreadChangeSet::iterator it = perThreadChanges.find(changes);
    if (it != perThreadChanges.end())
    {
        EAGLContext *tmpContext = [self setupTempContext:MaplyThreadCurrent];

        ThreadChanges theseChanges = *it;
        // Process the setupGL on this thread rather than making the main thread do it
        if (currentThread != mainThread)
            for (auto &change : theseChanges.changes) {
                if (change)
                    change->setupForRenderer(sceneRender->getRenderSetupInfo());
            }
        scene->addChangeRequests(theseChanges.changes);
        perThreadChanges.erase(it);
        
        [self clearTempContext:tmpContext];
    }
}

// Solved render target and shader names
// Have to do the shaders here because users are allowed to change the shaders as they run
- (void)resolveInfoDefaults:(NSDictionary *)inDesc info:(BaseInfo *)info defaultShader:(NSString *)defaultShaderName
{
    NSObject *shader = inDesc[kMaplyShader];
    if (shader)
    {
        // Translate the shader into an ID
        if ([shader isKindOfClass:[NSString class]])
        {
            NSString *shaderName = (NSString *)shader;
            SimpleIdentity shaderID = [self getProgramID:shaderName];
            info->programID = shaderID;
        }
    }
    
    if (info->programID == EmptyIdentity)
        info->programID = [self getProgramID:defaultShaderName];
     

    // Look for a render target
    MaplyRenderTarget *renderTarget = inDesc[@"rendertarget"];
    if ([renderTarget isKindOfClass:[MaplyRenderTarget class]])
    {
        info->renderTargetID = renderTarget.renderTargetID;
    }
}

// Copy vertex attributes from the source to the dest
- (void)resolveVertexAttrs:(SingleVertexAttributeSet &)destAttrs from:(NSArray *)srcAttrs
{
    for (MaplyVertexAttribute *attr in srcAttrs)
        destAttrs.insert(attr->attr);
}

- (void)resolveDrawPriority:(NSDictionary *)desc info:(BaseInfo *)info drawPriority:(int)drawPriority offset:(int)offsetPriority
{
    NSNumber *setting = desc[@"drawPriority"];
    int iVal = 0;
    if ([setting isKindOfClass:[NSNumber class]])
    {
        iVal = [setting intValue];
        info->drawPriority = iVal + offsetPriority;
    } else {
        info->drawPriority = drawPriority + offsetPriority;
    }
}

// Actually add the markers.
// Called in an unknown thread
- (void)addScreenMarkersRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *markers = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    TimeInterval now = scene->getCurrentTime();
    
    bool isMotionMarkers = false;
    if ([[markers objectAtIndex:0] isKindOfClass:[MaplyMovingScreenMarker class]])
        isMotionMarkers = true;
    
    iosDictionary dictWrap(inDesc);
    MarkerInfo markerInfo(dictWrap,true);

    // Might be a custom shader on these
    if (isMotionMarkers)
        [self resolveInfoDefaults:inDesc info:&markerInfo defaultShader:kMaplyScreenSpaceDefaultMotionProgram];
    else
        [self resolveInfoDefaults:inDesc info:&markerInfo defaultShader:kMaplyScreenSpaceDefaultProgram];
    [self resolveDrawPriority:inDesc info:&markerInfo drawPriority:kMaplyLabelDrawPriorityDefault offset:_screenObjectDrawPriorityOffset];
    
    // Convert to WG markers
    std::vector<WhirlyKit::Marker *> wgMarkers;
    for (MaplyScreenMarker *marker in markers)
    {
        WhirlyKit::Marker *wgMarker = new WhirlyKit::Marker();
        wgMarker->loc = GeoCoord(marker.loc.x,marker.loc.y);
        if (marker.uniqueID)
            wgMarker->uniqueID = [marker.uniqueID asStdString];
        std::vector<MaplyTexture *> texs;
        if (marker.image)
        {
            if ([marker.image isKindOfClass:[UIImage class]])
            {
                UIImage *image = marker.image;
                GLenum interpType = GL_LINEAR;
                if (image.size.width * image.scale == marker.size.width && image.size.height * image.scale == marker.size.height)
                    interpType = GL_NEAREST;
                texs.push_back([self addImage:marker.image imageFormat:MaplyImageIntRGBA wrapFlags:0 interpType:GL_LINEAR mode:threadMode]);
            } else if ([marker.image isKindOfClass:[MaplyTexture class]])
            {
                texs.push_back((MaplyTexture *)marker.image);
            }
        } else if (marker.images)
        {
            for (id image in marker.images)
            {
                if ([image isKindOfClass:[UIImage class]])
                    texs.push_back([self addImage:image imageFormat:MaplyImageIntRGBA wrapFlags:0 interpType:GL_LINEAR mode:threadMode]);
                else if ([image isKindOfClass:[MaplyTexture class]])
                    texs.push_back((MaplyTexture *)image);
            }
        }
        if (texs.size() > 1)
            wgMarker->period = marker.period;
        compObj->contents->texs.insert(texs.begin(),texs.end());
        if (marker.color) {
            wgMarker->color = [marker.color asRGBAColor];
            wgMarker->colorSet = true;
        }
        if (!texs.empty())
        {
            for (unsigned int ii=0;ii<texs.size();ii++)
                wgMarker->texIDs.push_back(texs[ii].texID);
        }
        wgMarker->width = marker.size.width;
        wgMarker->height = marker.size.height;
        if (marker.rotation != 0.0)
        {
            wgMarker->rotation = marker.rotation;
            wgMarker->lockRotation = true;
        }
        if (marker.selectable)
        {
            wgMarker->isSelectable = true;
            wgMarker->selectID = Identifiable::genId();
        }
        wgMarker->layoutImportance = marker.layoutImportance;

        if (marker.vertexAttributes)
            [self resolveVertexAttrs:wgMarker->vertexAttrs from:marker.vertexAttributes];
        
        if (wgMarker->width <= 0.0) {
            wgMarker->width = markerInfo.width;
            wgMarker->height = markerInfo.height;
        }
        
        if (marker.layoutSize.width >= 0.0)
        {
            wgMarker->layoutWidth = marker.layoutSize.width;
            wgMarker->layoutHeight = marker.layoutSize.height;
        } else {
            wgMarker->layoutWidth = wgMarker->width;
            wgMarker->layoutHeight = wgMarker->height;
        }
        wgMarker->offset = Point2d(marker.offset.x,marker.offset.y);
        
        // Now for the motion related fields
        if ([marker isKindOfClass:[MaplyMovingScreenMarker class]])
        {
            MaplyMovingScreenMarker *movingMarker = (MaplyMovingScreenMarker *)marker;
            wgMarker->hasMotion = true;
            wgMarker->endLoc = GeoCoord(movingMarker.endLoc.x,movingMarker.endLoc.y);
            wgMarker->startTime = now;
            wgMarker->endTime = now + movingMarker.duration;
        }
        
        wgMarkers.push_back(wgMarker);
        
        if (marker.selectable)
        {
            compManager->addSelectObject(wgMarker->selectID,marker);
            compObj->contents->selectIDs.insert(wgMarker->selectID);
        }
    }
    
    
    // Set up a description and create the markers in the marker layer
    ChangeSet changes;
    SimpleIdentity markerID = compManager->markerManager->addMarkers(wgMarkers, markerInfo, changes);
    if (markerID != EmptyIdentity)
        compObj->contents->markerIDs.insert(markerID);
    [self flushChanges:changes mode:threadMode];
    
    compManager->addComponentObject(compObj->contents);
}

// Called in the main thread.
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([markers count] == 0)
    {
        compManager->addComponentObject(compObj->contents);

        return compObj;
    }
    
    NSArray *argArray = @[markers, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addScreenMarkersRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addScreenMarkersRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

- (void)addClusterGenerator:(NSObject <MaplyClusterGenerator> *)clusterGen
{
    @synchronized(self)
    {
        clusterGens[clusterGen.clusterNumber] = clusterGen;
    }
}

- (void) startLayoutObjects
{
    // Started cluster generation, so keep track of the old textures
    oldClusterTex = currentClusterTex;
    currentClusterTex.clear();
    
    @synchronized(self) {
        for (ClusterGenMap::iterator it = clusterGens.begin();
             it != clusterGens.end(); ++it)
            [it->second startClusterGroup];
    }
}

- (void) makeLayoutObject:(int)clusterID layoutObjects:(const std::vector<LayoutObjectEntry *> &)layoutObjects retObj:(LayoutObject &)retObj
{
    // Find the right cluster generator
    NSObject <MaplyClusterGenerator> *clusterGen = nil;
    @synchronized(self)
    {
        clusterGen = clusterGens[clusterID];
    }
    
    if (!clusterGen)
        return;
    
    
    if ([clusterGen showMarkerWithHighestImportance])
    {
        [self setupLayoutObject:retObj asBestOfLayoutObjects:layoutObjects];
    } else {
        [self setupLayoutObject:retObj asAverageOfLayoutObjects:layoutObjects withClusterGenerator:clusterGen];
    }
}

- (void)setupLayoutObject:(LayoutObject &)retObj asBestOfLayoutObjects:(const std::vector<LayoutObjectEntry *> &)layoutObjects
{
    LayoutObjectEntry *topObject = nullptr;
    LayoutEntrySorter sorter;
    
    for (auto obj : layoutObjects)
        if (topObject == nullptr || sorter(obj, topObject))
            topObject = obj;
    
    if (topObject == nullptr || topObject->obj.getGeometry().empty())
        return;
    
    retObj.setWorldLoc(topObject->obj.getWorldLoc());
    retObj.setDrawPriority(topObject->obj.getDrawPriority());
    if (topObject->obj.hasRotation())
        retObj.setRotation(topObject->obj.getRotation());
    
    std::vector<ScreenSpaceObject::ConvexGeometry> allGeometry = topObject->obj.getGeometry();
    
    if (allGeometry.empty())
        return;
    
    retObj.layoutPts = allGeometry.back().coords;
    retObj.selectPts = allGeometry.back().coords;
    
    for (auto geometry : allGeometry)
        retObj.addGeometry(geometry);
}

- (void)setupLayoutObject:(LayoutObject &)retObj asAverageOfLayoutObjects:(const std::vector<LayoutObjectEntry *> &)layoutObjects withClusterGenerator:(NSObject<MaplyClusterGenerator> *)clusterGen
{
    // Pick a representive screen object
    int drawPriority = -1;
    LayoutObject *sampleObj = NULL;
    NSMutableArray *uniqueIDs = [NSMutableArray array];
    for (auto obj : layoutObjects)
    {
        if (obj->obj.getDrawPriority() > drawPriority)
        {
            drawPriority = obj->obj.getDrawPriority();
            sampleObj = &obj->obj;
        }
        if (!obj->obj.uniqueID.empty()) {
            NSString *newStr = [NSString stringWithFormat:@"%s",obj->obj.uniqueID.c_str()];
            if ([newStr length])
                [uniqueIDs addObject:newStr];
        }
    }
    SimpleIdentity progID = sampleObj->getTypicalProgramID();
    
    // Ask for a cluster image
    MaplyClusterInfo *clusterInfo = [[MaplyClusterInfo alloc] init];
    clusterInfo.numObjects = (int)layoutObjects.size();
    clusterInfo.uniqueIDs = uniqueIDs;
    MaplyClusterGroup *group = [clusterGen makeClusterGroup:clusterInfo];

    // Geometry for the new cluster object
    ScreenSpaceObject::ConvexGeometry smGeom;
    smGeom.progID = progID;
    smGeom.coords.push_back(Point2d(-group.size.width/2.0,-group.size.height/2.0));
    smGeom.texCoords.push_back(TexCoord(0,1));
    smGeom.coords.push_back(Point2d(group.size.width/2.0,-group.size.height/2.0));
    smGeom.texCoords.push_back(TexCoord(1,1));
    smGeom.coords.push_back(Point2d(group.size.width/2.0,group.size.height/2.0));
    smGeom.texCoords.push_back(TexCoord(1,0));
    smGeom.coords.push_back(Point2d(-group.size.width/2.0,group.size.height/2.0));
    smGeom.texCoords.push_back(TexCoord(0,0));
    smGeom.color = RGBAColor(255,255,255,255);
    
    retObj.layoutPts = smGeom.coords;
    retObj.selectPts = smGeom.coords;
    retObj.importance = sampleObj->importance;
    
    // Create the texture
    // Note: Keep this around
    MaplyTexture *maplyTex = [self addTexture:group.image desc:@{kMaplyTexFormat: @(MaplyImageIntRGBA),
                                                                 kMaplyTexAtlas: @(true),
                                                                 kMaplyTexMagFilter: kMaplyMinFilterNearest}
                                         mode:MaplyThreadCurrent];
    currentClusterTex.push_back(maplyTex);
    if (maplyTex.isSubTex)
    {
        SubTexture subTex = scene->getSubTexture(maplyTex.texID);
        subTex.processTexCoords(smGeom.texCoords);
        smGeom.texIDs.push_back(subTex.texId);
    } else
        smGeom.texIDs.push_back(maplyTex.texID);

    retObj.setDrawPriority(drawPriority);
    retObj.addGeometry(smGeom);
}

- (void)endLayoutObjects
{
    // Layout of new objects is over, so schedule the old textures for removal
    if (!oldClusterTex.empty())
    {
        NSMutableArray *texArr = [NSMutableArray array];
        for (auto tex : oldClusterTex)
            [texArr addObject:tex];
        [self performSelector:@selector(delayedRemoveTextures:) withObject:texArr afterDelay:2.0];
        oldClusterTex.clear();
    }
    
    @synchronized(self) {
        for (ClusterGenMap::iterator it = clusterGens.begin();
             it != clusterGens.end(); ++it)
            [it->second endClusterGroup];
    }
}

- (SimpleIdentity) getProgramID:(NSString *)name
{
    @synchronized (shaders) {
        for (int ii=[shaders count]-1;ii>=0;ii--) {
            MaplyShader *shader = [shaders objectAtIndex:ii];
            if ([shader.name isEqualToString:name])
                return [shader getShaderID];
        }
    }
    
    return EmptyIdentity;
}

- (void) clusterID:(SimpleIdentity)clusterID params:(ClusterGenerator::ClusterClassParams &)params
{
    NSObject <MaplyClusterGenerator> *clusterGen = nil;
    @synchronized(self)
    {
        clusterGen = clusterGens[(int)clusterID];
    }

    // Ask for the shader for moving objects
    params.motionShaderID = EmptyIdentity;
    MaplyShader *shader = [clusterGen motionShader];
    if (shader)
        params.motionShaderID = shader.program->getId();
    else {
        SimpleIdentity shaderID = [self getProgramID:kMaplyScreenSpaceDefaultMotionProgram];
        params.motionShaderID = shaderID;
    }
    
    CGSize size = clusterGen.clusterLayoutSize;
    params.clusterSize = Point2d(size.width,size.height);
    
    params.selectable = clusterGen.selectable;
    
    params.markerAnimationTime = clusterGen.markerAnimationTime;
}

// Actually add the markers.
// Called in an unknown thread.
- (void)addMarkersRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *markers = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    // Note: This assumes everything has images
    bool hasMultiTex = false;
    for (MaplyMarker *marker in markers)
        if (marker.images)
            hasMultiTex = true;

    iosDictionary dictWrap(inDesc);
    MarkerInfo markerInfo(dictWrap,false);
    [self resolveInfoDefaults:inDesc info:&markerInfo defaultShader:(hasMultiTex ? kMaplyShaderDefaultMarker : kMaplyShaderDefaultTri)];
    [self resolveDrawPriority:inDesc info:&markerInfo drawPriority:kMaplyMarkerDrawPriorityDefault offset:0];
    
    // Convert to WG markers
    std::vector<Marker *> wgMarkers;
    for (MaplyMarker *marker in markers)
    {
        Marker *wgMarker = new Marker();
        wgMarker->loc = GeoCoord(marker.loc.x,marker.loc.y);

        std::vector<MaplyTexture *> texs;
        if (marker.image)
        {
            if ([marker.image isKindOfClass:[UIImage class]])
            {
                texs.push_back([self addImage:marker.image imageFormat:MaplyImageIntRGBA mode:threadMode]);
            } else if ([marker.image isKindOfClass:[MaplyTexture class]])
            {
                texs.push_back((MaplyTexture *)marker.image);
            }
        } else if (marker.images)
        {
            for (id image in marker.images)
            {
                if ([image isKindOfClass:[UIImage class]])
                    texs.push_back([self addImage:image imageFormat:MaplyImageIntRGBA wrapFlags:0 interpType:GL_LINEAR mode:threadMode]);
                else if ([image isKindOfClass:[MaplyTexture class]])
                    texs.push_back((MaplyTexture *)image);
            }
        }
        if (texs.size() > 1)
            wgMarker->period = marker.period;
        compObj->contents->texs.insert(texs.begin(),texs.end());
        if (!texs.empty())
        {
            for (unsigned int ii=0;ii<texs.size();ii++)
                wgMarker->texIDs.push_back(texs[ii].texID);
        }

        wgMarker->width = marker.size.width;
        wgMarker->height = marker.size.height;
        if (marker.selectable)
        {
            wgMarker->isSelectable = true;
            wgMarker->selectID = Identifiable::genId();
        }
        
        wgMarkers.push_back(wgMarker);
        
        if (marker.selectable)
        {
            compManager->addSelectObject(wgMarker->selectID,marker);
            compObj->contents->selectIDs.insert(wgMarker->selectID);
        }
    }
    
    // Set up a description and create the markers in the marker layer
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    if (markerManager)
    {
        ChangeSet changes;
        SimpleIdentity markerID = markerManager->addMarkers(wgMarkers, markerInfo, changes);
        if (markerID != EmptyIdentity)
            compObj->contents->markerIDs.insert(markerID);
        [self flushChanges:changes mode:threadMode];
    }
    
    compManager->addComponentObject(compObj->contents);
}

// Add 3D markers
- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;

    if ([markers count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }

    NSArray *argArray = @[markers, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addMarkersRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addMarkersRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Make a temporary EAGL context if we need it.
// This happens if we're making OpenGL calls on a thread that doesn't have a context.
- (EAGLContext *)setupTempContext:(MaplyThreadMode)threadMode
{
    SceneRendererGLES_iOS *sceneRenderGL = dynamic_cast<SceneRendererGLES_iOS *>(sceneRender);
    if (!sceneRenderGL)
        return nil;
    
    threadMode = [self resolveThreadMode:threadMode];

    EAGLContext *tmpContext = nil;
    
    // Use the renderer's context
    if (threadMode == MaplyThreadCurrent && mainThread == [NSThread currentThread])
    {
        sceneRenderGL->useContext();
    }
    
    if (threadMode == MaplyThreadCurrent && ![EAGLContext currentContext])
    {
        std::lock_guard<std::mutex> guardLock(tempContextLock);

        // See if we need to create a new one
        if (tempContexts.empty())
        {
            tmpContext = [[EAGLContext alloc] initWithAPI:sceneRenderGL->getContext().API sharegroup:sceneRenderGL->getContext().sharegroup];
        } else {
            // We can use an existing one
            std::set<EAGLContext *>::iterator it = tempContexts.begin();
            tmpContext = *it;
            tempContexts.erase(it);
        }
        [EAGLContext setCurrentContext:tmpContext];
    }
    
    return tmpContext;
}

// This just releases the context, but we may want to keep a queue of these in future
- (void)clearTempContext:(EAGLContext *)context
{
    SceneRendererGLES_iOS *sceneRenderGL = dynamic_cast<SceneRendererGLES_iOS *>(sceneRender);
    if (!sceneRenderGL)
        return;

    if (mainThread == [NSThread currentThread] && context == sceneRenderGL->getContext())
    {
        [EAGLContext setCurrentContext:nil];
        return;
    }
    
    if (context)
    {
        glFlush();
        [EAGLContext setCurrentContext:nil];

        // Put this one back for use by another thread
        {
            std::lock_guard<std::mutex> guardLock(tempContextLock);
            tempContexts.insert(context);
        }
    }
}

// Actually add the labels.
// Called in an unknown thread.
- (void)addScreenLabelsRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *labels = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    // May need a temporary context when setting up screen label textures
    EAGLContext *tmpContext = [self setupTempContext:threadMode];

    TimeInterval now = scene->getCurrentTime();
    
    bool isMotionLabels = false;
    if ([[labels objectAtIndex:0] isKindOfClass:[MaplyMovingScreenLabel class]])
        isMotionLabels = true;

    iosDictionary dictWrap(inDesc);
    LabelInfo_iOS labelInfo(inDesc,dictWrap,true);
    [self resolveInfoDefaults:inDesc info:&labelInfo
                defaultShader:(isMotionLabels ? kMaplyScreenSpaceDefaultMotionProgram : kMaplyScreenSpaceDefaultProgram)];
    [self resolveDrawPriority:inDesc info:&labelInfo drawPriority:kMaplyLabelDrawPriorityDefault offset:_screenObjectDrawPriorityOffset];
    if (!labelInfo.font)
        labelInfo.font = [UIFont systemFontOfSize:32.0];

    // Convert to WG screen labels
    std::vector<SingleLabel *> wgLabels;
    for (MaplyScreenLabel *label in labels)
    {
        SingleLabel_iOS *wgLabel = new SingleLabel_iOS();
        wgLabel->loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel->rotation = label.rotation;
        wgLabel->text = label.text;
        if (label.uniqueID)
            wgLabel->uniqueID = [label.uniqueID asStdString];
        wgLabel->keepUpright = label.keepUpright;
        MaplyTexture *tex = nil;
        if (label.iconImage2) {
            tex = [self addImage:label.iconImage2 imageFormat:MaplyImageIntRGBA mode:threadMode];
            compObj->contents->texs.insert(tex);
        }
        if (tex)
            wgLabel->iconTexture = tex.texID;
        wgLabel->iconSize = Point2f(label.iconSize.width,label.iconSize.height);
        if (label.layoutSize.width >= 0.0)
        {
            wgLabel->layoutSize.x() = label.layoutSize.width;
            wgLabel->layoutSize.y() = label.layoutSize.height;
        }

        LabelInfoRef thisLabelInfo;
        if ([inDesc objectForKey:kMaplyTextColor] || label.layoutImportance < MAXFLOAT ||
            [inDesc objectForKey:kMaplyTextLineSpacing]) {
            thisLabelInfo = LabelInfoRef(new LabelInfo(true));

            if (label.color) {
                thisLabelInfo->hasTextColor = true;
                thisLabelInfo->textColor = [label.color asRGBAColor];
            }
            if (label.layoutImportance < MAXFLOAT)
            {
                thisLabelInfo->layoutEngine = true;
                thisLabelInfo->layoutImportance = label.layoutImportance;
                thisLabelInfo->layoutPlacement = label.layoutPlacement;
            }
        }
        wgLabel->infoOverride = thisLabelInfo;
        
        wgLabel->screenOffset = Point2d(label.offset.x,label.offset.y);
        if (label.selectable)
        {
            wgLabel->isSelectable = true;
            wgLabel->selectID = Identifiable::genId();
        }

        // Now for the motion related fields
        if ([label isKindOfClass:[MaplyMovingScreenLabel class]])
        {
            MaplyMovingScreenLabel *movingLabel = (MaplyMovingScreenLabel *)label;
            wgLabel->hasMotion = true;
            wgLabel->endLoc = GeoCoord(movingLabel.endLoc.x,movingLabel.endLoc.y);
            wgLabel->startTime = now;
            wgLabel->endTime = now + movingLabel.duration;
        }

        wgLabels.push_back(wgLabel);
        
        if (label.selectable)
        {
            compManager->addSelectObject(wgLabel->selectID,label);
            compObj->contents->selectIDs.insert(wgLabel->selectID);
        }
    }
    
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    if (labelManager)
    {
        // Set up a description and create the markers in the marker layer
        ChangeSet changes;
        SimpleIdentity labelID = labelManager->addLabels(wgLabels, labelInfo, changes);
        [self flushChanges:changes mode:threadMode];
        if (labelID != EmptyIdentity)
            compObj->contents->labelIDs.insert(labelID);
    }

    compManager->addComponentObject(compObj->contents);

    [self clearTempContext:tmpContext];
}

// Add screen space (2D) labels
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([labels count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[labels, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];

    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addScreenLabelsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addScreenLabelsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the labels.
// Called in an unknown thread.
- (void)addLabelsRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *labels = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    iosDictionary dictWrap(inDesc);
    LabelInfo_iOS labelInfo(inDesc,dictWrap,false);
    [self resolveInfoDefaults:inDesc info:&labelInfo defaultShader:kMaplyShaderDefaultTri];
    [self resolveDrawPriority:inDesc info:&labelInfo drawPriority:kMaplyLabelDrawPriorityDefault offset:0];

    // May need a temporary context when setting up label textures
    EAGLContext *tmpContext = [self setupTempContext:threadMode];

    // Convert to WG labels
    std::vector<SingleLabel *> wgLabels;
    for (MaplyLabel *label in labels)
    {
        SingleLabel_iOS *wgLabel = new SingleLabel_iOS();
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel->loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel->text = label.text;
        MaplyTexture *tex = nil;
        if (label.iconImage2) {
            tex = [self addImage:label.iconImage2 imageFormat:MaplyImageIntRGBA mode:threadMode];
            compObj->contents->texs.insert(tex);
        }
        wgLabel->iconTexture = tex.texID;
        if (label.size.width > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.width] forKey:@"width"];
        if (label.size.height > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.height] forKey:@"height"];
        if (label.color)
            [desc setObject:label.color forKey:@"textColor"];
        if (label.selectable)
        {
            wgLabel->isSelectable = true;
            wgLabel->selectID = Identifiable::genId();
        }
        switch (label.justify)
        {
            case MaplyLabelJustifyLeft:
                [desc setObject:@"left" forKey:@"justify"];
                break;
            case MaplyLabelJustifyMiddle:
                [desc setObject:@"middle" forKey:@"justify"];
                break;
            case MaplyLabelJustifyRight:
                [desc setObject:@"right" forKey:@"justify"];
                break;
        }
        
        wgLabels.push_back(wgLabel);
        
        if (label.selectable)
        {
            compManager->addSelectObject(wgLabel->selectID,label);
            compObj->contents->selectIDs.insert(wgLabel->selectID);
        }
    }
    
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    
    if (labelManager)
    {
        ChangeSet changes;
        // Set up a description and create the markers in the marker layer
        SimpleIdentity labelID = labelManager->addLabels(wgLabels, labelInfo, changes);
        [self flushChanges:changes mode:threadMode];
        if (labelID != EmptyIdentity)
            compObj->contents->labelIDs.insert(labelID);
    }
    
    compManager->addComponentObject(compObj->contents);

    [self clearTempContext:tmpContext];
}

// Add 3D labels
- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([labels count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[labels, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];

    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addLabelsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addLabelsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the vectors.
// Called in an unknown thread
- (void)addVectorsRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    bool makeVisible = [[argArray objectAtIndex:3] boolValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:4] intValue];
    
    iosDictionary dictWrap(inDesc);
    VectorInfo vectorInfo(dictWrap);

    // Might be a custom shader on these
    NSString *shaderName = !vectorInfo.filled ? kMaplyShaderDefaultLine : kMaplyDefaultTriangleShader;
    if (vectorInfo.texProj == TextureProjectionScreen)
        shaderName = kMaplyShaderDefaultTriScreenTex;
    [self resolveInfoDefaults:inDesc info:&vectorInfo defaultShader:shaderName];
    [self resolveDrawPriority:inDesc info:&vectorInfo drawPriority:kMaplyVectorDrawPriorityDefault offset:0];
    
    // Look for a texture and add it
    if (inDesc[kMaplyVecTexture])
    {
        UIImage *theImage = inDesc[kMaplyVecTexture];
        MaplyTexture *tex = nil;
        if ([theImage isKindOfClass:[UIImage class]])
            tex = [self addImage:theImage imageFormat:MaplyImage4Layer8Bit mode:threadMode];
        else if ([theImage isKindOfClass:[MaplyTexture class]])
            tex = (MaplyTexture *)theImage;
        if (tex.texID)
            vectorInfo.texId = tex.texID;
    }

    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
    {
        // Maybe need to make a copy if we're going to sample
        if (vectorInfo.subdivEps != 0.0)
        {
            float eps = vectorInfo.subdivEps;
            NSString *subdivType = inDesc[kMaplySubdivType];
            bool greatCircle = ![subdivType compare:kMaplySubdivGreatCircle];
            bool grid = ![subdivType compare:kMaplySubdivGrid];
            bool staticSubdiv = ![subdivType compare:kMaplySubdivStatic];
            MaplyVectorObject *newVecObj = [vecObj deepCopy2];
            // Note: This logic needs to be moved down a level
            //       Along with the subdivision routines above
            if (greatCircle)
                [newVecObj subdivideToGlobeGreatCircle:eps];
            else if (grid)
            {
                // The manager has to handle this one
            }
            else if (staticSubdiv)
            {
                // Note: Fill this in
            } else
                [newVecObj subdivideToGlobe:eps];

            shapes.insert(newVecObj->vObj->shapes.begin(),newVecObj->vObj->shapes.end());
        } else
            // We'll just reference it
            shapes.insert(vecObj->vObj->shapes.begin(),vecObj->vObj->shapes.end());
    }
    
    if (makeVisible)
    {
        VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
        
        if (vectorManager)
        {
            ChangeSet changes;
            SimpleIdentity vecID = vectorManager->addVectors(&shapes, vectorInfo, changes);
            [self flushChanges:changes mode:threadMode];
            if (vecID != EmptyIdentity)
                compObj->contents->vectorIDs.insert(vecID);
        }
    }
    
    // If the vectors are selectable we want to keep them around
    id selVal = inDesc[@"selectable"];
    if (selVal && [selVal boolValue])
    {
        if ([inDesc[kMaplyVecCentered] boolValue])
        {
            if (inDesc[kMaplyVecCenterX])
                compObj->contents->vectorOffset.x() = [inDesc[kMaplyVecCenterX] doubleValue];
            if (inDesc[kMaplyVecCenterY])
                compObj->contents->vectorOffset.y() = [inDesc[kMaplyVecCenterY] doubleValue];
        }
        for (MaplyVectorObject *vObj in vectors)
            compObj->contents->vecObjs.push_back(vObj->vObj);
    }
    
    compManager->addComponentObject(compObj->contents);
}

// Add vectors
- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([vectors count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:YES], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addVectorsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addVectorsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the widened vectors.
// Called in an unknown thread
- (void)addWideVectorsRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    iosDictionary dictWrap(inDesc);
    WideVectorInfo vectorInfo(dictWrap);
    [self resolveInfoDefaults:inDesc info:&vectorInfo defaultShader:kMaplyShaderDefaultWideVector];
    [self resolveDrawPriority:inDesc info:&vectorInfo drawPriority:kMaplyVectorDrawPriorityDefault offset:0];
    
    // Look for a texture and add it
    if (inDesc[kMaplyVecTexture])
    {
        UIImage *theImage = inDesc[kMaplyVecTexture];
        MaplyTexture *tex = nil;
        if ([theImage isKindOfClass:[UIImage class]])
            tex = [self addImage:theImage imageFormat:MaplyImage4Layer8Bit mode:threadMode];
        else if ([theImage isKindOfClass:[MaplyTexture class]])
            tex = (MaplyTexture *)theImage;
        if (tex.texID) {
            vectorInfo.texID = tex.texID;
            compObj->contents->texs.insert(tex);
        }
    }
    
    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
    {
        // Maybe need to make a copy if we're going to sample
        if (vectorInfo.subdivEps != 0.0)
        {
            float eps = vectorInfo.subdivEps;
            NSString *subdivType = inDesc[kMaplySubdivType];
            bool greatCircle = ![subdivType compare:kMaplySubdivGreatCircle];
            bool grid = ![subdivType compare:kMaplySubdivGrid];
            bool staticSubdiv = ![subdivType compare:kMaplySubdivStatic];
            MaplyVectorObject *newVecObj = [vecObj deepCopy2];
            // Note: This logic needs to be moved down a level
            //       Along with the subdivision routines above
            if (greatCircle)
                [newVecObj subdivideToGlobeGreatCircle:eps];
            else if (grid)
            {
                // The manager has to handle this one
            }
            else if (staticSubdiv)
            {
                // Note: Fill this in
            } else
                [newVecObj subdivideToGlobe:eps];

            shapes.insert(newVecObj->vObj->shapes.begin(),newVecObj->vObj->shapes.end());
        } else
            // We'll just reference it
            shapes.insert(vecObj->vObj->shapes.begin(),vecObj->vObj->shapes.end());
    }
    
    WideVectorManager *vectorManager = (WideVectorManager *)scene->getManager(kWKWideVectorManager);
    
    if (vectorManager)
    {
        ChangeSet changes;
        SimpleIdentity vecID = vectorManager->addVectors(&shapes, vectorInfo, changes);
        [self flushChanges:changes mode:threadMode];
        if (vecID != EmptyIdentity)
            compObj->contents->wideVectorIDs.insert(vecID);
    }
    
    // If the vectors are selectable we want to keep them around
    id selVal = inDesc[@"selectable"];
    if (selVal && [selVal boolValue]) {
        for (MaplyVectorObject *vObj in vectors)
            compObj->contents->vecObjs.push_back(vObj->vObj);

    }
    
    compManager->addComponentObject(compObj->contents);
}

- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([vectors count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addWideVectorsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addWideVectorsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Run the instancing logic
// Called in an unknown thread
- (void)instanceVectorsRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    MaplyComponentObject *baseObj = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    compObj->contents->vecObjs = baseObj->contents->vecObjs;
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    bool makeVisible = [[argArray objectAtIndex:3] boolValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:4] intValue];
    
    iosDictionary dictWrap(inDesc);
    VectorInfo vectorInfo(dictWrap);
    [self resolveInfoDefaults:inDesc info:&vectorInfo defaultShader:kMaplyDefaultTriangleShader];
    [self resolveDrawPriority:inDesc info:&vectorInfo drawPriority:kMaplyVectorDrawPriorityDefault offset:0];
    
    // Look for a texture and add it
    if (inDesc[kMaplyVecTexture])
    {
        UIImage *theImage = inDesc[kMaplyVecTexture];
        MaplyTexture *tex = nil;
        if ([theImage isKindOfClass:[UIImage class]])
            tex = [self addImage:theImage imageFormat:MaplyImage4Layer8Bit mode:threadMode];
        else if ([theImage isKindOfClass:[MaplyTexture class]])
            tex = (MaplyTexture *)theImage;
        if (tex.texID)
            vectorInfo.texId = tex.texID;
    }
    
    if (makeVisible)
    {
        VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
        WideVectorManager *wideVectorManager = (WideVectorManager *)scene->getManager(kWKWideVectorManager);

        ChangeSet changes;
        if (vectorManager && !baseObj->contents->vectorIDs.empty())
        {
            for (SimpleIDSet::iterator it = baseObj->contents->vectorIDs.begin();it != baseObj->contents->vectorIDs.end(); ++it)
            {
                SimpleIdentity instID = vectorManager->instanceVectors(*it, vectorInfo, changes);
                if (instID != EmptyIdentity)
                    compObj->contents->vectorIDs.insert(instID);
            }
        }
        if (wideVectorManager && !baseObj->contents->wideVectorIDs.empty())
        {
            iosDictionary dictWrap(inDesc);
            WideVectorInfo vectorInfo(dictWrap);

            for (SimpleIDSet::iterator it = baseObj->contents->wideVectorIDs.begin();it != baseObj->contents->wideVectorIDs.end(); ++it)
            {
                SimpleIdentity instID = wideVectorManager->instanceVectors(*it, vectorInfo, changes);
                if (instID != EmptyIdentity)
                    compObj->contents->wideVectorIDs.insert(instID);
            }
        }
        [self flushChanges:changes mode:threadMode];
    }
    
    compManager->addComponentObject(compObj->contents);
}

// Instance vectors
- (MaplyComponentObject *)instanceVectors:(MaplyComponentObject *)baseObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    NSArray *argArray = @[baseObj, compObj, [NSDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:YES], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self instanceVectorsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(instanceVectorsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Add vectors that we'll only use for selection
- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = false;
    
    if ([vectors count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:NO], @(MaplyThreadCurrent)];
    [self addVectorsRun:argArray];
    
    return compObj;
}

// Actually do the vector change
- (void)changeVectorRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    MaplyComponentObject *vecObj = [argArray objectAtIndex:0];
    NSDictionary *desc = [argArray objectAtIndex:1];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:2] intValue];
    
    @synchronized(vecObj)
    {
        if (!compManager->hasComponentObject(vecObj->contents->getId()))
            return;

        VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);

        if (vectorManager)
        {
            iosDictionary dictWrap(desc);
            VectorInfo vectorInfo(dictWrap);

            ChangeSet changes;
            for (SimpleIDSet::iterator it = vecObj->contents->vectorIDs.begin();
                 it != vecObj->contents->vectorIDs.end(); ++it)
                vectorManager->changeVectors(*it, vectorInfo, changes);
            [self flushChanges:changes mode:threadMode];
        }
    }
}

// Change vector representation
- (void)changeVectors:(MaplyComponentObject *)vecObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    if (!vecObj)
        return;
    
    if (!desc)
        desc = [NSDictionary dictionary];
    NSArray *argArray = @[vecObj, desc, @(threadMode)];
    
    // If the object is under construction, toss this over to the layer thread
    if (vecObj->contents->underConstruction)
        threadMode = MaplyThreadAny;
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self changeVectorRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(changeVectorRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}

// Called in the layer thread
- (void)addShapesRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    NSArray *shapes = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    iosDictionary dictWrap(inDesc);
    ShapeInfo shapeInfo(dictWrap);
    shapeInfo.insideOut = false;
    [self resolveInfoDefaults:inDesc info:&shapeInfo defaultShader:kMaplyDefaultTriangleShader];
    [self resolveDrawPriority:inDesc info:&shapeInfo drawPriority:kMaplyShapeDrawPriorityDefault offset:0];
    
    // Need to convert shapes to the form the API is expecting
    std::vector<Shape *> ourShapes,specialShapes;
    std::set<MaplyTexture *> textures;
    for (MaplyShape *shape in shapes)
    {
        Shape *baseShape = NULL;
        if ([shape isKindOfClass:[MaplyShapeCircle class]])
        {
            MaplyShapeCircle *circle = (MaplyShapeCircle *)shape;
            Circle *newCircle = (Circle *)[circle asWKShape:inDesc];
            baseShape = newCircle;
            ourShapes.push_back(baseShape);
        } else
        if ([shape isKindOfClass:[MaplyShapeSphere class]])
        {
            MaplyShapeSphere *sphere = (MaplyShapeSphere *)shape;
            Sphere *newSphere = (Sphere *)[sphere asWKShape:inDesc];
            baseShape = newSphere;
            ourShapes.push_back(baseShape);
        } else if ([shape isKindOfClass:[MaplyShapeCylinder class]])
        {
            MaplyShapeCylinder *cyl = (MaplyShapeCylinder *)shape;
            Cylinder *newCyl = (Cylinder *)[cyl asWKShape:inDesc];
            baseShape = newCyl;
            ourShapes.push_back(baseShape);
        } else if ([shape isKindOfClass:[MaplyShapeGreatCircle class]])
        {
            MaplyShapeGreatCircle *gc = (MaplyShapeGreatCircle *)shape;
            Linear *lin = new Linear();
            float eps = 0.001;
            if ([inDesc[kMaplySubdivEpsilon] isKindOfClass:[NSNumber class]])
                eps = [inDesc[kMaplySubdivEpsilon] floatValue];
            bool isStatic = [inDesc[kMaplySubdivType] isEqualToString:kMaplySubdivStatic];
            if (isStatic)
                SampleGreatCircleStatic(Point2d(gc.startPt.x,gc.startPt.y),Point2d(gc.endPt.x,gc.endPt.y),gc.height,lin->pts,visualView->coordAdapter,eps);
            else
                SampleGreatCircle(Point2d(gc.startPt.x,gc.startPt.y),Point2d(gc.endPt.x,gc.endPt.y),gc.height,lin->pts,visualView->coordAdapter,eps);
            lin->lineWidth = gc.lineWidth;
            if (gc.color)
            {
                lin->useColor = true;
                RGBAColor color = [gc.color asRGBAColor];
                lin->color = color;
            }
            baseShape = lin;
            specialShapes.push_back(lin);
        } else if ([shape isKindOfClass:[MaplyShapeRectangle class]])
        {
            MaplyShapeRectangle *rc = (MaplyShapeRectangle *)shape;
            Rectangle *rect = (Rectangle *)[rc asWKShape:inDesc];
            if (rc.color)
            {
                rect->useColor = true;
                RGBAColor color = [rc.color asRGBAColor];
                rect->color = color;
            }
            for (MaplyTexture *tex in rc.textures)
            {
                textures.insert(tex);
                rect->texIDs.push_back(tex.texID);
            }
            // Note: Selectability
            ourShapes.push_back(rect);
        } else if ([shape isKindOfClass:[MaplyShapeLinear class]])
        {
            MaplyShapeLinear *lin = (MaplyShapeLinear *)shape;
            Linear *newLin = (Linear *)[lin asWKShape:inDesc coordAdapter:coordAdapter];
            baseShape = newLin;
            ourShapes.push_back(newLin);
        } else if ([shape isKindOfClass:[MaplyShapeExtruded class]])
        {
            MaplyShapeExtruded *ex = (MaplyShapeExtruded *)shape;
            Extruded *newEx = (Extruded *)[ex asWKShape:inDesc];
            baseShape = newEx;

            ourShapes.push_back(newEx);
        }
        
        // Handle selection
        if (baseShape) {
            if (shape.selectable)
            {
                baseShape->isSelectable = true;
                baseShape->selectID = Identifiable::genId();
                compManager->addSelectObject(baseShape->selectID,shape);
                compObj->contents->selectIDs.insert(baseShape->selectID);
            }
        }
    }
    
    compObj->contents->texs = textures;
    
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    if (shapeManager)
    {
        ChangeSet changes;
        if (!ourShapes.empty())
        {
            SimpleIdentity shapeID = shapeManager->addShapes(ourShapes, shapeInfo, changes);
            if (shapeID != EmptyIdentity)
                compObj->contents->shapeIDs.insert(shapeID);
        }
        if (!specialShapes.empty())
        {
            // If they haven't overrided the shader already, we need the non-backface one for these objects
            if (!inDesc[kMaplyShader])
            {
                shapeInfo.programID = [self getProgramID:kMaplyShaderDefaultLineNoBackface];
            }
            SimpleIdentity shapeID = shapeManager->addShapes(specialShapes, shapeInfo, changes);
            if (shapeID != EmptyIdentity)
                compObj->contents->shapeIDs.insert(shapeID);
        }
        [self flushChanges:changes mode:threadMode];
    }
    
    compManager->addComponentObject(compObj->contents);
}

// Add shapes
- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([shapes count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[shapes, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addShapesRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addShapesRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Used to put geometry models with instances so we can group them
class GeomModelInstances
{
public:
    GeomModelInstances(MaplyGeomModel *model) : model(model) { }
    
    bool operator < (const GeomModelInstances &that) const
    {
        return model < that.model;
    }
    
    MaplyGeomModel *model;
    std::vector<MaplyGeomModelInstance *> instances;
};
struct GeomModelInstancesCmp
{
    bool operator ()(const GeomModelInstances *a,const GeomModelInstances *b) const
    {
        return *(a) < *(b);
    }
};
typedef std::set<GeomModelInstances *,struct GeomModelInstancesCmp> GeomModelInstancesSet;

// Called in the layer thread
- (void)addModelInstancesRun:(NSArray *)argArray
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    CoordSystem *coordSys = coordAdapter->getCoordSystem();
    NSArray *modelInstances = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    iosDictionary dictWrap(inDesc);
    GeometryInfo geomInfo(dictWrap);
    [self resolveInfoDefaults:inDesc info:&geomInfo defaultShader:kMaplyShaderDefaultModelTri];
    [self resolveDrawPriority:inDesc info:&geomInfo drawPriority:kMaplyModelDrawPriorityDefault offset:0];

    // May need a temporary context when setting up label textures
    EAGLContext *tmpContext = [self setupTempContext:threadMode];

    GeometryManager *geomManager = (GeometryManager *)scene->getManager(kWKGeometryManager);
    FontTextureManager_iOS *fontTexManager = (FontTextureManager_iOS *)scene->getFontTextureManager();

    // Sort the instances with their models
    GeomModelInstancesSet instSort;
    std::vector<MaplyGeomModelGPUInstance *> gpuInsts;
    for (id theInst in modelInstances)
    {
        if ([theInst isKindOfClass:[MaplyGeomModelInstance class]]) {
            MaplyGeomModelInstance *mInst = theInst;
            if (mInst.model)
            {
                GeomModelInstances searchInst(mInst.model);
                GeomModelInstancesSet::iterator it = instSort.find(&searchInst);
                if (it != instSort.end())
                {
                    (*it)->instances.push_back(mInst);
                } else {
                    GeomModelInstances *newInsts = new GeomModelInstances(mInst.model);
                    newInsts->instances.push_back(mInst);
                    instSort.insert(newInsts);
                }
            }
        } else if ([theInst isKindOfClass:[MaplyGeomModelGPUInstance class]]) {
            gpuInsts.push_back(theInst);
        }
    }
    
    // Add each model with its group of instances
    if (geomManager)
    {
        // Regular geometry instances
        ChangeSet changes;
        for (auto it : instSort)
        {
            // Set up the textures and convert the geometry
            MaplyGeomModel *model = it->model;
            
            // Return an existing base model or make a new one
            SimpleIdentity baseModelID = [model getBaseModel:self fontTexManager:fontTexManager compObj:compObj mode:threadMode];
            
            // Reference count the textures for this comp obj
            compObj->contents->texs.insert(model->maplyTextures.begin(),model->maplyTextures.end());

            if (baseModelID != EmptyIdentity)
            {
                // Convert the instances
                std::vector<GeometryInstance> matInst;
                for (unsigned int ii=0;ii<it->instances.size();ii++)
                {
                    MaplyGeomModelInstance *modelInst = it->instances[ii];
                    Matrix4d localMat = localMat.Identity();

                    // Local transformation, before the placement
                    if (modelInst.transform)
                        localMat = modelInst.transform.mat;
                    
                    // Add in the placement
                    Point3d localPt = coordSys->geographicToLocal(Point2d(modelInst.center.x,modelInst.center.y));
                    Point3d dispLoc = coordAdapter->localToDisplay(Point3d(localPt.x(),localPt.y(),modelInst.center.z));
                    Point3d norm = coordAdapter->normalForLocal(localPt);
                                        
                    // Construct a set of axes to build the shape around
                    Point3d xAxis,yAxis;
                    if (coordAdapter->isFlat())
                    {
                        xAxis = Point3d(1,0,0);
                        yAxis = Point3d(0,1,0);
                    } else {
                        Point3d north(0,0,1);
                        // Note: Also check if we're at a pole
                        xAxis = north.cross(norm);  xAxis.normalize();
                        yAxis = norm.cross(xAxis);  yAxis.normalize();
                    }
                    
                    // Set up a shift matrix that moves coordinate to the right orientation on the globe (or not)
                    //  and shifts it to the correct position
                    Matrix4d shiftMat;
                    shiftMat(0,0) = xAxis.x();
                    shiftMat(0,1) = yAxis.x();
                    shiftMat(0,2) = norm.x();
                    shiftMat(0,3) = 0.0;
                    
                    shiftMat(1,0) = xAxis.y();
                    shiftMat(1,1) = yAxis.y();
                    shiftMat(1,2) = norm.y();
                    shiftMat(1,3) = 0.0;
                    
                    shiftMat(2,0) = xAxis.z();
                    shiftMat(2,1) = yAxis.z();
                    shiftMat(2,2) = norm.z();
                    shiftMat(2,3) = 0.0;
                    
                    shiftMat(3,0) = 0.0;
                    shiftMat(3,1) = 0.0;
                    shiftMat(3,2) = 0.0;
                    shiftMat(3,3) = 1.0;

                    localMat = shiftMat * localMat;

                    // Basic geometry instance fields
                    GeometryInstance thisInst;
                    thisInst.center = dispLoc;
                    thisInst.mat = localMat;
                    thisInst.colorOverride = modelInst.colorOverride != nil;
                    if (thisInst.colorOverride)
                        thisInst.color = [modelInst.colorOverride asRGBAColor];
                    thisInst.selectable = modelInst.selectable;
                    if (thisInst.selectable)
                    {
                        compManager->addSelectObject(thisInst.getId(),modelInst);
                        compObj->contents->selectIDs.insert(thisInst.getId());
                    }
                    
                    // Motion related fields
                    if ([modelInst isKindOfClass:[MaplyMovingGeomModelInstance class]])
                    {
                        MaplyMovingGeomModelInstance *movingInst = (MaplyMovingGeomModelInstance *)modelInst;
                        if (movingInst.duration > 0.0)
                        {
                            // Placement for the end point
                            Point3d localPt = coordSys->geographicToLocal(Point2d(movingInst.endCenter.x,movingInst.endCenter.y));
                            Point3d dispLoc = coordAdapter->localToDisplay(Point3d(localPt.x(),localPt.y(),movingInst.endCenter.z));
                            thisInst.endCenter = dispLoc;
                            thisInst.duration = movingInst.duration;
                        }
                    }
                    
                    matInst.push_back(thisInst);
                }
                
                SimpleIdentity geomID = geomManager->addGeometryInstances(baseModelID, matInst, geomInfo, changes);
                if (geomID != EmptyIdentity)
                    compObj->contents->geomIDs.insert(geomID);
            }
        }
        
        // GPU Geometry Instances
        for (auto geomInst : gpuInsts) {
            // Set up the textures and convert the geometry
            MaplyGeomModel *model = geomInst.model;
            
            // Return an existing base model or make a new one
            SimpleIdentity baseModelID = [model getBaseModel:self fontTexManager:fontTexManager compObj:compObj mode:threadMode];
            
            // Reference count the textures for this comp obj
            compObj->contents->texs.insert(model->maplyTextures.begin(),model->maplyTextures.end());

            SimpleIdentity programID = EmptyIdentity;
            if (geomInst.shader && geomInst.shader.program)
                programID = geomInst.shader.program->getId();
            SimpleIdentity srcTexID = EmptyIdentity;
            if (geomInst.numInstSource)
                srcTexID = geomInst.numInstSource.texID;
            SimpleIdentity srcProgramID = EmptyIdentity;
            if (geomInst.numInstShader)
                srcProgramID = geomInst.numInstShader.program->getId();
            
            SimpleIdentity geomID = geomManager->addGPUGeomInstance(baseModelID, programID, srcTexID, srcProgramID, geomInfo, changes);
            if (geomID != EmptyIdentity)
                compObj->contents->geomIDs.insert(geomID);
        }
        
        [self flushChanges:changes mode:threadMode];
    }
    
    // Clean up the instances we sorted
    for (auto it : instSort)
        delete it;
    
    compManager->addComponentObject(compObj->contents);

    [self clearTempContext:tmpContext];
}

// Called in the layer thread
- (void)addGeometryRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *geom = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSMutableDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];

    GeometryInfo geomInfo;
    [self resolveInfoDefaults:inDesc info:&geomInfo defaultShader:kMaplyDefaultTriangleShader];
    [self resolveDrawPriority:inDesc info:&geomInfo drawPriority:kMaplyStickerDrawPriorityDefault offset:0];

    GeometryManager *geomManager = (GeometryManager *)scene->getManager(kWKGeometryManager);
    
    // Add each raw geometry model
    if (geomManager)
    {
        ChangeSet changes;
        
        for (MaplyGeomModel *model in geom)
        {
            // This is intended to be instanced, but we can use it
            SimpleIdentity geomID = geomManager->addBaseGeometry(model->rawGeom, geomInfo, changes);
            // If we turn it on
            SimpleIDSet geomIDs;
            geomIDs.insert(geomID);
            geomManager->enableGeometry(geomIDs, true, changes);
            
            if (geomID != EmptyIdentity)
                compObj->contents->geomIDs.insert(geomID);
        }
        
        [self flushChanges:changes mode:threadMode];
    }
    
    compManager->addComponentObject(compObj->contents);
}

- (MaplyComponentObject *)addModelInstances:(NSArray *)modelInstances desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([modelInstances count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[modelInstances, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addModelInstancesRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addModelInstancesRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

- (MaplyComponentObject *)addGeometry:(NSArray *)geom desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([geom count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[geom, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addGeometryRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addGeometryRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Called in the layer thread
- (void)addStickersRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *stickers = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    iosDictionary dictWrap(inDesc);
    SphericalChunkInfo chunkInfo(dictWrap);
    [self resolveInfoDefaults:inDesc info:&chunkInfo defaultShader:kMaplyDefaultTriangleShader];
    [self resolveDrawPriority:inDesc info:&chunkInfo drawPriority:kMaplyStickerDrawPriorityDefault offset:0];
    
    SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);
    ChangeSet changes;

    std::vector<SphericalChunk> chunks;
    chunks.reserve([stickers count]);
    for (MaplySticker *sticker in stickers)
    {
        std::vector<SimpleIdentity> texIDs;
        if (sticker.image) {
            if ([sticker.image isKindOfClass:[UIImage class]])
            {
                MaplyTexture *tex = [self addImage:sticker.image imageFormat:sticker.imageFormat mode:threadMode];
                if (tex)
                    texIDs.push_back(tex.texID);
                compObj->contents->texs.insert(tex);
            } else if ([sticker.image isKindOfClass:[MaplyTexture class]])
            {
                MaplyTexture *tex = (MaplyTexture *)sticker.image;
                texIDs.push_back(tex.texID);
                compObj->contents->texs.insert(tex);
            }
        }
        for (UIImage *image in sticker.images)
        {
            if ([image isKindOfClass:[UIImage class]])
            {
                MaplyTexture *tex = [self addImage:image imageFormat:sticker.imageFormat mode:threadMode];
                if (tex)
                    texIDs.push_back(tex.texID);
                compObj->contents->texs.insert(tex);
            } else if ([image isKindOfClass:[MaplyTexture class]])
            {
                MaplyTexture *tex = (MaplyTexture *)image;
                texIDs.push_back(tex.texID);
                compObj->contents->texs.insert(tex);
            }
        }
        SphericalChunk chunk;
        Mbr mbr(Point2f(sticker.ll.x,sticker.ll.y), Point2f(sticker.ur.x,sticker.ur.y));
        chunk.mbr = mbr;
        chunk.texIDs = texIDs;
        // Note: Move this over to info logic
        chunk.sampleX = [inDesc[@"sampleX"] intValue];
        chunk.sampleY = [inDesc[@"sampleY"] intValue];
        if (chunk.sampleX == 0)  chunk.sampleX = 10;
        if (chunk.sampleY == 0) chunk.sampleY = 10;
        chunk.programID = chunkInfo.programID;
        if (inDesc[kMaplySubdivEpsilon] != nil)
            chunk.eps = [inDesc[kMaplySubdivEpsilon] floatValue];
        if (sticker.coordSys)
            chunk.coordSys = [sticker.coordSys getCoordSystem];
        chunk.rotation = sticker.rotation;
        
        chunks.push_back(chunk);
    }
    
    if (chunkManager)
    {
        SimpleIdentity chunkID = chunkManager->addChunks(chunks, chunkInfo, changes);
        if (chunkID != EmptyIdentity)
            compObj->contents->chunkIDs.insert(chunkID);
    }

    [self flushChanges:changes mode:threadMode];
    compManager->addComponentObject(compObj->contents);
}

// Add stickers
- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([stickers count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[stickers, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addStickersRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addStickersRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually do the sticker change
- (void)changeStickerRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    MaplyComponentObject *stickerObj = [argArray objectAtIndex:0];
    NSDictionary *desc = [argArray objectAtIndex:1];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:2] intValue];
    
    @synchronized(stickerObj)
    {
        if (!compManager->hasComponentObject(stickerObj->contents->getId()))
            return;
        
        SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);
        
        if (chunkManager)
        {
            // Change the images being displayed
            NSArray *newImages = desc[kMaplyStickerImages];
            if ([newImages isKindOfClass:[NSArray class]])
            {
                MaplyQuadImageFormat newFormat = MaplyImageIntRGBA;
                if ([desc[kMaplyStickerImageFormat] isKindOfClass:[NSNumber class]])
                    newFormat = (MaplyQuadImageFormat)[desc[kMaplyStickerImageFormat] integerValue];
                std::vector<SimpleIdentity> newTexIDs;
                std::set<MaplyTexture *> oldTextures = stickerObj->contents->texs;
                stickerObj->contents->texs.clear();

                // Add in the new images
                for (UIImage *image in newImages)
                {
                    if ([image isKindOfClass:[UIImage class]])
                    {
                        MaplyTexture *tex = [self addImage:image imageFormat:newFormat mode:threadMode];
                        if (tex)
                            newTexIDs.push_back(tex.texID);
                        stickerObj->contents->texs.insert(tex);
                    } else if ([image isKindOfClass:[MaplyTexture class]])
                    {
                        MaplyTexture *tex = (MaplyTexture *)image;
                        newTexIDs.push_back(tex.texID);
                        stickerObj->contents->texs.insert(tex);
                    }
                }
                
                // Clear out the old images
                oldTextures.clear();
                
                ChangeSet changes;
                for (SimpleIDSet::iterator it = stickerObj->contents->chunkIDs.begin();
                     it != stickerObj->contents->chunkIDs.end(); ++it)
                    chunkManager->modifyChunkTextures(*it, newTexIDs, changes);
                [self flushChanges:changes mode:threadMode];
            }
        }
    }
}

// Change stickers
- (void)changeSticker:(MaplyComponentObject *)stickerObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    if (!stickerObj)
        return;
    
    if (!desc)
        desc = [NSDictionary dictionary];
    NSArray *argArray = @[stickerObj, desc, @(threadMode)];
    
    // If the object is under construction, toss this over to the layer thread
    if (stickerObj->contents->underConstruction)
        threadMode = MaplyThreadAny;
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self changeStickerRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(changeStickerRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}

// Actually add the lofted polys.
- (void)addLoftedPolysRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    iosDictionary dictWrap(inDesc);
    LoftedPolyInfo loftInfo(dictWrap);
    [self resolveInfoDefaults:inDesc info:&loftInfo defaultShader:kMaplyDefaultTriangleShader];
    [self resolveDrawPriority:inDesc info:&loftInfo drawPriority:kMaplyLoftedPolysDrawPriorityDefault offset:0];
    
    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
        shapes.insert(vecObj->vObj->shapes.begin(),vecObj->vObj->shapes.end());

    ChangeSet changes;
    LoftManager *loftManager = (LoftManager *)scene->getManager(kWKLoftedPolyManager);
    if (loftManager)
    {
        SimpleIdentity loftID = loftManager->addLoftedPolys(&shapes, loftInfo, changes);
        compObj->contents->loftIDs.insert(loftID);
        compObj->contents->isSelectable = false;
    }
    [self flushChanges:changes mode:threadMode];
    
    compManager->addComponentObject(compObj->contents);
}

// Add lofted polys
- (MaplyComponentObject *)addLoftedPolys:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([vectors count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addLoftedPolysRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addLoftedPolysRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Actually add the billboards.
- (void)addBillboardsRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *bills = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    CoordSystemDisplayAdapter *coordAdapter = visualView->coordAdapter;
    CoordSystem *coordSys = coordAdapter->getCoordSystem();
    
    iosDictionary dictWrap(inDesc);
    BillboardInfo billInfo(dictWrap);
    
    [self resolveInfoDefaults:inDesc info:&billInfo
                defaultShader:(billInfo.orient == WhirlyKit::BillboardInfo::Eye ? kMaplyShaderBillboardEye : kMaplyShaderBillboardGround)];
    [self resolveDrawPriority:inDesc info:&billInfo drawPriority:kMaplyBillboardDrawPriorityDefault offset:0];

    // May need a temporary context when setting up label textures
    EAGLContext *tmpContext = [self setupTempContext:threadMode];
    
    ChangeSet changes;
    BillboardManager *billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);
    FontTextureManager_iOS *fontTexManager = (FontTextureManager_iOS *)scene->getFontTextureManager();
    if (billManager && fontTexManager)
    {
        std::vector<Billboard *> wkBills;
        for (MaplyBillboard *bill in bills)
        {
            Billboard *wkBill = new Billboard();
            Point3d localPt = coordSys->geographicToLocal3d(GeoCoord(bill.center.x,bill.center.y));
            Point3d dispPt = coordAdapter->localToDisplay(Point3d(localPt.x(),localPt.y(),bill.center.z));
            wkBill->center = dispPt;
            wkBill->isSelectable = bill.selectable;
            if (wkBill->isSelectable)
                wkBill->selectID = Identifiable::genId();
            
            if (bill.selectable)
            {
                compManager->addSelectObject(wkBill->selectID,bill);
                compObj->contents->selectIDs.insert(wkBill->selectID);
            }

            MaplyScreenObject *screenObj = bill.screenObj;
            if (!screenObj)
                continue;
            MaplyBoundingBox size = [screenObj getSize];
            Point2d size2d = Point2d(size.ur.x-size.ll.x,size.ur.y-size.ll.y);
            wkBill->size = size2d;

            // Work through the individual polygons in a billboard
            for (SimplePolyRef thePoly : screenObj->screenObj.polys)
            {
                SimplePoly_iOSRef poly = std::dynamic_pointer_cast<SimplePoly_iOS>(thePoly);
                SingleBillboardPoly billPoly;
                billPoly.pts = poly->pts;
                billPoly.texCoords = poly->texCoords;
                billPoly.color = poly->color;
                if (bill.vertexAttributes)
                    [self resolveVertexAttrs:billPoly.vertexAttrs from:bill.vertexAttributes];
                if (poly->texture)
                {
                    MaplyTexture *tex = nil;
                    if ([poly->texture isKindOfClass:[UIImage class]])
                    {
                        tex = [self addImage:poly->texture imageFormat:MaplyImageIntRGBA mode:threadMode];
                    } else if ([poly->texture isKindOfClass:[MaplyTexture class]])
                    {
                        tex = (MaplyTexture *)poly->texture;
                    }
                    if (tex)
                    {
                        compObj->contents->texs.insert(tex);
                        billPoly.texId = tex.texID;
                    }
                }
                wkBill->polys.push_back(billPoly);
            }
            
            // Now for the strings
            for (auto theStrWrap : screenObj->screenObj.strings)
            {
                StringWrapper_iOSRef strWrap = std::dynamic_pointer_cast<StringWrapper_iOS>(theStrWrap);
                // Convert the string to polygons
                DrawableString *drawStr = fontTexManager->addString(strWrap->str,changes);
                for (const DrawableString::Rect &rect : drawStr->glyphPolys)
                {
                    SingleBillboardPoly billPoly;
                    billPoly.pts.resize(4);
                    billPoly.texCoords.resize(4);
                    billPoly.texId = rect.subTex.texId;
                    billPoly.texCoords[0] = rect.subTex.processTexCoord(TexCoord(0,0));
                    billPoly.texCoords[1] = rect.subTex.processTexCoord(TexCoord(1,0));
                    billPoly.texCoords[2] = rect.subTex.processTexCoord(TexCoord(1,1));
                    billPoly.texCoords[3] = rect.subTex.processTexCoord(TexCoord(0,1));
                    billPoly.pts[0] = Point2d(rect.pts[0].x(),rect.pts[0].y());
                    billPoly.pts[1] = Point2d(rect.pts[1].x(),rect.pts[0].y());
                    billPoly.pts[2] = Point2d(rect.pts[1].x(),rect.pts[1].y());
                    billPoly.pts[3] = Point2d(rect.pts[0].x(),rect.pts[1].y());
                    for (unsigned int ip=0;ip<4;ip++)
                    {
                        const Point2d &oldPt = billPoly.pts[ip];
                        Point3d newPt = strWrap->mat * Point3d(oldPt.x(),oldPt.y(),1.0);
                        billPoly.pts[ip] = Point2d(newPt.x(),newPt.y());
                    }
                    
                    wkBill->polys.push_back(billPoly);
                }
                
                compObj->contents->drawStringIDs.insert(drawStr->getId());
                delete drawStr;
            }
            
            wkBills.push_back(wkBill);
        }
        
        SimpleIdentity billId = billManager->addBillboards(wkBills, billInfo, changes);
        compObj->contents->billIDs.insert(billId);
        compObj->contents->isSelectable = false;
    }
    [self flushChanges:changes mode:threadMode];
    
    compManager->addComponentObject(compObj->contents);

    [self clearTempContext:tmpContext];
}

// Add billboards
- (MaplyComponentObject *)addBillboards:(NSArray *)bboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    if ([bboards count] == 0)
    {
        compManager->addComponentObject(compObj->contents);
        return compObj;
    }
    
    NSArray *argArray = @[bboards, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addBillboardsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addBillboardsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

- (void)addParticleSystemRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    MaplyParticleSystem *partSys = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    iosDictionary dictWrap(inDesc);
    BaseInfo partInfo(dictWrap);
    [self resolveInfoDefaults:inDesc info:&partInfo defaultShader:kMaplyShaderParticleSystemPointDefault];
    [self resolveDrawPriority:inDesc info:&partInfo drawPriority:kMaplyParticleSystemDrawPriorityDefault offset:0];
    
    // May need a temporary context
    EAGLContext *tmpContext = [self setupTempContext:threadMode];
    
    SimpleIdentity partSysShaderID = [inDesc[kMaplyShader] intValue];
    if (partSysShaderID == EmptyIdentity)
        partSysShaderID = [self getProgramID:kMaplyShaderParticleSystemPointDefault];
    if (partSys.renderShader) {
        partSysShaderID = [partSys.renderShader getShaderID];
    }
    SimpleIdentity calcShaderID = EmptyIdentity;
    if (partSys.positionShader) {
        calcShaderID = [partSys.positionShader getShaderID];
    }
    
    ParticleSystemManager *partSysManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);

    ChangeSet changes;
    if (partSysManager)
    {
        ParticleSystem wkPartSys;
        wkPartSys.setId(partSys.ident);
        wkPartSys.enable = inDesc[kMaplyEnable] != nil ? [inDesc[kMaplyEnable] boolValue] : true;
        wkPartSys.drawPriority = [inDesc[kMaplyDrawPriority] intValue];
        wkPartSys.pointSize = [inDesc[kMaplyPointSize] floatValue];
        wkPartSys.name = [partSys.name cStringUsingEncoding:NSASCIIStringEncoding];
        wkPartSys.renderShaderID = partSysShaderID;
        wkPartSys.calcShaderID = calcShaderID;
        wkPartSys.lifetime = partSys.lifetime;
        wkPartSys.batchSize = partSys.batchSize;
        wkPartSys.totalParticles = partSys.totalParticles;
        wkPartSys.vertexSize = partSys.vertexSize;
        wkPartSys.baseTime = partSys.baseTime;
        wkPartSys.continuousUpdate = partSys.continuousUpdate;
        wkPartSys.zBufferRead = [inDesc[kMaplyZBufferRead] boolValue];
        wkPartSys.zBufferWrite = [inDesc[kMaplyZBufferWrite] boolValue];
        wkPartSys.renderTargetID = partSys.renderTargetID;
        // Type
        switch (partSys.type)
        {
            case MaplyParticleSystemTypePoint:
                wkPartSys.type = ParticleSystemPoint;
                break;
            case MaplyParticleSystemTypeRectangle:
                wkPartSys.type = ParticleSystemRectangle;
                break;
        }
        // Do the attributes
        for (auto it : partSys.attrs)
        {
            SingleVertexAttributeInfo vertAttr;
            switch (it.type)
            {
                case MaplyShaderAttrTypeInt:
                    vertAttr.type = BDIntType;
                    break;
                case MaplyShaderAttrTypeFloat:
                    vertAttr.type = BDFloatType;
                    break;
                case MaplyShaderAttrTypeFloat2:
                    vertAttr.type = BDFloat2Type;
                    break;
                case MaplyShaderAttrTypeFloat3:
                    vertAttr.type = BDFloat3Type;
                    break;
                case MaplyShaderAttrTypeFloat4:
                    vertAttr.type = BDFloat4Type;
                    break;
                default:
                    NSLog(@"Missing attribute type in MaplyBaseInteractionLayer");
                    break;
            }
            vertAttr.nameID = StringIndexer::getStringID([it.name cStringUsingEncoding:NSASCIIStringEncoding]);
            if (it.varyName) {
                // This one is a varying attribute
                SingleVertexAttributeInfo varyAttr = vertAttr;
                wkPartSys.varyingAttrs.push_back(varyAttr);
            } else {
                wkPartSys.vertAttrs.push_back(vertAttr);
            }
        }
        // Now the textures
        for (id image : partSys.images)
        {
            MaplyTexture *maplyTex = nil;
            if ([image isKindOfClass:[UIImage class]])
            {
                maplyTex = [self addImage:image imageFormat:MaplyImageIntRGBA mode:threadMode];
            } else if ([image isKindOfClass:[MaplyTexture class]])
                maplyTex = image;
            wkPartSys.texIDs.push_back(maplyTex.texID);
            compObj->contents->texs.insert(maplyTex);
        }
        
        SimpleIdentity partSysID = partSysManager->addParticleSystem(wkPartSys, changes);
        partSys.ident = partSysID;
        compObj->contents->partSysIDs.insert(partSysID);
    }
    
    [self flushChanges:changes mode:threadMode];
    
    compManager->addComponentObject(compObj->contents);

    [self clearTempContext:tmpContext];
}

- (MaplyComponentObject *)addParticleSystem:(MaplyParticleSystem *)partSys desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    NSArray *argArray = @[partSys, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addParticleSystemRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addParticleSystemRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

- (void)changeParticleSystem:(MaplyComponentObject *)compObj renderTarget:(MaplyRenderTarget *)target
{
    ParticleSystemManager *partSysManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);

    if (partSysManager) {
        ChangeSet changes;

        SimpleIdentity targetID = target ? target.renderTargetID : EmptyIdentity;
        for (SimpleIdentity partSysID : compObj->contents->partSysIDs) {
            partSysManager->changeRenderTarget(partSysID,targetID,changes);
        }

        [self flushChanges:changes mode:MaplyThreadCurrent];
    }
}

- (void)addParticleSystemBatchRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    MaplyParticleBatch *batch = argArray[0];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:1] intValue];
    
    // May need a temporary context
    EAGLContext *tmpContext = [self setupTempContext:threadMode];
    
    ParticleSystemManager *partSysManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);

    ChangeSet changes;
    if (partSysManager)
    {
        bool validBatch = true;
        ParticleBatch wkBatch;
        wkBatch.batchSize = batch.partSys.batchSize;
        
        if (sceneRender->getType() == WhirlyKit::SceneRendererGLES_iOS::RenderGLES) {
            // Copy the attributes over in the right order
            for (auto mainAttr : batch.partSys.attrs)
            {
                if (mainAttr.varyName)
                    continue;
                
                bool found = false;
                // Find the one that matches
                for (auto thisAttr : batch.attrVals)
                {
                    if (thisAttr.attrID == mainAttr.getId())
                    {
                        found = true;
                        wkBatch.attrData.push_back([thisAttr.data bytes]);
                        break;
                    }
                }
                if (!found)
                {
                    NSLog(@"Missing attribute data for particle batch.  Dropping.");
                    validBatch = false;
                }
            }
        } else {
            // For Metal, we just pass through the data
            wkBatch.data = RawNSDataReaderRef(new RawNSDataReader(batch.data));
        }
        
        if (validBatch)
            partSysManager->addParticleBatch(batch.partSys.ident, wkBatch, changes);
    }
    
    // We always want a glFlush here
    changes.push_back(NULL);
    
    [self flushChanges:changes mode:threadMode];
    
    [self clearTempContext:tmpContext];
}

- (void)addParticleBatch:(MaplyParticleBatch *)batch mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    NSArray *argArray = @[batch, @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addParticleSystemBatchRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addParticleSystemBatchRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}

- (void)addPointsRun:(NSArray *)argArray
{
    NSArray *pointsArray = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    iosDictionary dictWrap(inDesc);
    GeometryInfo geomInfo(dictWrap);
    if (geomInfo.pointSize == 0.0)
        geomInfo.pointSize = kMaplyPointSizeDefault;
    [self resolveInfoDefaults:inDesc info:&geomInfo defaultShader:kMaplyShaderParticleSystemPointDefault];
    [self resolveDrawPriority:inDesc info:&geomInfo drawPriority:kMaplyParticleSystemDrawPriorityDefault offset:0];

    compObj->contents->isSelectable = false;

    // May need a temporary context
    EAGLContext *tmpContext = [self setupTempContext:threadMode];

    GeometryManager *geomManager = (GeometryManager *)scene->getManager(kWKGeometryManager);
    
    ChangeSet changes;
    if (geomManager)
    {
        for (MaplyPoints *points : pointsArray)
        {
            Matrix4d mat = Matrix4d::Identity();
            if (points.transform)
            {
                mat = points.transform.mat;
            }
            SimpleIdentity geomID = geomManager->addGeometryPoints(points->points, mat, geomInfo, changes);
            if (geomID != EmptyIdentity)
                compObj->contents->geomIDs.insert(geomID);
        }
    }
    
    [self flushChanges:changes mode:threadMode];
    
    compManager->addComponentObject(compObj->contents);

    [self clearTempContext:tmpContext];
}

- (MaplyComponentObject *)addPoints:(NSArray *)points desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj->contents->underConstruction = true;
    
    NSArray *argArray = @[points, compObj, [NSDictionary dictionaryWithDictionary:desc], @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self addPointsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(addPointsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
    
    return compObj;
}

// Add a render target to the renderer
- (void)addRenderTarget:(MaplyRenderTarget *)renderTarget
{
    if (renderTarget.renderTargetID == EmptyIdentity || renderTarget.texture == nil)
        return;
    
    ChangeSet changes;
    changes.push_back(new AddRenderTargetReq(renderTarget.renderTargetID,
                                             renderTarget.texture.width,
                                             renderTarget.texture.height,
                                             renderTarget.texture.texID,
                                             renderTarget.clearEveryFrame,
                                             renderTarget.blend,
                                             [renderTarget.clearColor asRGBAColor],
                                             renderTarget.clearVal,
                                             (RenderTargetMipmapType)renderTarget.mipmapType,
                                             renderTarget.calculateMinMax));
    
    [self flushChanges:changes mode:MaplyThreadCurrent];
}

- (void)changeRenderTarget:(MaplyRenderTarget *)renderTarget tex:(MaplyTexture *)tex
{
    ChangeSet changes;
    SimpleIdentity texID = tex ? tex.texID : EmptyIdentity;
    changes.push_back(new ChangeRenderTargetReq(renderTarget.renderTargetID,texID));
    
    [self flushChanges:changes mode:MaplyThreadCurrent];
}

- (void)clearRenderTargetRun:(NSArray *)argArray
{
    MaplyRenderTarget *renderTarget = argArray[0];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:1] intValue];
    
    ChangeSet changes;
    
    changes.push_back(new ClearRenderTargetReq(renderTarget.renderTargetID));
    
    [self flushChanges:changes mode:threadMode];
}

- (void)clearRenderTarget:(MaplyRenderTarget *)renderTarget mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    NSArray *argArray = @[renderTarget, @(threadMode)];
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self clearRenderTargetRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(clearRenderTargetRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}

// Stop rendering to a given render target
- (void)removeRenderTarget:(MaplyRenderTarget *)renderTarget
{
    ChangeSet changes;
    changes.push_back(new RemRenderTargetReq(renderTarget.renderTargetID));

    [self flushChanges:changes mode:MaplyThreadCurrent];
}

- (void)removeObjects:(NSArray *)userObjs changes:(WhirlyKit::ChangeSet &)changes
{
    for (MaplyComponentObject *compObj in userObjs)
    {
        @synchronized (compObj) {
            // And associated textures
            for (MaplyTexture *tex : compObj->contents->texs)
                [self removeImageTexture:tex changes:changes];
            compObj->contents->texs.clear();

            if (compObj->contents) {
                // This does the visual and the selection data
                compManager->removeComponentObject(compObj->contents->getId(),changes);
            }
        }
    }
}


// Remove the object, but do it on the layer thread
- (void)removeObjectRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;
    
    NSArray *inUserObjs = argArray[0];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:1] intValue];
    
    ChangeSet changes;

    [self removeObjects:inUserObjs changes:changes];

    [self flushChanges:changes mode:threadMode];
}

// Remove a group of objects at once
- (void)removeObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    NSArray *argArray = @[userObjs, @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj->contents->underConstruction)
                anyUnderConstruction = true;
        if (anyUnderConstruction)
            threadMode = MaplyThreadAny;
    }
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self removeObjectRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(removeObjectRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}

- (void)setUniformBlockRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *userObjs = [argArray objectAtIndex:0];
    NSData *uniBlock = [argArray objectAtIndex:1];
    int bufferID = [[argArray objectAtIndex:2] intValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    RawNSDataReaderRef dataWrap(new RawNSDataReader(uniBlock));

    SimpleIDSet compIDs;
    for (MaplyComponentObject *compObj in userObjs) {
        compIDs.insert(compObj->contents->getId());
    }
    ChangeSet changes;
    compManager->setUniformBlock(compIDs,dataWrap,bufferID,changes);
    
    [self flushChanges:changes mode:threadMode];
}

- (void)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID forObjects:(NSArray<MaplyComponentObject *> *__nonnull)userObjs mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];
    
    NSArray *argArray = @[userObjs,uniBlock,@(bufferID),@(threadMode)];
    
    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj->contents->underConstruction)
                anyUnderConstruction = true;
        if (anyUnderConstruction)
            threadMode = MaplyThreadAny;
    }
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self setUniformBlockRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(setUniformBlockRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}


// This is called directly from outside.
// Not great, but it means we need the start/work calls in here to catch locking shutdowns
- (void)enableObjects:(NSArray *)userObjs changes:(ChangeSet &)changes
{
    if (![self startOfWork])
        return;
    [self enableObjectsImpl:userObjs enable:true changes:changes];
    [self endOfWork];
}

- (void)disableObjects:(NSArray *)userObjs changes:(ChangeSet &)changes
{
    [self enableObjectsImpl:userObjs enable:false changes:changes];
}

- (void)enableObjectsImpl:(NSArray *)userObjs enable:(bool)enable changes:(ChangeSet &)changes
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    for (MaplyComponentObject *compObj in userObjs)
        compManager->enableComponentObject(compObj->contents->getId(), enable, changes);
}

- (void)enableObjectsRun:(NSArray *)argArray
{
    if (isShuttingDown || (!layerThread && !offlineMode))
        return;

    NSArray *theObjs = argArray[0];
    bool enable = [argArray[1] boolValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:2] intValue];

    ChangeSet changes;
    [self enableObjectsImpl:theObjs enable:enable changes:changes];

    [self flushChanges:changes mode:threadMode];
}

// Enable objects
- (void)enableObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    NSArray *argArray = @[userObjs, @(true), @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj->contents->underConstruction)
                anyUnderConstruction = true;
        if (anyUnderConstruction)
            threadMode = MaplyThreadAny;
    }
    
    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self enableObjectsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(enableObjectsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }    
}

// Disable objects
- (void)disableObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode
{
    threadMode = [self resolveThreadMode:threadMode];

    NSArray *argArray = @[userObjs, @(false), @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj->contents->underConstruction)
                anyUnderConstruction = true;
        if (anyUnderConstruction)
            threadMode = MaplyThreadAny;
    }

    switch (threadMode)
    {
        case MaplyThreadCurrent:
            [self enableObjectsRun:argArray];
            break;
        case MaplyThreadAny:
            [self performSelector:@selector(enableObjectsRun:) onThread:layerThread withObject:argArray waitUntilDone:NO];
            break;
    }
}


// Search for a point inside any of our vector objects
// Runs in layer thread
- (NSArray *)findVectorsInPoint:(Point2f)pt
{
    return [self findVectorsInPoint:pt inView:nil multi:true];
}

- (NSArray *)findVectorsInPoint:(Point2f)pt inView:(MaplyBaseViewController *)vc multi:(bool)multi
{
    if (!layerThread)
        return nil;
    
    NSMutableArray *foundObjs = [NSMutableArray array];
    
    pt = visualView->unwrapCoordinate(pt);
    
    ViewStateRef viewState = vc->renderControl->visualView->makeViewState(vc->renderControl->sceneRenderer.get());
    
    auto rets = compManager->findVectors(Point2d(pt.x(),pt.y()),20.0,viewState,vc->renderControl->sceneRenderer->getFramebufferSizeScaled(),multi);
    
    for (auto foundObj : rets) {
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithRef:foundObj.second];
        [foundObjs addObject:vecObj];
    }
    
    return foundObjs;
}

- (NSObject *)getSelectableObject:(WhirlyKit::SimpleIdentity)objId
{
    return compManager->getSelectObject(objId);
}

- (NSObject*)selectLabelsAndMarkerForScreenPoint:(CGPoint)screenPoint
{
    return nil;
}

- (void)dumpStats
{
    if (compManager)
        compManager->dumpStats();
    
    [atlasGroup dumpStats];
}

@end

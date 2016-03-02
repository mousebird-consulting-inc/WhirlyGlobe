/*
 *  MaplyBaseInteractionLayer.mm
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

#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyScreenMarker.h"
#import "MaplyMarker.h"
#import "MaplyScreenLabel.h"
#import "MaplyLabel.h"
#import "MaplyVectorObject_private.h"
#import "MaplyShape.h"
#import "MaplySticker.h"
#import "MaplyBillboard.h"
#import "MaplyCoordinate.h"
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

using namespace Eigen;
using namespace WhirlyKit;

// Sample a great circle and throw in an interpolated height at each point
void SampleGreatCircle(MaplyCoordinate startPt,MaplyCoordinate endPt,float height,std::vector<Point3f> &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,float eps)
{
    bool isFlat = coordAdapter->isFlat();

    // We can subdivide the great circle with this routine
    if (isFlat)
    {
        pts.resize(2);
        pts[0] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(startPt.x,startPt.y)));
        pts[1] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(endPt.x,endPt.y)));
    } else {
        VectorRing inPts;
        inPts.push_back(Point2f(startPt.x,startPt.y));
        inPts.push_back(Point2f(endPt.x,endPt.y));
        SubdivideEdgesToSurfaceGC(inPts, pts, false, coordAdapter, eps);

        // To apply the height, we'll need the total length
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        
        // Now we'll walk along, apply the height (max at the middle)
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            Point3f &pt = pts[ii];
            float len = (pts[ii+1]-pt).norm();
            float t = lenSoFar/totLen;
            lenSoFar += len;
            
            // Parabolic curve
            float b = 4*height;
            float a = -b;
            float thisHeight = a*(t*t) + b*t;
            
            if (isFlat)
                pt.z() = thisHeight;
            else
                pt *= 1.0+thisHeight;        
        }
    }
}

// Sample a great circle and throw in an interpolated height at each point
void SampleGreatCircleStatic(MaplyCoordinate startPt,MaplyCoordinate endPt,float height,std::vector<Point3f> &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,float samples)
{
    bool isFlat = coordAdapter->isFlat();
    
    // We can subdivide the great circle with this routine
    if (isFlat)
    {
        pts.resize(2);
        pts[0] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(startPt.x,startPt.y)));
        pts[1] = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(endPt.x,endPt.y)));
    } else {
        VectorRing inPts;
        inPts.push_back(Point2f(startPt.x,startPt.y));
        inPts.push_back(Point2f(endPt.x,endPt.y));
        SubdivideEdgesToSurfaceGC(inPts, pts, false, coordAdapter, 1.0, 0.0, samples);
        
        // To apply the height, we'll need the total length
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        
        // Now we'll walk along, apply the height (max at the middle)
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            Point3f &pt = pts[ii];
            float len = (pts[ii+1]-pt).norm();
            float t = lenSoFar/totLen;
            lenSoFar += len;
            
            // Parabolic curve
            float b = 4*height;
            float a = -b;
            float thisHeight = a*(t*t) + b*t;
            
            if (isFlat)
            pt.z() = thisHeight;
            else
            pt *= 1.0+thisHeight;
        }
    }
}

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

@implementation MaplyBaseInteractionLayer
{
    pthread_mutex_t changeLock;
    ThreadChangeSet perThreadChanges;
    pthread_mutex_t workLock;
    pthread_cond_t workWait;
    int numActiveWorkers;
}

- (id)initWithView:(WhirlyKitView *)inVisualView
{
    self = [super init];
    if (!self)
        return nil;
    visualView = inVisualView;
    pthread_mutex_init(&selectLock, NULL);
    pthread_mutex_init(&imageLock, NULL);
    pthread_mutex_init(&changeLock,NULL);
    pthread_mutex_init(&tempContextLock,NULL);
    pthread_mutex_init(&workLock,NULL);
    numActiveWorkers = 0;
    pthread_cond_init(&workWait, NULL);
    
    return self;
}

- (void)dealloc
{
    pthread_mutex_destroy(&selectLock);
    pthread_mutex_destroy(&imageLock);
    pthread_mutex_destroy(&changeLock);
    pthread_mutex_destroy(&tempContextLock);
    pthread_mutex_destroy(&workLock);
    pthread_cond_destroy(&workWait);
    
    for (ThreadChangeSet::iterator it = perThreadChanges.begin();
         it != perThreadChanges.end();++it)
    {
        ThreadChanges threadChanges = *it;
        for (unsigned int ii=0;ii<threadChanges.changes.size();ii++)
            delete threadChanges.changes[ii];
    }
    perThreadChanges.clear();
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = (WhirlyGlobe::GlobeScene *)inScene;
    userObjects = [NSMutableSet set];
    atlasGroup = [[MaplyTextureAtlasGroup alloc] initWithScene:scene];
    
    glSetupInfo = [[WhirlyKitGLSetupInfo alloc] init];
    glSetupInfo->minZres = [visualView calcZbufferRes];
}

- (void)shutdown
{
    layerThread = nil;
    scene = NULL;
    imageTextures.clear();
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

- (void)lockingShutdown
{
    // This shouldn't happen
    if (isShuttingDown || !layerThread)
        return;
    
    if ([NSThread currentThread] != layerThread)
    {
        [self performSelector:@selector(lockingShutdown) onThread:layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    pthread_mutex_lock(&workLock);
    isShuttingDown = true;
    while (numActiveWorkers > 0)
        pthread_cond_wait(&workWait, &workLock);

    [self shutdown];
    
    pthread_mutex_unlock(&workLock);
}

- (bool)startOfWork
{
    bool ret = true;
    
    pthread_mutex_lock(&workLock);
    ret = !isShuttingDown;
    if (ret)
        numActiveWorkers++;
    pthread_mutex_unlock(&workLock);
    
    return ret;
}

- (void)endOfWork
{
    pthread_mutex_lock(&workLock);
    numActiveWorkers--;
    pthread_cond_signal(&workWait);
    pthread_mutex_unlock(&workLock);
}

- (Texture *)createTexture:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    int imageFormat = [desc intForKey:kMaplyTexFormat default:MaplyImageIntRGBA];
    bool wrapX = [desc boolForKey:kMaplyTexWrapX default:false];
    bool wrapY = [desc boolForKey:kMaplyTexWrapX default:false];
    int magFilter = [desc enumForKey:kMaplyTexMagFilter values:@[kMaplyMinFilterNearest,kMaplyMinFilterLinear] default:0];
    
    int imgWidth = image.size.width * image.scale;
    int imgHeight = image.size.height * image.scale;
    imgWidth = NextPowOf2(imgWidth);
    imgHeight = NextPowOf2(imgHeight);
    
    // Add it and download it
    Texture *tex = new Texture("MaplyBaseInteraction",image,imgWidth,imgHeight);
    tex->setWrap(wrapX, wrapY);
    tex->setUsesMipmaps(false);
    tex->setInterpType(magFilter == 0 ? GL_NEAREST : GL_LINEAR);
    switch (imageFormat)
    {
        case MaplyImageIntRGBA:
        case MaplyImage4Layer8Bit:
        default:
            tex->setFormat(GL_UNSIGNED_BYTE);
            break;
        case MaplyImageUShort565:
            tex->setFormat(GL_UNSIGNED_SHORT_5_6_5);
            break;
        case MaplyImageUShort4444:
            tex->setFormat(GL_UNSIGNED_SHORT_4_4_4_4);
            break;
        case MaplyImageUShort5551:
            tex->setFormat(GL_UNSIGNED_SHORT_5_5_5_1);
            break;
        case MaplyImageUByteRed:
            tex->setFormat(GL_ALPHA);
            tex->setSingleByteSource(WKSingleRed);
            break;
        case MaplyImageUByteGreen:
            tex->setFormat(GL_ALPHA);
            tex->setSingleByteSource(WKSingleGreen);
            break;
        case MaplyImageUByteBlue:
            tex->setFormat(GL_ALPHA);
            tex->setSingleByteSource(WKSingleBlue);
            break;
        case MaplyImageUByteAlpha:
            tex->setFormat(GL_ALPHA);
            tex->setSingleByteSource(WKSingleAlpha);
            break;
        case MaplyImageUByteRGB:
            tex->setFormat(GL_ALPHA);
            tex->setSingleByteSource(WKSingleRGB);
            break;
    }
    
    return tex;
}

// Explicitly add a texture
- (MaplyTexture *)addTexture:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    pthread_mutex_lock(&imageLock);
    
    // Look for an image texture that's already representing our UIImage
    MaplyTexture *maplyTex = nil;
    std::vector<MaplyImageTextureList::iterator> toRemove;
    for (MaplyImageTextureList::iterator theImageTex = imageTextures.begin();
         theImageTex != imageTextures.end(); ++theImageTex)
    {
        if (*theImageTex)
        {
            if ((*theImageTex).image == image)
            {
                maplyTex = *theImageTex;
                break;
            }
        } else
            toRemove.push_back(theImageTex);
    }
    for (auto rem : toRemove)
        imageTextures.erase(rem);

    // Takes the altas path instead
    if (!maplyTex && [desc boolForKey:kMaplyTexAtlas default:false])
    {
        pthread_mutex_unlock(&imageLock);
        return [self addTextureToAtlas:image desc:desc mode:threadMode];
    }
    
    ChangeSet changes;
    if (!maplyTex)
    {
        maplyTex = [[MaplyTexture alloc] init];
        
        Texture *tex = [self createTexture:image desc:desc mode:threadMode];
        maplyTex.texID = tex->getId();
        maplyTex.interactLayer = self;
        maplyTex.image = image;
        
        changes.push_back(new AddTextureReq(tex));
        imageTextures.push_back(maplyTex);
    }
    
    pthread_mutex_unlock(&imageLock);

    if (!changes.empty())
        [self flushChanges:changes mode:threadMode];

    return maplyTex;
}

- (MaplyTexture *)addTextureToAtlas:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
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
        imageTextures.push_back(maplyTex);
    }
    delete tex;

    // If we're making changes in this thread, do the flushes
    if (threadMode == MaplyThreadCurrent)
    {
        // Note: Borrowed from layer thread
        bool requiresFlush = false;
        
        // Set up anything that needs to be set up
        ChangeSet changesToAdd;
        for (unsigned int ii=0;ii<changes.size();ii++)
        {
            ChangeRequest *change = changes[ii];
            if (change)
            {
                requiresFlush |= change->needsFlush();
                change->setupGL(glSetupInfo, scene->getMemManager());
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

- (void)removeTextures:(NSArray *)textures mode:(MaplyThreadMode)threadMode
{
    for (MaplyTexture *texture in textures)
        [texture clear];
}

// Called by the texture dealloc
- (void)clearTexture:(MaplyTexture *)tex
{
    if (!layerThread || isShuttingDown)
        return;
    
    ChangeSet changes;

    if (tex.isSubTex)
    {
        if (atlasGroup)
        {
            [atlasGroup removeTexture:tex.texID changes:changes];
            scene->removeSubTexture(tex.texID);
        }
    } else {
        if (scene)
            changes.push_back(new RemTextureReq(tex.texID));
    }

    [self flushChanges:changes mode:MaplyThreadCurrent];
}

- (MaplyTexture *)addImage:(id)image imageFormat:(MaplyQuadImageFormat)imageFormat mode:(MaplyThreadMode)threadMode
{
    return [self addImage:image imageFormat:imageFormat wrapFlags:MaplyImageWrapNone interpType:GL_NEAREST mode:threadMode];
}

// Add an image to the cache, or find an existing one
// Called in the layer thread
- (MaplyTexture *)addImage:(id)image imageFormat:(MaplyQuadImageFormat)imageFormat wrapFlags:(int)wrapFlags interpType:(GLenum)interpType mode:(MaplyThreadMode)threadMode
{
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
    pthread_mutex_lock(&imageLock);
    
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
    if (tex.interactLayer)
        [self performSelector:@selector(delayedRemoveTexture:) withObject:tex afterDelay:2.0];
    else {
        // If we created it in this object, we'll clean it up
        if (tex.texID != EmptyIdentity)
        {
            changes.push_back(new RemTextureReq(tex.texID));
            tex.texID = EmptyIdentity;
        }
    }
    
    pthread_mutex_unlock(&imageLock);
}

// Remove the given Texture ID after a delay
// Note: This is a hack to work around fade problems
- (void)delayedRemoveTexture:(MaplyTexture *)maplyTex
{
    // Holding the object until there delays the deletion
}

// We flush out changes in different ways depending on the thread mode
- (void)flushChanges:(ChangeSet &)changes mode:(MaplyThreadMode)threadMode
{
    if (changes.empty())
        return;

    // This means we beat the layer thread setup, so we'll put this in orbit
    if (!scene)
        threadMode = MaplyThreadAny;

    switch (threadMode)
    {
        case MaplyThreadCurrent:
        {
            pthread_mutex_lock(&changeLock);

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
            } else
                // We're not, so execute the changes
                scene->addChangeRequests(changes);

            pthread_mutex_unlock(&changeLock);
        }
            break;
        case MaplyThreadAny:
            [layerThread addChangeRequests:changes];
            break;
    }
}

- (void)startChanges
{
    pthread_mutex_lock(&changeLock);

    // Look for changes in the current thread
    NSThread *currentThread = [NSThread currentThread];
    ThreadChanges changes(currentThread);
    ThreadChangeSet::iterator it = perThreadChanges.find(changes);
    // If there isn't one, we add it.  That's how we know we're doing this.
    if (it == perThreadChanges.end())
        perThreadChanges.insert(changes);

    pthread_mutex_unlock(&changeLock);
}

- (void)endChanges
{
    pthread_mutex_lock(&changeLock);

    // Look for outstanding changes
    NSThread *currentThread = [NSThread currentThread];
    ThreadChanges changes(currentThread);
    ThreadChangeSet::iterator it = perThreadChanges.find(changes);
    if (it != perThreadChanges.end())
    {
        ThreadChanges theseChanges = *it;
        scene->addChangeRequests(theseChanges.changes);
        perThreadChanges.erase(it);
    }

    pthread_mutex_unlock(&changeLock);
}

// We can refer to shaders by ID or by name.  Figure that out.
- (void)resolveShader:(NSMutableDictionary *)inDesc defaultShader:(NSString *)defaultShaderName
{
    NSObject *shader = inDesc[kMaplyShader];
    if (shader)
    {
        // Translate the shader into an ID
        if ([shader isKindOfClass:[NSString class]])
        {
            NSString *shaderName = (NSString *)shader;
            SimpleIdentity shaderID = scene->getProgramIDBySceneName([shaderName cStringUsingEncoding:NSASCIIStringEncoding]);
            if (shaderID == EmptyIdentity)
                [inDesc removeObjectForKey:@"shader"];
            else
                inDesc[kMaplyShader] = @(shaderID);
        }
    } else if (defaultShaderName)
    {
        SimpleIdentity shaderID = scene->getProgramIDBySceneName([defaultShaderName cStringUsingEncoding:NSASCIIStringEncoding]);
        if (shaderID != EmptyIdentity)
            inDesc[kMaplyShader] = @(shaderID);
    }
}

// Copy vertex attributes from the source to the dest
- (void)resolveVertexAttrs:(SingleVertexAttributeSet &)destAttrs from:(NSArray *)srcAttrs
{
    for (MaplyVertexAttribute *attr in srcAttrs)
        destAttrs.insert(attr->attr);
}

// Apply a default value to the dictionary
-(void) applyDefaultName:(NSString *)key value:(NSObject *)val toDict:(NSMutableDictionary *)dict
{
    if (!dict[key])
        dict[key] = val;
}

- (void)resolveDrawPriority:(NSMutableDictionary *)desc offset:(int)offsetPriority
{
    NSNumber *setting = desc[@"drawPriority"];
    int iVal = 0;
    if ([setting isKindOfClass:[NSNumber class]])
    {
        iVal = [setting intValue];
    }
    iVal += offsetPriority;
    desc[@"drawPriority"] = @(iVal);
}

// Actually add the markers.
// Called in an unknown thread
- (void)addScreenMarkersRun:(NSArray *)argArray
{
    if (isShuttingDown || !layerThread)
        return;

    NSArray *markers = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    
    bool isMotionMarkers = false;
    if ([[markers objectAtIndex:0] isKindOfClass:[MaplyMovingScreenMarker class]])
        isMotionMarkers = true;
    // Note: Check that the caller isn't mixing in regular markers
    
    // Might be a custom shader on these
    if (isMotionMarkers)
        [self resolveShader:inDesc defaultShader:@(kToolkitDefaultScreenSpaceMotionProgram)];
    else
        [self resolveShader:inDesc defaultShader:@(kToolkitDefaultScreenSpaceProgram)];
    [self resolveDrawPriority:inDesc offset:_screenObjectDrawPriorityOffset];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (MaplyScreenMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.x,marker.loc.y);
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
            wgMarker.period = marker.period;
        compObj.textures.insert(texs.begin(),texs.end());
        wgMarker.color = marker.color;
        if (!texs.empty())
        {
            for (unsigned int ii=0;ii<texs.size();ii++)
                wgMarker.texIDs.push_back(texs[ii].texID);
        }
        wgMarker.width = marker.size.width;
        wgMarker.height = marker.size.height;
        if (marker.rotation != 0.0)
        {
            wgMarker.rotation = marker.rotation;
            wgMarker.lockRotation = true;
        }
        if (marker.selectable)
        {
            wgMarker.isSelectable = true;
            wgMarker.selectID = Identifiable::genId();
        }
        wgMarker.layoutImportance = marker.layoutImportance;

        if (marker.vertexAttributes)
            [self resolveVertexAttrs:wgMarker.vertexAttrs from:marker.vertexAttributes];
        
        if (marker.layoutSize.width >= 0.0)
        {
            wgMarker.layoutWidth = marker.layoutSize.width;
            wgMarker.layoutHeight = marker.layoutSize.height;
        } else {
            wgMarker.layoutWidth = wgMarker.width;
            wgMarker.layoutHeight = wgMarker.height;
        }
        wgMarker.offset = Point2d(marker.offset.x,marker.offset.y);
        
        // Now for the motion related fields
        if ([marker isKindOfClass:[MaplyMovingScreenMarker class]])
        {
            MaplyMovingScreenMarker *movingMarker = (MaplyMovingScreenMarker *)marker;
            wgMarker.hasMotion = true;
            wgMarker.endLoc = GeoCoord(movingMarker.endLoc.x,movingMarker.endLoc.y);
            wgMarker.startTime = now;
            wgMarker.endTime = now + movingMarker.duration;
        }

        [wgMarkers addObject:wgMarker];
        
        if (marker.selectable)
        {
            pthread_mutex_lock(&selectLock);
            selectObjectSet.insert(SelectObject(wgMarker.selectID,marker));
            pthread_mutex_unlock(&selectLock);
            compObj.selectIDs.insert(wgMarker.selectID);
        }
    }
    
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    
    if (markerManager)
    {
        // Set up a description and create the markers in the marker layer
        NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
        [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
        ChangeSet changes;
        SimpleIdentity markerID = markerManager->addMarkers(wgMarkers, desc, changes);
        if (markerID != EmptyIdentity)
            compObj.markerIDs.insert(markerID);
        [self flushChanges:changes mode:threadMode];
    }
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

// Called in the main thread.
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([markers count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[markers, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
    
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

// Actually add the markers.
// Called in an unknown thread.
- (void)addMarkersRun:(NSArray *)argArray
{
    if (isShuttingDown || !layerThread)
        return;

    NSArray *markers = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];

    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyMarkerDrawPriorityDefault) toDict:inDesc];
    
    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (MaplyMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.x,marker.loc.y);
        MaplyTexture *tex = nil;
        if ([marker.image isKindOfClass:[UIImage class]])
        {
            tex = [self addImage:marker.image imageFormat:MaplyImageIntRGBA mode:threadMode];
        } else if ([marker.image isKindOfClass:[MaplyTexture class]])
        {
            tex = (MaplyTexture *)marker.image;
        }
        compObj.textures.insert(tex);
        if (tex)
            wgMarker.texIDs.push_back(tex.texID);
        wgMarker.width = marker.size.width;
        wgMarker.height = marker.size.height;
        if (marker.selectable)
        {
            wgMarker.isSelectable = true;
            wgMarker.selectID = Identifiable::genId();
        }
        
        [wgMarkers addObject:wgMarker];
        
        if (marker.selectable)
        {
            pthread_mutex_lock(&selectLock);
            selectObjectSet.insert(SelectObject(wgMarker.selectID,marker));
            pthread_mutex_unlock(&selectLock);
            compObj.selectIDs.insert(wgMarker.selectID);
        }
    }
    
    // Set up a description and create the markers in the marker layer
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    if (markerManager)
    {
        ChangeSet changes;
        SimpleIdentity markerID = markerManager->addMarkers(wgMarkers, inDesc, changes);
        if (markerID != EmptyIdentity)
            compObj.markerIDs.insert(markerID);
        [self flushChanges:changes mode:threadMode];
    }
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

// Add 3D markers
- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;

    if ([markers count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }

    NSArray *argArray = @[markers, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
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
    EAGLContext *tmpContext = nil;
    
    // Use the renderer's context
    if (threadMode == MaplyThreadCurrent && [NSThread mainThread] == [NSThread currentThread])
    {
        tmpContext = layerThread.renderer.context;
        [EAGLContext setCurrentContext:tmpContext];
    }
    
    if (threadMode == MaplyThreadCurrent && ![EAGLContext currentContext])
    {
        pthread_mutex_lock(&tempContextLock);

        // See if we need to create a new one
        if (tempContexts.empty())
        {
            tmpContext = [[EAGLContext alloc] initWithAPI:layerThread.renderer.context.API sharegroup:layerThread.renderer.context.sharegroup];
        } else {
            // We can use an existing one
            std::set<EAGLContext *>::iterator it = tempContexts.begin();
            tmpContext = *it;
            tempContexts.erase(it);
        }
        [EAGLContext setCurrentContext:tmpContext];
        
        pthread_mutex_unlock(&tempContextLock);
    }
    
    return tmpContext;
}

// This just releases the context, but we may want to keep a queue of these in future
- (void)clearTempContext:(EAGLContext *)context
{
    if ([NSThread mainThread] == [NSThread currentThread] && context == layerThread.renderer.context)
    {
        [EAGLContext setCurrentContext:nil];
        return;
    }
    
    if (context)
    {
        glFlush();
        [EAGLContext setCurrentContext:nil];

        // Put this one back for use by another thread
        pthread_mutex_lock(&tempContextLock);
        tempContexts.insert(context);
        pthread_mutex_unlock(&tempContextLock);
    }
}

// Actually add the labels.
// Called in an unknown thread.
- (void)addScreenLabelsRun:(NSArray *)argArray
{
    if (isShuttingDown || !layerThread)
        return;

    NSArray *labels = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    // May need a temporary context when setting up screen label textures
    EAGLContext *tmpContext = [self setupTempContext:threadMode];

    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    
    bool isMotionLabels = false;
    if ([[labels objectAtIndex:0] isKindOfClass:[MaplyMovingScreenLabel class]])
        isMotionLabels = true;
    // Note: Check that the caller isn't mixing in regular markers

    // Might be a custom shader on these
    if (isMotionLabels)
        [self resolveShader:inDesc defaultShader:@(kToolkitDefaultScreenSpaceMotionProgram)];
    else
        [self resolveShader:inDesc defaultShader:@(kToolkitDefaultScreenSpaceProgram)];
    [self resolveDrawPriority:inDesc offset:_screenObjectDrawPriorityOffset];

    // Convert to WG screen labels
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (MaplyScreenLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel.rotation = label.rotation;
        wgLabel.text = label.text;
        wgLabel.keepUpright = label.keepUpright;
        MaplyTexture *tex = nil;
        if (label.iconImage2) {
            tex = [self addImage:label.iconImage2 imageFormat:MaplyImageIntRGBA mode:threadMode];
            compObj.textures.insert(tex);
        }
        if (tex)
            wgLabel.iconTexture = tex.texID;
        wgLabel.iconSize = label.iconSize;
        if (label.color)
            [desc setObject:label.color forKey:@"textColor"];
        if (label.layoutImportance != MAXFLOAT)
        {
            [desc setObject:@(YES) forKey:@"layout"];
            [desc setObject:@(label.layoutImportance) forKey:@"layoutImportance"];
            [desc setObject:@(label.layoutPlacement) forKey:@"layoutPlacement"];
        }
        wgLabel.screenOffset = CGSizeMake(label.offset.x,label.offset.y);
        if (label.selectable)
        {
            wgLabel.isSelectable = true;
            wgLabel.selectID = Identifiable::genId();
        }
        if ([desc count] > 0)
            wgLabel.desc = desc;

        // Now for the motion related fields
        if ([label isKindOfClass:[MaplyMovingScreenLabel class]])
        {
            MaplyMovingScreenLabel *movingLabel = (MaplyMovingScreenLabel *)label;
            wgLabel.hasMotion = true;
            wgLabel.endLoc = GeoCoord(movingLabel.endLoc.x,movingLabel.endLoc.y);
            wgLabel.startTime = now;
            wgLabel.endTime = now + movingLabel.duration;
        }

        [wgLabels addObject:wgLabel];
        
        if (label.selectable)
        {
            pthread_mutex_lock(&selectLock);
            selectObjectSet.insert(SelectObject(wgLabel.selectID,label));
            pthread_mutex_unlock(&selectLock);
            compObj.selectIDs.insert(wgLabel.selectID);
        }
    }
    
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    if (labelManager)
    {
        // Set up a description and create the markers in the marker layer
        NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
        [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
        ChangeSet changes;
        SimpleIdentity labelID = labelManager->addLabels(wgLabels, desc, changes);
        [self flushChanges:changes mode:threadMode];
        if (labelID != EmptyIdentity)
            compObj.labelIDs.insert(labelID);
    }

    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
    
    [self clearTempContext:tmpContext];
}

// Add screen space (2D) labels
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([labels count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[labels, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];

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
    if (isShuttingDown || !layerThread)
        return;

    NSArray *labels = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];

    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyLabelDrawPriorityDefault) toDict:inDesc];

    // May need a temporary context when setting up label textures
    EAGLContext *tmpContext = [self setupTempContext:threadMode];

    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];

    // Convert to WG labels
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (MaplyLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.x,label.loc.y);
        wgLabel.text = label.text;
        MaplyTexture *tex = nil;
        if (label.iconImage2) {
            tex = [self addImage:label.iconImage2 imageFormat:MaplyImageIntRGBA mode:threadMode];
            compObj.textures.insert(tex);
        }
        wgLabel.iconTexture = tex.texID;
        if (label.size.width > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.width] forKey:@"width"];
        if (label.size.height > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.height] forKey:@"height"];
        if (label.color)
            [desc setObject:label.color forKey:@"textColor"];
        if (label.selectable)
        {
            wgLabel.isSelectable = true;
            wgLabel.selectID = Identifiable::genId();
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
        wgLabel.desc = desc;
        
        [wgLabels addObject:wgLabel];
        
        if (label.selectable)
        {
            pthread_mutex_lock(&selectLock);
            selectObjectSet.insert(SelectObject(wgLabel.selectID,label));
            pthread_mutex_unlock(&selectLock);
            compObj.selectIDs.insert(wgLabel.selectID);
        }
    }
    
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    
    if (labelManager)
    {
        ChangeSet changes;
        // Set up a description and create the markers in the marker layer
        SimpleIdentity labelID = labelManager->addLabels(wgLabels, inDesc, changes);
        [self flushChanges:changes mode:threadMode];
        if (labelID != EmptyIdentity)
            compObj.labelIDs.insert(labelID);
    }
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
    
    [self clearTempContext:tmpContext];
}

// Add 3D labels
- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([labels count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[labels, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];

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
    if (isShuttingDown || !layerThread)
        return;

    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    bool makeVisible = [[argArray objectAtIndex:3] boolValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:4] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyVectorDrawPriorityDefault) toDict:inDesc];
    
    // Might be a custom shader on these
    NSString *shaderName = kMaplyDefaultTriangleShader;
    if ([inDesc[kMaplyVecTextureProjection] isEqualToString:kMaplyProjectionScreen])
        shaderName = kMaplyShaderDefaultTriScreenTex;
    [self resolveShader:inDesc defaultShader:shaderName];
    
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
            inDesc[kMaplyVecTexture] = @(tex.texID);
        else
            [inDesc removeObjectForKey:kMaplyVecTexture];        
    }

    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
    {
        // Maybe need to make a copy if we're going to sample
        if (inDesc[kMaplySubdivEpsilon])
        {
            float eps = [inDesc[kMaplySubdivEpsilon] floatValue];
            NSString *subdivType = inDesc[kMaplySubdivType];
            bool greatCircle = ![subdivType compare:kMaplySubdivGreatCircle];
            bool grid = ![subdivType compare:kMaplySubdivGrid];
            bool staticSubdiv = ![subdivType compare:kMaplySubdivStatic];
            MaplyVectorObject *newVecObj = [vecObj deepCopy2];
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

            shapes.insert(newVecObj.shapes.begin(),newVecObj.shapes.end());
        } else
            // We'll just reference it
            shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());
    }
    
    if (makeVisible)
    {
        VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
        
        if (vectorManager)
        {
            ChangeSet changes;
            SimpleIdentity vecID = vectorManager->addVectors(&shapes, inDesc, changes);
            [self flushChanges:changes mode:threadMode];
            if (vecID != EmptyIdentity)
                compObj.vectorIDs.insert(vecID);
        }
    }
    
    // If the vectors are selectable we want to keep them around
    id selVal = inDesc[@"selectable"];
    if (selVal && [selVal boolValue])
    {
        if ([inDesc[kMaplyVecCentered] boolValue])
        {
            if (inDesc[kMaplyVecCenterX])
                compObj.vectorOffset.x() = [inDesc[kMaplyVecCenterX] doubleValue];
            if (inDesc[kMaplyVecCenterY])
                compObj.vectorOffset.y() = [inDesc[kMaplyVecCenterY] doubleValue];
        }
        compObj.vectors = vectors;
    }
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

// Add vectors
- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([vectors count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[vectors, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:YES], @(threadMode)];
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
    if (isShuttingDown || !layerThread)
        return;

    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyVectorDrawPriorityDefault) toDict:inDesc];
    
    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];
    
    // If there's no shader, we'll apply the default one
    if (!inDesc[kMaplyShader])
        inDesc[kMaplyShader] = @(scene->getProgramIDBySceneName(kToolkitDefaultWideVectorProgram));

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
            inDesc[kMaplyVecTexture] = @(tex.texID);
        else
            [inDesc removeObjectForKey:kMaplyVecTexture];
    }
    
    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
        shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());
    
    WideVectorManager *vectorManager = (WideVectorManager *)scene->getManager(kWKWideVectorManager);
    
    if (vectorManager)
    {
        ChangeSet changes;
        SimpleIdentity vecID = vectorManager->addVectors(&shapes, inDesc, changes);
        [self flushChanges:changes mode:threadMode];
        if (vecID != EmptyIdentity)
            compObj.wideVectorIDs.insert(vecID);
    }
    
    // If the vectors are selectable we want to keep them around
    id selVal = inDesc[@"selectable"];
    if (selVal && [selVal boolValue])
        compObj.vectors = vectors;
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([vectors count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[vectors, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
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
    if (isShuttingDown || !layerThread)
        return;

    MaplyComponentObject *baseObj = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    compObj.vectors = baseObj.vectors;
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    bool makeVisible = [[argArray objectAtIndex:3] boolValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:4] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyVectorDrawPriorityDefault) toDict:inDesc];
    
    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];
    
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
            inDesc[kMaplyVecTexture] = @(tex.texID);
        else
            [inDesc removeObjectForKey:kMaplyVecTexture];
    }
    
    if (makeVisible)
    {
        VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
        WideVectorManager *wideVectorManager = (WideVectorManager *)scene->getManager(kWKWideVectorManager);
        
        ChangeSet changes;
        if (vectorManager)
        {
            for (SimpleIDSet::iterator it = baseObj.vectorIDs.begin();it != baseObj.vectorIDs.end(); ++it)
            {
                SimpleIdentity instID = vectorManager->instanceVectors(*it, inDesc, changes);
                if (instID != EmptyIdentity)
                    compObj.vectorIDs.insert(instID);
            }
        }
        if (wideVectorManager)
        {
            for (SimpleIDSet::iterator it = baseObj.wideVectorIDs.begin();it != baseObj.wideVectorIDs.end(); ++it)
            {
                SimpleIdentity instID = wideVectorManager->instanceVectors(*it, inDesc, changes);
                if (instID != EmptyIdentity)
                    compObj.wideVectorIDs.insert(instID);
            }
        }
        [self flushChanges:changes mode:threadMode];
    }
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

// Instance vectors
- (MaplyComponentObject *)instanceVectors:(MaplyComponentObject *)baseObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[baseObj, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:YES], @(threadMode)];
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
    compObj.underConstruction = false;
    
    if ([vectors count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[vectors, compObj, [NSDictionary dictionaryWithDictionary:desc], [NSNumber numberWithBool:NO], @(MaplyThreadCurrent)];
    [self addVectorsRun:argArray];
    
    return compObj;
}

// Actually do the vector change
- (void)changeVectorRun:(NSArray *)argArray
{
    if (isShuttingDown || !layerThread)
        return;

    MaplyComponentObject *vecObj = [argArray objectAtIndex:0];
    NSDictionary *desc = [argArray objectAtIndex:1];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:2] intValue];
    
    @synchronized(vecObj)
    {
        bool isHere = false;
        @synchronized(userObjects)
        {
            isHere = [userObjects containsObject:vecObj];
        }
        
        if (!isHere)
            return;

        VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);

        if (vectorManager)
        {
            ChangeSet changes;
            for (SimpleIDSet::iterator it = vecObj.vectorIDs.begin();
                 it != vecObj.vectorIDs.end(); ++it)
                vectorManager->changeVectors(*it, desc, changes);
            [self flushChanges:changes mode:threadMode];
        }
    }
}

// Change vector representation
- (void)changeVectors:(MaplyComponentObject *)vecObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (!vecObj)
        return;
    
    if (!desc)
        desc = [NSDictionary dictionary];
    NSArray *argArray = @[vecObj, desc, @(threadMode)];
    
    // If the object is under construction, toss this over to the layer thread
    if (vecObj.underConstruction)
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
    if (isShuttingDown || !layerThread)
        return;

    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    NSArray *shapes = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyShapeDrawPriorityDefault) toDict:inDesc];
    [self applyDefaultName:kMaplyShapeInsideOut value:@(NO) toDict:inDesc];
    [self applyDefaultName:kMaplyShapeSampleX value:@(10) toDict:inDesc];
    [self applyDefaultName:kMaplyShapeSampleY value:@(10) toDict:inDesc];

    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];

    // Need to convert shapes to the form the API is expecting
    NSMutableArray *ourShapes = [NSMutableArray array];
    NSMutableArray *specialShapes = [NSMutableArray array];
    for (NSObject *shape in shapes)
    {
        if ([shape isKindOfClass:[MaplyShapeCircle class]])
        {
            MaplyShapeCircle *circle = (MaplyShapeCircle *)shape;
            WhirlyKitCircle *newCircle = [circle asWKShape:inDesc];
            
            if (circle.selectable)
            {
                newCircle.isSelectable = true;
                newCircle.selectID = Identifiable::genId();
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(newCircle.selectID,circle));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(newCircle.selectID);
            }
            [ourShapes addObject:newCircle];
        } else if ([shape isKindOfClass:[MaplyShapeSphere class]])
        {
            MaplyShapeSphere *sphere = (MaplyShapeSphere *)shape;
            WhirlyKitSphere *newSphere = [sphere asWKShape:inDesc];
            
            if (sphere.selectable)
            {
                newSphere.isSelectable = true;
                newSphere.selectID = Identifiable::genId();
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(newSphere.selectID,sphere));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(newSphere.selectID);
            }
            [ourShapes addObject:newSphere];
        } else if ([shape isKindOfClass:[MaplyShapeCylinder class]])
        {
            MaplyShapeCylinder *cyl = (MaplyShapeCylinder *)shape;
            WhirlyKitCylinder *newCyl = [cyl asWKShape:inDesc];
            
            if (cyl.selectable)
            {
                newCyl.isSelectable = true;
                newCyl.selectID = Identifiable::genId();
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(newCyl.selectID,cyl));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(newCyl.selectID);
            }
            [ourShapes addObject:newCyl];
        } else if ([shape isKindOfClass:[MaplyShapeGreatCircle class]])
        {
            MaplyShapeGreatCircle *gc = (MaplyShapeGreatCircle *)shape;
            WhirlyKitShapeLinear *lin = [[WhirlyKitShapeLinear alloc] init];
            float eps = 0.001;
            if ([inDesc[kMaplySubdivEpsilon] isKindOfClass:[NSNumber class]])
                eps = [inDesc[kMaplySubdivEpsilon] floatValue];
            bool isStatic = [inDesc[kMaplySubdivType] isEqualToString:kMaplySubdivStatic];
            if (isStatic)
                SampleGreatCircleStatic(gc.startPt,gc.endPt,gc.height,lin.pts,visualView.coordAdapter,eps);
            else
                SampleGreatCircle(gc.startPt,gc.endPt,gc.height,lin.pts,visualView.coordAdapter,eps);
            lin.lineWidth = gc.lineWidth;
            if (gc.color)
            {
                lin.useColor = true;
                RGBAColor color = [gc.color asRGBAColor];
                lin.color = color;
            }
            if (gc.selectable)
            {
                lin.isSelectable = true;
                lin.selectID = Identifiable::genId();
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(lin.selectID,gc));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(lin.selectID);
            }
            [specialShapes addObject:lin];
        } else if ([shape isKindOfClass:[MaplyShapeLinear class]])
        {
            MaplyShapeLinear *lin = (MaplyShapeLinear *)shape;
            WhirlyKitShapeLinear *newLin = [[WhirlyKitShapeLinear alloc] init];
            MaplyCoordinate3d *coords = NULL;
            int numCoords = [lin getCoords:&coords];
            for (unsigned int ii=0;ii<numCoords;ii++)
            {
                MaplyCoordinate3d &coord = coords[ii];
                Point3f pt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(coord.x,coord.y)));
                if (coordAdapter->isFlat())
                    pt.z() = coord.z;
                else
                    pt *= (1.0+coord.z);
                newLin.pts.push_back(pt);
            }
            newLin.lineWidth = lin.lineWidth;
            if (lin.color)
            {
                newLin.useColor = true;
                RGBAColor color = [lin.color asRGBAColor];
                newLin.color = color;
            }
            if (lin.selectable)
            {
                newLin.isSelectable = true;
                newLin.selectID = Identifiable::genId();
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(newLin.selectID,lin));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(newLin.selectID);
            }
            [ourShapes addObject:newLin];
        } else if ([shape isKindOfClass:[MaplyShapeExtruded class]])
        {
            MaplyShapeExtruded *ex = (MaplyShapeExtruded *)shape;
            WhirlyKitShapeExtruded *newEx = [ex asWKShape:inDesc];

            if (ex.selectable)
            {
                newEx.isSelectable = true;
                newEx.selectID = Identifiable::genId();
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(newEx.selectID,ex));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(newEx.selectID);
            }
            [ourShapes addObject:newEx];
        }
    }
    
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    if (shapeManager)
    {
        ChangeSet changes;
        if ([ourShapes count] > 0)
        {
            SimpleIdentity shapeID = shapeManager->addShapes(ourShapes, inDesc, changes);
            if (shapeID != EmptyIdentity)
                compObj.shapeIDs.insert(shapeID);
        }
        if ([specialShapes count] > 0)
        {
            // If they haven't overrided the shader already, we need the non-backface one for these objects
            NSMutableDictionary *newDesc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
            if (!newDesc[kMaplyShader])
            {
                SimpleIdentity shaderID = scene->getProgramIDBySceneName(kToolkitDefaultLineNoBackfaceProgram);
                newDesc[kMaplyShader] = @(shaderID);
            }
            SimpleIdentity shapeID = shapeManager->addShapes(specialShapes, newDesc, changes);
            if (shapeID != EmptyIdentity)
                compObj.shapeIDs.insert(shapeID);
        }
        [self flushChanges:changes mode:threadMode];
    }
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

// Add shapes
- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([shapes count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[shapes, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
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
    bool operator ()(const GeomModelInstances *a,const GeomModelInstances *b)
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
    NSMutableDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyModelDrawPriorityDefault) toDict:inDesc];
    
    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:kMaplyShaderDefaultModelTri];
    
    GeometryManager *geomManager = (GeometryManager *)scene->getManager(kWKGeometryManager);

    // Sort the instances with their models
    GeomModelInstancesSet instSort;
    for (MaplyGeomModelInstance *mInst in modelInstances)
    {
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
    }
    
    // Add each model with its group of instances
    if (geomManager)
    {
        ChangeSet changes;
        for (auto it : instSort)
        {
            // Set up the textures and convert the geometry
            MaplyGeomModel *model = it->model;
            
            // Return an existing base model or make a new one
            SimpleIdentity baseModelID = [model getBaseModel:self mode:threadMode];
            
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
                        compObj.selectIDs.insert(thisInst.getId());
                        pthread_mutex_lock(&selectLock);
                        selectObjectSet.insert(SelectObject(thisInst.getId(),modelInst));
                        pthread_mutex_unlock(&selectLock);
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
                
                SimpleIdentity geomID = geomManager->addGeometryInstances(baseModelID, matInst, inDesc, changes);
                if (geomID != EmptyIdentity)
                    compObj.geomIDs.insert(geomID);
            }
        }
        
        [self flushChanges:changes mode:threadMode];
    }
    
    // Clean up the instances we sorted
    for (auto it : instSort)
        delete it;
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

// Called in the layer thread
- (void)addGeometryRun:(NSArray *)argArray
{
    if (isShuttingDown || !layerThread)
        return;

    NSArray *geom = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSMutableDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyModelDrawPriorityDefault) toDict:inDesc];
    
    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];
    
    GeometryManager *geomManager = (GeometryManager *)scene->getManager(kWKGeometryManager);
    
    // Add each raw geometry model
    if (geomManager)
    {
        ChangeSet changes;
        
        for (MaplyGeomModel *model in geom)
        {
            // This is intended to be instanced, but we can use it
            SimpleIdentity geomID = geomManager->addBaseGeometry(model->rawGeom, changes);
            // If we turn it on
            SimpleIDSet geomIDs;
            geomIDs.insert(geomID);
            geomManager->enableGeometry(geomIDs, true, changes);
            
            if (geomID != EmptyIdentity)
                compObj.geomIDs.insert(geomID);
        }
        
        [self flushChanges:changes mode:threadMode];
    }
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

- (MaplyComponentObject *)addModelInstances:(NSArray *)modelInstances desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([modelInstances count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[modelInstances, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
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
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([geom count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[geom, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
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
    if (isShuttingDown || !layerThread)
        return;

    NSArray *stickers = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSMutableDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyStickerDrawPriorityDefault) toDict:inDesc];

    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];

    SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);
    
    for (MaplySticker *sticker in stickers)
    {
        std::vector<SimpleIdentity> texIDs;
        if (sticker.image) {
            if ([sticker.image isKindOfClass:[UIImage class]])
            {
                MaplyTexture *tex = [self addImage:sticker.image imageFormat:sticker.imageFormat mode:threadMode];
                if (tex)
                    texIDs.push_back(tex.texID);
                compObj.textures.insert(tex);
            } else if ([sticker.image isKindOfClass:[MaplyTexture class]])
            {
                MaplyTexture *tex = (MaplyTexture *)sticker.image;
                texIDs.push_back(tex.texID);
                compObj.textures.insert(tex);
            }
        }
        for (UIImage *image in sticker.images)
        {
            if ([image isKindOfClass:[UIImage class]])
            {
                MaplyTexture *tex = [self addImage:image imageFormat:sticker.imageFormat mode:threadMode];
                if (tex)
                    texIDs.push_back(tex.texID);
                compObj.textures.insert(tex);
            } else if ([image isKindOfClass:[MaplyTexture class]])
            {
                MaplyTexture *tex = (MaplyTexture *)image;
                texIDs.push_back(tex.texID);
                compObj.textures.insert(tex);
            }
        }
        WhirlyKitSphericalChunk *chunk = [[WhirlyKitSphericalChunk alloc] init];
        Mbr mbr(Point2f(sticker.ll.x,sticker.ll.y), Point2f(sticker.ur.x,sticker.ur.y));
        chunk.mbr = mbr;
        chunk.texIDs = texIDs;
        chunk.drawOffset = [inDesc[@"drawOffset"] floatValue];
        chunk.drawPriority = [inDesc[@"drawPriority"] floatValue];
        chunk.sampleX = [inDesc[@"sampleX"] intValue];
        chunk.sampleY = [inDesc[@"sampleY"] intValue];
        chunk.programID = [inDesc[kMaplyShader] intValue];
        if (inDesc[kMaplySubdivEpsilon] != nil)
            chunk.eps = [inDesc[kMaplySubdivEpsilon] floatValue];
        if (sticker.coordSys)
            chunk.coordSys = [sticker.coordSys getCoordSystem];
        if (inDesc[kMaplyMinVis] != nil)
            chunk.minVis = [inDesc[kMaplyMinVis] floatValue];
        if (inDesc[kMaplyMaxVis] != nil)
            chunk.maxVis = [inDesc[kMaplyMaxVis] floatValue];
        NSNumber *bufRead = inDesc[kMaplyZBufferRead];
        if (bufRead)
            chunk.readZBuffer = [bufRead boolValue];
        NSNumber *bufWrite = inDesc[kMaplyZBufferWrite];
        if (bufWrite)
            chunk.writeZBuffer = [bufWrite boolValue];
        chunk.rotation = sticker.rotation;
        if (chunkManager)
        {
            ChangeSet changes;
            SimpleIdentity chunkID = chunkManager->addChunk(chunk, false, true, changes);
            if (chunkID != EmptyIdentity)
                compObj.chunkIDs.insert(chunkID);
            [self flushChanges:changes mode:threadMode];
        }
    }
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

// Add stickers
- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([stickers count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[stickers, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
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
    if (isShuttingDown || !layerThread)
        return;

    MaplyComponentObject *stickerObj = [argArray objectAtIndex:0];
    NSDictionary *desc = [argArray objectAtIndex:1];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:2] intValue];
    
    @synchronized(stickerObj)
    {
        bool isHere = false;
        @synchronized(userObjects)
        {
            isHere = [userObjects containsObject:stickerObj];
        }
        
        if (!isHere)
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
                std::set<MaplyTexture *> oldTextures = stickerObj.textures;
                stickerObj.textures.clear();

                // Add in the new images
                for (UIImage *image in newImages)
                {
                    if ([image isKindOfClass:[UIImage class]])
                    {
                        MaplyTexture *tex = [self addImage:image imageFormat:newFormat mode:threadMode];
                        if (tex)
                            newTexIDs.push_back(tex.texID);
                        stickerObj.textures.insert(tex);
                    } else if ([image isKindOfClass:[MaplyTexture class]])
                    {
                        MaplyTexture *tex = (MaplyTexture *)image;
                        newTexIDs.push_back(tex.texID);
                        stickerObj.textures.insert(tex);
                    }
                }
                
                // Clear out the old images
                oldTextures.clear();
                
                ChangeSet changes;
                for (SimpleIDSet::iterator it = stickerObj.chunkIDs.begin();
                     it != stickerObj.chunkIDs.end(); ++it)
                    chunkManager->modifyChunkTextures(*it, newTexIDs, changes);
                [self flushChanges:changes mode:threadMode];
            }
        }
    }
}

// Change stickers
- (void)changeSticker:(MaplyComponentObject *)stickerObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (!stickerObj)
        return;
    
    if (!desc)
        desc = [NSDictionary dictionary];
    NSArray *argArray = @[stickerObj, desc, @(threadMode)];
    
    // If the object is under construction, toss this over to the layer thread
    if (stickerObj.underConstruction)
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
    if (isShuttingDown || !layerThread)
        return;

    NSArray *vectors = [argArray objectAtIndex:0];
    MaplyComponentObject *compObj = [argArray objectAtIndex:1];
    compObj.vectors = vectors;
    NSMutableDictionary *inDesc = [argArray objectAtIndex:2];
    NSString *key = argArray[3];
    if ([key isKindOfClass:[NSNull class]])
        key = nil;
    NSObject<WhirlyKitLoftedPolyCache> *cache = argArray[4];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:5] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyLoftedPolysDrawPriorityDefault) toDict:inDesc];
    
    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];
    
    ShapeSet shapes;
    for (MaplyVectorObject *vecObj in vectors)
        shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());

    ChangeSet changes;
    LoftManager *loftManager = (LoftManager *)scene->getManager(kWKLoftedPolyManager);
    if (loftManager)
    {
        // 10 degress by default
        float gridSize = 10.0 / 180.0 * M_PI;
        if (inDesc[kMaplyLoftedPolyGridSize])
            gridSize = [inDesc[kMaplyLoftedPolyGridSize] floatValue];
        SimpleIdentity loftID = loftManager->addLoftedPolys(&shapes, inDesc, key, cache, gridSize, changes);
        compObj.loftIDs.insert(loftID);
        compObj.isSelectable = false;
    }
    [self flushChanges:changes mode:threadMode];
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
}

// Add lofted polys
- (MaplyComponentObject *)addLoftedPolys:(NSArray *)vectors desc:(NSDictionary *)desc key:(NSString *)key cache:(NSObject<WhirlyKitLoftedPolyCache> *)cache mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([vectors count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[vectors, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], (key ? key : [NSNull null]), (cache ? cache : [NSNull null]), @(threadMode)];
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
    if (isShuttingDown || !layerThread)
        return;

    NSArray *bills = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSMutableDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    CoordSystemDisplayAdapter *coordAdapter = visualView.coordAdapter;
    CoordSystem *coordSys = coordAdapter->getCoordSystem();
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyBillboardDrawPriorityDefault) toDict:inDesc];
    [self applyDefaultName:kMaplyBillboardOrient value:kMaplyBillboardOrientGround toDict:inDesc];

    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:nil];

    // May need a temporary context when setting up label textures
    EAGLContext *tmpContext = [self setupTempContext:threadMode];

    SimpleIdentity billShaderID = [inDesc[kMaplyShader] intValue];
    if (billShaderID == EmptyIdentity)
    {
        if ([inDesc[kMaplyBillboardOrient] isEqualToString:kMaplyBillboardOrientEye])
            billShaderID = scene->getProgramIDBySceneName([kMaplyShaderBillboardEye cStringUsingEncoding:NSASCIIStringEncoding]);
        else
            billShaderID = scene->getProgramIDBySceneName([kMaplyShaderBillboardGround cStringUsingEncoding:NSASCIIStringEncoding]);
    }
    
    ChangeSet changes;
    BillboardManager *billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);
    WhirlyKitFontTextureManager *fontTexManager = scene->getFontTextureManager();
    if (billManager && fontTexManager)
    {
        NSMutableArray *wkBills = [NSMutableArray array];
        for (MaplyBillboard *bill in bills)
        {
            WhirlyKitBillboard *wkBill = [[WhirlyKitBillboard alloc] init];
            Point3d localPt = coordSys->geographicToLocal3d(GeoCoord(bill.center.x,bill.center.y));
            Point3d dispPt = coordAdapter->localToDisplay(Point3d(localPt.x(),localPt.y(),bill.center.z));
            wkBill.center = dispPt;
            wkBill.isSelectable = bill.selectable;
            if (wkBill.isSelectable)
                wkBill.selectID = Identifiable::genId();
            
            if (bill.selectable)
            {
                pthread_mutex_lock(&selectLock);
                selectObjectSet.insert(SelectObject(wkBill.selectID,bill));
                pthread_mutex_unlock(&selectLock);
                compObj.selectIDs.insert(wkBill.selectID);
            }

            MaplyScreenObject *screenObj = bill.screenObj;
            if (!screenObj)
                continue;
            MaplyBoundingBox size = [screenObj getSize];
            Point2d size2d = Point2d(size.ur.x-size.ll.x,size.ur.y-size.ll.y);
            wkBill.size = size2d;

            // Work through the individual polygons in a billboard
            for (const SimplePoly &poly : screenObj->polys)
            {
                SingleBillboardPoly billPoly;
                billPoly.pts = poly.pts;
                billPoly.texCoords = poly.texCoords;
                billPoly.color = poly.color;
                if (bill.vertexAttributes)
                    [self resolveVertexAttrs:billPoly.vertexAttrs from:bill.vertexAttributes];
                if (poly.texture)
                {
                    MaplyTexture *tex = nil;
                    if ([poly.texture isKindOfClass:[UIImage class]])
                    {
                        tex = [self addImage:poly.texture imageFormat:MaplyImageIntRGBA mode:threadMode];
                    } else if ([poly.texture isKindOfClass:[MaplyTexture class]])
                    {
                        tex = (MaplyTexture *)poly.texture;
                    }
                    if (tex)
                    {
                        compObj.textures.insert(tex);
                        billPoly.texId = tex.texID;
                    }
                }
                wkBill.polys.push_back(billPoly);
            }
            
            // Now for the strings
            for (const StringWrapper &strWrap : screenObj->strings)
            {
                // Convert the string to polygons
                DrawableString *drawStr = [fontTexManager addString:strWrap.str changes:changes];
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
                        Point3d newPt = strWrap.mat * Point3d(oldPt.x(),oldPt.y(),1.0);
                        billPoly.pts[ip] = Point2d(newPt.x(),newPt.y());
                    }
                    
                    wkBill.polys.push_back(billPoly);
                }
                
                compObj.drawStringIDs.insert(drawStr->getId());
                delete drawStr;
            }
            
            [wkBills addObject:wkBill];
        }
        
        SimpleIdentity billId = billManager->addBillboards(wkBills, inDesc, billShaderID, changes);
        compObj.billIDs.insert(billId);
        compObj.isSelectable = false;
    }
    [self flushChanges:changes mode:threadMode];
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
    
    [self clearTempContext:tmpContext];
}

// Add billboards
- (MaplyComponentObject *)addBillboards:(NSArray *)bboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    if ([bboards count] == 0)
    {
        @synchronized(userObjects)
        {
            [userObjects addObject:compObj];
            compObj.underConstruction = false;
        }
        return compObj;
    }
    
    NSArray *argArray = @[bboards, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
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
    if (isShuttingDown || !layerThread)
        return;

    MaplyParticleSystem *partSys = argArray[0];
    MaplyComponentObject *compObj = argArray[1];
    NSMutableDictionary *inDesc = argArray[2];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:3] intValue];
    
    [self applyDefaultName:kMaplyDrawPriority value:@(kMaplyParticleSystemDrawPriorityDefault) toDict:inDesc];
    [self applyDefaultName:kMaplyPointSize value:@(kMaplyPointSizeDefault) toDict:inDesc];
    
    // Might be a custom shader on these
    [self resolveShader:inDesc defaultShader:kMaplyShaderParticleSystemPointDefault];
    
    // May need a temporary context
    EAGLContext *tmpContext = [self setupTempContext:threadMode];
    
    SimpleIdentity partSysShaderID = [inDesc[kMaplyShader] intValue];
    if (partSysShaderID == EmptyIdentity)
        partSysShaderID = scene->getProgramIDBySceneName([kMaplyShaderParticleSystemPointDefault cStringUsingEncoding:NSASCIIStringEncoding]);
    if (partSys.shader)
    {
        partSysShaderID = scene->getProgramIDBySceneName([partSys.shader cStringUsingEncoding:NSASCIIStringEncoding]);
    }
    
    ParticleSystemManager *partSysManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);

    ChangeSet changes;
    if (partSysManager)
    {
        ParticleSystem wkPartSys;
        wkPartSys.setId(partSys.ident);
        wkPartSys.drawPriority = [inDesc[kMaplyDrawPriority] intValue];
        wkPartSys.pointSize = [inDesc[kMaplyPointSize] floatValue];
        wkPartSys.name = [partSys.name cStringUsingEncoding:NSASCIIStringEncoding];
        wkPartSys.shaderID = partSysShaderID;
        wkPartSys.lifetime = partSys.lifetime;
        wkPartSys.batchSize = partSys.batchSize;
        wkPartSys.totalParticles = partSys.totalParticles;
        wkPartSys.baseTime = partSys.baseTime;
        wkPartSys.continuousUpdate = partSys.continuousUpdate;
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
            vertAttr.name = [it.name cStringUsingEncoding:NSASCIIStringEncoding];
            wkPartSys.vertAttrs.push_back(vertAttr);
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
            compObj.textures.insert(maplyTex);
        }
        
        SimpleIdentity partSysID = partSysManager->addParticleSystem(wkPartSys, changes);
        partSys.ident = partSysID;
        compObj.partSysIDs.insert(partSysID);
    }
    
    [self flushChanges:changes mode:threadMode];
    
    @synchronized(userObjects)
    {
        [userObjects addObject:compObj];
        compObj.underConstruction = false;
    }
    
    [self clearTempContext:tmpContext];
}

- (MaplyComponentObject *)addParticleSystem:(MaplyParticleSystem *)partSys desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [[MaplyComponentObject alloc] initWithDesc:desc];
    compObj.underConstruction = true;
    
    NSArray *argArray = @[partSys, compObj, [NSMutableDictionary dictionaryWithDictionary:desc], @(threadMode)];
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

- (void)addParticleSystemBatchRun:(NSArray *)argArray
{
    if (isShuttingDown || !layerThread)
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
        // Copy the attributes over in the right order
        for (auto mainAttr : batch.partSys.attrs)
        {
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

// Remove the object, but do it on the layer thread
- (void)removeObjectRun:(NSArray *)argArray
{
    if (isShuttingDown || !layerThread)
        return;
    
    NSArray *inUserObjs = argArray[0];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:1] intValue];
    
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    WideVectorManager *wideVectorManager = (WideVectorManager *)scene->getManager(kWKWideVectorManager);
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);
    LoftManager *loftManager = (LoftManager *)scene->getManager(kWKLoftedPolyManager);
    BillboardManager *billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);
    GeometryManager *geomManager = (GeometryManager *)scene->getManager(kWKGeometryManager);
    WhirlyKitFontTextureManager *fontTexManager = scene->getFontTextureManager();
    ParticleSystemManager *partSysManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);

    ChangeSet changes;
        
    // First, let's make sure we're representing it
    for (MaplyComponentObject *userObj in inUserObjs)
    {
        bool isHere = false;
        @synchronized(userObjects)
        {
            isHere = [userObjects containsObject:userObj];
        }

        if (isHere)
        {
            if (userObj.underConstruction)
                NSLog(@"Deleting an object that's under construction");
            
            @synchronized(userObj)
            {
                // Get rid of the various layer objects
                if (markerManager && !userObj.markerIDs.empty())
                    markerManager->removeMarkers(userObj.markerIDs, changes);
                if (labelManager && !userObj.labelIDs.empty())
                    labelManager->removeLabels(userObj.labelIDs, changes);
                if (vectorManager && !userObj.vectorIDs.empty())
                    vectorManager->removeVectors(userObj.vectorIDs, changes);
                if (wideVectorManager && !userObj.wideVectorIDs.empty())
                    wideVectorManager->removeVectors(userObj.wideVectorIDs, changes);
                if (shapeManager && !userObj.shapeIDs.empty())
                    shapeManager->removeShapes(userObj.shapeIDs, changes);
                if (loftManager && !userObj.loftIDs.empty())
                    loftManager->removeLoftedPolys(userObj.loftIDs, changes);
                if (chunkManager && !userObj.chunkIDs.empty())
                    chunkManager->removeChunks(userObj.chunkIDs, changes);
                if (billManager && !userObj.billIDs.empty())
                    billManager->removeBillboards(userObj.billIDs, changes);
                if (geomManager && !userObj.geomIDs.empty())
                    geomManager->removeGeometry(userObj.geomIDs, changes);
                if (fontTexManager && !userObj.drawStringIDs.empty())
                    for (SimpleIdentity dStrID : userObj.drawStringIDs)
                        [fontTexManager removeString:dStrID changes:changes];
                if (partSysManager && !userObj.partSysIDs.empty())
                {
                    for (SimpleIdentity partSysID : userObj.partSysIDs)
                        partSysManager->removeParticleSystem(partSysID, changes);
                }
                
                // And associated textures
                for (std::set<MaplyTexture *>::iterator it = userObj.textures.begin();
                     it != userObj.textures.end(); ++it)
                    [self removeImageTexture:*it changes:changes];
                userObj.textures.clear();

                // And any references to selection objects
                if (!userObj.selectIDs.empty())
                {
                    pthread_mutex_lock(&selectLock);
                    for (SimpleIDSet::iterator it = userObj.selectIDs.begin();
                         it != userObj.selectIDs.end(); ++it)
                    {
                        SelectObjectSet::iterator sit = selectObjectSet.find(SelectObject(*it));
                        if (sit != selectObjectSet.end())
                            selectObjectSet.erase(sit);
                        else
                            NSLog(@"Tried to delete non-existent selection ID");
                    }
                    pthread_mutex_unlock(&selectLock);
                }
                
            }
            
            @synchronized(userObjects)
            {
                [userObjects removeObject:userObj];
            }
            
//            NSLog(@"Deleted object %lx",(unsigned long)userObj);
        } else {
            NSLog(@"Tried to delete object that doesn't exist");
        }
    }
    
    [self flushChanges:changes mode:threadMode];
}

// Remove a group of objects at once
- (void)removeObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode
{
    NSArray *argArray = @[userObjs, @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj.underConstruction)
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

- (void)enableObjectsRun:(NSArray *)argArray
{
    if (isShuttingDown || !layerThread)
        return;

    NSArray *theObjs = argArray[0];
    bool enable = [argArray[1] boolValue];
    MaplyThreadMode threadMode = (MaplyThreadMode)[[argArray objectAtIndex:2] intValue];

    VectorManager *vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    WideVectorManager *wideVectorManager = (WideVectorManager *)scene->getManager(kWKWideVectorManager);
    MarkerManager *markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);
    BillboardManager *billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);
    LoftManager *loftManager = (LoftManager *)scene->getManager(kWKLoftedPolyManager);
    GeometryManager *geomManager = (GeometryManager *)scene->getManager(kWKGeometryManager);

    ChangeSet changes;
    for (MaplyComponentObject *compObj in theObjs)
    {
        bool isHere = false;
        @synchronized(userObjects)
        {
            isHere = [userObjects containsObject:compObj];
        }

        if (isHere)
        {
            compObj.enable = enable;
            if (vectorManager && !compObj.vectorIDs.empty())
                vectorManager->enableVectors(compObj.vectorIDs, enable, changes);
            if (wideVectorManager && !compObj.wideVectorIDs.empty())
                wideVectorManager->enableVectors(compObj.wideVectorIDs, enable, changes);
            if (markerManager && !compObj.markerIDs.empty())
                markerManager->enableMarkers(compObj.markerIDs, enable, changes);
            if (labelManager && !compObj.labelIDs.empty())
                labelManager->enableLabels(compObj.labelIDs, enable, changes);
            if (shapeManager && !compObj.shapeIDs.empty())
                shapeManager->enableShapes(compObj.shapeIDs, enable, changes);
            if (billManager && !compObj.billIDs.empty())
                billManager->enableBillboards(compObj.billIDs, enable, changes);
            if (loftManager && !compObj.loftIDs.empty())
                loftManager->enableLoftedPolys(compObj.loftIDs, enable, changes);
            if (geomManager && !compObj.geomIDs.empty())
                geomManager->enableGeometry(compObj.geomIDs, enable, changes);
            if (chunkManager && !compObj.chunkIDs.empty())
            {
                for (SimpleIDSet::iterator it = compObj.chunkIDs.begin();
                     it != compObj.chunkIDs.end(); ++it)
                    chunkManager->enableChunk(*it, enable, changes);
            }
        }
    }

    [self flushChanges:changes mode:threadMode];
}

// Enable objects
- (void)enableObjects:(NSArray *)userObjs mode:(MaplyThreadMode)threadMode
{
    NSArray *argArray = @[userObjs, @(true), @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj.underConstruction)
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
    NSArray *argArray = @[userObjs, @(false), @(threadMode)];

    // If any are under construction, we need to toss this over to the layer thread
    if (threadMode == MaplyThreadCurrent)
    {
        bool anyUnderConstruction = false;
        for (MaplyComponentObject *userObj in userObjs)
            if (userObj.underConstruction)
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


- (NSArray *)findVectorsInPoint:(Point2f)pt inView:(MaplyBaseViewController*)vc multi:(bool)multi
{
    NSMutableArray *foundObjs = [NSMutableArray array];
    
    pt = [visualView unwrapCoordinate:pt];
    
    @synchronized(userObjects)
    {
        for (MaplyComponentObject *userObj in userObjects)
        {
            if (userObj.vectors && userObj.isSelectable && userObj.enable)
            {
                for (MaplyVectorObject *vecObj in userObj.vectors)
                {
                    if (vecObj.selectable && userObj.enable)
                    {
                        // Note: Take visibility into account too
                        MaplyCoordinate coord;
                        coord.x = pt.x()-userObj.vectorOffset.x();
                        coord.y = pt.y()-userObj.vectorOffset.y();
                        if ([vecObj pointInAreal:coord])
                        {
                            [foundObjs addObject:vecObj];
                            if (!multi)
                                break;
                        } else if (vc && [vecObj pointNearLinear:coord distance:20 inViewController:vc]) {
                            [foundObjs addObject:vecObj];
                            if (!multi)
                                break;
                        }
                    }
                }
                
                if (!multi && [foundObjs count] > 0)
                    break;
            }
        }
    }

    return foundObjs;
}

- (NSObject *)getSelectableObject:(WhirlyKit::SimpleIdentity)objId
{
    NSObject *ret = nil;
    
    pthread_mutex_lock(&selectLock);
    SelectObjectSet::iterator sit = selectObjectSet.find(SelectObject(objId));
    if (sit != selectObjectSet.end())
        ret = sit->obj;

    pthread_mutex_unlock(&selectLock);
    
    return ret;
}


@end

/*
 *  MaplyQuadLoader.mm
 *
 *  Created by Steve Gifford on 2/12/19.
 *  Copyright 2012-2019 Saildrone Inc
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

#import "QuadTileBuilder.h"
#import "MaplyImageTile_private.h"
#import "MaplyRenderController_private.h"
#import "MaplyQuadSampler_private.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyRenderTarget_private.h"
#import "visual_objects/MaplyScreenLabel.h"
#import "MaplyQuadLoader_private.h"
#import "RawData_NSData.h"

using namespace WhirlyKit;

@implementation MaplyLoaderReturn

- (id)initWithLoader:(MaplyQuadLoaderBase *)loader
{
    self = [super init];
    
    loadReturn = std::make_shared<QuadLoaderReturn>(loader->loader->getGeneration());
    viewC = loader.viewC;
    
    return self;	
}

- (void)setTileID:(MaplyTileID)tileID
{
    loadReturn->ident = QuadTreeIdentifier(tileID.x,tileID.y,tileID.level);
}

- (MaplyTileID)tileID
{
    MaplyTileID tileID;
    tileID.level = loadReturn->ident.level;
    tileID.x = loadReturn->ident.x;  tileID.y = loadReturn->ident.y;
    
    return tileID;
}

- (int)frame
{
    return loadReturn->frame->frameIndex;
}

- (void)addTileData:(id __nonnull) inTileData
{
    tileData.push_back(inTileData);
}

- (NSArray<id> *)getTileData
{
    NSMutableArray *ret = [[NSMutableArray alloc] init];
    for (id data : tileData) {
        if (data)
            [ret addObject:data];
    }
    
    return ret;
}

- (id __nullable)getFirstData
{
    if (tileData.empty())
        return nil;
    
    return tileData[0];
}

- (void)setError:(NSError *)error
{
    _error = error;
    loadReturn->hasError = true;
}

@end

@implementation MaplyQuadLoaderBase

- (instancetype)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)inViewC
{
    self = [super init];
    _flipY = true;
    _viewC = inViewC;
    _numSimultaneousTiles = 8;
    
    return self;
}

- (bool)delayedInit
{
    return true;
}

- (bool)isLoading
{
    // Maybe we're still setting up
    if (!loader)
        return true;
    
    return loader->getLoadingStatus();
}

- (MaplyBoundingBox)geoBoundsForTile:(MaplyTileID)tileID
{
    if (!samplingLayer)
        return kMaplyNullBoundingBox;
    
    MaplyBoundingBox bounds;
    MaplyBoundingBoxD boundsD = [self geoBoundsForTileD:tileID];
    bounds.ll = MaplyCoordinateMake(boundsD.ll.x,boundsD.ll.y);
    bounds.ur = MaplyCoordinateMake(boundsD.ur.x,boundsD.ur.y);
    
    return bounds;
}

- (MaplyBoundingBoxD)geoBoundsForTileD:(MaplyTileID)tileID
{
    if (!samplingLayer)
        return kMaplyNullBoundingBoxD;
    
    MaplyBoundingBoxD bounds;
    QuadDisplayControllerNewRef control = [samplingLayer.quadLayer getController];
    if (!control)
        return bounds;

    MbrD mbrD = control->getQuadTree()->generateMbrForNode(WhirlyKit::QuadTreeNew::Node(tileID.x,tileID.y,tileID.level));
    
    CoordSystem *wkCoordSys = control->getCoordSys();
    Point2d pts[4];
    pts[0] = wkCoordSys->localToGeographicD(Point3d(mbrD.ll().x(),mbrD.ll().y(),0.0));
    pts[1] = wkCoordSys->localToGeographicD(Point3d(mbrD.ur().x(),mbrD.ll().y(),0.0));
    pts[2] = wkCoordSys->localToGeographicD(Point3d(mbrD.ur().x(),mbrD.ur().y(),0.0));
    pts[3] = wkCoordSys->localToGeographicD(Point3d(mbrD.ll().x(),mbrD.ur().y(),0.0));
    Point2d minPt(pts[0].x(),pts[0].y()),  maxPt(pts[0].x(),pts[0].y());
    for (unsigned int ii=1;ii<4;ii++)
    {
        minPt.x() = std::min(minPt.x(),pts[ii].x());
        minPt.y() = std::min(minPt.y(),pts[ii].y());
        maxPt.x() = std::max(maxPt.x(),pts[ii].x());
        maxPt.y() = std::max(maxPt.y(),pts[ii].y());
    }
    bounds.ll = MaplyCoordinateDMake(minPt.x(), minPt.y());
    bounds.ur = MaplyCoordinateDMake(maxPt.x(), maxPt.y());
    
    return bounds;
}

- (MaplyBoundingBox)boundsForTile:(MaplyTileID)tileID
{
    MaplyBoundingBox bounds;
    MaplyBoundingBoxD boundsD;
    
    boundsD = [self boundsForTileD:tileID];
    bounds.ll = MaplyCoordinateMake(boundsD.ll.x, boundsD.ll.y);
    bounds.ur = MaplyCoordinateMake(boundsD.ur.x, boundsD.ur.y);
    
    return bounds;
}

- (MaplyBoundingBoxD)boundsForTileD:(MaplyTileID)tileID
{
     if (!samplingLayer)
        return kMaplyNullBoundingBoxD;
    
    MaplyBoundingBoxD bounds;
    
    
    QuadDisplayControllerNewRef control = [samplingLayer.quadLayer getController];
    if (!control)
        return bounds;
    
    MbrD mbrD = control->getQuadTree()->generateMbrForNode(WhirlyKit::QuadTreeNew::Node(tileID.x,tileID.y,tileID.level));
    bounds.ll = MaplyCoordinateDMake(mbrD.ll().x(), mbrD.ll().y());
    bounds.ur = MaplyCoordinateDMake(mbrD.ur().x(), mbrD.ur().y());
    
    return bounds;
}

- (MaplyCoordinate3d)displayCenterForTile:(MaplyTileID)tileID
{
    QuadDisplayControllerNewRef control = [samplingLayer.quadLayer getController];
    if (!control)
        return MaplyCoordinate3dMake(0.0, 0.0, 0.0);

    Mbr mbr = control->getQuadTree()->generateMbrForNode(WhirlyKit::QuadTreeNew::Node(tileID.x,tileID.y,tileID.level));
    Point2d pt((mbr.ll().x()+mbr.ur().x())/2.0,(mbr.ll().y()+mbr.ur().y())/2.0);
    Scene *scene = control->getScene();
    Point3d locCoord = CoordSystemConvert3d(control->getCoordSys(), scene->getCoordAdapter()->getCoordSystem(), Point3d(pt.x(),pt.y(),0.0));
    Point3d dispCoord = scene->getCoordAdapter()->localToDisplay(locCoord);
    
    return MaplyCoordinate3dMake(dispCoord.x(), dispCoord.y(), dispCoord.z());
}

- (int)getZoomSlot
{
    if (!samplingLayer)
        return -1;
    
    return samplingLayer->sampleControl.getDisplayControl()->getZoomSlot();
}


- (void)setInterpreter:(NSObject<MaplyLoaderInterpreter> *)interp
{
    if (loadInterp) {
        NSLog(@"Caller tried to set loader interpreter after startup in MaplyQuadImageLoader.  Ignoring.");
        return;
    }
    
    loadInterp = interp;
}

- (void)setTileFetcher:(NSObject<MaplyTileFetcher> *)inTileFetcher
{
    tileFetcher = inTileFetcher;
}

- (MaplyLoaderReturn *)makeLoaderReturn
{
    return nil;
}

- (void)changeTileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *)tileInfos
{
    if (!samplingLayer)
        return;
    
    if ([NSThread currentThread] != samplingLayer.layerThread) {
        [self performSelector:@selector(changeTileInfos:) onThread:samplingLayer.layerThread withObject:tileInfos waitUntilDone:false];
        return;
    }

    
    ChangeSet changes;
    loader->setTileInfos(tileInfos);
    loader->reload(NULL,-1,changes);
    [samplingLayer.layerThread addChangeRequests:changes];
}

- (void)changeInterpreter:(NSObject<MaplyLoaderInterpreter> *)interp
{
    if (!samplingLayer)
        return;
    
    if ([NSThread currentThread] != samplingLayer.layerThread) {
        [self performSelector:@selector(changeInterpreter:) onThread:samplingLayer.layerThread withObject:interp waitUntilDone:false];
        return;
    }
    
    ChangeSet changes;
    loadInterp = interp;
    loader->reload(NULL,-1,changes);
    [samplingLayer.layerThread addChangeRequests:changes];
}

- (void)reload
{
    [self reloadAreas:nil];
}

- (void)reloadArea:(MaplyBoundingBox)bounds
{
    [self reloadAreas:@[[NSValue valueWithMaplyBoundingBox:bounds]]];
}

- (void)reloadAreas:(NSArray<NSValue*>*)bounds
{
    if (!samplingLayer)
        return;

    if ([NSThread currentThread] != samplingLayer.layerThread) {
        [self performSelector:@selector(reloadAreas:) onThread:samplingLayer.layerThread withObject:bounds waitUntilDone:false];
        return;
    }

    std::vector<Mbr> boxes;
    const auto count = bounds ? [bounds count] : 0;
    if (count) {
        boxes.reserve(count);
        for (int i = 0; i < count; ++i) {
            const auto v = [bounds[i] maplyBoundingBoxValue];
            boxes.emplace_back(Point2f(v.ll.x,v.ll.y), Point2f(v.ur.x,v.ur.y));
        }
    }
    auto const* boxPtr = boxes.empty() ? nullptr : &boxes[0];
    
    ChangeSet changes;
    loader->reload(nullptr,-1,boxPtr,(int)boxes.size(),changes);
    [samplingLayer.layerThread addChangeRequests:changes];
}

// Called on a random dispatch queue
- (void)fetchRequestSuccess:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame data:(id)data;
{
    if (!loader || !valid)
        return;
    
    if (loader->getDebugMode())
        NSLog(@"MaplyQuadImageLoader: Got fetch back for tile %d: (%d,%d) frame %d",tileID.level,tileID.x,tileID.y,frame);
    
    MaplyLoaderReturn *loadData = nil;
    if ([data isKindOfClass:[MaplyLoaderReturn class]]) {
        loadData = data;
        loadData.tileID = tileID;
        loadData->loadReturn->frame = loader->getFrameInfo(frame);
    } else {
        loadData = [self makeLoaderReturn];
        loadData.tileID = tileID;
        loadData->loadReturn->frame = loader->getFrameInfo(frame);
        if ([data isKindOfClass:[NSData class]]) {
            [loadData addTileData:data];
        } else if (data != nil) {
            NSLog(@"MaplyQuadLader:fetchRequestSuccess: client return unknown data type.  Dropping.");
        }
    }
    
    [self performSelector:@selector(mergeFetchRequest:) onThread:self->samplingLayer.layerThread withObject:loadData waitUntilDone:NO];
}

// Called on SamplingLayer.layerThread
- (void)fetchRequestFail:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame error:(NSError *)error
{
    if (!loader || !valid)
        return;
    // Note: Need to do something more here for single frame cases
    
    NSLog(@"MaplyQuadLoader: Failed to fetch tile %d: (%d,%d) frame %d because:\n%@",tileID.level,tileID.x,tileID.y,frame,[error localizedDescription]);
}

- (void)tileUnloaded:(MaplyTileID)tileID {
    if (!loader || !valid)
        return;

    dispatch_queue_t theQueue = _queue;
    if (!theQueue)
        theQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_async(theQueue, ^{
        [self->loadInterp tileUnloaded:tileID];
    });
}

// If we parsed the data, but need to drop it before it gets merged, we do it here
// TODO: Not doing anything with the change list in loadReturn
//       And this seems to have an ordering problem
- (void)cleanupLoadedData:(MaplyLoaderReturn *)loadReturn
{
//    NSLog(@"MaplyQuadLoader: Cleaning orphaned data.");
    MaplyRenderController *renderC = [_viewC getRenderControl];

    SimpleIDSet compIDs;
    for (auto comp: loadReturn->loadReturn->compObjs)
        compIDs.insert(comp->getId());
    for (auto comp: loadReturn->loadReturn->ovlCompObjs)
        compIDs.insert(comp->getId());
    [renderC removeObjectsByID:compIDs mode:MaplyThreadCurrent];
}

// Called on the SamplingLayer.LayerThread
- (void)mergeFetchRequest:(MaplyLoaderReturn *)loadReturn
{
    if (!loader || !valid)
        return;
    
    // Could do this at startup too
    if (_numSimultaneousTiles > 0 && !serialQueue) {
        serialQueue = dispatch_queue_create("Quad Loader Serial", DISPATCH_QUEUE_SERIAL);
        serialSemaphore = dispatch_semaphore_create(_numSimultaneousTiles);
    }
    
    QuadTreeIdentifier tileID = loadReturn->loadReturn->ident;
    // Don't actually want this one
    if (!loader->isFrameLoading(tileID,loadReturn->loadReturn->frame)) {
        if (_debugMode)
            NSLog(@"MaplyQuadImageLoader: Dropping fetched tile %d: (%d,%d) frame %d",tileID.level,tileID.x,tileID.y,loadReturn->loadReturn->frame->frameIndex);
        return;
    }
    
    // Might be keeping the data coming back per frame
    // If we are, this tells us to merge when all the data has come back
    auto dataWrap = std::make_shared<RawNSDataReader>([loadReturn getFirstData]);
    std::vector<RawDataRef> allData;
    //allData.reserve(?)
    if (loader->mergeLoadedFrame(loadReturn->loadReturn->ident,loadReturn->loadReturn->frame,dataWrap,allData))
    {
        // In this mode we need to adjust the loader return to contain everything at once
        if (loader->getMode() == QuadImageFrameLoader::SingleFrame && loader->getNumFrames() > 1) {
            loadReturn->tileData.clear();
            loadReturn->loadReturn->frame = std::make_shared<QuadFrameInfo>();
            loadReturn->loadReturn->frame->setId(loader->getFrameInfo(0)->getId());
            loadReturn->loadReturn->frame->frameIndex = 0;
            for (const auto &data : allData) {
                if (const auto rawData = dynamic_cast<RawNSDataReader *>(data.get())) {
                    loadReturn->tileData.push_back(rawData->getData());
                }
            }
        }
        
        loader->setLoadReturnRef(tileID,loadReturn->loadReturn->frame,loadReturn->loadReturn);
                
        // Do the parsing on another thread since it can be slow
        dispatch_queue_t theQueue = _queue;
        if (!theQueue)
            theQueue = serialQueue;
        dispatch_semaphore_t theSemaphore = serialSemaphore;

        // Hold on to these till the task runs
        NSObject<MaplyLoaderInterpreter> *theLoadInterp = self->loadInterp;
        MaplyQuadSamplingLayer *samplingLayer = self->samplingLayer;

        dispatch_async(theQueue, ^{
            if (!self->valid || !self->_viewC)
                return;

            if (theSemaphore) {
                // Need to limit the number of simultaneous loader return parses
                dispatch_semaphore_wait(theSemaphore, DISPATCH_TIME_FOREVER);
                
                dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                    // No load interpreter means the fetcher created the objects.  Hopefully.
                    if (theLoadInterp && !loadReturn->loadReturn->cancel)
                        [theLoadInterp dataForTile:loadReturn loader:self];
                    
                    // Need to clean up the loader return objects
                    if ([samplingLayer.layerThread isCancelled]) {
                        [self cleanupLoadedData:loadReturn];
                        return;
                    }

                    [self performSelector:@selector(mergeLoadedTile:) onThread:self->samplingLayer.layerThread withObject:loadReturn waitUntilDone:NO];
                    
                    dispatch_semaphore_signal(theSemaphore);
                });
            } else {
                // Just run it on this queue right here
                
                // No load interpreter means the fetcher created the objects.  Hopefully.
                if (theLoadInterp)
                    [theLoadInterp dataForTile:loadReturn loader:self];
                
                // Need to clean up the loader return objects
                if ([samplingLayer.layerThread isCancelled]) {
                    [self cleanupLoadedData:loadReturn];
                    return;
                }

                [self performSelector:@selector(mergeLoadedTile:) onThread:self->samplingLayer.layerThread withObject:loadReturn waitUntilDone:NO];
            }
        });
    }
}

// Called on the SamplingLayer.LayerThread
- (void)mergeLoadedTile:(MaplyLoaderReturn *)loadReturn
{
    if (!loader || !valid) {
        [self cleanupLoadedData:loadReturn];
        return;
    }
    
    ChangeSet changes;
    if (!loadReturn->loadReturn->changes.empty()) {
        [samplingLayer.layerThread addChangeRequests:loadReturn->loadReturn->changes];
        loadReturn->loadReturn->changes.clear();
    }
    loader->mergeLoadedTile(NULL,loadReturn->loadReturn.get(),changes);

    loader->setLoadReturnRef(loadReturn->loadReturn->ident,loadReturn->loadReturn->frame,NULL);

    [samplingLayer.layerThread addChangeRequests:changes];
}

- (void)cleanup
{
    ChangeSet changes;
    
    loader->cleanup(NULL,changes);
    [samplingLayer.layerThread addChangeRequests:changes];
    [samplingLayer.layerThread flushChangeRequests];

    dispatch_async(dispatch_get_main_queue(), ^{
        [[self.viewC getRenderControl] releaseSamplingLayer:self->samplingLayer forUser:self->loader];
        self->loadInterp = nil;
        self->loader = nil;
        self->serialSemaphore = nil;
        self->serialQueue = nil;
    });
}

- (void)shutdown
{
    valid = false;
    
    if (self->samplingLayer && self->samplingLayer.layerThread)
        [self performSelector:@selector(cleanup) onThread:self->samplingLayer.layerThread withObject:nil waitUntilDone:NO];
}

@end

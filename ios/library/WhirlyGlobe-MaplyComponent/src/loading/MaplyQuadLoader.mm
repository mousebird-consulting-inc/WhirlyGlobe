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
    
    loadReturn = QuadLoaderReturnRef(new QuadLoaderReturn(loader->loader->getGeneration()));
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
    if (!samplingLayer)
        return;
    
    if ([NSThread currentThread] != samplingLayer.layerThread) {
        [self performSelector:@selector(reload) onThread:samplingLayer.layerThread withObject:nil waitUntilDone:false];
        return;
    }

    ChangeSet changes;
    loader->reload(NULL,-1,changes);
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

// Called on the SamplingLayer.LayerThread
- (void)mergeFetchRequest:(MaplyLoaderReturn *)loadReturn
{
    if (!loader || !valid)
        return;
    
    QuadTreeIdentifier tileID = loadReturn->loadReturn->ident;
    // Don't actually want this one
    if (!loader->isFrameLoading(tileID,loadReturn->loadReturn->frame)) {
        if (_debugMode)
            NSLog(@"MaplyQuadImageLoader: Dropping fetched tile %d: (%d,%d) frame %d",tileID.level,tileID.x,tileID.y,loadReturn->loadReturn->frame->frameIndex);
        return;
    }
    
    // Might be keeping the data coming back per frame
    // If we are, this tells us to merge when all the data has come back
    RawDataRef dataWrap(new RawNSDataReader([loadReturn getFirstData]));
    std::vector<RawDataRef> allData;
    if (loader->mergeLoadedFrame(loadReturn->loadReturn->ident,loadReturn->loadReturn->frame,dataWrap,allData))
    {
        // In this mode we need to adjust the loader return to contain everything at once
        if (loader->getMode() == QuadImageFrameLoader::SingleFrame && loader->getNumFrames() > 1) {
            loadReturn->tileData.clear();
            loadReturn->loadReturn->frame = QuadFrameInfoRef(new QuadFrameInfo());
            loadReturn->loadReturn->frame->setId(loader->getFrameInfo(0)->getId());
            loadReturn->loadReturn->frame->frameIndex = 0;
            for (auto data : allData) {
                RawNSDataReader *rawData = dynamic_cast<RawNSDataReader *>(data.get());
                if (rawData) {
                    loadReturn->tileData.push_back(rawData->getData());
                }
            }
        }
        
        // Do the parsing on another thread since it can be slow
        dispatch_queue_t theQueue = _queue;
        if (!theQueue)
            theQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_async(theQueue, ^{
            // No load interpreter means the fetcher created the objects.  Hopefully.
            if (self->loadInterp)
                [self->loadInterp dataForTile:loadReturn loader:self];
            
            [self performSelector:@selector(mergeLoadedTile:) onThread:self->samplingLayer.layerThread withObject:loadReturn waitUntilDone:NO];
        });
    }
}

// Called on the SamplingLayer.LayerThread
- (void)mergeLoadedTile:(MaplyLoaderReturn *)loadReturn
{
    if (!loader || !valid)
        return;
    
    ChangeSet changes;
    if (!loadReturn->loadReturn->changes.empty()) {
        [samplingLayer.layerThread addChangeRequests:loadReturn->loadReturn->changes];
        loadReturn->loadReturn->changes.clear();
    }
    loader->mergeLoadedTile(NULL,loadReturn->loadReturn.get(),changes);
    
    [samplingLayer.layerThread addChangeRequests:changes];
}

- (void)cleanup
{
    ChangeSet changes;
    
    loader->cleanup(NULL,changes);
    [samplingLayer.layerThread addChangeRequests:changes];
    
    loader = nil;
}

- (void)shutdown
{
    valid = false;
    
    if (self->samplingLayer && self->samplingLayer.layerThread)
        [self performSelector:@selector(cleanup) onThread:self->samplingLayer.layerThread withObject:nil waitUntilDone:NO];
    
    [[self.viewC getRenderControl] releaseSamplingLayer:samplingLayer forUser:loader];
}

@end

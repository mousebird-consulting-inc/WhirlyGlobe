/*
 *  MaplyQuadImageFrameLoader.mm
 *
 *  Created by Steve Gifford on 9/13/18.
 *  Copyright 2012-2018 mousebird consulting inc
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

#import "MaplyQuadImageFrameLoader.h"
#import "MaplyQuadImageLoader_private.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyShader_private.h"
#import "MaplyRenderTarget_private.h"
#import "MaplyScreenLabel.h"
#import "QuadImageFrameLoader_iOS.h"

using namespace WhirlyKit;

@implementation MaplyImageLoaderInterpreter

- (void)dataForTile:(MaplyImageLoaderReturn * __nonnull)loadReturn
{
    NSArray *tileDatas = [loadReturn getTileData];
    
    for (NSData *tileData in tileDatas) {
        MaplyImageTile *imageTile = [[MaplyImageTile alloc] initWithPNGorJPEGData:tileData];
        [loadReturn addImageTile:imageTile];
    }
}

@end

@implementation MaplyOvlDebugImageLoaderInterpreter
{
    MaplyBaseViewController * __weak viewC;
    MaplyQuadImageLoaderBase * __weak loader;
    UIFont *font;
}

- (id)initWithLoader:(MaplyQuadImageLoaderBase *)inLoader viewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    loader = inLoader;
    viewC = inViewC;
    font = [UIFont systemFontOfSize:12.0];
    
    return self;
}

- (void)dataForTile:(MaplyImageLoaderReturn * __nonnull)loadReturn
{
    [super dataForTile:loadReturn];
    
    MaplyBoundingBox bbox = [loader geoBoundsForTile:loadReturn.tileID];
    MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
    MaplyCoordinate center;
    center.x = (bbox.ll.x+bbox.ur.x)/2.0;  center.y = (bbox.ll.y+bbox.ur.y)/2.0;
    label.loc = center;
    label.text = [NSString stringWithFormat:@"%d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y];
    label.layoutImportance = MAXFLOAT;
    
    MaplyComponentObject *labelObj = [viewC addScreenLabels:@[label] desc:
                                      @{kMaplyFont: font,
                                        kMaplyTextColor: UIColor.blackColor,
                                        kMaplyTextOutlineColor: UIColor.whiteColor,
                                        kMaplyTextOutlineSize: @(2.0)
                                        }
                                                       mode:MaplyThreadCurrent];
    
    MaplyCoordinate coords[5];
    coords[0] = bbox.ll;  coords[1] = MaplyCoordinateMake(bbox.ur.x, bbox.ll.y);
    coords[2] = bbox.ur;  coords[3] = MaplyCoordinateMake(bbox.ll.x, bbox.ur.y);
    coords[4] = coords[0];
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:5 attributes:nil];
    [vecObj subdivideToGlobe:0.001];
    MaplyComponentObject *outlineObj = [viewC addVectors:@[vecObj] desc:nil mode:MaplyThreadCurrent];
    
    [loadReturn addCompObjs:@[labelObj,outlineObj]];
}

@end

@implementation MaplyDebugImageLoaderInterpreter
{
    MaplyBaseViewController * __weak viewC;
}

- (instancetype)initWithLoader:(MaplyQuadImageLoaderBase *)inLoader viewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    
    viewC = inViewC;
    
    return self;
}

static const int MaxDebugColors = 10;
static const int debugColors[MaxDebugColors] = {0x86812D, 0x5EB9C9, 0x2A7E3E, 0x4F256F, 0xD89CDE, 0x773B28, 0x333D99, 0x862D52, 0xC2C653, 0xB8583D};

- (void)dataForTile:(MaplyImageLoaderReturn *)loadReturn
{
    MaplyTileID tileID = loadReturn.tileID;
    
    CGSize size;  size = CGSizeMake(256,256);
    UIGraphicsBeginImageContext(size);
    
    // Draw into the image context
    int hexColor = debugColors[loadReturn.tileID.level % MaxDebugColors];
    float red = (((hexColor) >> 16) & 0xFF)/255.0;
    float green = (((hexColor) >> 8) & 0xFF)/255.0;
    float blue = (((hexColor) >> 0) & 0xFF)/255.0;
    UIColor *backColor = nil;
    UIColor *fillColor = [UIColor whiteColor];
    backColor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.7];
    fillColor = [UIColor colorWithRed:red green:green blue:blue alpha:0.7];
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    // Draw a rectangle around the edges for testing
    [backColor setFill];
    CGContextFillRect(ctx, CGRectMake(0, 0, size.width, size.height));
    [fillColor setStroke];
    CGContextStrokeRect(ctx, CGRectMake(0, 0, size.width-1, size.height-1));
    
    [fillColor setStroke];
    [fillColor setFill];
    CGContextSetTextDrawingMode(ctx, kCGTextFill);
    NSString *textStr = nil;
    if (loadReturn.frame == -1) {
        textStr = [NSString stringWithFormat:@"%d: (%d,%d)",tileID.level,tileID.x,tileID.y];
    }
    else
        textStr = [NSString stringWithFormat:@"%d: (%d,%d); %d",tileID.level,tileID.x,tileID.y,loadReturn.frame];
    [[UIColor whiteColor] setStroke];
    [[UIColor whiteColor] setFill];
    [textStr drawInRect:CGRectMake(0,0,size.width,size.height) withAttributes:@{NSFontAttributeName:[UIFont systemFontOfSize:24.0]}];
    
    // Grab the image and shut things down
    UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    [loadReturn addImage:retImage];
}

@end

// Note: Put the animator back

//@implementation MaplyQuadImageFrameAnimator
//{
//    MaplyBaseViewController * __weak viewC;
//    MaplyQuadImageFrameLoader * __weak loader;
//    TimeInterval startTime;
//    int numFrames;
//}
//
//- (instancetype)initWithFrameLoader:(MaplyQuadImageFrameLoader *)inLoader viewC:(MaplyBaseViewController * __nonnull)inViewC
//{
//    self = [super init];
//    loader = inLoader;
//    viewC = inViewC;
//    startTime = TimeGetCurrent();
//    _period = 10.0;
//    _pauseLength = 0.0;
//    numFrames = [loader getNumFrames];
//
//    [viewC addActiveObject:self];
//
//    return self;
//}
//
//- (void)shutdown
//{
//    [viewC removeActiveObject:self];
//    loader = nil;
//    viewC = nil;
//}
//
//// MARK: ActiveObject overrides
//
//// Have to do the position update in the setCurrentImage so we're not messing with the rendering loop
//- (bool)hasUpdate
//{
//    if (!viewC || !loader)
//        return false;
//
//    TimeInterval now = TimeGetCurrent();
//    TimeInterval totalPeriod = _period + _pauseLength;
//    double when = fmod(now-startTime,totalPeriod);
//    if (when >= _period)
//        // Snap it to the end for a while
//        [loader setCurrentImage:numFrames-1];
//    else {
//        double where = when/_period * (numFrames-1);
//        [loader setCurrentImage:where];
//    }
//
//    return false;
//}
//
//- (void)updateForFrame:(void *)frameInfo
//{
//}
//
//- (void)teardown
//{
//    loader = nil;
//}
//
//@end
//
//// An active updater called every frame the by the renderer
//// We use this to process rendering state from the layer thread
//@interface MaplyQuadImageFrameLoaderUpdater : MaplyActiveObject
//
//@property (nonatomic,weak) MaplyQuadImageFrameLoader * __weak loader;
//
//@end
//
//@implementation MaplyQuadImageFrameLoaderUpdater
//
//- (bool)hasUpdate
//{
//    return [_loader hasUpdate];
//}
//
//- (void)updateForFrame:(void *)frameInfo
//{
//    return [_loader updateForFrame:(RendererFrameInfo *)frameInfo];
//}
//
//- (void)teardown
//{
//    _loader = nil;
//}
//
//@end

@implementation MaplyQuadImageFrameLoader
{
    bool valid;
    
    // Used for initialization and then not
    // Also need to hold on to the CoordinateSystem lest it disappear
    MaplySamplingParams *params;
    
    // What part of the animation we're displaying
    double curFrame;
    
    // Active updater used to update rendering state
    // Note: Put this back
//    MaplyQuadImageFrameLoaderUpdater *updater;
    
    QuadImageFrameLoader_iosRef loader;
}

- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)params tileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *__nonnull)frameInfos viewC:(MaplyBaseViewController * __nonnull)inViewC
{
    if (!params.singleLevel) {
        NSLog(@"MaplyQuadImageFrameLoader only supports samplers with singleLevel set to true");
        return nil;
    }

    // Loader does all the work.  The Obj-C version is just a wrapper
    loader = QuadImageFrameLoader_iosRef(new QuadImageFrameLoader_ios(params->params,frameInfos));

    self->viewC = inViewC;
    
    self.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault;
    self.drawPriorityPerLevel = 100;
    
    self = [super init];
    
    self.flipY = true;
    self.debugMode = false;
    self->minLevel = 10000;
    self->maxLevel = -1;
    for (MaplyRemoteTileInfoNew *frameInfo in frameInfos) {
        self->minLevel = std::min(self->minLevel,frameInfo.minZoom);
        self->maxLevel = std::max(self->maxLevel,frameInfo.maxZoom);
    }
    self.imageFormat = MaplyImageIntRGBA;
    self.borderTexel = 0;
    self.color = [UIColor whiteColor];
    valid = true;
    
    MaplyQuadImageFrameLoader * __weak weakSelf = self;
    
    // Start things out after a delay
    // This lets the caller mess with settings
    dispatch_async(dispatch_get_main_queue(), ^{
        [weakSelf delayedInit];
    });
    
    return self;
}

- (void)delayedInit
{
    if (!valid)
        return;
    
    if (!viewC || !viewC->renderControl || !viewC->renderControl->scene)
        return;
    
    if (!tileFetcher) {
        tileFetcher = [viewC addTileFetcher:MaplyQuadImageLoaderFetcherName];
    }
    if (!loadInterp) {
        loadInterp = [[MaplyImageLoaderInterpreter alloc] init];
    }
    
    samplingLayer = [viewC findSamplingLayer:params forUser:self->loader];
    // Do this again in case they changed them
    loader->setSamplingParams(params->params);
    
    // Sort out the texture format
    switch (self.imageFormat) {
        case MaplyImageIntRGBA:
        case MaplyImage4Layer8Bit:
        default:
            loader->setTexType(GL_UNSIGNED_BYTE);
            break;
        case MaplyImageUShort565:
            loader->setTexType(GL_UNSIGNED_SHORT_5_6_5);
            break;
        case MaplyImageUShort4444:
            loader->setTexType(GL_UNSIGNED_SHORT_4_4_4_4);
            break;
        case MaplyImageUShort5551:
            loader->setTexType(GL_UNSIGNED_SHORT_5_5_5_1);
            break;
        case MaplyImageUByteRed:
        case MaplyImageUByteGreen:
        case MaplyImageUByteBlue:
        case MaplyImageUByteAlpha:
        case MaplyImageUByteRGB:
            loader->setTexType(GL_ALPHA);
            break;
    }
    
    if (loader->getShaderID() == EmptyIdentity) {
        MaplyShader *theShader = [viewC getShaderByName:kMaplyShaderDefaultTriMultiTex];
        if (theShader)
            loader->setShaderID([theShader getShaderID]);
    }
}

- (void)setShader:(MaplyShader *)shader
{
    if (!loader)
        return;
    
    loader->setShaderID([shader getShaderID]);
}

- (void)setCurrentImage:(double)where
{
    curFrame = std::min(std::max(where,0.0),(double)([loader->frameInfos count]-1));
}

- (int)getNumFrames
{
    return [loader->frameInfos count];
}

// Called on a random dispatch queue
- (void)fetchRequestSuccess:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame data:(NSData *)data;
{
    if (loader->getDebugMode())
        NSLog(@"MaplyQuadImageLoader: Got fetch back for tile %d: (%d,%d) frame %d",tileID.level,tileID.x,tileID.y,frame);
    
    // Ask the interpreter to parse it
    MaplyLoaderReturn *loadData = [[MaplyLoaderReturn alloc] init];
    loadData.tileID = tileID;
    loadData.frame = frame;
    [loadData addTileData:data];
    
    [self performSelector:@selector(mergeFetchRequest:) onThread:self->samplingLayer.layerThread withObject:loadData waitUntilDone:NO];
}

 // Called on SamplingLayer.layerThread
 - (void)fetchRequestFail:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame error:(NSError *)error
 {
     // Note: Need to do something more here for single frame cases
     
     NSLog(@"MaplyQuadImageLoader: Failed to fetch tile %d: (%d,%d) frame %d because:\n%@",tileID.level,tileID.x,tileID.y,frame,[error localizedDescription]);
 }

 // Called on the SamplingLayer.LayerThread
 - (void)mergeFetchRequest:(MaplyLoaderReturn *)loadReturn
 {
     if (!loader)
         return;
     
     // Don't actually want this one
     if (!loader->isFrameLoading(loadReturn->loadReturn->ident,loadReturn->loadReturn->frame))
         return;
     
     // Do the parsing on another thread since it can be slow
     dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
         [self->loadInterp dataForTile:loadReturn];
         
         [self performSelector:@selector(mergeLoadedTile:) onThread:self->samplingLayer.layerThread withObject:loadReturn waitUntilDone:NO];
     });
 }


- (void)cleanup
{
    ChangeSet changes;

    loader->cleanup(changes);
    [layer.layerThread addChangeRequests:changes];

    loader = nil;
}

- (void)shutdown
{
    ChangeSet changes;
    
    valid = false;
    
    if (self->samplingLayer && self->samplingLayer.layerThread)
        [self performSelector:@selector(cleanup) onThread:self->samplingLayer.layerThread withObject:nil waitUntilDone:NO];
    
    // Note: Put this back
//    [viewC removeActiveObject:updater];
    [viewC releaseSamplingLayer:samplingLayer forUser:loader];
    
//    updater = nil;
}

@end

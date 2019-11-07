/*
*  MaplyGlobeRenderController.mm
*  WhirlyGlobeComponent
*
*  Created by Steve Gifford on 10/23/10.
*  Copyright 2011-2019 mousebird consulting
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

#import "MaplyGlobeRenderController.h"
#import "MaplyRenderController_private.h"
#import "MaplyRenderTarget_private.h"
#import "MaplyGlobeRenderController_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeViewControllerAnimationState

- (instancetype)init
{
    self = [super init];
    _heading = DBL_MAX;
    _height = 1.0;
    _tilt = DBL_MAX;
    _roll = 0.0;
    _pos.x = _pos.y = 0.0;
    _screenPos = {-1,-1};
    _globeCenter = {-1000,-1000};
    
    return self;
}

+ (WhirlyGlobeViewControllerAnimationState *)Interpolate:(double)t from:(WhirlyGlobeViewControllerAnimationState *)stateA to:(WhirlyGlobeViewControllerAnimationState *)stateB
{
    WhirlyGlobeViewControllerAnimationState *newState = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    
    newState.heading = (stateB.heading-stateA.heading)*t + stateA.heading;
    newState.height = (stateB.height-stateA.height)*t + stateA.height;
    newState.tilt = (stateB.tilt-stateA.tilt)*t + stateA.tilt;
    newState.pos = MaplyCoordinateDMake((stateB.pos.x-stateA.pos.x)*t + stateA.pos.x,(stateB.pos.y-stateA.pos.y)*t + stateA.pos.y);
    newState.roll = (stateB.roll-stateA.roll)*t + stateA.roll;
    if (stateA.screenPos.x >= 0.0 && stateA.screenPos.y >= 0.0 &&
        stateB.screenPos.x >= 0.0 && stateB.screenPos.y >= 0.0)
    {
        newState.screenPos = CGPointMake((stateB.screenPos.x - stateA.screenPos.x)*t + stateA.screenPos.x,
                                         (stateB.screenPos.y - stateA.screenPos.y)*t + stateA.screenPos.y);
    } else
        newState.screenPos = stateB.screenPos;
    if (stateA.globeCenter.x != -1000 && stateB.globeCenter.x != -1000) {
        newState.globeCenter = CGPointMake((stateB.globeCenter.x - stateA.globeCenter.x)*t + stateA.globeCenter.x,
                                           (stateB.globeCenter.y - stateA.globeCenter.y)*t + stateA.globeCenter.y);
    }
    
    return newState;
}

@end

// Target for screen snapshot
@interface SnapshotTargetGlobe : NSObject<WhirlyKitSnapshot>
@property (nonatomic,weak) WhirlyGlobeRenderController *viewC;
@property (nonatomic) NSData *data;
@property (nonatomic) SimpleIdentity renderTargetID;
@property (nonatomic) CGRect subsetRect;
//@property (nonatomic) NSObject<MaplySnapshotDelegate> *outsideDelegate;
@end

@implementation SnapshotTargetGlobe

- (instancetype)initWithViewC:(WhirlyGlobeRenderController *)inViewC
{
    self = [super init];
    
    _viewC = inViewC;
    _data = nil;
    _renderTargetID = EmptyIdentity;
    _subsetRect = CGRectZero;
    
    return self;
}

- (void)setSubsetRect:(CGRect)subsetRect
{
    _subsetRect = subsetRect;
}

- (CGRect)snapshotRect
{
    return _subsetRect;
}

- (void)snapshotData:(NSData *)snapshotData {
    _data = snapshotData;
}

- (bool)needSnapshot:(NSTimeInterval)now {
    return true;
}

- (SimpleIdentity)renderTargetID
{
    return _renderTargetID;
}

- (UIImage *)asImage:(MaplyRenderController *)renderControl
{
    if (!_data)
        return nil;
    
    // Courtesy: https://developer.apple.com/library/ios/qa/qa1704/_index.html
    // Create a CGImage with the pixel data
    // If your OpenGL ES content is opaque, use kCGImageAlphaNoneSkipLast to ignore the alpha channel
    // otherwise, use kCGImageAlphaPremultipliedLast
    CGDataProviderRef ref = CGDataProviderCreateWithData(NULL, [_data bytes], [_data length], NULL);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    int framebufferWidth = renderControl->sceneRenderer->framebufferWidth;
    int framebufferHeight = renderControl->sceneRenderer->framebufferHeight;
    CGImageRef iref = CGImageCreate(framebufferWidth, framebufferHeight, 8, 32, framebufferWidth * 4, colorspace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast,
                                    ref, NULL, true, kCGRenderingIntentDefault);
    
    // OpenGL ES measures data in PIXELS
    // Create a graphics context with the target size measured in POINTS
    NSInteger widthInPoints, heightInPoints;
    {
        // On iOS 4 and later, use UIGraphicsBeginImageContextWithOptions to take the scale into consideration
        // Set the scale parameter to your OpenGL ES view's contentScaleFactor
        // so that you get a high-resolution snapshot when its value is greater than 1.0
        CGFloat scale = 1.0;
        widthInPoints = framebufferWidth / scale;
        heightInPoints = framebufferHeight / scale;
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(widthInPoints, heightInPoints), NO, scale);
    }
    
    CGContextRef cgcontext = UIGraphicsGetCurrentContext();
    
    // UIKit coordinate system is upside down to GL/Quartz coordinate system
    // Flip the CGImage by rendering it to the flipped bitmap context
    // The size of the destination area is measured in POINTS
    CGContextSetBlendMode(cgcontext, kCGBlendModeCopy);
    CGContextDrawImage(cgcontext, CGRectMake(0.0, 0.0, widthInPoints, heightInPoints), iref);
    
    // Retrieve the UIImage from the current context
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();
    
    // Clean up
    CFRelease(ref);
    CFRelease(colorspace);
    CGImageRelease(iref);

    return image;
}

@end


@implementation WhirlyGlobeRenderController

- (instancetype __nullable)initWithSize:(CGSize)size
{
    return [self initWithSize:size mode:MaplyRenderMetal];
}

- (instancetype) initWithSize:(CGSize)screenSize mode:(MaplyRenderType)renderType
{
    globeView = GlobeView_iOSRef(new GlobeView_iOS());
    globeView->continuousZoom = true;
    visualView = globeView;
    coordAdapter = globeView->coordAdapter;
    
    self = [super initWithSize:screenSize mode:renderType];
    [self resetLights];

    return self;
}

- (void)loadSetup
{
    [super loadSetup];
    
    if (renderType == SceneRenderer::RenderMetal) {
        SceneRendererMTLRef sceneRendererMTL = SceneRendererMTLRef(new SceneRendererMTL(MTLCreateSystemDefaultDevice(),1.0));
        // By default we're assuming offscreen renderers are dumb splats, but we're not doing that here
        sceneRendererMTL->offscreenBlendEnable = true;
    }
}

- (NSTimeInterval)currentTime
{
    return scene->getCurrentTime();
}

- (void)setCurrentTime:(NSTimeInterval)currentTime
{
    scene->setCurrentTime(currentTime);
}

- (void)setViewState:(WhirlyGlobeViewControllerAnimationState *)animState
{
    Vector3d startLoc(0,0,1);
    
    if (animState.screenPos.x >= 0.0 && animState.screenPos.y >= 0.0)
    {
        Matrix4d heightTrans = Eigen::Affine3d(Eigen::Translation3d(0,0,-globeView->calcEarthZOffset())).matrix();
        Point3d hit;
        if (globeView->pointOnSphereFromScreen(Point2f(animState.screenPos.x,animState.screenPos.y), heightTrans, sceneRenderer->getFramebufferSizeScaled(), hit, true))
        {
            startLoc = hit;
        }
    }
    
    // Start with a rotation from the clean start state to the location
    Point3d worldLoc = globeView->coordAdapter->localToDisplay(globeView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(animState.pos.x,animState.pos.y)));
    Eigen::Quaterniond posRot = QuatFromTwoVectors(worldLoc, startLoc);
    
    // Orient with north up.  Either because we want that or we're about do do a heading
    Eigen::Quaterniond posRotNorth = posRot;
    if (_keepNorthUp || animState.heading < MAXFLOAT)
    {
        // We'd like to keep the north pole pointed up
        // So we look at where the north pole is going
        Vector3d northPole = (posRot * Vector3d(0,0,1)).normalized();
        if (northPole.y() != 0.0)
        {
            // Then rotate it back on to the YZ axis
            // This will keep it upward
            float ang = atan(northPole.x()/northPole.y());
            // However, the pole might be down now
            // If so, rotate it back up
            if (northPole.y() < 0.0)
                ang += M_PI;
            Eigen::AngleAxisd upRot(ang,worldLoc);
            posRotNorth = posRot * upRot;
        }
    }
    
    // We can't have both northUp and a heading
    Eigen::Quaterniond finalQuat = posRotNorth;
    if (!_keepNorthUp && animState.heading < MAXFLOAT)
    {
        Eigen::AngleAxisd headingRot(animState.heading,worldLoc);
        finalQuat = posRotNorth * headingRot;
    }
    
    // Set the height (easy)
    globeView->setHeightAboveGlobe(animState.height,false);
    
    // Set the tilt either directly or as a consequence of the height
    if (animState.tilt < MAXFLOAT)
        globeView->setTilt(animState.tilt);
    globeView->setRoll(animState.roll, false);

    globeView->setRotQuat(finalQuat, true);
}

- (double)getHeading
{
    double retHeading = 0.0;

    // Figure out where the north pole went
    Vector3d northPole = (globeView->getRotQuat() * Vector3d(0,0,1)).normalized();
    if (northPole.y() != 0.0)
        retHeading = atan2(-northPole.x(),northPole.y());
    
    return retHeading;
}

- (MaplyCoordinate)getPosition
{
    GeoCoord geoCoord = globeView->coordAdapter->getCoordSystem()->localToGeographic(globeView->coordAdapter->displayToLocal(globeView->currentUp()));

    return {.x = geoCoord.lon(), .y = geoCoord.lat()};
}

- (double)getHeight
{
    if (!globeView)
        return 0.0;
    
    return globeView->getHeightAboveGlobe();
}

- (void)getPositionD:(MaplyCoordinateD *)pos height:(double *)height
{
    *height = globeView->getHeightAboveGlobe();
    Point3d localPt = globeView->currentUp();
    Point2d geoCoord = globeView->coordAdapter->getCoordSystem()->localToGeographicD(globeView->coordAdapter->displayToLocal(localPt));
    pos->x = geoCoord.x();  pos->y = geoCoord.y();
}

- (WhirlyGlobeViewControllerAnimationState *)getViewState
{
    // Figure out the current state
    WhirlyGlobeViewControllerAnimationState *state = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    state.heading = [self getHeading];
    state.tilt = globeView->getTilt();
    state.roll = globeView->getRoll();
    MaplyCoordinateD pos;
    double height;
    [self getPositionD:&pos height:&height];
    state.pos = pos;
    state.height = height;
//    state.globeCenter = [self globeCenter];
    
    return state;
}

- (SnapshotTargetGlobe *)snapshotTarget
{
    if ([NSThread currentThread] != mainThread) {
        NSLog(@"Can only render on the thread this renderControl was created on");
        return nil;
    }
    
    SnapshotTargetGlobe *target = [[SnapshotTargetGlobe alloc] initWithViewC:self];

    switch ([self getRenderType])
    {
        case MaplyRenderGLES:
        {
            SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(sceneRenderer);
            sceneRenderGLES->addSnapshotDelegate(target);
            
            sceneRenderGLES->forceDrawNextFrame();
            sceneRenderGLES->render(1/60.0);  // TODO: Set this value for reals
            
            sceneRenderGLES->removeSnapshotDelegate(target);
        }
            break;
        case MaplyRenderMetal:
        {
            SceneRendererMTLRef sceneRenderMTL = std::dynamic_pointer_cast<SceneRendererMTL>(sceneRenderer);
            sceneRenderMTL->addSnapshotDelegate(target);
            
            sceneRenderMTL->forceDrawNextFrame();
            sceneRenderMTL->render(1/60.0,nil,nil);  // TODO: Set this value for reals
            
            sceneRenderMTL->removeSnapshotDelegate(target);
        }
            break;
        default:
            break;
    }
    
    return target;
}

- (UIImage *__nullable)snapshot
{
    SnapshotTargetGlobe *target = [self snapshotTarget];
 
    UIImage *image = [target asImage:self];
    target.data = nil;
    
    return image;
}

- (NSData *__nullable)snapshotData
{
   SnapshotTargetGlobe *target = [self snapshotTarget];

   return target.data;
}

@end

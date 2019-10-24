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

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

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
{
    WhirlyGlobe::GlobeView_iOSRef globeView;
}

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

- (NSTimeInterval)currentTime
{
    return scene->getCurrentTime();
}

- (void)setCurrentTime:(NSTimeInterval)currentTime
{
    scene->setCurrentTime(currentTime);
}

- (void)setViewState:(WhirlyGlobeViewControllerAnimationState *__nonnull)viewState
{
    // TODO: Fill this in
}

- (WhirlyGlobeViewControllerAnimationState *)getViewState
{
    // TODO: Fill this in
    return nil;
}

- (UIImage *__nullable)snapshot
{
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
    
 
    return [target asImage:self];
}

@end

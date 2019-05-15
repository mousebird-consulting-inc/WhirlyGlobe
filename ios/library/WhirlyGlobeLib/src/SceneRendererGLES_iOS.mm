/*
 *  SceneRenderereES_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/28/19.
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

#import <UIKit/UIKit.h>
#import "SceneRendererGLES_iOS.h"

namespace WhirlyKit {
    
SceneRendererGLES_iOS::SceneRendererGLES_iOS()
    : layer(nil), context(nil), snapshotDelegate(nil)
{
    int version = kEAGLRenderingAPIOpenGLES3;
    
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!context) {
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        version = kEAGLRenderingAPIOpenGLES2;
    }
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    useContext();
    
    setup(version,0,0);
    
    [EAGLContext setCurrentContext:oldContext];
}
    
SceneRendererGLES_iOS::SceneRendererGLES_iOS(int width,int height)
{
    int version = kEAGLRenderingAPIOpenGLES3;
    
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!context) {
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        version = kEAGLRenderingAPIOpenGLES2;
    }
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    useContext();
    
    setup(version,width,height);
    
    [EAGLContext setCurrentContext:oldContext];
}
    
SceneRendererGLES_iOS::~SceneRendererGLES_iOS()
{
    EAGLContext *oldContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:context];

    // Have to clear these out here because we can set the context
    for (RenderTargetRef target : renderTargets)
        target->clear();
    
    renderTargets.clear();

    [EAGLContext setCurrentContext:oldContext];
}
    
EAGLContext *SceneRendererGLES_iOS::getContext()
{
    return context;
}

void SceneRendererGLES_iOS::useContext()
{
    if (!context)
        return;
    [EAGLContext setCurrentContext:context];
}

void SceneRendererGLES_iOS::defaultTargetInit(RenderTarget *renderTarget)
{
    if (!layer)
        return;
    
    if (![context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)layer])
        NSLog(@"SceneRendererES: Failure in renderbufferStorage");
}
    
void SceneRendererGLES_iOS::setLayer(CAEAGLLayer *inLayer)
{
    layer = inLayer;
}
    
void SceneRendererGLES_iOS::presentRender()
{
    if (!layer)
        return;
    
    [context presentRenderbuffer:GL_RENDERBUFFER];
}

void SceneRendererGLES_iOS::setSnapshotDelegate(NSObject<WhirlyKitSnapshot> *newDelegate)
{
    snapshotDelegate = newDelegate;
}

void SceneRendererGLES_iOS::snapshotCallback()
{
    // The user wants help with a screen snapshot
    if (snapshotDelegate)
    {
        if (snapshotDelegate.renderTargetID == EmptyIdentity)
        {
            // Courtesy: https://developer.apple.com/library/ios/qa/qa1704/_index.html
            NSInteger dataLength = framebufferWidth * framebufferHeight * 4;
            GLubyte *data = (GLubyte*)malloc(dataLength * sizeof(GLubyte));
            
            // Read pixel data from the framebuffer
            glPixelStorei(GL_PACK_ALIGNMENT, 4);
            glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
            
            // Create a CGImage with the pixel data
            // If your OpenGL ES content is opaque, use kCGImageAlphaNoneSkipLast to ignore the alpha channel
            // otherwise, use kCGImageAlphaPremultipliedLast
            CGDataProviderRef ref = CGDataProviderCreateWithData(NULL, data, dataLength, NULL);
            CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
            CGImageRef iref = CGImageCreate(framebufferWidth, framebufferHeight, 8, 32, framebufferWidth * 4, colorspace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast,
                                            ref, NULL, true, kCGRenderingIntentDefault);
            
            // OpenGL ES measures data in PIXELS
            // Create a graphics context with the target size measured in POINTS
            NSInteger widthInPoints, heightInPoints;
            {
                // On iOS 4 and later, use UIGraphicsBeginImageContextWithOptions to take the scale into consideration
                // Set the scale parameter to your OpenGL ES view's contentScaleFactor
                // so that you get a high-resolution snapshot when its value is greater than 1.0
                CGFloat scale = DeviceScreenScale();
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
            
            // Also wrap up the raw data
            NSData *dataWrapper = [[NSData alloc] initWithBytesNoCopy:data length:dataLength];
            
            UIGraphicsEndImageContext();
            
            // Clean up
            CFRelease(ref);
            CFRelease(colorspace);
            CGImageRelease(iref);
            
            [snapshotDelegate snapshotImage:image];
            [snapshotDelegate snapshotData:dataWrapper];
        } else {
            // Was a specific render target, not the general screen
            for (auto target: renderTargets) {
                if (target->getId() == snapshotDelegate.renderTargetID) {
                    RawDataRef rawData = target->snapshot();
                    // Note: This is an extra copy
                    NSData *data = [[NSData alloc] initWithBytes:rawData->getRawData() length:rawData->getLen()];
                    
                    [snapshotDelegate snapshotData:data];
                    break;
                }
            }
        }

        // Snapshots are a one time thing
        snapshotDelegate = nil;
    }
}
    
}

/*
 *  UIImage+Stuff.m
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/11/11.
 *  Copyright 2011-2013 mousebird consulting
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

#import "UIImage+Stuff.h"
#import "WhirlyGeometry.h"

using namespace WhirlyKit;

@implementation UIImage(Stuff)

// Courtesy: http://forum.sparrow-framework.org/topic/create-uiimage-from-pixel-data-problems
+ (id)imageWithRawData:(NSData *)imageData width:(unsigned int)width height:(unsigned int)height
{
    unsigned char *rawImageData = (unsigned char *)[imageData bytes];
    UIImage *newImage = nil;
    
    int nrOfColorComponents = 4; //RGBA
    int bitsPerColorComponent = 8;
    int rawImageDataLength = width * height * nrOfColorComponents;
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGDataProviderRef dataProviderRef;
    CGColorSpaceRef colorSpaceRef;
    CGImageRef imageRef;
    
    @try
    {
        GLubyte *rawImageDataBuffer = rawImageData;
        
        dataProviderRef = CGDataProviderCreateWithData(NULL, rawImageDataBuffer, rawImageDataLength, nil);
        colorSpaceRef = CGColorSpaceCreateDeviceRGB();
        imageRef = CGImageCreate(width, height, bitsPerColorComponent, bitsPerColorComponent * nrOfColorComponents, width * nrOfColorComponents, colorSpaceRef, bitmapInfo, dataProviderRef, NULL, NO, renderingIntent);
        newImage = [UIImage imageWithCGImage:imageRef];
    }
    @finally
    {
        CGDataProviderRelease(dataProviderRef);
        CGColorSpaceRelease(colorSpaceRef);
        CGImageRelease(imageRef);
    }
    
    return newImage;
}

-(NSData *)rawDataRetWidth:(unsigned int *)width height:(unsigned int *)height roundUp:(bool)roundUp
{
	CGImageRef cgImage = self.CGImage;
	*width = CGImageGetWidth(cgImage);
	*height = CGImageGetHeight(cgImage);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	
    // If we're not rounding up, round down
    if (!roundUp)
    {
        unsigned int upWidth = NextPowOf2(*width);
        unsigned int upHeight = NextPowOf2(*height);

        if (upWidth > *width && upWidth > 4)
            upWidth /= 2;
        if (upHeight > *height && upHeight > 4)
            upHeight /= 2;

        *width = upWidth;
        *height = upHeight;
    }

	NSMutableData *retData = [NSMutableData dataWithLength:(*width)*(*height)*4];
	CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], (*width), (*height), 8, (*width) * 4, colorSpace, kCGImageAlphaPremultipliedLast);
//	CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], *width, *height, 8, (*width) * 4, CGImageGetColorSpace(cgImage), kCGImageAlphaPremultipliedLast);
	CGContextDrawImage(theContext, CGRectMake(0.0, 0.0, (CGFloat)(*width), (CGFloat)(*height)), cgImage);
	CGContextRelease(theContext);
        CGColorSpaceRelease(colorSpace);
	
	return retData;
}

-(NSData *)rawDataScaleWidth:(unsigned int)destWidth height:(unsigned int)destHeight border:(int)border
{
	CGImageRef cgImage = self.CGImage;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	
    
	NSMutableData *retData = [NSMutableData dataWithLength:destWidth*destHeight*4];
	CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], destWidth, destHeight, 8, destWidth * 4, colorSpace, kCGImageAlphaPremultipliedLast);
	CGContextDrawImage(theContext, CGRectMake((float)border, (float)border, (CGFloat)(destWidth-2*border), (CGFloat)(destWidth-2*border)), cgImage);
	CGContextRelease(theContext);
    CGColorSpaceRelease(colorSpace);
    
    // Copy over the extra pixels
    // Note: Only supporting one pixel
    unsigned int *buf = (unsigned int *)[retData mutableBytes];
    if (border > 0)
    {
        int ix,iy;
        // Bottom
        for (iy=border-1;iy>=0;iy--)
            for (ix=0;ix<destWidth;ix++)
            buf[iy*destWidth + ix] = buf[(iy+1)*destWidth + ix];
        // Top
        for (iy=destHeight-(1+border);iy<destHeight;iy++)
        for (ix=0,iy=destHeight-1;ix<destWidth;ix++)
            buf[iy*destWidth + ix] = buf[(iy-1)*destWidth + ix];
        // Left
        for (ix=border-1;ix>=0;ix--)
            for (iy=0;iy<destHeight;iy++)
            buf[iy*destWidth + ix] = buf[iy*destWidth + (ix+1)];
        // Right
        for (ix=destWidth-(1+border);ix<destWidth;ix++)
            for (iy=0;iy<destHeight;iy++)
            buf[iy*destWidth + ix] = buf[iy*destWidth + (ix-1)];
    }
	
	return retData;
    
}


@end

void UIImageDummyFunc()
{    
}

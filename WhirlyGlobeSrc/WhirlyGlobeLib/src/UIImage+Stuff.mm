/*
 *  UIImage+Stuff.m
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/11/11.
 *  Copyright 2011 mousebird consulting
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


@implementation UIImage(Stuff)

-(NSData *)rawDataRetWidth:(unsigned int *)width height:(unsigned int *)height
{
	CGImageRef cgImage = self.CGImage;
	*width = CGImageGetWidth(cgImage);
	*height = CGImageGetHeight(cgImage);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	
	NSMutableData *retData = [NSMutableData dataWithLength:(*width)*(*height)*4];
	CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], *width, *height, 8, (*width) * 4, colorSpace, kCGImageAlphaPremultipliedLast);
//	CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], *width, *height, 8, (*width) * 4, CGImageGetColorSpace(cgImage), kCGImageAlphaPremultipliedLast);
	CGContextDrawImage(theContext, CGRectMake(0.0, 0.0, (CGFloat)(*width), (CGFloat)(*height)), cgImage);
	CGContextRelease(theContext);
	
	return retData;
}

@end

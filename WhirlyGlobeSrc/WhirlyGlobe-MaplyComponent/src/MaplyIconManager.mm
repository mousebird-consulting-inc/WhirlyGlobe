/*
 *  MaplyIconManager.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/11/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "MaplyIconManager.h"
#import "UIColor+Stuff.h"

@implementation MaplyIconManager
{
    NSCache *imageCache;
}

+ (MaplyIconManager *)shared
{
    static MaplyIconManager *iInst = nil;
    
    @synchronized(self)
    {
        if (iInst == NULL)
            iInst = [[self alloc] init];
    }
    
    return iInst;
}

- (id)init
{
    self = [super init];
    imageCache = [[NSCache alloc] init];
    
    return self;
}

- (UIImage *)iconForName:(NSString *)name size:(CGSize)size color:(UIColor *)color circleColor:(UIColor *)circleColor strokeSize:(float)strokeSize strokeColor:(UIColor *)strokeColor
{
    // Look for the cached version
    NSString *cacheKey = [NSString stringWithFormat:@"%@_%d_%d_%.1f_%0.6X_%0.6X", name,
                          (int)size.width, (int)size.height,
                          strokeSize, [color asHexRGB], [strokeColor asHexRGB]];
    
    id cached = [imageCache objectForKey:cacheKey];
    if (cached)
        return cached;
    
    NSString *fullName = nil;
    UIImage *iconImage;
    if (name)
    {
        NSString *fileName = [name lastPathComponent];
        iconImage = [UIImage imageNamed:fileName];
        if (!iconImage)
        {
            fullName = [NSString stringWithFormat:@"%@-24@2x.png",name];
            iconImage = [UIImage imageNamed:fullName];
            if (!iconImage)
            {
                // Try without the extension
                NSString *shortName = [name stringByDeletingPathExtension];
                if (shortName)
                {
                    fullName = [NSString stringWithFormat:@"%@@2x.png",shortName];
                    iconImage = [UIImage imageNamed:fullName];
                }
                
                if (!iconImage)
                {
                    [imageCache setObject:[NSNull null] forKey:cacheKey];
                    NSLog(@"Couldn't find: %@",shortName);
                    return nil;
                }
            }
        }
    }
    
    // Draw it into a circle
    UIGraphicsBeginImageContext(size);
    
    // Draw into the image context
    [[UIColor clearColor] setFill];
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextFillRect(ctx, CGRectMake(0,0,size.width,size.height));
    
    if (strokeColor)
    {
        CGContextBeginPath(ctx);
        CGContextAddEllipseInRect(ctx, CGRectMake(1,1,size.width-2,size.height-2));
        [strokeColor setFill];
        CGContextDrawPath(ctx, kCGPathFill);
    }
    
    if (circleColor)
    {
        CGContextBeginPath(ctx);
        CGContextAddEllipseInRect(ctx, CGRectMake(1+strokeSize,1+strokeSize,size.width-2-2*strokeSize,size.height-2-2*strokeSize));
        [circleColor setFill];
        CGContextDrawPath(ctx, kCGPathFill);
    }
    
    if (name && color)
    {
        CGContextTranslateCTM(ctx, 0, size.height);
        CGContextScaleCTM(ctx, 1.0, -1.0);
        [color setFill];
        CGContextDrawImage(ctx, CGRectMake(4, 4, size.width-8, size.height-8), iconImage.CGImage);
    }
    
    // Grab the image and shut things down
    UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    // Cache it
    [imageCache setObject:retImage forKey:cacheKey];
    
    return retImage;
}

+ (UIImage *)iconForName:(NSString *)name size:(CGSize)size
{
    return [[self shared] iconForName:name size:size color:[UIColor blackColor] circleColor:[UIColor whiteColor] strokeSize:1.0 strokeColor:[UIColor blackColor]];
}

+ (UIImage *)iconForName:(NSString *)name size:(CGSize)size color:(UIColor *)color circleColor:(UIColor *)circleColor strokeSize:(float)strokeSize strokeColor:(UIColor *)strokeColor
{
    return [[self shared] iconForName:name size:size color:color circleColor:circleColor strokeSize:strokeSize strokeColor:strokeColor];
}

@end

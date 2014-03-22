//
//  MaplyIconManager.mm
//  WhirlyGlobe-MaplyComponent
//
//  Created by Steve Gifford on 1/11/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import "MaplyIconManager.h"

@implementation MaplyIconManager
{
    NSMutableDictionary *imgDict;
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
    imgDict = [NSMutableDictionary dictionary];
    
    return self;
}

- (UIImage *)iconForName:(NSString *)name size:(CGSize)size color:(UIColor *)color strokeSize:(float)strokeSize strokeColor:(UIColor *)strokeColor
{
    // Look for the cached version
    NSString *fakeName = [NSString stringWithFormat:@"%@_%d_%d",name,(int)size.width,(int)size.height];
    @synchronized(self)
    {
        if (imgDict[fakeName])
            return imgDict[fakeName];
    }
    
    NSString *fullName = nil;
    UIImage *iconImage;
    if (name)
    {
        fullName = [NSString stringWithFormat:@"%@-24@2x.png",name];
        iconImage = [UIImage imageNamed:fullName];
        if (!iconImage)
        {
            // Try without the extension
            NSString *shortName = [name stringByDeletingPathExtension];
            if (shortName)
            {
                fullName = [NSString stringWithFormat:@"%@-24@2x.png",shortName];
                iconImage = [UIImage imageNamed:fullName];
            }
            
            if (!iconImage)
            {
                imgDict[fakeName] = [NSNull null];
                return nil;
            }
        }
    }
    
    // Draw it into a circle
    UIGraphicsBeginImageContext(size);
    
    // Draw into the image context
    [[UIColor clearColor] setFill];
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextFillRect(ctx, CGRectMake(0,0,size.width,size.height));
    
    CGContextBeginPath(ctx);
    CGContextAddEllipseInRect(ctx, CGRectMake(1,1,size.width-2,size.height-2));
    [strokeColor setFill];
    CGContextDrawPath(ctx, kCGPathFill);
    
    CGContextBeginPath(ctx);
    CGContextAddEllipseInRect(ctx, CGRectMake(1+strokeSize,1+strokeSize,size.width-2-2*strokeSize,size.height-2-2*strokeSize));
    [color setFill];
    CGContextDrawPath(ctx, kCGPathFill);
    
    if (name)
    {
        CGContextTranslateCTM(ctx, 0, size.height);
        CGContextScaleCTM(ctx, 1.0, -1.0);
        CGContextDrawImage(ctx, CGRectMake(4, 4, size.width-8, size.height-8), iconImage.CGImage);
    }
    
    // Grab the image and shut things down
    UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    // Cache it
    @synchronized(self)
    {
        imgDict[fakeName] = retImage;
    }
    
    return retImage;
}

+ (UIImage *)iconForName:(NSString *)name size:(CGSize)size
{
    return [[self shared] iconForName:name size:size color:[UIColor whiteColor] strokeSize:1.0 strokeColor:[UIColor blackColor]];
}

+ (UIImage *)iconForName:(NSString *)name size:(CGSize)size color:(UIColor *)color strokeSize:(float)strokeSize strokeColor:(UIColor *)strokeColor
{
    return [[self shared] iconForName:name size:size color:color strokeSize:strokeSize strokeColor:strokeColor];
}

@end

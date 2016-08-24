//
//  SLDWellKnownMarkers.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-23.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "SLDWellKnownMarkers.h"

#define SLD_MARKER_SIZE 64

@implementation SLDWellKnownMarkers


+ (UIImage *)circleImage {
    static UIImage *img = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(SLD_MARKER_SIZE, SLD_MARKER_SIZE), NO, 0.0f);
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextSaveGState(ctx);
        
        CGRect rect = CGRectMake(0, 0, SLD_MARKER_SIZE, SLD_MARKER_SIZE);
        CGContextSetFillColorWithColor(ctx, [UIColor blackColor].CGColor);
        CGContextFillEllipseInRect(ctx, rect);
        
        CGContextRestoreGState(ctx);
        img = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
    });
    return img;
}

+ (UIImage *)squareImage {
    static UIImage *img = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(SLD_MARKER_SIZE, SLD_MARKER_SIZE), NO, 0.0f);
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextSaveGState(ctx);
        
        CGRect rect = CGRectMake(0, 0, SLD_MARKER_SIZE, SLD_MARKER_SIZE);
        CGContextSetFillColorWithColor(ctx, [UIColor blackColor].CGColor);
        CGContextFillRect(ctx, rect);
        
        CGContextRestoreGState(ctx);
        img = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
    });
    return img;
}


+ (UIImage *)triangleImage {
    static UIImage *img = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(SLD_MARKER_SIZE, SLD_MARKER_SIZE), NO, 0.0f);
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextSaveGState(ctx);
        
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathMoveToPoint(path, NULL, 0, SLD_MARKER_SIZE * 0.93);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE, SLD_MARKER_SIZE * 0.93);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE/2, SLD_MARKER_SIZE * 0.07);
        CGPathCloseSubpath(path);
        
        CGContextSetFillColorWithColor(ctx, [UIColor blackColor].CGColor);
        CGContextAddPath(ctx, path);
        CGContextFillPath(ctx);
        CGPathRelease(path);
        
        CGContextRestoreGState(ctx);
        img = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
    });
    return img;
}

+ (UIImage *)starImage {
    static UIImage *img = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(SLD_MARKER_SIZE, SLD_MARKER_SIZE), NO, 0.0f);
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextSaveGState(ctx);
        
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathMoveToPoint(path, NULL, SLD_MARKER_SIZE * 0.2, SLD_MARKER_SIZE * 0.95);
        CGPathAddLineToPoint(path, NULL,SLD_MARKER_SIZE * 0.3, SLD_MARKER_SIZE * 0.615);
        CGPathAddLineToPoint(path, NULL, 0, SLD_MARKER_SIZE * 0.395);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.38, SLD_MARKER_SIZE * 0.395);
        CGPathAddLineToPoint(path, NULL,SLD_MARKER_SIZE * 0.5, SLD_MARKER_SIZE * 0.03);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.62, SLD_MARKER_SIZE * 0.395);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE, SLD_MARKER_SIZE * 0.395);
        CGPathAddLineToPoint(path, NULL,SLD_MARKER_SIZE * 0.7, SLD_MARKER_SIZE * 0.615);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.8, SLD_MARKER_SIZE * 0.95);
        CGPathAddLineToPoint(path, NULL,SLD_MARKER_SIZE * 0.5, SLD_MARKER_SIZE * 0.755);
        CGPathCloseSubpath(path);
        
        CGContextSetFillColorWithColor(ctx, [UIColor blackColor].CGColor);
        CGContextAddPath(ctx, path);
        CGContextFillPath(ctx);
        CGPathRelease(path);
        
        CGContextRestoreGState(ctx);
        img = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
    });
    return img;
}

+ (UIImage *)crossImage {
    static UIImage *img = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(SLD_MARKER_SIZE, SLD_MARKER_SIZE), NO, 0.0f);
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextSaveGState(ctx);
        
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathMoveToPoint(path, NULL, SLD_MARKER_SIZE * 0.35, SLD_MARKER_SIZE * 0.55);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.35, SLD_MARKER_SIZE * 0.45);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.45, SLD_MARKER_SIZE * 0.45);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.45, SLD_MARKER_SIZE * 0.35);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.55, SLD_MARKER_SIZE * 0.35);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.55, SLD_MARKER_SIZE * 0.45);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.65, SLD_MARKER_SIZE * 0.45);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.65, SLD_MARKER_SIZE * 0.55);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.55, SLD_MARKER_SIZE * 0.55);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.55, SLD_MARKER_SIZE * 0.65);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.45, SLD_MARKER_SIZE * 0.65);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.45, SLD_MARKER_SIZE * 0.55);
        CGPathCloseSubpath(path);
        
        CGContextSetFillColorWithColor(ctx, [UIColor blackColor].CGColor);
        CGContextAddPath(ctx, path);
        CGContextFillPath(ctx);
        CGPathRelease(path);
        
        CGContextRestoreGState(ctx);
        img = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
    });
    return img;
}

+ (UIImage *)xImage {
    static UIImage *img = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(SLD_MARKER_SIZE, SLD_MARKER_SIZE), NO, 0.0f);
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextSaveGState(ctx);
        
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathMoveToPoint(path, NULL, SLD_MARKER_SIZE * 0.36, SLD_MARKER_SIZE * 0.43);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.43, SLD_MARKER_SIZE * 0.36);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.5, SLD_MARKER_SIZE * 0.43);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.57, SLD_MARKER_SIZE * 0.36);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.64, SLD_MARKER_SIZE * 0.43);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.57, SLD_MARKER_SIZE * 0.5);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.64, SLD_MARKER_SIZE * 0.57);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.57, SLD_MARKER_SIZE * 0.64);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.5, SLD_MARKER_SIZE * 0.57);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.43, SLD_MARKER_SIZE * 0.64);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.36, SLD_MARKER_SIZE * 0.57);
        CGPathAddLineToPoint(path, NULL, SLD_MARKER_SIZE * 0.43, SLD_MARKER_SIZE * 0.5);
        CGPathCloseSubpath(path);
        
        CGContextSetFillColorWithColor(ctx, [UIColor blackColor].CGColor);
        CGContextAddPath(ctx, path);
        CGContextFillPath(ctx);
        CGPathRelease(path);
        
        CGContextRestoreGState(ctx);
        img = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
    });
    return img;
}

@end

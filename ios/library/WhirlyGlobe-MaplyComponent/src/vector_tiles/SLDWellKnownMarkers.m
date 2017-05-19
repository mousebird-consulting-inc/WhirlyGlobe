//
//  SLDWellKnownMarkers.m
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-23.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "SLDWellKnownMarkers.h"

@implementation SLDWellKnownMarkers

+ (UIImage *)imageWithName:(NSString *)wellKnownName strokeColor:(UIColor *)strokeColor fillColor:(UIColor *)fillColor size:(int)size {
    
    if ([wellKnownName isEqualToString:@"square"])
        return [SLDWellKnownMarkers squareImageWithStrokeColor:strokeColor fillColor:fillColor size:size];
    else if ([wellKnownName isEqualToString:@"circle"])
        return [SLDWellKnownMarkers circleImageWithStrokeColor:strokeColor fillColor:fillColor size:size];
    else if ([wellKnownName isEqualToString:@"triangle"])
        return [SLDWellKnownMarkers triangleImageWithStrokeColor:strokeColor fillColor:fillColor size:size];
    else if ([wellKnownName isEqualToString:@"star"])
        return [SLDWellKnownMarkers starImageWithStrokeColor:strokeColor fillColor:fillColor size:size];
    else if ([wellKnownName isEqualToString:@"cross"])
        return [SLDWellKnownMarkers crossImageWithStrokeColor:strokeColor fillColor:fillColor size:size];
    else if ([wellKnownName isEqualToString:@"x"])
        return [SLDWellKnownMarkers xImageWithStrokeColor:strokeColor fillColor:fillColor size:size];
    else {
        NSLog(@"Skipping MarkerSymbolizer with unrecognized WellKnownName for Mark.");
        return nil;
    }
}

+ (UIImage *)circleImageWithStrokeColor:(UIColor *)strokeColor fillColor:(UIColor *)fillColor size:(int)size {
    UIImage *img;
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size, size), NO, 0.0f);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);
    
    CGRect rect = CGRectMake(0.5, 0.5, size-1, size-1);
    CGContextSetFillColorWithColor(ctx, fillColor.CGColor);
    CGContextSetStrokeColorWithColor(ctx, strokeColor.CGColor);
    CGContextFillEllipseInRect(ctx, rect);
    CGContextStrokeEllipseInRect(ctx, rect);
    
    CGContextRestoreGState(ctx);
    img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
        
    return img;
}

+ (UIImage *)squareImageWithStrokeColor:(UIColor *)strokeColor fillColor:(UIColor *)fillColor size:(int)size {
    
    UIImage *img;
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size, size), NO, 0.0f);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);
    
    CGRect rect = CGRectMake(0.5, 0.5, size-1, size-1);
    CGContextSetFillColorWithColor(ctx, fillColor.CGColor);
    CGContextSetStrokeColorWithColor(ctx, strokeColor.CGColor);
    CGContextFillRect(ctx, rect);
    CGContextStrokeRect(ctx, rect);
    
    CGContextRestoreGState(ctx);
    img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
        
    return img;
}

+ (UIImage *)triangleImageWithStrokeColor:(UIColor *)strokeColor fillColor:(UIColor *)fillColor size:(int)size {
    UIImage *img;
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size, size), NO, 0.0f);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);
    
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, NULL, 0, size * 0.93);
    CGPathAddLineToPoint(path, NULL, size, size * 0.93);
    CGPathAddLineToPoint(path, NULL, size/2, size * 0.07);
    CGPathCloseSubpath(path);
    
    CGContextSetFillColorWithColor(ctx, fillColor.CGColor);
    CGContextSetStrokeColorWithColor(ctx, strokeColor.CGColor);
    CGContextAddPath(ctx, path);
    CGContextFillPath(ctx);
    CGContextAddPath(ctx, path);
    CGContextStrokePath(ctx);
    CGPathRelease(path);
    
    CGContextRestoreGState(ctx);
    img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
        
    return img;
}

+ (UIImage *)starImageWithStrokeColor:(UIColor *)strokeColor fillColor:(UIColor *)fillColor size:(int)size {
    UIImage *img;
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size, size), NO, 0.0f);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);
    
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, NULL, size * 0.2, size * 0.95);
    CGPathAddLineToPoint(path, NULL,size * 0.3, size * 0.615);
    CGPathAddLineToPoint(path, NULL, 0, size * 0.395);
    CGPathAddLineToPoint(path, NULL, size * 0.38, size * 0.395);
    CGPathAddLineToPoint(path, NULL,size * 0.5, size * 0.03);
    CGPathAddLineToPoint(path, NULL, size * 0.62, size * 0.395);
    CGPathAddLineToPoint(path, NULL, size, size * 0.395);
    CGPathAddLineToPoint(path, NULL,size * 0.7, size * 0.615);
    CGPathAddLineToPoint(path, NULL, size * 0.8, size * 0.95);
    CGPathAddLineToPoint(path, NULL,size * 0.5, size * 0.755);
    CGPathCloseSubpath(path);
    
    CGContextSetFillColorWithColor(ctx, fillColor.CGColor);
    CGContextSetStrokeColorWithColor(ctx, strokeColor.CGColor);
    CGContextAddPath(ctx, path);
    CGContextFillPath(ctx);
    CGContextAddPath(ctx, path);
    CGContextStrokePath(ctx);
    CGPathRelease(path);
    
    CGContextRestoreGState(ctx);
    img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
        
    return img;
}

+ (UIImage *)crossImageWithStrokeColor:(UIColor *)strokeColor fillColor:(UIColor *)fillColor size:(int)size {
    UIImage *img;
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size, size), NO, 0.0f);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);
    
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, NULL, size * 0.05, size * 0.65);
    CGPathAddLineToPoint(path, NULL, size * 0.05, size * 0.35);
    CGPathAddLineToPoint(path, NULL, size * 0.35, size * 0.35);
    CGPathAddLineToPoint(path, NULL, size * 0.35, size * 0.05);
    CGPathAddLineToPoint(path, NULL, size * 0.65, size * 0.05);
    CGPathAddLineToPoint(path, NULL, size * 0.65, size * 0.35);
    CGPathAddLineToPoint(path, NULL, size * 0.95, size * 0.35);
    CGPathAddLineToPoint(path, NULL, size * 0.95, size * 0.65);
    CGPathAddLineToPoint(path, NULL, size * 0.65, size * 0.65);
    CGPathAddLineToPoint(path, NULL, size * 0.65, size * 0.95);
    CGPathAddLineToPoint(path, NULL, size * 0.35, size * 0.95);
    CGPathAddLineToPoint(path, NULL, size * 0.35, size * 0.65);
    CGPathCloseSubpath(path);
    
    CGContextSetFillColorWithColor(ctx, fillColor.CGColor);
    CGContextSetStrokeColorWithColor(ctx, strokeColor.CGColor);
    CGContextAddPath(ctx, path);
    CGContextFillPath(ctx);
    CGContextAddPath(ctx, path);
    CGContextStrokePath(ctx);
    CGPathRelease(path);
    
    CGContextRestoreGState(ctx);
    img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
        
    return img;
}

+ (UIImage *)xImageWithStrokeColor:(UIColor *)strokeColor fillColor:(UIColor *)fillColor size:(int)size {
    
    UIImage *img;
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size, size), NO, 0.0f);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);
    
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, NULL, size * 0.08, size * 0.29);
    CGPathAddLineToPoint(path, NULL, size * 0.29, size * 0.08);
    CGPathAddLineToPoint(path, NULL, size * 0.5, size * 0.29);
    CGPathAddLineToPoint(path, NULL, size * 0.71, size * 0.08);
    CGPathAddLineToPoint(path, NULL, size * 0.92, size * 0.29);
    CGPathAddLineToPoint(path, NULL, size * 0.71, size * 0.5);
    CGPathAddLineToPoint(path, NULL, size * 0.92, size * 0.71);
    CGPathAddLineToPoint(path, NULL, size * 0.71, size * 0.92);
    CGPathAddLineToPoint(path, NULL, size * 0.5, size * 0.71);
    CGPathAddLineToPoint(path, NULL, size * 0.29, size * 0.92);
    CGPathAddLineToPoint(path, NULL, size * 0.08, size * 0.71);
    CGPathAddLineToPoint(path, NULL, size * 0.29, size * 0.5);
    CGPathCloseSubpath(path);
    
    CGContextSetFillColorWithColor(ctx, fillColor.CGColor);
    CGContextSetStrokeColorWithColor(ctx, strokeColor.CGColor);
    CGContextAddPath(ctx, path);
    CGContextFillPath(ctx);
    CGContextAddPath(ctx, path);
    CGContextStrokePath(ctx);
    CGPathRelease(path);
    
    CGContextRestoreGState(ctx);
    img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
        
    return img;
}

@end

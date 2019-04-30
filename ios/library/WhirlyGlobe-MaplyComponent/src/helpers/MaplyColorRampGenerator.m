//
//  MaplyColorRampGenerator.m
//  WhirlyGlobe-MaplyComponent
//
//  Created by Steve Gifford on 4/20/16.
//
//

#import "rendering/MaplyColorRampGenerator.h"

@implementation MaplyColorRampGenerator
{
    NSMutableArray *entryColors;
    NSMutableArray *colors;
}

- (id)init
{
    self = [super init];
    entryColors = [NSMutableArray array];
    colors = [NSMutableArray array];
    _stretch = true;
    
    return self;
}

- (void)addSingleEntryColor:(UIColor *)color
{
    [entryColors addObject:color];
}

- (void)addHexColor:(int)hexColor
{
    float red = (((hexColor) >> 16) & 0xFF)/255.0;
    float green = (((hexColor) >> 8) & 0xFF)/255.0;
    float blue = (((hexColor) >> 0) & 0xFF)/255.0;
    
    UIColor *color = [UIColor colorWithRed:red green:green blue:blue alpha:1.0];
    [colors addObject:color];
}

- (void)addHexColorWithAlpha:(int)hexColor
{
    float alpha = (((hexColor) >> 24) & 0xFF)/255.0;
    float red = (((hexColor) >> 16) & 0xFF)/255.0;
    float green = (((hexColor) >> 8) & 0xFF)/255.0;
    float blue = (((hexColor) >> 0) & 0xFF)/255.0;
    
    NSLog(@"0x%x",hexColor);
        
    UIColor *color = [UIColor colorWithRed:red*alpha green:green*alpha blue:blue*alpha alpha:alpha];
    [colors addObject:color];
}

- (void)addColor:(UIColor *)color
{
    [colors addObject:color];
}

- (void)addByteRed:(int)red green:(int)green blue:(int)blue alpha:(int)alpha
{
    [colors addObject:[UIColor colorWithRed:red/255.0 green:green/255.0 blue:blue/255.0 alpha:alpha/255.0]];
}

- (UIColor *)interpColor:(float)where
{
    if ([colors count] == 0)
        return nil;
    
    if ([colors count] == 1)
        return [colors objectAtIndex:0];
    
    float pos = where*[colors count];
    int aIdx = floor(pos);
    int bIdx = ceil(pos);
    if (bIdx >= [colors count])
        bIdx = (int)[colors count]-1;
    if (aIdx >= [colors count])
        aIdx = (int)[colors count]-1;
    float t = pos-aIdx;
    
    UIColor *a = [colors objectAtIndex:aIdx];
    UIColor *b = [colors objectAtIndex:bIdx];
    CGFloat aVals[4];
    CGFloat bVals[4];
    [a getRed:&aVals[0] green:&aVals[1] blue:&aVals[2] alpha:&aVals[3]];
    [b getRed:&bVals[0] green:&bVals[1] blue:&bVals[2] alpha:&bVals[3]];
    
    CGFloat iVals[4];
    for (unsigned int ii=0;ii<4;ii++)
        iVals[ii] = (bVals[ii]-aVals[ii])*t + aVals[ii];
    
    return [UIColor colorWithRed:iVals[0] green:iVals[1] blue:iVals[2] alpha:iVals[3]];
}

- (UIImage *)makeImage:(CGSize)size
{
    int width = size.width;
    int height = size.height;
    CGSize imgSize = size;
    UIGraphicsBeginImageContext(imgSize);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    CGContextClearRect(ctx, CGRectMake(0, 0, imgSize.width, imgSize.height));

    // Work our way through the pixels by height
    for (unsigned int xx=0;xx<width;xx++)
    {
        UIColor *color = nil;
        if (xx < [entryColors count]) {
            color = [entryColors objectAtIndex:xx];
        } else {
            if (_stretch)
                color = [self interpColor:(xx/(float)(width-1))];
            else {
                if (xx >= [colors count])
                    color = [UIColor clearColor];
                else
                    color = [colors objectAtIndex:xx];
            }
        }
        [color setFill];
        CGContextFillRect(ctx, CGRectMake(xx, 0.0, 1, height));
    }
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

- (NSArray *)getColors
{
    return colors;
}

@end

/*
 *  MaplyIconManager.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/11/14.
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

#import "helpers/MaplyIconManager.h"
#import "visual_objects/MaplyScreenLabel.h"
#import "UIColor+Stuff.h"

@implementation MaplySimpleStyle
@end

// Courtesy: https://stackoverflow.com/questions/11598043/get-slightly-lighter-and-darker-color-from-uicolor
@implementation UIColor (Lighter)

- (UIColor *)lighterColor
{
    CGFloat h, s, b, a;
    if ([self getHue:&h saturation:&s brightness:&b alpha:&a])
        return [UIColor colorWithHue:h
                          saturation:s
                          brightness:MIN(b * 1.3, 1.0)
                               alpha:a];
    return nil;
}

@end


namespace WhirlyKit
{

class MarkerRep
{
public:
    MarkerRep();
    
    // Symbol is small/medium/large
    typedef enum {MarkerSmall,MarkerMedium,MarkerLarge} IconSize;
    IconSize size;

    // Symbol is the name of a UIImage or a single character
    std::string symbol;
    
    // Background color of the marker
    RGBAColor color;
};

}

@implementation MaplySimpleStyleManager
{
    NSCache *imageCache;
    NSMutableDictionary *texCache;
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
}

+ (MaplySimpleStyleManager *)shared
{
    static MaplySimpleStyleManager *iInst = nil;
    
    @synchronized(self)
    {
        if (iInst == NULL)
            iInst = [[self alloc] init];
    }
    
    return iInst;
}

- (instancetype)init
{
    self = [super init];
    imageCache = [[NSCache alloc] init];
    texCache = [NSMutableDictionary dictionary];
    _scale = [UIScreen mainScreen].scale;
    _smallSize = CGSizeMake(16.0, 16.0);
    _medSize = CGSizeMake(32.0, 32.0);
    _largeSize = CGSizeMake(64.0, 64.0);
    _strokeWidthForIcons = 1.0 * _scale;
    _centerIcon = true;

    return self;
}

- (nonnull id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)inViewC
{
    self = [self init];
    viewC = inViewC;
    
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
    
    UIImage *iconImage;
    if(name.isAbsolutePath)
    {
        iconImage = [UIImage imageWithContentsOfFile:name];
    } else if (name)
    {
        NSString *fullName = nil;
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

- (UIColor *)parseColor:(NSString *)colorStr default:(UIColor *)defColor
{
    UIColor *color = defColor;
    NSString *str = colorStr;
    
    if ([str length] > 0 && [str characterAtIndex:0] == '#') {
        NSScanner *scanner = [NSScanner scannerWithString:str];
        [scanner setScanLocation:1];
        unsigned int val;
        if ([scanner scanHexInt:&val]) {
            color = [UIColor colorFromHexRGB:val];
        }
    }

    return color;
}

- (CGFloat)parseNumber:(NSNumber *)num default:(CGFloat)defVal
{
    if (num)
        return [num doubleValue];
    return defVal;
}

- (bool)parseBool:(NSString *)val default:(bool)defVal
{
    if (val)
        return [val boolValue];
    return defVal;
}

- (UIImage *)loadImage:(NSString *)symbol cacheKey:(NSString *)cacheKey
{
    UIImage *mainImage = nil;
    
    if(symbol.isAbsolutePath) {
        mainImage = [UIImage imageWithContentsOfFile:symbol];
    } else if (symbol)
    {
        NSString *fullName = nil;
        NSString *fileName = [symbol lastPathComponent];
        mainImage = [UIImage imageNamed:fileName];
        if (!mainImage)
        {
            fullName = [NSString stringWithFormat:@"%@-24@2x.png",symbol];
            mainImage = [UIImage imageNamed:fullName];
            if (!mainImage)
            {
                // Try without the extension
                NSString *shortName = [symbol stringByDeletingPathExtension];
                if (shortName)
                {
                    fullName = [NSString stringWithFormat:@"%@@2x.png",shortName];
                    mainImage = [UIImage imageNamed:fullName];
                }
                
                if (!mainImage)
                {
                    [texCache setObject:[NSNull null] forKey:cacheKey];
                    NSLog(@"Couldn't find: %@",shortName);
                    return nil;
                }
            }
        }
    }
    
    return mainImage;
}

// Does much the same work as the image version above, but is slightly different for the simple styel
- (MaplyTexture *)textureForStyle:(MaplySimpleStyle *)style
                       backSymbol:(NSString *)backSymbol
                           symbol:(NSString *)symbol
                       strokeSize:(CGFloat)strokeSize
                           center:(CGPoint)center
                  clearBackground:(bool)clearBackground
{
    NSString *cacheKey = [NSString stringWithFormat:@"%@_%@_%d_%d_%.1f_%0.6X",
                          backSymbol,
                          symbol,
                          (int)style.markerSize.width, (int)style.markerSize.height,
                          strokeSize, [style.color asHexRGB]];
    
    id cached = [texCache objectForKey:cacheKey];
    if ([cached isKindOfClass:[MaplyTexture class]])
        return cached;

    UIImage *mainImage = [self loadImage:symbol cacheKey:[cacheKey stringByAppendingString:@"_main"]];
    UIImage *backImage = [self loadImage:backSymbol cacheKey:[cacheKey stringByAppendingString:@"_back"]];
    
    // Draw it into a circle
    UIGraphicsBeginImageContext(style.markerSize);
    
    // Draw into the image context
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSetBlendMode(ctx, kCGBlendModeNormal);
    [[UIColor blackColor] setFill];
    CGRect rect = CGRectMake(0,0,style.markerSize.width,style.markerSize.height);
    CGContextFillRect(ctx, rect);
        
    // We want a custom background image, rather than just the circle
    if (backImage) {
        // Courtesy: https://stackoverflow.com/questions/3514066/how-to-tint-a-transparent-png-image-in-iphone
        CGContextSaveGState(ctx);
        CGContextTranslateCTM(ctx, 0, style.markerSize.height);
        CGContextScaleCTM(ctx, 1.0, -1.0);
        CGContextSetBlendMode(ctx, kCGBlendModeNormal);
        
        [style.color setFill];
        CGContextFillRect(ctx, rect);
        CGContextSetBlendMode(ctx, kCGBlendModeDestinationIn);
        CGContextDrawImage(ctx, rect, backImage.CGImage);
        
        CGContextRestoreGState(ctx);
    } else {
        if (strokeSize > 0.0) {
            UIColor *strokeColor = [style.color lighterColor];
            CGContextBeginPath(ctx);
            CGContextAddEllipseInRect(ctx, CGRectMake(1,1,style.markerSize.width-2,style.markerSize.height-2));
            [strokeColor setFill];
            CGContextDrawPath(ctx, kCGPathFill);
        }

        if (!clearBackground) {
            CGContextBeginPath(ctx);
            CGContextAddEllipseInRect(ctx, CGRectMake(1+strokeSize,1+strokeSize,style.markerSize.width-2-2*strokeSize,style.markerSize.height-2-2*strokeSize));
            [style.color setFill];
            CGContextDrawPath(ctx, kCGPathFill);
        }
    }
    
    if (mainImage) {
        CGContextTranslateCTM(ctx, 0, style.markerSize.height);
        CGContextScaleCTM(ctx, 1.0, -1.0);
        [[UIColor blackColor] setFill];
        CGPoint theCenter;
        theCenter.x = center.x > -1000.0 ? center.x/mainImage.size.width * style.markerSize.width : style.markerSize.width/2.0;
        theCenter.y = center.y > -1000.0 && backImage ? center.y/backImage.size.height * style.markerSize.height : style.markerSize.height/2.0;
        CGContextDrawImage(ctx, CGRectMake(theCenter.x - mainImage.size.width/2.0, (style.markerSize.height-theCenter.y) - mainImage.size.height/2.0,
                                           mainImage.size.width, mainImage.size.height), mainImage.CGImage);
    }
    
    // Grab the image and shut things down
    UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    if (retImage) {
        MaplyTexture *tex = [viewC addTexture:retImage desc:nil mode:MaplyThreadCurrent];
        [texCache setObject:tex forKey:cacheKey];
        return tex;
    }
    
    return nil;
}

- (void)shutdown
{
    @synchronized (self) {
        NSMutableArray<MaplyTexture *> *texs = [NSMutableArray array];
        for (MaplyTexture *tex in texCache.allValues)
            if ([tex isKindOfClass:[MaplyTexture class]])
                [texs addObject:tex];
        [viewC removeTextures:texs mode:MaplyThreadCurrent];
        texCache = nil;
    }
}

- (MaplySimpleStyle * __nonnull)makeStyle:(NSDictionary *__nonnull)dict
{
    @synchronized (self) {
        MaplySimpleStyle *style = [[MaplySimpleStyle alloc] init];
        style.title = dict[@"title"];
        style.desc = dict[@"description"];
        
        // Sort out marker size
        NSString *markerSizeStr = dict[@"marker-size"];
        int markerSizeInt = 1;  // Medium
        if (markerSizeStr) {
            markerSizeStr = [markerSizeStr lowercaseString];
            if ([markerSizeStr isEqualToString:@"small"])
                markerSizeInt = 0;
            else if ([markerSizeStr isEqualToString:@"medium"])
                markerSizeInt = 1;
            else if ([markerSizeStr isEqualToString:@"large"])
                markerSizeInt = 2;
        }
        switch (markerSizeInt) {
            case 0:
                style.markerSize = _smallSize;
                break;
            case 1:
                style.markerSize = _medSize;
                break;
            case 2:
                style.markerSize = _largeSize;
                break;
        }
        
        // It's either a texture or a single character
        NSString *symbol = dict[@"marker-symbol"];
        if ([symbol length] == 1) {
            style.markerString = symbol;
            symbol = nil;
        }
        NSString *backSymbol = dict[@"marker-background-symbol"];
        
        style.color = [self parseColor:dict[@"marker-color"] default:[UIColor whiteColor]];
        style.strokeColor = [self parseColor:dict[@"stroke"] default:[UIColor colorFromHexRGB:0x555555]];
        style.strokeOpacity = [self parseNumber:dict[@"stroke-opactiy"] default:1.0];
        style.strokeWidth = [self parseNumber:dict[@"stroke-width"] default:2.0];
        style.fillColor = [self parseColor:dict[@"fill"] default:[UIColor colorFromHexRGB:0x555555]];
        style.fillOpacity = [self parseNumber:dict[@"fill-opacity"] default:0.6];
        CGPoint center = CGPointMake(-1000.0, -1000.0);
        center.x = [self parseNumber:dict[@"marker-background-center-x"] default:center.x];
        center.y = [self parseNumber:dict[@"marker-background-center-y"] default:center.y];
        bool clearBackground = [self parseBool:dict[@"marker-circle"] default:true];

        // Need a texture for the marker
        float strokeWidth = [self parseNumber:dict[@"stroke-width"] default:_strokeWidthForIcons];
        style.markerTex = [self textureForStyle:style backSymbol:backSymbol symbol:symbol strokeSize:strokeWidth center:center clearBackground:clearBackground];
        // TODO: Handle the single character case
        
        return style;
    }
}

- (UIColor *)resolveColor:(UIColor *)color opacity:(CGFloat)opacity
{
    CGFloat red,green,blue,alpha;
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    
    return [UIColor colorWithRed:red*opacity green:green*opacity blue:blue*opacity alpha:alpha*opacity];
}

- (MaplyComponentObject * __nullable)addFeature:(MaplyVectorObject * __nonnull)vecObj mode:(MaplyThreadMode)mode
{
    MaplyComponentObject *compObj = nil;
    MaplySimpleStyle *style = [self makeStyle:vecObj.attributes];
    
    switch ([vecObj vectorType]) {
        case MaplyVectorPointType:
            // It's a screen marker
            if (style.markerTex) {
                MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
                marker.loc = [vecObj center];
                marker.image = style.markerTex;
                marker.size = style.markerSize;
                marker.offset = style.markerOffset;
                compObj = [viewC addScreenMarkers:@[marker] desc:nil mode:mode];
            }
            break;
        case MaplyVectorLinearType:
            compObj = [viewC addWideVectors:@[vecObj]
                                   desc:@{kMaplyColor: [self resolveColor:style.strokeColor opacity:style.strokeOpacity],
                                          kMaplyVecWidth: @(style.strokeWidth)}
                                   mode:mode];
            break;
        case MaplyVectorArealType:
            compObj = [viewC addVectors:@[vecObj]
                                   desc:@{kMaplyColor: [self resolveColor:style.fillColor opacity:style.fillOpacity],
                                          kMaplyFilled: @(true)
                                   }
                                   mode:mode];
            break;
        case MaplyVectorNoneType:
        case MaplyVectorMultiType:
        case MaplyVectorLinear3dType:
            NSLog(@"Passed a MaplyVectorObject into MaplySimpleStyleManager::addFeature that has more than one type of feature.");
            break;
    }
    
    return compObj;
}

- (NSArray<MaplyComponentObject *> * __nonnull)addFeatures:(NSArray<MaplyVectorObject *> * __nonnull)vecObjs mode:(MaplyThreadMode)mode
{
    NSMutableArray *compObjs = [[NSMutableArray alloc] init];
    
    for (MaplyVectorObject *vecObj in vecObjs) {
        for (MaplyVectorObject *vecObj2 in [vecObj splitVectors]) {
            MaplyComponentObject *compObj = [self addFeature:vecObj2 mode:mode];
            if (compObj)
                [compObjs addObject:compObj];
        }
    }
    
    return compObjs;
}


@end

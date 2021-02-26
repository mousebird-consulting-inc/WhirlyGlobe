/*  MaplyIconManager.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/11/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "helpers/MaplyIconManager.h"
#import "visual_objects/MaplyScreenLabel.h"
#import "UIColor+Stuff.h"

@implementation MaplySimpleStyle
@end

@implementation MaplySimpleStyleManager
{
    NSCache *imageCache;
    NSMutableDictionary *texCache;
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
}

+ (MaplySimpleStyleManager *)shared
{
    static MaplySimpleStyleManager *iInst = nil;
    
    @synchronized(self) {
        if (iInst == nullptr) {
            iInst = [[self alloc] init];
        }
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

- (UIImage *)iconForName:(NSString *)name size:(CGSize)size color:(UIColor *)color
             circleColor:(UIColor *)circleColor strokeSize:(float)strokeSize strokeColor:(UIColor *)strokeColor
{
    // Look for the cached version
    NSString *cacheKey = [NSString stringWithFormat:@"%@_%d_%d_%.1f_%0.6X_%0.6X", name,
                          (int)size.width, (int)size.height,
                          strokeSize, [color asHexRGB], [strokeColor asHexRGB]];
    
    if (id cached = [imageCache objectForKey:cacheKey])
    {
        return cached;
    }
    
    UIImage *iconImage;
    if(name.isAbsolutePath)
    {
        iconImage = [UIImage imageWithContentsOfFile:name];
    }
    else if (name)
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
    return [[MaplySimpleStyleManager shared] iconForName:name size:size color:[UIColor blackColor]
                                             circleColor:[UIColor whiteColor] strokeSize:1.0 strokeColor:[UIColor blackColor]];
}

+ (UIImage *)iconForName:(NSString *)name size:(CGSize)size color:(UIColor *)color
             circleColor:(UIColor *)circleColor strokeSize:(float)strokeSize strokeColor:(UIColor *)strokeColor
{
    return [[MaplySimpleStyleManager shared] iconForName:name size:size color:color
                                             circleColor:circleColor strokeSize:strokeSize strokeColor:strokeColor];
}

// Colors can be in short form:
//   "#ace"
// or long form
//   "#aaccee"
// with or without the # prefix.
// Colors are interpreted the same as in CSS, in #RRGGBB and #RGB order.
// But other color formats or named colors are not supported.
+ (UIColor *)parseColor:(NSString *)str default:(UIColor *)defColor
{
    const int len = [str length];
    if (len > 2)
    {
        NSScanner *scanner = [NSScanner scannerWithString:str];
        if ([str characterAtIndex:0] == '#') {
            [scanner setScanLocation:1];
        }
        unsigned int val;
        if ([scanner scanHexInt:&val]) {
            return (len < 6) ? [UIColor colorFromShortHexRGB:val] : [UIColor colorFromHexRGB:val];
        }
    }
    return defColor;
}

+ (CGFloat)parseNumber:(NSNumber *)num default:(CGFloat)defVal
{
    return num ? [num doubleValue] : defVal;
}

+ (bool)parseBool:(NSString *)val default:(bool)defVal
{
    return val ? [val boolValue] : defVal;
}

- (UIImage *)loadImage:(NSString *)symbol cacheKey:(NSString *)cacheKey
{
    UIImage *mainImage = nil;
    
    if (!symbol || [symbol length] == 0)
    {
        return nil;
    }
    
    if(symbol.isAbsolutePath)
    {
        mainImage = [UIImage imageWithContentsOfFile:symbol];
    }
    else if (symbol)
    {
        NSString *fullName = nil;
        NSString *fileName = [symbol length] > 0 ? [symbol lastPathComponent] : symbol;
        mainImage = [fileName length] > 0 ? [UIImage imageNamed:fileName] : nil;
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

// Does much the same work as the image version above, but is slightly different for the simple style
- (MaplyTexture *)textureForStyle:(MaplySimpleStyle *)style
                       backSymbol:(NSString *)backSymbol
                           symbol:(NSString *)symbol
                       strokeSize:(CGFloat)strokeSize
                           center:(CGPoint)center
                  clearBackground:(bool)clearBackground
{
    NSString *cacheKey = [NSString stringWithFormat:@"%@_%@_%d_%d_%.1f_%0.6X_%d",
                          backSymbol,
                          symbol,
                          (int)style.markerSize.width, (int)style.markerSize.height,
                          strokeSize, [style.color asHexRGB], clearBackground];
    
    id cached = [texCache objectForKey:cacheKey];
    if ([cached isKindOfClass:[MaplyTexture class]])
        return cached;

    UIImage *mainImage = [self loadImage:symbol cacheKey:[cacheKey stringByAppendingString:@"_main"]];
    UIImage *backImage = [self loadImage:backSymbol cacheKey:[cacheKey stringByAppendingString:@"_back"]];
    
    CGFloat renderScale = [UIScreen mainScreen].scale;
    CGSize renderSize = CGSizeMake(style.markerSize.width * renderScale, style.markerSize.height * renderScale);
    
    // Draw it into a circle
    UIGraphicsBeginImageContext(renderSize);
    
    // Draw into the image context
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSetBlendMode(ctx, kCGBlendModeNormal);
    CGRect rect = CGRectMake(0,0,renderSize.width,renderSize.height);

    // We want a custom background image, rather than just the circle
    if (backImage) {
        // Courtesy: https://stackoverflow.com/questions/3514066/how-to-tint-a-transparent-png-image-in-iphone
        CGContextSaveGState(ctx);
        CGContextTranslateCTM(ctx, 0, renderSize.height);
        CGContextScaleCTM(ctx, 1.0, -1.0);
        CGContextSetBlendMode(ctx, kCGBlendModeNormal);
        
        [style.color setFill];
        CGContextFillRect(ctx, rect);
        CGContextSetBlendMode(ctx, kCGBlendModeDestinationIn);
        CGContextDrawImage(ctx, rect, backImage.CGImage);
        
        CGContextRestoreGState(ctx);
    } else {
        if (strokeSize > 0.0) {
            UIColor *strokeColor = [style.color lighterColor:1.3];
            CGContextBeginPath(ctx);
            CGContextAddEllipseInRect(ctx, CGRectMake(1,1,renderSize.width-2,renderSize.height-2));
            [strokeColor setStroke];
            CGContextDrawPath(ctx, kCGPathStroke);
        }

        if (!clearBackground) {
            CGContextBeginPath(ctx);
            CGContextAddEllipseInRect(ctx, CGRectMake(1+strokeSize,1+strokeSize,renderSize.width-2-2*strokeSize,renderSize.height-2-2*strokeSize));
            [style.color setFill];
            CGContextDrawPath(ctx, kCGPathFill);
        }
    }
    
    if (mainImage) {
        CGContextTranslateCTM(ctx, 0, renderSize.height);
        CGContextScaleCTM(ctx, 1.0, -1.0);
        [[UIColor blackColor] setFill];
        CGPoint theCenter;
        CGFloat scale = backImage ? renderSize.width / backImage.size.width : 1.0;
        //CGFloat scaleY = backImage ? renderSize.height / backImage.size.height : 1.0;
        theCenter.x = center.x > -1000.0 ? center.x * scale : renderSize.width/2.0;
        theCenter.y = center.y > -1000.0 && backImage ? center.y/backImage.size.height * renderSize.height : renderSize.height/2.0;
        CGContextDrawImage(ctx, CGRectMake(theCenter.x - scale * mainImage.size.width/2.0, (renderSize.height-theCenter.y) - scale * mainImage.size.height/2.0,
                                           mainImage.size.width * scale, mainImage.size.height * scale), mainImage.CGImage);
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
    style.layoutSize = style.markerSize;
    
    // It's either a texture or a single character
    NSString *symbol = dict[@"marker-symbol"];
    if ([symbol length] == 1) {
        style.markerString = symbol;
        symbol = nil;
    }
    NSString *backSymbol = dict[@"marker-background-symbol"];
    
    style.color = [MaplySimpleStyleManager parseColor:dict[@"marker-color"] default:[UIColor whiteColor]];
    style.strokeColor = [MaplySimpleStyleManager parseColor:dict[@"stroke"] default:[UIColor colorFromHexRGB:0x555555]];
    style.strokeOpacity = [MaplySimpleStyleManager parseNumber:dict[@"stroke-opactiy"] default:1.0];
    style.strokeWidth = [MaplySimpleStyleManager parseNumber:dict[@"stroke-width"] default:2.0];
    style.fillColor = [MaplySimpleStyleManager parseColor:dict[@"fill"] default:[UIColor colorFromHexRGB:0x555555]];
    style.fillOpacity = [MaplySimpleStyleManager parseNumber:dict[@"fill-opacity"] default:0.6];
    const CGPoint center = CGPointMake(
        [MaplySimpleStyleManager parseNumber:dict[@"marker-background-center-x"] default:-1000.0],
        [MaplySimpleStyleManager parseNumber:dict[@"marker-background-center-y"] default:-1000.0]);
    if (dict[@"marker-offset-x"] || dict[@"marker-offset-y"]) {
        const CGFloat offsetX = [MaplySimpleStyleManager parseNumber:dict[@"marker-offset-x"] default:0.0];
        const CGFloat offsetY = [MaplySimpleStyleManager parseNumber:dict[@"marker-offset-y"] default:0.0];
        style.markerOffset = CGPointMake(offsetX * style.markerSize.width, offsetY * style.markerSize.height);
    }
    const bool clearBackground = [MaplySimpleStyleManager parseBool:dict[@"marker-circle"] default:true];

    // Need a texture for the marker
    const float strokeWidth = [MaplySimpleStyleManager parseNumber:dict[@"stroke-width"] default:_strokeWidthForIcons];
    style.markerTex = [self textureForStyle:style backSymbol:backSymbol symbol:symbol strokeSize:strokeWidth center:center clearBackground:clearBackground];
    // TODO: Handle the single character case
    
    return style;
}

+ (UIColor *)resolveColor:(UIColor *)color opacity:(CGFloat)opacity
{
    CGFloat red,green,blue,alpha;
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    return [UIColor colorWithRed:red*opacity green:green*opacity blue:blue*opacity alpha:alpha*opacity];
}

- (MaplyComponentObject * __nullable)addFeature:(MaplyVectorObject * __nonnull)vecObj mode:(MaplyThreadMode)mode
{
    const MaplySimpleStyle *style = [self makeStyle:vecObj.attributes];

    const auto __strong vc = viewC;
    switch ([vecObj vectorType]) {
        case MaplyVectorPointType:
            // It's a screen marker
            if (style.markerTex) {
                MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
                marker.loc = [vecObj center];
                marker.image = style.markerTex;
                marker.size = style.markerSize;
                marker.offset = style.markerOffset;
                //marker.layoutImportance = MAXFLOAT;
                return [vc addScreenMarkers:@[marker] desc:nil mode:mode];
            }
            break;
        case MaplyVectorLinearType:
            return [vc addWideVectors:@[vecObj]
                                   desc:@{kMaplyColor: [MaplySimpleStyleManager resolveColor:style.strokeColor opacity:style.strokeOpacity],
                                          kMaplyVecWidth: @(style.strokeWidth)}
                                   mode:mode];
        case MaplyVectorArealType:
            return [vc addVectors:@[vecObj]
                                   desc:@{kMaplyColor: [MaplySimpleStyleManager resolveColor:style.fillColor opacity:style.fillOpacity],
                                          kMaplyFilled: @(true)
                                   }
                                   mode:mode];
        case MaplyVectorNoneType:
        case MaplyVectorMultiType:
        case MaplyVectorLinear3dType:
            NSLog(@"Passed a MaplyVectorObject into MaplySimpleStyleManager::addFeature that has more than one type of feature.");
            break;
    }
    
    return nil;
}

- (NSArray<MaplyComponentObject *> * __nonnull)addFeatures:(NSArray<MaplyVectorObject *> * __nonnull)vecObjs mode:(MaplyThreadMode)mode
{
    NSMutableArray *compObjs = [[NSMutableArray alloc] init];
    
    for (MaplyVectorObject *vecObj in vecObjs) {
        for (MaplyVectorObject *vecObj2 in [vecObj splitVectors]) {
            if (auto compObj = [self addFeature:vecObj2 mode:mode]) {
                [compObjs addObject:compObj];
            }
        }
    }
    
    return compObjs;
}


@end

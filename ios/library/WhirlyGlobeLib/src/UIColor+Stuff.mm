/*
 *  UIColor+Stuff.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/15/11.
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

#import "UIColor+Stuff.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation UIColor(Stuff)

- (UIColor *)lighterColor
{
    return [self lighterColor:1.3];
}

// Courtesy: https://stackoverflow.com/questions/11598043/get-slightly-lighter-and-darker-color-from-uicolor
- (UIColor *)lighterColor:(float)withFactor
{
    CGFloat h, s, b, a;
    if ([self getHue:&h saturation:&s brightness:&b alpha:&a])
        return [UIColor colorWithHue:h
                          saturation:s
                          brightness:MIN(b * withFactor, 1.0)
                               alpha:a];
    return nil;
}

+ (UIColor *) colorFromHexRGB:(int)hexColor
{
    float red = (((hexColor) >> 16) & 0xFF)/255.0;
    float green = (((hexColor) >> 8) & 0xFF)/255.0;
    float blue = (((hexColor) >> 0) & 0xFF)/255.0;
    
    return [UIColor colorWithRed:red green:green blue:blue alpha:1.0];
}

+ (UIColor *) colorFromShortHexRGB:(int)hexColor
{
    int red = (((hexColor) >> 12) & 0xF); red |= red << 4;
    int green = (((hexColor) >> 4) & 0xF); green |= green << 4;
    int blue = (((hexColor) >> 0) & 0xF); blue |= blue << 4;
    
    return [UIColor colorWithRed:red/255.0f green:green/255.0f blue:blue/255.0f alpha:1.0];
}

+ (UIColor *) colorFromRGBA:(const WhirlyKit::RGBAColor &)color
{
    float red = color.r / 255.0;
    float green = color.g / 255.0;
    float blue = color.b / 255.0;
    float alpha = color.a / 255.0;

    return [UIColor colorWithRed:red green:green blue:blue alpha:alpha];
}

- (int) asHexRGB
{
  RGBAColor rgb = [self asRGBAColor];
  return (rgb.r << 16) | (rgb.g << 8)| rgb.b;
}

- (NSString *) asHexRGBAString
{
    RGBAColor rgb = [self asRGBAColor];
    return [NSString stringWithFormat:@"#%2d%2d%2d%2d",rgb.r,rgb.g,rgb.b,rgb.a];
}

- (RGBAColor) asRGBAColor
{
    RGBAColor color;
    int numComponents = (int)CGColorGetNumberOfComponents(self.CGColor);
    const CGFloat *colors = CGColorGetComponents(self.CGColor);
    
    switch (numComponents)
    {
        case 2:
            color.r = color.g = color.b = colors[0] * 255;
            color.a = colors[1] * 255;
            break;
        case 4:
            color.r = colors[0]*255;
            color.g = colors[1]*255;
            color.b = colors[2]*255;
            color.a = colors[3]*255;
            break;
        default:
            color.r = color.g = color.b = color.a = 255;
            break;
    }
    
    return color;
}

- (Vector4f) asVec4
{
    Vector4f color;
    int numComponents = (int)CGColorGetNumberOfComponents(self.CGColor);
    const CGFloat *colors = CGColorGetComponents(self.CGColor);
    
    switch (numComponents)
    {
        case 2:
            color.x() = color.y() = color.z() = colors[0] * 255;
            color.w() = colors[1] * 255;
            break;
        case 4:
            color.x() = colors[0];
            color.y() = colors[1];
            color.z() = colors[2];
            color.w() = colors[3];
            break;
        default:
            color.x() = color.y() = color.z() = color.w() = 255;
            break;
    }
    
    return color;
}

@end

void UIColorDummyFunc()
{
}

/*
 *  MaplyTextureBuilder.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/30/14.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MaplyTextureBuilder.h"
#import <vector>
#import <WhirlyGlobe.h>

@implementation MaplyLinearTextureBuilder
{
    CGSize size;
    std::vector<int> elements;
}

- (id)initWithSize:(CGSize)inSize
{
    self = [super init];
    if (!self)
        return nil;
    
    size = inSize;
    _opacityFunc = MaplyOpacitySin3;
    
    return self;
}

- (void)setPattern:(NSArray *)inElements
{
    for (NSNumber *num in inElements)
    {
        if (![num isKindOfClass:[NSNumber class]])
            return;
        
        elements.push_back([num integerValue]);
    }
}

- (UIImage *)makeImage
{
    // Need to scale these elements to the texture size
    int eleSum = 0;
    for (unsigned int ii=0;ii<elements.size();ii++)
        eleSum += elements[ii];
    
    if (eleSum == 0)
        return nil;
    
    UIGraphicsBeginImageContext(size);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    CGContextClearRect(ctx, CGRectMake(0, 0, size.width, size.height));
    [[UIColor whiteColor] setFill];
    [[UIColor whiteColor] setStroke];
    
    // Precalculate the opacity values since they're the same for every row
    std::vector<float> opacityVals;
    opacityVals.resize((int)size.width);
    for (unsigned int ii=0;ii<opacityVals.size();ii++)
    {
        float opacityVal = 0.0;
        float t = ii/(float)(size.width-1);
        switch (_opacityFunc)
        {
            case MaplyOpacityFlat:
                opacityVal = 1.0;
                break;
            case MaplyOpacityLinear:
                opacityVal = (t < 0.5 ? 2*t : (1.0-t)*2);
                break;
            case MaplyOpacitySin1:
            {
                float sinVal = sinf((ii/(float)(size.width-1))*M_PI);
                opacityVal = (sinVal > 0.0 ? sinVal : 0.0);
            }
                break;
            case MaplyOpacitySin2:
            {
                float sinVal = sinf((ii/(float)(size.width-1))*M_PI);
                opacityVal = (sinVal > 0.0 ? powf(sinVal,0.5) : 0.0);
            }
                break;
            case MaplyOpacitySin3:
            {
                float sinVal = sinf((ii/(float)(size.width-1))*M_PI);
                opacityVal = (sinVal > 0.0 ? powf(sinVal,0.33) : 0.0);
            }
                break;
        }
        opacityVals[ii] = opacityVal;
    }
    
    // Work our way through the elements
    int curY = 0;
    bool onOrOff = 1;
    for (unsigned int ii=0;ii<elements.size();ii++)
    {
        int eleLen = elements[ii];
        if (onOrOff)
            for (unsigned int jj=0;jj<eleLen;jj++)
            {
                for (unsigned int xx=0;xx<size.width;xx++)
                {
                    float opacity = opacityVals[xx];
                    // Do a gradiant across the image
                    [[UIColor colorWithWhite:1.0 alpha:opacity] setFill];
                    [[UIColor colorWithWhite:1.0 alpha:opacity] setStroke];
                    CGContextFillRect(ctx, CGRectMake(xx, (curY+jj)/(float)eleSum * size.height, 1, 1));
                }
            }
        onOrOff = !onOrOff;
        curY += eleLen;
    }
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    return image;
}

@end

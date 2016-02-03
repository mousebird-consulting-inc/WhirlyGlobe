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
    int size;
    std::vector<int> elements;
}

- (id)init
{
    self = [super init];
    
    size = 0;
    
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
    size = eleSum;
    
    if (size == 0)
        return nil;
    
    // Note: If OpenGL ES doesn't like this size, just bump it up
    int width = 1;
    CGSize imgSize = CGSizeMake(width, size);
    UIGraphicsBeginImageContext(imgSize);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    CGContextClearRect(ctx, CGRectMake(0, 0, imgSize.width, imgSize.height));
    [[UIColor whiteColor] setFill];
    [[UIColor whiteColor] setStroke];
    
    // Work our way through the elements
    int curY = 0;
    bool onOrOff = 1;
    for (unsigned int ii=0;ii<elements.size();ii++)
    {
        int eleLen = elements[ii];
        if (onOrOff)
            for (unsigned int jj=0;jj<eleLen;jj++)
            {
                CGContextFillRect(ctx, CGRectMake(0.0, (curY+jj)/(float)eleSum * imgSize.height, width, 1));
            }
        onOrOff = !onOrOff;
        curY += eleLen;
    }
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    return image;
}

@end

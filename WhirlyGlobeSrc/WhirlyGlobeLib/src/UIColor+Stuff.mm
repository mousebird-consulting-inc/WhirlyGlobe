/*
 *  UIColor+Stuff.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/15/11.
 *  Copyright 2011-2012 mousebird consulting
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

@implementation UIColor(Stuff)

- (RGBAColor) asRGBAColor
{
    RGBAColor color;
    int numComponents = CGColorGetNumberOfComponents(self.CGColor);
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
            color.a = 255;
            break;
    }
    
    return color;
}

@end

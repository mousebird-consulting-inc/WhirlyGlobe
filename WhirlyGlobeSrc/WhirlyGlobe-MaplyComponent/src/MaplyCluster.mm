/*
 *  MaplyCluster.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/29/15.
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

#import "MaplyCluster.h"
#import "MaplyBaseViewController.h"

@implementation MaplyClusterInfo
@end

@implementation MaplyClusterGroup
@end

@implementation MaplyBasicClusterGenerator
{
    NSArray *colors;
    CGSize size;
    UIFont *font;
    float scale;
    NSMutableDictionary *imagesByNumber;
    MaplyBaseViewController * __weak viewC;
}

- (nonnull instancetype)initWithColors:(NSArray *__nonnull)inColors clusterNumber:(int)clusterNumber size:(CGSize)markerSize viewC:(MaplyBaseViewController *)inViewC;
{
    self = [super init];

    if (self)
    {
        colors = inColors;
        size = markerSize;
        self.clusterNumber = clusterNumber;
        scale = [UIScreen mainScreen].scale;
        font = [UIFont boldSystemFontOfSize:markerSize.height*scale/2.0];
        if ([colors count] == 0)
            return nil;
        self.clusterLayoutSize = markerSize;
        self.selectable = true;
        self.markerAnimationTime = 0.2;
        viewC = inViewC;
    }
    
    return self;
}

- (void)startClusterGroup
{
    imagesByNumber = [NSMutableDictionary dictionary];    
}

- (MaplyClusterGroup *__nonnull) makeClusterGroup:(MaplyClusterInfo *__nonnull)clusterInfo
{
    MaplyClusterGroup *group = [[MaplyClusterGroup alloc] init];
    
    UIImage *image = imagesByNumber[@(clusterInfo.numObjects)];
    if (!image)
    {
        // Note: Pick the color based on number of markers
        UIColor *color = [colors objectAtIndex:0];
        
        CGSize scaleSize = CGSizeMake(size.width * scale, size.height * scale);
        UIGraphicsBeginImageContext(scaleSize);
        
        // Clear out the background
        [[UIColor clearColor] setFill];
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextFillRect(ctx, CGRectMake(0,0,scaleSize.width,scaleSize.height));

        CGContextBeginPath(ctx);
        [color setFill];
        [[UIColor whiteColor] setStroke];
        CGContextAddEllipseInRect(ctx, CGRectMake(1, 1, scaleSize.width-2, scaleSize.height-2));
        CGContextDrawPath(ctx, kCGPathFillStroke);
        
        [[UIColor whiteColor] setFill];
        [[UIColor whiteColor] setStroke];
        NSString *numStr = [NSString stringWithFormat:@"%d",clusterInfo.numObjects];
        CGSize textSize = [numStr sizeWithAttributes:@{NSFontAttributeName: font}];
        [numStr drawAtPoint:CGPointMake((scaleSize.width-textSize.width)/2.0,(scaleSize.height-textSize.height)/2.0) withFont:font];
    
        image = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
        imagesByNumber[@(clusterInfo.numObjects)] = image;
    }

    group.image = image;
    group.size = size;
    
    return group;
}

- (void)endClusterGroup
{
    imagesByNumber = nil;
}

@end

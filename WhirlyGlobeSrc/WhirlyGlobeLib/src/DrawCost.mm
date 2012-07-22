//
//  DrawCost.m
//  WhirlyGlobeLib
//
//  Created by Stephen Gifford on 7/11/11.
//  Copyright 2011 mousebird consulting. All rights reserved.
//

#import "DrawCost.h"

@implementation WhirlyKitDrawCost

@synthesize numDrawables;
@synthesize numTextures;

- (id)init
{
    if ((self = [super init]))
    {
        numDrawables = 0.0;
        numTextures = 0.0;
    }
    
    return self;
}

@end

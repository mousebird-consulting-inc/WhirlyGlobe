/*
 *  MaplyTexture_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/25/13.
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

#import <Foundation/Foundation.h>
#import "MaplyTexture.h"
#import "MaplyBaseViewController.h"
#import "WhirlyGlobe.h"

@class MaplyBaseInteractionLayer;

@interface MaplyTexture()

// The view controller the texture is nominally associated with
@property (nonatomic,weak) MaplyBaseInteractionLayer *interactLayer;

// If this came from a UIImage, the UIImage it came from
@property (nonatomic,weak) UIImage *image;

// Set if this is a sub texture reference
@property (nonatomic) bool isSubTex;

// If set, the texture ID associated with this texture
@property (nonatomic) WhirlyKit::SimpleIdentity texID;

// Clear out the texture we're holding
- (void)clear;

@end

/*
 *  QuadDisplayLayerNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/26/18.
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

#import <Foundation/Foundation.h>
#import <math.h>
#import "QuadDisplayControllerNew.h"
#import "DataLayer.h"

/** This layer turns view state updates into quad tree tiles to load.
  */
@interface WhirlyKitQuadDisplayLayerNew : NSObject<WhirlyKitLayer>

/// Layer thread we're attached to
@property (nonatomic,weak,readonly,nullable) WhirlyKitLayerThread *layerThread;

/// Construct with a renderer and data source for the tiles
- (nonnull)initWithController:(WhirlyKit::QuadDisplayControllerNewRef)controller;

- (WhirlyKit::QuadDisplayControllerNewRef)getController;

@end

/*
 *  MaplyViewControllerLayer.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/25/12.
 *  Copyright 2011-2013 mousebird consulting
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

/** @brief The View Controller Layer is a base class for other display layers.
    @details You don't create these directory.  This is a base class for things like the MaplyQuadPagingLayer and the MaplyQuadImageTilesLayer.
  */
@interface MaplyViewControllerLayer : NSObject

/** @brief Set the priority for drawing.
    @details This is how you control where the geometry produced by this layer shows up with respect to other layers and other geometry.  This must be set immediately after creation.  It will have undefined behavior after the layer has started.
  */
@property (nonatomic,assign) int drawPriority;

@end

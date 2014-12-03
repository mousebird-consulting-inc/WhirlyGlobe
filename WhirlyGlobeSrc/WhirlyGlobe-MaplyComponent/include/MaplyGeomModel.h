/*
 *  MaplyGeomModel.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 11/26/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import <UIKit/UIKit.h>
#import "MaplyCoordinate.h"
#import "MaplyMatrix.h"

/** @brief Contains a big pile of geometry and textures (e.g. a model).
    @details The geometry model
 */
@interface MaplyGeomModel : NSObject

/** @brief Initialize with the full path to a Wavefront OBJ model file.
    @details This creates a model from a Wavefront OBJ file, a standard, simple file format for models.  You can then instance and place this model where you might like.
  */
- (id)initWithObj:(NSString *)fullPath;

@end


/** @brief Place a geometry model at a given location
    @details Geometry models tend to be expensive so we load and place them in a two step process.  First you create the MaplyGeomModel and then you place it in one or more spots with this MaplyGeomModelInstance.
  */
@interface MaplyGeomModelInstance : NSObject

/// @brief The model to instance
@property (nonatomic) MaplyGeomModel *model;

/** @brief Where we'd like to place the instanced model.
    @details This is the center of the object in geographic radians.
 */
@property (nonatomic) MaplyCoordinate center;

/// @brief Transform used to oriented the model instance
@property (nonatomic) MaplyMatrix *transform;

/// @brief Set if you want to select these
@property (nonatomic) bool selectable;

@end

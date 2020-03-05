/*
 *  MaplyGeomModel.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 11/26/14.
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

#import <UIKit/UIKit.h>
#import "math/MaplyCoordinate.h"
#import "math/MaplyMatrix.h"
#import "visual_objects/MaplyShape.h"

@class MaplyShader;

/** 
    Contains a big pile of geometry and textures (e.g. a model).
    
    The geometry model
 */
@interface MaplyGeomModel : NSObject

/** 
    Initialize with the full path to a Wavefront OBJ model file.
    
    This creates a model from a Wavefront OBJ file, a standard, simple file format for models.  You can then instance and place this model where you might like.
  */
- (nullable instancetype)initWithObj:(NSString *__nonnull)fullPath;

/** 
    Initialize with a shape.
    
    The given shape will be turned into a geometry model so it can be instanced.
  */
- (nonnull instancetype)initWithShape:(MaplyShape *__nonnull)shape;

@end


/** 
    Place a geometry model at a given location
    
    Geometry models tend to be expensive so we load and place them in a two step process.  First you create the MaplyGeomModel and then you place it in one or more spots with this MaplyGeomModelInstance.
  */
@interface MaplyGeomModelInstance : NSObject

/** 
    User data object for selection
 
    When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the model instance means to them.
 */
@property (nonatomic,strong,nullable) id userObject;

/// The model to instance
@property (nonatomic,strong,nullable) MaplyGeomModel *model;

/** 
    Where we'd like to place the instanced model.
    
    This is the center of the object in geographic radians.
 */
@property (nonatomic) MaplyCoordinate3d center;

/// Transform used to oriented the model instance
@property (nonatomic,strong,nullable) MaplyMatrix *transform;

/// Color to force all polygons to use.
/// If set, this will force all polygons to use this color.  nil by default.
@property (nonatomic,strong,nullable) UIColor *colorOverride;

/// Set if you want to select these
@property (nonatomic) bool selectable;

@end

/** 
    A version of the geometry model instance that moves.
    
    This version of the geometry model instance can move in a limited way over time.
  */
@interface MaplyMovingGeomModelInstance : MaplyGeomModelInstance

/// The end point for animation
@property (nonatomic,assign) MaplyCoordinate3d endCenter;

/// How long it will take to get to the endCenter
@property (nonatomic,assign) NSTimeInterval duration;

@end

/**
 Sometimes we don't know how many instances there will be of a model until
 some logic runs on the GPU.  We can then take that number and run that
 number of instances of the given model.  [Metal only]
 */
@interface MaplyGeomModelGPUInstance : NSObject

/// The model to instance
@property (nonatomic,strong,nullable) MaplyGeomModel *model;

/// Texture to derive the number of instances from (picked out of the highest, smallest level)
@property (nonatomic,nullable) MaplyTexture *numInstSource;

/// Need a shader to pull the number of instances out of a texture and shove them in the indirect buffer
@property (nonatomic,nullable) MaplyShader *numInstShader;

/// Shader to run over for this instance
@property (nonatomic,nullable) MaplyShader *shader;

@end

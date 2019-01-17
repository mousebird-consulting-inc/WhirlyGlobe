/*
 *  MaplyLight.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/30/13.
 *  Copyright 2012-2017 mousebird consulting
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

/** 
    The Light provides a simple interface to basic lighting within the toolkit.
    
    You can have up to 8 lights in the current version of the toolkit.  Obviously this is all shader implementation with OpenGL ES 2.0, so you can always just bypass this and do what you like.  However, the default shaders will look for these lights.
    
    The lights are very simple, suitable for the globe, and contain a position, a couple of colors, and a view dependent flag.
  */
@interface MaplyLight : NSObject

/** 
    The location of this particular light in display space.
    
    This is a single light's location in display space.  Display space for the globe is based on a radius of 1.0.
  */
@property (nonatomic,assign) MaplyCoordinate3d pos;

/** 
    Controls whether the light takes the model matrix into account or not.
    
    If set, this light moves with the model (usually the globe).  You'd use this for a real sun position. If not set, the light is static and does not move or rotate.
  */
@property (nonatomic,assign) bool viewDependent;

/** 
    Ambient color for the light.
    
    This color will always be added to any given pixel.  It provides a baseline lighting value.
  */
@property (nonatomic,strong) UIColor *__nullable ambient;

/** 
    Diffuse light color.
    
    The diffuse color is multiplied by a directional value and so will vary depending on geometry normals.
  */
@property (nonatomic,strong) UIColor *__nullable diffuse;

@end

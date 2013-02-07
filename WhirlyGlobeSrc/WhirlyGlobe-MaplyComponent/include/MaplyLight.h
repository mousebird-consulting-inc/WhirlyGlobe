/*
 *  MaplyLight.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/30/13.
 *  Copyright 2012 mousebird consulting
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

/** The Maply Light can be used to control lighting with the globe or map.
  */
@interface MaplyLight : NSObject
{
    /// Light position relative to the globe or map
    MaplyCoordinate3d pos;
    /// If set, this light moves with the model (usually the globe).  You'd use this for a real sun position.
    /// If not set, the light is static and does not move or rotate.
    bool viewDependent;
    /// Ambient color.  This is always present so no pixel should be darker than this
    UIColor *ambient;
    /// The diffuse color is multiplied with the light vector and so is directional.
    UIColor *diffuse;
}

@property (nonatomic,assign) MaplyCoordinate3d pos;
@property (nonatomic,assign) bool viewDependent;
@property (nonatomic,strong) UIColor *ambient,*diffuse;

@end

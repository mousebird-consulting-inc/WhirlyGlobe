/*
 *  MaplyVertexAttribute.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 11/29/13.
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

#import <UIKit/UIKit.h>

/** @brief Vertex Attributes are passed all the way though on objects to shaders.
    @details If you have your own custom shader, you often need a way to feed it data.  The toolkit will set up the standard data, like vertices, texture coordinates and normals, but sometimes you need something custom.
    @details Vertex attributes are the mechanism you use to pass that custom data all the way down to the shader.
    @details How the vertex attributes are used depends on the data type, so consult the appropriate object.
  */
@interface MaplyVertexAttribute : NSObject

/// Construct a vertex attribute as a single float
- (nonnull instancetype)initWithName:(NSString * __nonnull)name float:(float)val;

/// Construct a vertex attribute as two floats
- (nonnull instancetype)initWithName:(NSString * __nonnull)name floatX:(float)x y:(float)y;

/// Construct a vertex attribute as three flaots
- (nonnull instancetype)initWithName:(NSString * __nonnull)name floatX:(float)x y:(float)y z:(float)z;

/// Construct a vertex attribute as an RGBA value
- (nonnull instancetype)initWithName:(NSString * __nonnull)name color:(UIColor * __nonnull)color;

/// Construct a vertex attribute as an int
- (nonnull instancetype)initWithName:(NSString * __nonnull)name int:(int)val;

@end

/*
 *  MaplyMatrix.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/16/14.
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

#import <Foundation/Foundation.h>

/** @brief Represents a matrix for position manipulation.
    @details Encapsulates a 4x4 matrix used for object placement and other things.  This is more a wrapper than a full service object.
  */
@interface MaplyMatrix : NSObject

/** @brief Construct with yaw, pitch, and roll parameters.
    @details Construct the matrix with the standard yaw, pitch, and roll used by aircraft.
  */
- (id)initWithYaw:(double)yaw pitch:(double)pitch roll:(double)roll;

/** @brief Construct with a consistent scale in each dimension.
    @details Construct with the same scale in x,y, and z.
  */
- (id)initWithScale:(double)scale;

/** @brief Construct a rotation around the given axis.
    @details Build a matrix that rotates the given amount (in radians) around the given axis.
  */
- (id)initWithAngle:(double)ang axisX:(double)x axisY:(double)y axisZ:(double)z;

/** @brief Multiply the given matrix with this one and return a new one.
    @details Multiply the given matrix like so:  ret = this * other.  Return the new one.
  */
- (id)multiplyWith:(MaplyMatrix *)other;

@end

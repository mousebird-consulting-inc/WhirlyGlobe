/*
 *  MaplyShape.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/28/12.
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
#import "WGCoordinate.h"

/** This will display a circle at the given location
    with the given radius.
 */
@interface MaplyShapeCircle : NSObject

/// Center of the circle in local coordinates
@property (nonatomic,assign) MaplyCoordinate center;
/// Radius of the circle in display units.
/// For WhirlyGlobe, remember that the radius of the sphere is 1.0.
@property (nonatomic,assign) float radius;
/// Height above the globe.  This is in units of radius = 1.0
@property (nonatomic,assign) float height;
/// Optional color
@property (nonatomic,strong) UIColor *color;

@end

typedef MaplyShapeCircle WGShapeCircle;

/** This will display a sphere at the given location
    with the given radius.
  */
@interface MaplyShapeSphere : NSObject

/// Put yer user data here
@property (nonatomic,strong) NSObject *userObject;
/// Center of the sphere in local coordinates
@property (nonatomic,assign) MaplyCoordinate center;
/// Radius in display units (1.0 is the size of the earth)
@property (nonatomic,assign) float radius;
/// Offset from the globe (in display units)
@property (nonatomic,assign) float height;
/// Optional color
@property (nonatomic,strong) UIColor *color;
/// If set, this cylinder can be selected.  On by default.
@property (nonatomic,assign) bool selectable;

@end

typedef MaplyShapeSphere WGShapeSphere;

/** This will display a cylinder with its base at the
    given loation with the given radius and height.
 */
@interface MaplyShapeCylinder : NSObject

/// Put yer user data here
@property (nonatomic,strong) NSObject *userObject;
/// Center of the base in local coordinates
@property (nonatomic,assign) MaplyCoordinate baseCenter;
/// An optional height offset for the base (e.g. cylinder starts at this height)
@property (nonatomic,assign) float baseHeight;
/// Radius in display units (1.0 is the size of the earth)
@property (nonatomic,assign) float radius;
/// Height in display units
@property (nonatomic,assign) float height;
/// Optional color
@property (nonatomic,strong) UIColor *color;
/// If set, this cylinder can be selected.  On by default.
@property (nonatomic,assign) bool selectable;

@end

typedef MaplyShapeCylinder WGShapeCylinder;

/** A great circle with start and end points
    and a height we'll reach above the globe in the middle.
 */
@interface MaplyShapeGreatCircle : NSObject

/// Put yer user data here
@property (nonatomic,strong) NSObject *userObject;
/// Start and end points in geographic
@property (nonatomic,assign) MaplyCoordinate startPt,endPt;
/// Height is related to radius == 1.0 for the earth
@property (nonatomic,assign) float height;
/// Line width is in pixels
@property (nonatomic,assign) float lineWidth;
/// Optional color
@property (nonatomic,strong) UIColor *color;

/// Return the angle between the two points in radians
- (float)calcAngleBetween;

@end

/** A linear feature that's offset from the globe.
    The feature can be arbitrarily large.  The first two
    values of each coordinate are lon/lat and the third
    is a Z offset in display units.
  */
@interface MaplyShapeLinear : NSObject
/// Line width in pixels
@property (nonatomic,assign) float lineWidth;
/// Optional color
@property (nonatomic,strong) UIColor *color;

/// Initialize with the coordinate data (will be copied in)
- (id)initWithCoords:(MaplyCoordinate3d *)coords numCoords:(int)numCoords;

/// Return the coordinates
- (int)getCoords:(MaplyCoordinate3d **)retCoords;

@end

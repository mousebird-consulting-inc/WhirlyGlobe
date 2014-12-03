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
#import "MaplyMatrix.h"

/** @brief Maply Shape is the base class for the actual shape objects.
    @details The maply shape is just the base class.  Look to MaplyShapeCircle, MaplyShapeCylinder, MaplyShapeSphere, MaplyShapeGreatCircle, and MaplyShapeLinear.
  */
@interface MaplyShape : NSObject

/** @brief The color of the shape.
    @details We can set object color in the NSDictionary passed in with the add method.  We can also override that here.
 */
@property (nonatomic,strong) UIColor *color;

/** @brief If set, the object is selectable
 @details The object is selectable if this is set when the object is passed in to an add call.  If not set, you'll never see it in selection.
 */
@property (nonatomic,assign) bool selectable;

/** @brief User data object for selection
 @details When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the shape means to them.
 */
@property (nonatomic,strong) id userObject;

@end

/** @brief Shows a circle at the given location on the globe or map.
    @details This object represents a circle at the given geographic location.  It needs a radius (in display coordinates) and can optionally have a height above the globe or map.
 */
@interface MaplyShapeCircle : MaplyShape

/** @brief Center of the circle in local coordinates.
    @details This is the center of the circle in geographic.
  */
@property (nonatomic,assign) MaplyCoordinate center;

/** @brief Radius of the circle in display units.
    @details This is the radius of the circle, but not in geographic.  It's in display units.  Display units for the globe are based on a radius of 1.0.  Scale accordingly.  For the map, display units typically run from -PI to +PI, depending on the coordinate system.
  */
@property (nonatomic,assign) float radius;

/** @brief Offset height above the globe in display units.
    @details This is the height above the globe for the center of the circle.  It's also in display units, like the radius.
  */
@property (nonatomic,assign) float height;

@end

typedef MaplyShapeCircle WGShapeCircle;

/** @brief Display a sphere at the given location with the given radius.
    @details This object represents a sphere at the
  */
@interface MaplyShapeSphere : MaplyShape

/** @brief Center of the sphere in local coordinates.
    @details The x and y coordinates correspond to longitude and latitude and are in geographic (radians).  The Z value is in display coordinates.  For that globe that's based on a radius of 1.0.  Scale accordingly.
 */
@property (nonatomic,assign) MaplyCoordinate center;

/** @brief Radius of the sphere in display units.
    @details This is the radius of the sphere, but not in geographic.  It's in display units.  Display units for the globe are based on a radius of 1.0.  Scale accordingly.  For the map, display units typically run from -PI to +PI, depending on the coordinate system.
 */
@property (nonatomic,assign) float radius;

/** @brief Offset height above the globe in display units.
    @details This is the height above the globe for the center of the sphere.  It's also in display units, like the radius.
 */
@property (nonatomic,assign) float height;

@end

typedef MaplyShapeSphere WGShapeSphere;

/** @brief Represent a cyclinder on the globe or map.
    @details This object represents a cylinder with it's base tied to the surface of the globe or map and it's top pointed outward (on the globe anyway).  The base can be offset and the overall radius and height are adjustable.
  */
@interface MaplyShapeCylinder : MaplyShape

/** @brief Center of the cylinder's base in geographic.
    @details The x and y coordinates correspond to longitude and latitude and are in geographic (radians).
 */
@property (nonatomic,assign) MaplyCoordinate baseCenter;

/** @brief Base height above the globe in display units.
    @details This is an optional base offset from the globe or map.  The cylinder will be offset by this amount.  It's also in display units, like the radius.
 */
@property (nonatomic,assign) float baseHeight;

/** @brief Radius of the cylinder in display units.
    @details This is the radius of the cylinder, but not in geographic.  It's in display units.  Display units for the globe are based on a radius of 1.0.  Scale accordingly.  For the map, display units typically run from -PI to +PI, depending on the coordinate system.
 */
@property (nonatomic,assign) float radius;

/** @brief Height of the cylinder in display units.
    @details This is the height of the cylinder.  The top of the cylinder will be at baseHeight+height.  It's also in display units, like the radius.
 */
@property (nonatomic,assign) float height;

@end

typedef MaplyShapeCylinder WGShapeCylinder;

/** @brief Represent an great circle or great circle with height.
    @details Great circles are the shortest distance between two points on a globe.  We extend that a bit here, by adding height.  The result is a curved object that can either sit on top of the globe or rise above it.  In either case it begins and ends at the specified points on the globe.
 */
@interface MaplyShapeGreatCircle : MaplyShape

/// @brief Starting point in geographic coordinates.
@property (nonatomic,assign) MaplyCoordinate startPt;

/// @brief End point in geographic coordinates
@property (nonatomic,assign) MaplyCoordinate endPt;

/** @brief Height of the great circle shape right in its middle.
    @details This is the height of the great circle right in the middle.  It will built toward that height and then go back down as it reaches the endPt.  The height is in display units.  For the globe that's based on a radius of 1.0.
 */
@property (nonatomic,assign) float height;

/** @brief Line width for the great circle geometry.
    @details The great circle is implemented as a set of lines. This is the width, in pixels, of those lines.
  */
@property (nonatomic,assign) float lineWidth;

/** @brief Angle between start and end points in radians
  */
- (float)calcAngleBetween;

@end

/** @brief A linear feature offset from the globe.
    @details The main difference between this object and a regular MaplyVectorObject is that you specify coordiantes in 3D.  You can use this to create linear features that are offset from the globe.
  */
@interface MaplyShapeLinear : MaplyShape

/** @brief Line width in pixels
    @details The linear is implemented as a set of line segments in OpenGL ES.  This is their line width in pixels.
  */
@property (nonatomic,assign) float lineWidth;

/** @brief Initialize with coordinates and coordinate array size
    @details This initializer will make a copy of the coordinates and use them to draw the lines.  The x and y values are in geographic.  The z values are offsets from the globe (or map) and are in display units.  For the globe display units are based on a radius of 1.0.
  */
- (id)initWithCoords:(MaplyCoordinate3d *)coords numCoords:(int)numCoords;

/** @brief Return the coordinates for this linear feature.
    @return Returns the number of coordinates and a pointer to the coordinate array.
  */
- (int)getCoords:(MaplyCoordinate3d **)retCoords;

@end

/** @brief An extruded shape with an arbitrary outline.
    @details This object represents an extruded shape with the given thickness.  It can be oriented according to the pitch, roll, yaw and height.
  */
@interface MaplyShapeExtruded : MaplyShape

/** @brief Construct with the coordinates for the outline to extrude.
    @details Pass in pairs of doubles that correspond to the
  */
- (id)initWithOutline:(double *)coords numCoordPairs:(int)numCoordPairs;

/** @brief Number of coordinate pairs in this shape.
  */
@property (nonatomic,readonly) int numCoordPairs;

/** @brief Array of coordinate values.
  */
@property (nonatomic,readonly) double *coordData;

/** @brief Where we'd like to place the extruded shape.
    @details This is the center of the object in geographic radians.
  */
@property (nonatomic) MaplyCoordinate center;

/** @brief Scale for coordinates.  Set to 1/EarthRadiusInMeters by default.
    @details The coordinates will be scaled by this before creating the geometry.  By default this is set to 1/EarthRadius(m) so you can build your shape in meters.  Also applies to thickness and height.
  */
@property (nonatomic,assign) double scale;

/** @brief Thickness for the resulting shape.
    @details We build an extruded shape out of this information and this is its thickness.  If this is zero, you just get a double sided polygon.
  */
@property (nonatomic,assign) double thickness;

/** @brief Height to place the resulting shape at.
    @details We'll put this shape at the given height above the surface of the globe or map.
  */
@property (nonatomic,assign) double height;

/** @brief The transform to apply to this shape.
    @details If set, this transform is applied before placing the feature.  You can set a transform matrix up with roll, pitch, and yaw.
  */
@property (nonatomic) MaplyMatrix *transform;

@end

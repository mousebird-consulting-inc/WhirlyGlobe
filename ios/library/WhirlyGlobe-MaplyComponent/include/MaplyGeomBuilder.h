/*
 *  MaplyGeomModelBuilder.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 1/20/16
 *  Copyright 2011-2017 mousebird consulting
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
#import "MaplyBaseViewController.h"
#import "MaplyGeomModel.h"

/** 
    Geometry State is used to describe the visual look of objects as they're added.
    
    Set the various fields in the geometry state to control how geometry looks when you add it.
            There are defaults for all of these fields.
  */
@interface MaplyGeomState : NSObject

/// Color to use for the geometry
@property (nonatomic,strong) UIColor *color;

/// Background color for text
@property (nonatomic,strong) UIColor *backColor;

/// UIImage for MaplyTexture to apply to geometry
@property (nonatomic,strong) id texture;

@end

/** 
    The geometry builder is used to construct 3D models.
    
    You use the geometry builder when you have a custom 3D model to build.  This can include things like airport runways, buildings, or anything else that's particular to a certain location.
    
    Start by construting the builder and then adding polygons and strings to it.
    
    You can combine multiple geometry builders together to build up repeated portions of a model.
    
    The builder has an "immediate mode" where you add points individually to build up polygons and then add those.  This is one of the simpler ways to use the system.
  */
@interface MaplyGeomBuilder : NSObject

/// Intialize with the view controller the model will be used in.
- (id)initWithViewC:(MaplyBaseViewController *)viewC;

/** 
    Create a rectangle around the origin.
    
    This creates a flat rectangle around the origin (0,0,0) with z = 0.
    
    @param size The size of the rectangle (x,y).
    
    @param state The visual state to use when creating the geometry.
  */
- (void)addRectangleAroundOrigin:(MaplyCoordinateD)size state:(MaplyGeomState *)state;

/** 
    Create a rectangle around the origin.
 
    This creates a flat rectangle around the origin (0,0,0) with z = 0.
 
    @param x Horizontal size of the rectangle.
 
    @param y Vertical size of the rectangle.
 
    @param state The visual state to use when creating the geometry.
 */
- (void)addRectangleAroundOriginX:(double)x y:(double)y state:(MaplyGeomState *)state;

/** 
    Create a rectangle around the given point.
 
    This creates a flat rectangle around the point x,y with z = 0.
 
    @param x X location around which to create the rectangle.
 
    @param y Y location around which to create the rectangle.
 
    @param width Horizontal size of the rectangle.
 
    @param height Vertical size of the rectangle.
 
    @param state The visual state to use when creating the geometry.
 */
- (void)addRectangleAroundX:(double)x y:(double)y width:(double)width height:(double)height state:(MaplyGeomState *)state;

/** 
    Add an attributed string to the geometry builder.
    
    Add an attributed string, which contains information about color and front to the geometry builder.
    
    @param str String to add to the geometry.
    
    @param state The visual state to use when creating the geometry.
  */
- (void)addAttributedString:(NSAttributedString *)str state:(MaplyGeomState *)state;

/** 
    Add a string to the geometry.
    
    Add a string at (0,0) to the geometry with the given font and visual state.  The font determines size.
    
    @param str String to add to geometry.
    
    @param font Font to use for the string.
    
    @param state The visual state to use when creating the geometry.
  */
- (void)addString:(NSString *)str font:(UIFont *)font state:(MaplyGeomState *)state;

/** 
    Add a string to the geometry.
 
    Add a string at (0,0) to the geometry with the given font and visual state.
 
    The string will be scaled to match the width and/or height given.  If one of width or height is 0.0 it will be calculated from the other.
 
    @param str String to add to geometry.
 
    @param width Width of the string in the final geometry.  If set to 0.0, will be calculated.
 
    @param height Height of the string in the final geometry.  If set to 0.0, will be calculated.
 
    @param font Font to use for the string.
 
    @param state The visual state to use when creating the geometry.
 */
- (void)addString:(NSString *)str width:(double)width height:(double)height font:(UIFont *)font state:(MaplyGeomState *)state;

/** 
    Add a polygon with the given visual state.
    
    Tesselates the given polygon and adds the resulting triangles to the geometry.
    
    @param pts An array of 3D points.
    
    @param numPts Number of points in the 3D array.
    
    @param state The visual state to use when creating the geometry.
  */
- (void)addPolygonWithPts:(MaplyCoordinate3dD *)pts numPts:(int)numPts state:(MaplyGeomState *)state;

/** 
    Add a polygon with the given visual state.
 
    Tesselates the given polygon and adds the resulting triangles to the geometry.
 
    @param pts An array of 3D points.
 
    @param tex Texture coordinates.  One for each point.
 
    @param norms Normals to go with the points.
 
    @param numPts Number of points in the 3D array.
 
    @param state The visual state to use when creating the geometry.
 */
- (void)addPolygonWithPts:(MaplyCoordinate3dD *)pts tex:(MaplyCoordinateD *)tex norms:(MaplyCoordinate3dD *)norms numPts:(int)numPts state:(MaplyGeomState *)state;

/** 
    Add a point in immediate mode.
    
    When in immediate mode points are collected until an addCurPoly call.  This adds a 3D point.
  */
- (void)addCurPointX:(double)x y:(double)y z:(double)z;

/** 
    Add a point in immediate mode
    
    When in immediate mode points are collected until an addCurPoly call.  This add a point at (x,y,0).
  */
- (void)addCurPointX:(double)x y:(double)y;

/** 
    Add the current polygon in immediate mode.
    
    When in immediate mode points are collected until you call this method.  Then the points will be tesselated into a set of triangles and added to the geometry with the given visual state.
  */
- (void)addCurPoly:(MaplyGeomState *)state;

/** 
    Scale the geometry and strings by this amount.
    
    The geometry and strings are scaled by the given amount in each dimension.
  */
- (void)scale:(MaplyCoordinate3dD)scale;

/** 
    Scale the geometry and strings by the given amount.
    
    The geometry and strings are scaled by (x,y,z).
  */
- (void)scaleX:(double)x y:(double)y z:(double)z;

/** 
    Translate the geometry and strings by the given amount.
    
    The geometry and strings are translated by the given coordinate.
  */
- (void)translate:(MaplyCoordinate3dD)trans;

/** 
    Translate the geometry and strings by the given amount.
    
    The geometry and strings are translated by (x,y,z).
 */
- (void)translateX:(double)x y:(double)y z:(double)z;

/** 
    Rotate the geometry around the given axis by the given angle.
    
    Roate around the given 3D axis by the number of radians in angle.
    
    @param angle The angle in radians to rotate by.
    
    @param axis The axis to rotate around.  (0,0,1) would rotate around the Z axis.
  */
- (void)rotate:(double)angle around:(MaplyCoordinate3dD)axis;

/** 
    Rotate the geometry around the given axis by the given angle.
 
    Roate around the given 3D axis by the number of radians in angle.
 
    @param angle The angle in radians to rotate by.
 
    @param x X component of the axis to rotate around.
 
    @param y Y component of the axis to rotate around.
 
    @param z Z component of the axis to rotate around.
 */
- (void)rotate:(double)angle aroundX:(double)x y:(double)y z:(double)z;

/** 
    Apply a transform to the geometry and strings.
    
    This applies a general 4x4 transform to the geometry and strings.  You can construct the MaplyMatrix using a number of different options and combine multiple matrices.
  */
- (void)transform:(MaplyMatrix *)matrix;

/** 
    Add the geometry from another builder.
    
    Multiple geometry builders can be combined to build complex objects.
    
    This method copies geometry and strings, including their transforms to the current builder.
  */
- (void)addGeomFromBuilder:(MaplyGeomBuilder *)modelBuilder;

/** 
    Add the geometry from another builder, applying the given transform.
    
    Multiple geometry builders can be combined to build complex objects.
    
    This method transform the source geometry and strings and copies them into the current builder.
  */
- (void)addGeomFromBuilder:(MaplyGeomBuilder *)modelBuilder transform:(MaplyMatrix *)matrix;

/** 
    Calculate the bounding box of geometry and strings.
    
    Calculates the lower left and upper right corners of a rectangular solid that surrounds the geometry and strings for this builder.
    
    This returns false if there is no valid geometry (or strings) and takes transforms into account if there is.
  */
- (bool)getSizeLL:(MaplyCoordinate3dD *)ll ur:(MaplyCoordinate3dD *)ur;

/** 
    Calculate and returns the size of the geometry and strings.
    
    Calculates the size of the geometry and strings in the builder, taking transforms into account.
  */
- (MaplyCoordinate3dD)getSize;

/** 
    Generate a valid MaplyGeomModel that can be instanced and used as a 3D model.
    
    This call returns a MaplyGeomModel.  You'll need a model to make MaplyGeomModelInstance objects and for the addModelInstances:desc:mode: call to a MaplyBaseViewController (map or globe).
  */
- (MaplyGeomModel *)makeGeomModel:(MaplyThreadMode)threadMode;

@end

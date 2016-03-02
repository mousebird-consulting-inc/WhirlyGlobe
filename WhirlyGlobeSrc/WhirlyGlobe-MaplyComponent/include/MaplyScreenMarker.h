/*
 *  WGScreenMarker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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
#import "MaplyCoordinate.h"

/** @brief The Screen Marker is a 2D object that displays an image on the screen tracking a given location.
    @details The screen marker will track the given geographic location and display a centered rectangle with the given image.  Essentially it's a free floating icon, similar to the MaplyScreenLabel and it will always be drawn on top of other objects.  If the location would be behind the globe (in globe mode), the marker will disappear.
    @details If you're looking for a 3D marker object, that's the MaplyMarker.
  */
@interface MaplyScreenMarker : NSObject

/** @brief The location we're tracking for this particular screen marker.
    @details Locations are in geographic (lon/lat in radians).
  */
@property (nonatomic,assign) MaplyCoordinate loc;

/** @brief Screen size in points.
    @details The marker will always be this size on the screen.  The size is specified in display coordinates.  For the globe those are based on a radius of 1.0.
  */
@property (nonatomic,assign) CGSize size;

/** @brief An optional rotation to apply to the screen marker.
    @details This is a rotation we'll apply after the screen position has been calculated.  The angle is in radians counter-clockwise from north.
 */
@property (nonatomic,assign) double rotation;

/** @brief Image or texture to use for the marker.
    @details If set we'll stretch this UIImage (or MaplyTexture) out over the marker rectangle.  If not set, the marker will just be a colored rectange.  The view controller tracks this object and will reuse its texture and dispose of it as needed.
  */
@property (nonatomic,strong) id image;

/** @brief Images to display on the sticker.
    @details If this is set rather than image, we will cycle through these images on the screen marker.  It will take period time to cycle through them all.
  */
@property (nonatomic,strong) NSArray *images;

/** @brief The time we'll take to cycle through all the images for the marker.
    @details If images are passed in, this is the time it will take to cycle through them all.  By default this is 5s.
  */
@property (nonatomic) double period;

/** @brief Color for this particular marker.
    @details If set, this the color we'll use for the marker or how we'll tint the image.
    @details This overrides the color set in the description dictionary.
  */
@property (nonatomic,strong) UIColor *color;

/** @brief The layout importance compared to other features, 0 by default.
    @details The toolkit has a simple layout engine that runs several times per second.  It controls the placement of all participating screen based features, such as MaplyScreenLabel and MaplyScreenMaker objects.  This value controls the relative importance of this particular object.  By default that importance is 0 so the object must compete with others.  Setting it to MAXFLOAT will bypass the layout engine entirely and the object will always appear.
 */
@property (nonatomic,assign) float layoutImportance;

/** @brief The size of the marker for layout purposes.
    @details If layoutImportance is not set to MAXFLOAT, the screen marker will get throw into the mix when doing screen layout.  With this, you can set the size of the marker when considering layout.  If you set this to (0,0) the maker will take up no space, but still be considered in the layout.
  */
@property (nonatomic,assign) CGSize layoutSize;

/** @brief Offset in screen coordinates.
    @details Set to zero by default, this is the offset we'll apply to a given screen marker before it's drawn.  The values are screen points.
  */
@property (nonatomic,assign) CGPoint offset;

/** @brief Vertex attributes to apply to this screen marker.
    @details MaplyVertexAttribute objects are passed all the way to the shader.  Read that page for details on what they do.
    @details The array of vertex attributes provided here will be copied onto all the vertices we create for the shader.  This means you can use these to do things for a single billboard in your shader.
 */
@property (nonatomic,strong) NSArray *vertexAttributes;

/** @brief Screen marker selectability.  On by default
    @details If set, this marker can be selected by the user.  If not set, this screen marker will never appear in selection results.
 */
@property (nonatomic,assign) bool selectable;

/** @brief User data object for selection
 @details When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the screen marker means to them.
 */
@property (nonatomic,strong) id userObject;

@end

/** @brief A version of the maply screen marker that moves.
    @details This version of the screen marker can move in a limited way over time.
  */
@interface MaplyMovingScreenMarker : MaplyScreenMarker

/// @brief The end point for animation
@property (nonatomic,assign) MaplyCoordinate endLoc;

/// @brief How long it will take the screen marker to get to endLoc
@property (nonatomic,assign) NSTimeInterval duration;

@end

typedef MaplyScreenMarker WGScreenMarker;

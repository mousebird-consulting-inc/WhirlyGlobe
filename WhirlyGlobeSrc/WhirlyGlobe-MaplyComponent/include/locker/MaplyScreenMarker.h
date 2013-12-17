/*
 *  WGScreenMarker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2013 mousebird consulting
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
    @details This is a rotation we'll apply after the screen position has been calculated.
 */
@property (nonatomic,assign) float rotation;

/** @brief Image or texture to use for the marker.
    @details If set we'll stretch this UIImage (or MaplyTexture) out over the marker rectangle.  If not set, the marker will just be a colored rectange.  The view controller tracks this object and will reuse its texture and dispose of it as needed.
  */
@property (nonatomic,strong) id image;

/** @brief Color for this particular marker.
    @details If set, this the color we'll use for the marker or how we'll tint the image.
    @details This overrides the color set in the description dictionary.
  */
@property (nonatomic,strong) UIColor *color;

/** @brief The layout importance compared to other features. MAXFLOAT (always) by default.
    @details The toolkit has a simple layout engine that runs several times per second.  It controls the placement of all participating screen based features, such as MaplyScreenLabel and MaplyScreenMaker objects.  This value controls the relative importance of this particular marker.  By default that importance is infinite (MAXFLOAT) and so the label will always appearing.  Setting this value to 1.0, for example, will mean that this marker competes with other screen objects for space.
 */
@property (nonatomic,assign) float layoutImportance;

/** @brief Offset in screen coordinates.
    @details Set to zero by default, this is the offset we'll apply to a given screen marker before it's drawn.  The values are screen points.
  */
@property (nonatomic,assign) CGPoint offset;

/** @brief Screen marker selectability.  On by default
    @details If set, this marker can be selected by the user.  If not set, this screen marker will never appear in selection results.
 */
@property (nonatomic,assign) bool selectable;

/** @brief User data object for selection
    @details When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the screen label means to them.
 */
@property (nonatomic,strong) NSObject *userObject;

@end

typedef MaplyScreenMarker WGScreenMarker;

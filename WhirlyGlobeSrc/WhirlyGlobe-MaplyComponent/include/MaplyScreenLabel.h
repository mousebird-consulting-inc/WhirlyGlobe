/*
 *  WGScreenLabel.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/24/12.
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

/// Okay to place centered on point
#define kMaplyLayoutCenter  (1<<0)
/// Okay to place to the right of a point
#define kMaplyLayoutRight  (1<<1)
/// Okay to place it to the left of a point
#define kMaplyLayoutLeft   (1<<2)
/// Okay to place on top of a point
#define kMaplyLayoutAbove  (1<<3)
/// Okay to place below a point
#define kMaplyLayoutBelow  (1<<4)

/** @brief The Screen Label is a 2D label that tracks a given geographic location.
    @details This screen label will track the given geographic position.  If it's behind the globe it will disappear.  The label is rendered in a fixed size and will always appear on top of other geometry.
  */
@interface MaplyScreenLabel : NSObject

/** @brief Location of the screen label in geographic (lat/lon) in radians.
    @details The screen label will track this position.  If it would be behind the globe (in globe mode), then it will disappear.
  */
@property (nonatomic,assign) MaplyCoordinate loc;

/** @brief An optional rotation to apply to the screen label.
    @details This is a rotation we'll apply after the screen position has been calculated.  You can use this to do things like track the orientation of roads.
  */
@property (nonatomic,assign) float rotation;

/** @brief When the screen is rotated, try to keep the label upright.
    @details This tells the layout and display engine to keep the label oriented upright no matter what.  In practice this means it will manipulate the rotation by 180 degrees.
  */
@property (nonatomic,assign) bool keepUpright;

/** @brief The actual text to display.
    @details This is a simple NSString for the text.  Things like font are set in the NSDictionary passed in to the add call in the view controller.
  */
@property (nonatomic,strong) NSString *text;

/** @brief Text can be accompanied by an optional icon image.
    @details If set, we'll put this image to the left of the text in the screen label.  The UIImage will be tracked by the view controller and reused as needed or disposed of when no longer needed.
    @details The name had to change because Apple's private selector search is somewhat weak.
  */
@property (nonatomic,strong) UIImage *iconImage2;

/** @brief Icon size in points.
    @details If there is an icon image, this is how big it is.
  */
@property (nonatomic,assign) CGSize iconSize;

/** @brief An optional offset for the whole screen label.
    @details If set, we'll move the screen label around by this amount before rendering it.  These are screen coordinates, not geographic.
  */
@property (nonatomic,assign) CGPoint offset;

/** @brief An option color override.
    @details If set, this color will override the color passed in with the NSDictionary in the view controller's add method.
  */
@property (nonatomic,strong) UIColor *color;

/** @brief Label selectability.  On by default
    @details If set, this label can be selected by the user.  If not set, this screen label will never appear in selection results.
  */
@property (nonatomic,assign) bool selectable;

/** @brief The layout importance compared to other features. MAXFLOAT (always) by default.
    @details The toolkit has a simple layout engine that runs several times per second.  It controls the placement of all participating screen based features, such as MaplyScreenLabel and MaplyScreenMaker objects.  This value controls the relative importance of this particular label.  By default that importance is infinite (MAXFLOAT) and so the label will always appearing.  Setting this value to 1.0, for example, will mean that this label competes with other screen objects for space.
  */
@property (nonatomic,assign) float layoutImportance;

/** @brief The placement rules for the layout engine to follow.
    @details The layout engine isn't the brightest bulb in the string and it needs placement hints.  This value tells the engine where we can move the label around.  These are bit flags, so OR them together as needed.  The default is everything.
 
|Layout Flag|Meaning|
|:----------|:------|
|kMaplyLayoutRight|The layout engine can put this label to the right of the location point.|
|kMaplyLayoutLeft|The layout engine can put this label to the left of the location point.|
|kMaplyLayoutAbove|The layout engine may put this label above the location point, centered.|
|kMaplyLayoutBelow|The layout engine may put this albel below the location point, centered.|
 */
@property (nonatomic,assign) int layoutPlacement;

/** @brief User data object for selection
    @details When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the screen label means to them.
  */
@property (nonatomic,strong) id userObject;

@end

typedef MaplyScreenLabel WGScreenLabel;

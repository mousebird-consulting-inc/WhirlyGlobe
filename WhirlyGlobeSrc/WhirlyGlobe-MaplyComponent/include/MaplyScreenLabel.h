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

/// Okay to place to the right of a point
#define kMaplyLayoutRight  (1<<0)
/// Okay to place it to the left of a point
#define kMaplyLayoutLeft   (1<<1)
/// Okay to place on top of a point
#define kMaplyLayoutAbove  (1<<2)
/// Okay to place below a point
#define kMaplyLayoutBelow  (1<<3)

/** The Maply Screen Label is a 2D label that tracks a certain position on the globe (or map).
    It's an overlay and will always stay the same size no matter the position.
  */
@interface MaplyScreenLabel : NSObject

/// Put yer user data here
@property (nonatomic,strong) NSObject *userObject;
/// Location in geographic (lat/lon) in radians
@property (nonatomic,assign) MaplyCoordinate loc;
/// Optional rotation
@property (nonatomic,assign) float rotation;
/// Size on the screen, in points.  In general, set the height, but not the width.
@property (nonatomic,assign) CGSize size;
/// Text to display
@property (nonatomic,strong) NSString *text;
/// If set, this is the image to use for the marker
@property (nonatomic,strong) UIImage *iconImage;
/// Size of the icon on screen
@property (nonatomic,assign) CGSize iconSize;
/// Offset the text on screen by this amount.  Defaults to zero.
@property (nonatomic,assign) CGSize offset;
/// If set, this color overrides the default
@property (nonatomic,strong) UIColor *color;
/// If set, this label can be selected.  On by default.
@property (nonatomic,assign) bool selectable;
/// For the label layout engine, this is the importance of this particular
///  label.  It's set to MAXFLOAT by defaut, which means it always shows up.
/// Set it to another value to actually be laid out with constraints.
@property (nonatomic,assign) float layoutImportance;
/// If we're using label importance, how we're allowed to place the label in the layout engine
@property (nonatomic,assign) int layoutPlacement;

@end

typedef MaplyScreenLabel WGScreenLabel;

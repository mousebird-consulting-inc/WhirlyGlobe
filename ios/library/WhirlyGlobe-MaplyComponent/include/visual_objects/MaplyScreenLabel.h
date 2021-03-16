/*
 *  WGScreenLabel.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/24/12.
 *  Copyright 2011-2019 mousebird consulting
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
#import "math/MaplyCoordinate.h"

@class MaplyVectorObject;

/// Don't move the label at all
#define kMaplyLayoutNone (1<<0)
/// Okay to place centered on point
#define kMaplyLayoutCenter  (1<<1)
/// Okay to place to the right of a point
#define kMaplyLayoutRight  (1<<2)
/// Okay to place it to the left of a point
#define kMaplyLayoutLeft   (1<<3)
/// Okay to place on top of a point
#define kMaplyLayoutAbove  (1<<4)
/// Okay to place below a point
#define kMaplyLayoutBelow  (1<<5)

/** 
    The Screen Label is a 2D label that tracks a given geographic location.
    
    This screen label will track the given geographic position.  If it's behind the globe it will disappear.  The label is rendered in a fixed size and will always appear on top of other geometry.
  */
@interface MaplyScreenLabel : NSObject

/** 
    Location of the screen label in geographic (lat/lon) in radians.
    
    The screen label will track this position.  If it would be behind the globe (in globe mode), then it will disappear.
  */
@property (nonatomic,assign) MaplyCoordinate loc;

/** 
    An optional rotation to apply to the screen label.
    
    This is a rotation we'll apply after the screen position has been calculated.  You can use this to do things like track the orientation of roads.
    
    Rotation is in radians counter-clockwise from north.
  */
@property (nonatomic,assign) float rotation;

/** 
    When the screen is rotated, try to keep the label upright.
    
    This tells the layout and display engine to keep the label oriented upright no matter what.  In practice this means it will manipulate the rotation by 180 degrees.
  */
@property (nonatomic,assign) bool keepUpright;

/** 
    The actual text to display.
    
    This is a simple NSString for the text.  Things like font are set in the NSDictionary passed in to the add call in the view controller.
  */
@property (nonatomic,strong) NSString * __nullable text;

/** 
    Text can be accompanied by an optional icon image.
    
    If set, we'll put this image to the left of the text in the screen label.  The UIImage will be tracked by the view controller and reused as needed or disposed of when no longer needed.
    
    The name had to change because Apple's private selector search is somewhat weak.
  */
@property (nonatomic,strong) UIImage * __nullable iconImage2;

/** 
    Icon size in points.
    
    If there is an icon image, this is how big it is.
  */
@property (nonatomic,assign) CGSize iconSize;

/** 
    An optional offset for the whole screen label.
    
    If set, we'll move the screen label around by this amount before rendering it.  These are screen coordinates, not geographic.
  */
@property (nonatomic,assign) CGPoint offset;

/** 
    An optional color override.
    
    If set, this color will override the color passed in with the NSDictionary in the view controller's add method.
  */
@property (nonatomic,strong) UIColor * __nullable color;

/** 
    Label selectability.  On by default
    
    If set, this label can be selected by the user.  If not set, this screen label will never appear in selection results.
  */
@property (nonatomic,assign) bool selectable;

/** 
    The layout importance compared to other features, 0 by default.
 
    The toolkit has a simple layout engine that runs several times per second.  It controls the placement of all participating screen based features, such as MaplyScreenLabel and MaplyScreenMaker objects.  This value controls the relative importance of this particular object.  By default that importance is 0 so the object must compete with others.  Setting it to MAXFLOAT will bypass the layout engine entirely and the object will always appear.
 */
@property (nonatomic,assign) float layoutImportance;

/** 
    The placement rules for the layout engine to follow.
    
    The layout engine isn't the brightest bulb in the string and it needs placement hints.  This value tells the engine where we can move the label around.  These are bit flags, so OR them together as needed.  The default is everything.
 
|Layout Flag|Meaning|
|:----------|:------|
|kMaplyLayoutRight|The layout engine can put this label to the right of the location point.|
|kMaplyLayoutLeft|The layout engine can put this label to the left of the location point.|
|kMaplyLayoutAbove|The layout engine may put this label above the location point, centered.|
|kMaplyLayoutBelow|The layout engine may put this albel below the location point, centered.|
 */
@property (nonatomic,assign) int layoutPlacement;

/**
 The size of the label for layout purposes.
 
 If layoutImportance is not set to MAXFLOAT, the screen label will get throw into the mix when doing screen layout.  With this, you can set the size of the label when considering layout.  If you set this to (0,0) the maker will take up no space, but still be considered in the layout.
 */
@property (nonatomic,assign) CGSize layoutSize;

/**
    If this is present, we'll render an ID into the mask layer to be used by other features to mask against.
 */
@property (nonatomic,retain,nullable) NSString *maskID;

/**
    If set, we'll lay out the the text along the given linear or areal feature.
    Takes the first feature in the vector, if there are multiple.
 */
@property (nonatomic,retain,nullable) MaplyVectorObject *layoutVec;


/**
    Used to resolve to resolve labels that show the same thing.
 
    By default this is nil and not used to resolve conflicts.  When you set it to
    something, such as a string, it will be used to resolve display conflicts.
    Only one object that has this unique ID (evaluated with isEqualToString:) will be displayed.
    Importance is evaluated first, so the most important will be placed first.
  */
@property (nonatomic,retain,nullable) NSString *uniqueID;

/** 
    User data object for selection
    
    When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the screen label means to them.
  */
@property (nonatomic,strong) id  __nullable userObject;

@end

/** 
    A version of the maply screen label that moves.
    
    This version of the screen label can move in a limited way over time.
 */
@interface MaplyMovingScreenLabel : MaplyScreenLabel

/// The end point for animation
@property (nonatomic,assign) MaplyCoordinate endLoc;

/// How long it will take the screen label to get to endLoc
@property (nonatomic,assign) NSTimeInterval duration;

@end

typedef MaplyScreenLabel WGScreenLabel;

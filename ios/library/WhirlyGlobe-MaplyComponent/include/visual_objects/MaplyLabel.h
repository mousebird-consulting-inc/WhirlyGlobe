/*
 *  WGLabel.h
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
#import "MaplyCoordinate.h"

typedef NS_ENUM(NSInteger, MaplyLabelJustify) {
	MaplyLabelJustifyLeft,
	MaplyLabelJustifyMiddle,
	MaplyLabelJustifyRight
};

/** 
    This is a 3D label.
    
    The Maply Label is a 3D object that sits on top of the globe (or map) at a specified location.  If you want a 2D label that sits on top of everything else, you want the MaplyScreenLabel.  Seriously, you probably want that.
  */
@interface MaplyLabel : NSObject

/** 
    The location (in geographic) for this label.
    
    The Maply Label is a 3D object and this is its location on the globe or map.  The coordinates are in geographic (lon/lat) and the actual layout is controleld by justify.
  */
@property (nonatomic,assign) MaplyCoordinate loc;

/** 
    Size of the label in display units.
    
    The Maply Label is a 3D object placed on top of the globe or map. This controls the size of that label in display coordinates.  For the globe display coordinates are based on a radius of 1.0.
    
    One or both values of the size can be set.  Typically you want to set the height and let the toolkit calculate the width.
  */
@property (nonatomic,assign) CGSize size;

/// The text to display on the globe or map at the given location.
@property (nonatomic,strong) NSString * __nullable text;

/** 
    Text can be accompanied by an optional icon image.
    
    If set, we'll put this image to the left of the text in the label.  The UIImage (or MaplyTexture) will be tracked by the view controller and reused as needed or disposed of when no longer needed.
    
    The name had to change because Apple's private selector search is somewhat weak.
 */
@property (nonatomic,strong) id __nullable iconImage2;

/** 
    An option color override.
    
    If set, this color will override the color passed in with the NSDictionary in the view controller's add method.
 */
@property (nonatomic,strong) UIColor * __nullable color;

/** 
    Label selectability.  On by default
    
    If set, this label can be selected by the user.  If not set, this label will never appear in selection results.
 */
@property (nonatomic,assign) bool selectable;

/** 
    The text justification based on the location.
    
    Text can be placed around the location based on this value.
 
|Justify Value|Description|
|:------------|:----------|
|MaplyLabelJustifyLeft|The label will be placed with its left side on the location.|
|MaplyLabelJustifyMiddle|The label will be centered on the location.|
|MaplyLabelJustifyRight|The label will be placed with its right side on the location.|
  */
@property (nonatomic,assign) MaplyLabelJustify justify;

/** 
    User data object for selection
    
    When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the label means to them.
 */
@property (nonatomic,strong) id __nullable userObject;

@end

typedef MaplyLabel WGLabel;

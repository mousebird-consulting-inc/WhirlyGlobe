/*
 *  MaplyScreenObject
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 2/28/15
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
#import "control/MaplyRenderController.h"

/** 
    The Maply Screen Object is used to build up a more complex screen object from multiple pieces.
    
    You can use one or more of these to build up a combination of labels and images that form a single marker, label, or billboard.
  */
@interface MaplyScreenObject : NSObject

/** 
    Add a string to the screen object
    
    @param str The string to add
    
    @param font The font to use
    
    @param color The foreground color of the string.
  */
- (void)addString:(NSString *)str font:(UIFont *)font color:(UIColor *)color;

/** 
    Add an attributed string to the screen object.
    
    This adds an annotated string to the screen object.  The size will be based on the length of the string and the font.
  */
- (void)addAttributedString:(NSAttributedString *)str;

/** 
    Add an image scaled to the given size.
    
    Adds a UIImage or MaplyTexture object scaled to the given size.
  */
- (void)addImage:(id)image color:(UIColor *)color size:(CGSize)size;

/** 
    Calculate and return the current bounding box of the screen object.
  */
- (MaplyBoundingBox)getSize;

/** 
    Apply a scale to all the pieces of the screen object.
  */
- (void)scaleX:(double)x y:(double)y;

/** 
    Apply a translation to all the pieces of the screen object.
  */
- (void)translateX:(double)x y:(double)y;

/** 
    Add the contents of the given screen object to this screen object.
  */
- (void)addScreenObject:(MaplyScreenObject *)screenObj;

@end

/**
 Features missing to replicate ScreenMarker and ScreenLabel.
    rotation
    images (for animation)
    period (for animation)
    color
    layoutImportance
    layoutSize
    layoutPlacement (right, left, above, below)
    vertexAttributes
    keepUpright
 
 ScreenObjectInstance (new object)
    location
    endLoc/duration (moving fields)
    selectable
    ScreenObject pointer
    Selected version ScreenObject pointer
    uniqueID
    userObject (for selection)
 */

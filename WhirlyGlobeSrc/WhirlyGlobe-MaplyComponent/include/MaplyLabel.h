/*
 *  WGLabel.h
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

typedef enum {MaplyLabelJustifyLeft,MaplyLabelJustiyMiddle,MaplyLabelJustifyRight} MaplyLabelJustify;

/** The Maply Label is a 3D label.  That is, it sits on top of the globe (or map) as a 3D object
    and gets bigger and smaller and you move around.
  */
@interface MaplyLabel : NSObject

/// For user data
@property (nonatomic,strong) NSObject *userObject;
/// Location in geographic (lat/lon) in radians
@property (nonatomic,assign) MaplyCoordinate loc;
/// Size on the screen, in points.  In general, set the height, but not the width.
@property (nonatomic,assign) CGSize size;
/// Text to display
@property (nonatomic,strong) NSString *text;
/// If set, this is the image to use for the marker
@property (nonatomic,strong) UIImage *iconImage;
/// If set, this color overrides the default
@property (nonatomic,strong) UIColor *color;
/// If set, this label can be selected.  On by default.
@property (nonatomic,assign) bool selectable;
/// Text justification
@property (nonatomic,assign) MaplyLabelJustify justify;

@end

typedef MaplyLabel WGLabel;

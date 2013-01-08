/*
 *  WGScreenLabel.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/24/12.
 *  Copyright 2011-2012 mousebird consulting
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

/** Screen Space (2D) Label.
    Set this is up and hand it over to the WhirlyGlobeViewController for display.
 */
@interface MaplyScreenLabel : NSObject
{
    /// Put yer user data here
    NSObject *userObject;
    /// Location in geographic (lat/lon) in radians
    MaplyCoordinate loc;
    /// Size on the screen, in points.  In general, set the height, but not the width.
    CGSize size;
    /// Text to display
    NSString *text;
    /// If set, this is the image to use for the marker
    UIImage *iconImage;
    /// Offset the text on screen by this amount.  Defaults to zero.
    CGSize offset;
    /// If set, this color overrides the default
    UIColor *color;
    /// If set, this label can be selected.  On by default.
    bool selectable;
    /// For the label layout engine, this is the importance of this particular
    ///  label.  It's set to MAXFLOAT by defaut, which means it always shows up.
    /// Set it to another value to actually be laid out with constraints.
    float layoutImportance;
}

@property (nonatomic,strong) NSObject *userObject;
@property (nonatomic,assign) MaplyCoordinate loc;
@property (nonatomic,assign) CGSize size;
@property (nonatomic,strong) NSString *text;
@property (nonatomic,strong) UIImage *iconImage;
@property (nonatomic,assign) CGSize offset;
@property (nonatomic,strong) UIColor *color;
@property (nonatomic,assign) bool selectable;
@property (nonatomic,assign) float layoutImportance;

@end

typedef MaplyScreenLabel WGScreenLabel;

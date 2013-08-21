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

/** The Maply Screen Marker is a 2D object that displays an image on the screen
    tracking a given location on the globe (or map).  It's an overlay and it will
    not change size depending on the position of the viewer.
  */
@interface MaplyScreenMarker : NSObject

/// For user data
@property (nonatomic,strong) NSObject *userObject;
/// Location in geographic (lat/lon) in radians
@property (nonatomic,assign) MaplyCoordinate loc;
/// Size on the screen, in points
@property (nonatomic,assign) CGSize size;
/// If set, this is the image to use for the marker
@property (nonatomic,strong) UIImage *image;
/// If set, this label can be selected.  On by default.
@property (nonatomic,assign) bool selectable;
/// For the layout engine, this is the importance of this particular
///  marker.  It's set to MAXFLOAT by defaut, which means it always shows up.
/// Set it to another value to actually be laid out with constraints.
@property (nonatomic,assign) float layoutImportance;

@end

typedef MaplyScreenMarker WGScreenMarker;

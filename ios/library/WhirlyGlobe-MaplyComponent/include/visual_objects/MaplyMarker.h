/*
 *  WGMarker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/24/12.
 *  Copyright 2011-2017 mousebird consulting
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

/** 
    The Marker places a UIImage on the globe or map at a given location.
    
    The Maply Marker takes a location and image, using those to display a textured rectangle on the globe (or map).  Since it's a real 3D object it will get larger and smaller as the user moves around.
    
    If you want a screen based object that stays the same size and is displayed on top of everything else, look to the MaplyScreenMarker.
  */
@interface MaplyMarker : NSObject

/** 
    Center of the marker in geographic coordinates (lon/lat in radians).
    
    The Maply Marker is a 3D object so this is the center of the marker on the globe or map.
  */
@property (nonatomic,assign) MaplyCoordinate loc;


/** 
    Size of the marker in display coordinates.
    
    This is the size of the marker in display coordinates.  For the globe display coordinates are based on a radius of 1.0.
  */
@property (nonatomic,assign) CGSize size;

/** 
    Image or MaplyTexture to use for the marker.
    
    If set, we'll display a UIImage at the given location of the given size.  If not set, it's just a color rectangle which is not very exciting.  The view controller tracks the UIImage and will reuse it as necessary and release it when finished.
  */
@property (nonatomic,strong) id __nullable image;

/** 
    Images or MaplyTextures to use for the marker.
    
    If set we'll animate these images one after the other over the duration.
  */
@property (nonatomic,strong) NSArray * __nullable images;

/** 
    The time we'll take to cycle through all the images for the marker.
    
    If images are passed in, this is the time it will take to cycle through them all.  By default this is 5s.
 */
@property (nonatomic) double period;

/** 
    Marker selectability.  On by default
    
    If set, this marker can be selected by the user.  If not set, this marker will never appear in selection results.
 */
@property (nonatomic,assign) bool selectable;

/** 
    User data object for selection
    
    When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the label means to them.
 */
@property (nonatomic,strong) id  __nullable userObject;

@end

typedef MaplyMarker WGMarker;

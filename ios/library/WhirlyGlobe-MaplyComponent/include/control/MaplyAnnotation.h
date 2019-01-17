/*
 *  MaplyAnnotation.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/13/13.
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
    This object displays an annotation at a particular point and will track that point as the map or globe moves.
    
    An annotation is used to point out some feature on the globe or map, typically that the user has tapped on.  It's a multi-part beast that may contain titles, subtitles, images, background views and such.
    
    To add one, create the MaplyAnnotation and then call addAnnotation:forPoint:offset: on the MaplyBaseViewController.
    
    The MaplyAnnotation is a wrapper around the SMCalloutView by Nick Farina.  It exposes much of the functionality, but sets things up correctly and deals with moving the annotation around.
  */
@interface MaplyAnnotation : NSObject

/// The minimum viewer height this annotation is visible at.
/// This is viewer height above the globe or map.  The annotation will only be visible if the user is above this height.
@property (nonatomic,assign) float minVis;

/// The maximum viewer height this annotation is visible at.
/// This is viewer height above the globe or map.  The annotation will only be visible if the user is below this height.
@property (nonatomic,assign) float maxVis;

/// Set the popup's title
@property (nonatomic,strong) NSString *title;

/// Set the popup's subtitle
@property (nonatomic,strong) NSString *subTitle;

/// If set, the (optional) accessory view on the left
@property (nonatomic,strong) UIView *leftAccessoryView;

/// If set, the (optional) accessory view on the right
@property (nonatomic,strong) UIView *rightAccessoryView;

/// If set, the custom title view containing whatever you like.
@property (nonatomic,strong) UIView *titleView;

/// If set, the custom subtitle view containing whatever you put in there.
@property (nonatomic,strong) UIView *subtitleView;

/// If set, a custom content view.  Title, subtitle and views are ignored.
@property (nonatomic,strong) UIView *contentView;

/// The location of the annotation
@property (nonatomic,readonly) MaplyCoordinate loc;

/// If set, we'll reposition the globe or map to make the annotation visible.
/// If the annotation would be off screen we would normally reposition the globe or map to make it visible.  If this is et to false, we won't.
@property (nonatomic) bool repositionForVisibility;

@end

/*
 *  MaplyAnnotation.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/13/13.
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

/** @brief This object displays an annotation at a particular point and will track that point as the map or globe moves.
    @details An annotation is used to point out some feature on the globe or map, typically that the user has tapped on.  It's a multi-part beast that may contain titles, subtitles, images, background views and such.
    @details To add one, create the MaplyAnnotation and then call addAnnotation:forPoint:offset: on the MaplyBaseViewController.
    @details The MaplyAnnotation is a wrapper around the SMCalloutView by Nick Farina.  It exposes much of the functionality, but sets things up correctly and deals with moving the annotation around.
  */
@interface MaplyAnnotation : NSObject

/// @brief The minimum viewer height this annotation is visible at.
/// @details This is viewer height above the globe or map.  The annotation will only be visible if the user is above this height.
@property (nonatomic,assign) float minVis;

/// @brief The maximum viewer height this annotation is visible at.
/// @details This is viewer height above the globe or map.  The annotation will only be visible if the user is below this height.
@property (nonatomic,assign) float maxVis;

/// @brief Set the popup's title
@property (nonatomic) NSString *title;

/// @brief Set the popup's subtitle
@property (nonatomic) NSString *subTitle;

/// @brief If set, the (optional) accessory view on the left
@property (nonatomic) UIView *leftAccessoryView;

/// @brief If set, the (optional) accessory view on the right
@property (nonatomic) UIView *rightAccessoryView;

/// @brief If set, the custom title view containing whatever you like.
@property (nonatomic) UIView *titleView;

/// @brief If set, the custom subtitle view containing whatever you put in there.
@property (nonatomic) UIView *subtitleView;

/// @brief If set, a custom content view.  Title, subtitle and views are ignored.
@property (nonatomic) UIView *contentView;

/// @brief If set, we'll reposition the globe or map to make the annotation visible.
/// @details If the annotation would be off screen we would normally reposition the globe or map to make it visible.  If this is et to false, we won't.
@property (nonatomic) bool repositionForVisibility;

@end

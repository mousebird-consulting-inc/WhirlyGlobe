/*
 *  MaplyCluster.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/29/15.
 *  Copyright 2011-2015 mousebird consulting
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
#import "MaplyScreenMarker.h"
#import "MaplyShader.h"

@class MaplyBaseViewController;

/** @brief Information about the group of objects to cluster.
    @details This object is passed in when the developer needs to make an image for a group of objects.
  */
@interface MaplyClusterInfo : NSObject

/// @brief Number of objects being clustered
@property (nonatomic,assign) int numObjects;

@end

/** @brief Visual representation for a group of markers.
    @details Fill this in for the
  */
@interface MaplyClusterGroup : NSObject

/// @brief The image to use for the group
@property (nonatomic) id __nonnull image;

/// @brief Screen size to use for the resulting marker
@property (nonatomic,assign) CGSize size;

@end

/** @brief Fill in this protocol to provide images when individual markers/labels are clustered.
    @details This is the protocol for marker/label clustering.  You must fill this in and register the cluster
  */
@protocol MaplyClusterGenerator <NSObject>

/** @brief Called at the start of clustering.
    @details Called right before we start generating clusters.  Do you setup here if need be.
  */
- (void) startClusterGroup;

/** @brief Generate a cluster group for a given collection of markers.
    @details Generate an image and size to represent the number of marker/labels we're consolidating.
  */
- (MaplyClusterGroup *__nonnull) makeClusterGroup:(MaplyClusterInfo *__nonnull)clusterInfo;

/** @brief Called at the end of clustering.
    @details If you were doing optimization (for image reuse, say) clean it up here.
  */
- (void) endClusterGroup;

/// @brief Return the cluster number we're covering
- (int) clusterNumber;

/// @brief The size of the cluster that will be created.
/// @details This is the biggest cluster you're likely to create.  We use it to figure overlaps between clusters.
- (CGSize) clusterLayoutSize;

/// @brief Set this if you want cluster to be user selectable.  On by default.
- (bool) selectable;

/// @brief How long to animate markers the join and leave a cluster
- (double) markerAnimationTime;

/// @brief The shader to use for moving objects around
/// @details If you're doing animation from point to cluster you need to provide a suitable shader.
- (MaplyShader *__nullable) motionShader;

@end

/** @brief The basic cluster generator installed by default.
    @details This cluster generator will make images for grouped clusters of markers/labels.
  */
@interface MaplyBasicClusterGenerator : NSObject <MaplyClusterGenerator>

/** @brief Initialize with a list of colors.
    @details Initialize with a list of colors.  Each order of magnitude will use another color.  Must provide at least 1.
  */
- (nonnull instancetype)initWithColors:(NSArray *__nonnull)colors clusterNumber:(int)clusterNumber size:(CGSize)markerSize viewC:(MaplyBaseViewController *__nonnull)viewC;

/// @brief The ID number corresponding to the cluster.  Every marker/label with this cluster ID will be grouped together.
@property (nonatomic,assign) int clusterNumber;

/// @brief The size of the cluster that will be created.
/// @details This is the biggest cluster you're likely to create.  We use it to figure overlaps between clusters.
@property (nonatomic) CGSize clusterLayoutSize;

/// @brief Set this if you want cluster to be user selectable.  On by default.
@property (nonatomic) bool selectable;

/// @brief How long to animate markers the join and leave a cluster
@property (nonatomic) double markerAnimationTime;

/// @brief The shader to use when moving objects around
/// @details When warping objects to their new locations we use a motion shader.  Set this if you want to override the default.
@property (nonatomic) MaplyShader * __nullable motionShader;

@end

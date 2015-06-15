/*
 *  MaplyQuadTracker.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/27/15.
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
#import "MaplyTileSource.h"
#import "WhirlyGlobeViewController.h"

/** @brief Return data for one or more point queries.
    @details You'll pass in an array of these to tiles:forScreenPts:numPts:
  */
typedef struct
{
    /// @brief Location on screen scaled between (0,1)
    double screenU,screenV;
    /// @brief The tile the corresponding point belonged to.  Level set to -1 if invalid.
    MaplyTileID tileID;
    // Required to make the C/C++ bridge happy
    int padding;
    /// @brief Location in coordinate system
    double locX,locY;
    /// @brief Location within tile (scaled from 0-1)
    double tileU,tileV;
} MaplyQuadTrackerPointReturn;

/** @brief The quad tracker keeps track of quad tree nodes.
    @details This object tracks quad tree nodes as they're added to and removed from an internal quad tree that tracks them by screen importance.  This version is intended for use in Objective-C and is much simpler than the internal version.
    @details All the methods are thread safe.
  */
@interface MaplyQuadTracker : NSObject

/// @brief Coordinate system for the quad tiles we're tracking
@property (nonatomic,strong) MaplyCoordinateSystem *coordSys;

/** @brief Init with a globe view controller
    @details Initialize with a globe view controller.  Only valid for globe at the moment.
  */
- (id)initWithViewC:(WhirlyGlobeViewController *)viewC;

/** @brief Query the quad tracker for tiles and locations within them for a group of points.
    @details This is a bulk query for points within the tiles being tracked.
    @param tilesInfo This is both an input and output parameter.  Fill in the screenU and screenV values and you'll get back tileID and tileU and tileV.  tileID.level will be -1 if there was no hit for that point.
    @param numPts The number of points in the tilesInfo array.
  */
- (void)tiles:(MaplyQuadTrackerPointReturn *)tilesInfo forPoints:(int)numPts;

/** @brief Add a tile to track.
  */
- (void)addTile:(MaplyTileID)tileID;

/** @brief Remove a tile from tracking
  */
- (void)removeTile:(MaplyTileID)tileID;

@end

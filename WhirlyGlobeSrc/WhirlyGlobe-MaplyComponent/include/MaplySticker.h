/*
 *  MaplySticker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 11/27/12.
 *  Copyright 2012 mousebird consulting
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
#import <MaplyCoordinate.h>

/** A Maply Sticker is a rectangle that we'll stretch over the given coordinates
    and then slap an optional image on top of.
  */
@interface MaplySticker : NSObject
{
    /// Extents of the stick
    MaplyCoordinate ll,ur;
    /// Angle of rotation around center
    float rotation;
    /// Image to stretch over the sticker
    UIImage *image;
}

@property (nonatomic,assign) MaplyCoordinate ll,ur;
@property (nonatomic,assign) float rotation;
@property (nonatomic,strong) UIImage *image;

@end

/*
 *  MaplyActiveScreenLabel.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/10/13.
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

#import "MaplyActiveObject.h"
#import "MaplyCoordinate.h"
#import "MaplyScreenLabel.h"

/** An active screen label allows for changes
  */
@interface MaplyActiveScreenLabel : MaplyActiveObject

/// This is what the current screen label looks like.
/// Assign a new one to change things.  Don't just change
///  the screen label itself.  If you do, just assign it here again
///  so we know that you did.
@property (nonatomic,strong) MaplyScreenLabel *screenLabel;

/// Initialize with various visual constraints
- (id)initWithDesc:(NSDictionary *)descDict;

@end

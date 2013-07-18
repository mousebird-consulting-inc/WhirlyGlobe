/*
 *  MaplyActiveObject.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/3/13.
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

#import <Foundation/Foundation.h>

/** An Active object is one that can be manipulated directly on the main thread.
    You make changes in the various subclasses and those changes are reflected
    on the sceen on the next frame render.
  */
@interface MaplyActiveObject : NSObject

/// Initialize with the appropriate description dictionary
- (id)initWithDesc:(NSDictionary *)descDict;

/// Description dictionary for the object
@property (nonatomic) NSDictionary *desc;

@end

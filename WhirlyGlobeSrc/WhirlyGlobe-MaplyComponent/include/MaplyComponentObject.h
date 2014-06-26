/*
 *  MaplyComponentObject.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/18/12.
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

/** @brief Used to represent the view controller resources attached to one or more visual objects.
    @details When you add one or more objects to a view controller, you'll get a component object back. It's an opaque object (seriously, don't look inside) that we use to track various resources within the toolkit.
    @details You can keep these around to remove the visual objects you added earlier, but that's about all the interaction you'll have with them.
  */
@interface MaplyComponentObject : NSObject

/// @brief Construct with a description.  Uses the kMaplyEnable.
- (id)initWithDesc:(NSDictionary *)desc;

@end

typedef MaplyComponentObject WGComponentObject;

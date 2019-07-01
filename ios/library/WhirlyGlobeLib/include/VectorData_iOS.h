/*
 *  VectorData_iOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/6/19.
 *  Copyright 2011-2019 mousebird consulting
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
#import "VectorData.h"

namespace WhirlyKit {

/** Helper routine to parse geoJSON into a collection of vectors.
 We don't know for sure what we'll get back, so you have to go
 looking through it.  Return false on parse failure.
 */
bool VectorParseGeoJSON(ShapeSet &shapes,NSDictionary *jsonDict);

}

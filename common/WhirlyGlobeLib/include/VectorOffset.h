/*
 *  VectorOffset.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/4/21.
 *  Copyright 2011-2021 mousebird consulting
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

#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "VectorData.h"

namespace WhirlyKit
{

/// Offset/buffer the given linear feature
std::vector<VectorRing> BufferLinear(const VectorRing &ring, float offset);

/// Offset/buffer a polygon into one or more loops
std::vector<VectorRing> BufferPolygon(const VectorRing &ring, float offset);

}

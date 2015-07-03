/*
 *  WhirlyOctEncoding.h
 *  WhirlyGlobeLib
 *
 *  Created by @jmnavarro on 7/2/15.
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

 #import "WhirlyVector.h"

//
// The 'oct' encoding is described in
// "A Survey of Efficient Representations of Independent Unit Vectors",
// Cigolle et al 2014: http://jcgt.org/published/0003/02/01/
//

namespace WhirlyKit
{

/** Decodes x,y oct-encoded values to vec3
 */
Point3f OctDecode(uint8_t x, uint8_t y);

}

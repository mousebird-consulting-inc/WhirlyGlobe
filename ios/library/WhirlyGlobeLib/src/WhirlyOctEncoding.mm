/*
 *  WhirlyOctEncoding.mm
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

#import "WhirlyOctEncoding.h"

using namespace Eigen;

namespace WhirlyKit
{

Point3f OctDecode(uint8_t enc_x, uint8_t enc_y)
{
	double_t x = enc_x / 255.0f * 2.0f - 1;
	double_t y = enc_y / 255.0f * 2.0f - 1;
	double_t z = 1.0 - (fabs(x) + fabs(y));

	if (z < 0.0) {
		double_t oldVX = x;
		x = (1 - fabs(y)    ) * (oldVX < 0.0 ? -1.0 : 1.0);
		y = (1 - fabs(oldVX)) * (y     < 0.0 ? -1.0 : 1.0);
	}

	double_t magnitude = sqrtf(x * x + y * y + z * z);

	x /= magnitude;
	y /= magnitude;
	z /= magnitude;

	return Point3f(x, y, z);
}
	
}

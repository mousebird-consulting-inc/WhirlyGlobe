/*  GreatCircle_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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
 */

#import "GreatCircle_Android.h"

namespace WhirlyKit
{

Linear *GreatCircle_Android::asLinear(const CoordSystemDisplayAdapter *coordAdapter) const
{
    auto *lin = new Linear();
    if (sampleNum > 0)
        SampleGreatCircleStatic(startPt,endPt,height,lin->pts,coordAdapter,sampleNum);
    else
        SampleGreatCircle(startPt,endPt,height,lin->pts,coordAdapter,samplingEps);
    lin->color = color;
    lin->useColor = useColor;

    return lin;
}

}

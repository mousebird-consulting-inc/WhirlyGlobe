/*
 *  Sun.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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

#include "AA+.h"
#include <WhirlyVector.h>

namespace WhirlyKit
{
class Sun {
public:
    Sun();
    Sun(int year, int month, int day, int hour, int minutes, int second);
    ~Sun();

    double sunLon, sunLat;
    double time;
    
    // Set the UTC time
    void setTime(int year, int month, int day, int hour, int minutes, int second);
    
    // Return a direction suitable for passing to a light
    Point3d getDirection();

private:
    void runCalculation(CAADate aaDate);
};

}

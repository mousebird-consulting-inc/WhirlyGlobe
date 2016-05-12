/*
 *  Sun.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting. All rights reserved.
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

#include "Sun.h"


namespace WhirlyKit {

Sun::Sun()
    : sunLon(0.0), sunLat(0.0), time(0.0)
{
}

Sun::Sun(int year, int month, int day, int hour, int minutes, int second)
    : sunLon(0.0), sunLat(0.0), time(0.0)
{
    setTime(year,month,day,hour,minutes,second);
}

Sun::~Sun()
{
}
    
void Sun::setTime(int year, int month, int day, int hour, int minutes, int second)
{
    CAADate aaDate(year, month, day, hour, minutes, second, true);
    this->runCalculation(aaDate);    
}

void Sun::runCalculation(CAADate aaDate)
{
    double jdSun = CAADynamicalTime::UTC2TT(aaDate.Julian());

    // Position of the sun in equatorial
    double sunEclipticLong = CAASun::ApparentEclipticLongitude(jdSun);
    double sunEclipticLat = CAASun::ApparentEclipticLatitude(jdSun);
    double obliquity = CAANutation::TrueObliquityOfEcliptic(jdSun);
    CAA2DCoordinate sunEquatorial = CAACoordinateTransformation::Ecliptic2Equatorial(sunEclipticLong,sunEclipticLat,obliquity);

    double siderealTime = CAASidereal::MeanGreenwichSiderealTime(jdSun);

    sunLon = CAACoordinateTransformation::DegreesToRadians(15*(sunEquatorial.X-siderealTime));
    sunLat = CAACoordinateTransformation::DegreesToRadians(sunEquatorial.Y);
}
    
Point3d Sun::getDirection()
{
    double z = sin(sunLat);
    double rad = sqrt(1.0-z*z);
    Point3d pt(rad*cos(sunLon),rad*sin(sunLon),z);

    return pt;
}

}

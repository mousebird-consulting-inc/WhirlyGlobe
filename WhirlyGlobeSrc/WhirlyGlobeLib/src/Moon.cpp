/*
 *  Moon.cpp
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

#include "Moon.h"

namespace WhirlyKit {

Moon::Moon(int year, int month, int day, int hour, int minutes, int second) :
    moonLon(0.0),
    moonLat(0.0),
    illuminatedFraction(0.0),
    phase(0.0)
{
    CAADate aaDate(year, month, day, hour, minutes, second, true);
    this->calculeValues(aaDate);
}

Moon::~Moon()
{
}
    
void Moon::calculeValues(CAADate aaDate)
{
    // Start with the Julian Date
    double jd = aaDate.Julian();
    
    // Position of the moon in equatorial
    double moonEclipticLong = CAAMoon::EclipticLongitude(jd);
    double moonEclipticLat = CAAMoon::EclipticLatitude(jd);
    double obliquity = CAANutation::MeanObliquityOfEcliptic(jd);
    CAA2DCoordinate moonEquatorial = CAACoordinateTransformation::Ecliptic2Equatorial(moonEclipticLong,moonEclipticLat,obliquity);
    
    double siderealTime = CAASidereal::MeanGreenwichSiderealTime(jd);
    
    this->moonLon = CAACoordinateTransformation::DegreesToRadians(15*(moonEquatorial.X-siderealTime));
    this->moonLat = CAACoordinateTransformation::DegreesToRadians(moonEquatorial.Y);
    
    // Need the sun too for the next bit
    double sunEclipticLong = CAASun::ApparentEclipticLongitude(jd);
    double sunEclipticLat = CAASun::ApparentEclipticLatitude(jd);
    CAA2DCoordinate sunEquatorial = CAACoordinateTransformation::Ecliptic2Equatorial(sunEclipticLong,sunEclipticLat,obliquity);
    
    // Now for the phase
    double geo_elongation = CAAMoonIlluminatedFraction::GeocentricElongation(moonEquatorial.X, moonEquatorial.Y, sunEquatorial.X, sunEquatorial.Y);
    
    double phaseAngle = CAAMoonIlluminatedFraction::PhaseAngle(geo_elongation, 368410.0, 149971520.0);
    double positionAngle = CAAMoonIlluminatedFraction::PositionAngle(moonEquatorial.X, moonEquatorial.Y, sunEquatorial.X, sunEquatorial.Y);
    this->illuminatedFraction = CAAMoonIlluminatedFraction::IlluminatedFraction(phaseAngle);
    
    this->phase = (positionAngle < 180 ? phaseAngle + 180 : 180 - phaseAngle);
}

}

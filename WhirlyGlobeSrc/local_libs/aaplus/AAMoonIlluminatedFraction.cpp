/*
Module : AAMOONILLUMINATEDFRACTION.CPP
Purpose: Implementation for the algorithms for the Moon's Elongation, Phase Angle and Illuminated Fraction
Created: PJN / 29-12-2003
History: PJN / 26-01-2007 1. Changed name of CAAMoonIlluminatedFraction::IluminatedFraction to 
                          CAAMoonIlluminatedFraction::IlluminatedFraction. Thanks to Ing. Taras Kapuszczak
                          for reporting this typo!.

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


//////////////////// Includes /////////////////////////////////////////////////

#include "stdafx.h"
#include "AAMoonIlluminatedFraction.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////// Implementation ///////////////////////////////////////////

double CAAMoonIlluminatedFraction::GeocentricElongation(double ObjectAlpha, double ObjectDelta, double SunAlpha, double SunDelta)
{
  //Convert the RA's to radians
  ObjectAlpha = CAACoordinateTransformation::DegreesToRadians(ObjectAlpha*15);
  SunAlpha = CAACoordinateTransformation::DegreesToRadians(SunAlpha*15);

  //Convert the declinations to radians
  ObjectDelta = CAACoordinateTransformation::DegreesToRadians(ObjectDelta);
  SunDelta = CAACoordinateTransformation::DegreesToRadians(SunDelta);

  //Return the result
  return CAACoordinateTransformation::RadiansToDegrees(acos(sin(SunDelta)*sin(ObjectDelta) + cos(SunDelta)*cos(ObjectDelta)*cos(SunAlpha - ObjectAlpha)));
}

double CAAMoonIlluminatedFraction::PhaseAngle(double GeocentricElongation, double EarthObjectDistance, double EarthSunDistance)
{
  //Convert from degrees to radians
  GeocentricElongation = CAACoordinateTransformation::DegreesToRadians(GeocentricElongation);

  //Return the result
  return CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(atan2(EarthSunDistance * sin(GeocentricElongation), EarthObjectDistance - EarthSunDistance*cos(GeocentricElongation))));
}

double CAAMoonIlluminatedFraction::IlluminatedFraction(double PhaseAngle)
{
  //Convert from degrees to radians
  PhaseAngle = CAACoordinateTransformation::DegreesToRadians(PhaseAngle);

  //Return the result
  return (1 + cos(PhaseAngle)) / 2;
}

double CAAMoonIlluminatedFraction::PositionAngle(double Alpha0, double Delta0, double Alpha, double Delta)
{
  //Convert to radians
  Alpha0 = CAACoordinateTransformation::HoursToRadians(Alpha0);
  Alpha = CAACoordinateTransformation::HoursToRadians(Alpha);
  Delta0 = CAACoordinateTransformation::DegreesToRadians(Delta0);
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);

  return CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(atan2(cos(Delta0)*sin(Alpha0 - Alpha), sin(Delta0)*cos(Delta) - cos(Delta0)*sin(Delta)*cos(Alpha0 - Alpha))));
}

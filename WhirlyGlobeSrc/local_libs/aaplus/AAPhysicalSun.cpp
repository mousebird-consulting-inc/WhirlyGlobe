/*
Module : AAPHYSICALSUN.CPP
Purpose: Implementation for the algorithms which obtain the physical parameters of the Sun
Created: PJN / 29-12-2003
History: PJN / 16-06-2004 1) Fixed a typo in the calculation of SunLongDash in CAAPhysicalSun::Calculate.
                          Thanks to Brian Orme for spotting this problem.

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////////////////// Includes ////////////////////////////////////

#include "stdafx.h"
#include "AAPhysicalSun.h"
#include "AASun.h"
#include "AAEarth.h"
#include "AANutation.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////////////////// Implementation ///////////////////////////////

CAAPhysicalSunDetails CAAPhysicalSun::Calculate(double JD)
{
  double theta = CAACoordinateTransformation::MapTo0To360Range((JD - 2398220) * 360 / 25.38);
  double I = 7.25;
  double K = 73.6667 + 1.3958333*(JD - 2396758)/36525;

  //Calculate the apparent longitude of the sun (excluding the effect of nutation)
  double L = CAAEarth::EclipticLongitude(JD);
  double R = CAAEarth::RadiusVector(JD);
  double SunLong = L + 180 - CAACoordinateTransformation::DMSToDegrees(0, 0, 20.4898 / R);

  double epsilon = CAANutation::TrueObliquityOfEcliptic(JD);

  //Convert to radians
  epsilon = CAACoordinateTransformation::DegreesToRadians(epsilon);
  SunLong = CAACoordinateTransformation::DegreesToRadians(SunLong);
  K = CAACoordinateTransformation::DegreesToRadians(K);
  I = CAACoordinateTransformation::DegreesToRadians(I);
  theta = CAACoordinateTransformation::DegreesToRadians(theta);

  double x = atan(-cos(SunLong)*tan(epsilon));
  double y = atan(-cos(SunLong - K)*tan(I));

  CAAPhysicalSunDetails details;

  details.P = CAACoordinateTransformation::RadiansToDegrees(x + y);
  details.B0 = CAACoordinateTransformation::RadiansToDegrees(asin(sin(SunLong - K)*sin(I)));

  double eta = atan(tan(SunLong - K)*cos(I));
  details.L0 = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(eta - theta));

  return details;
}

double CAAPhysicalSun::TimeOfStartOfRotation(long C)
{
  double JED = 2398140.2270 + 27.2752316*C;

  double M = CAACoordinateTransformation::MapTo0To360Range(281.96 + 26.882476*C);
  M = CAACoordinateTransformation::DegreesToRadians(M);

  JED += (0.1454*sin(M) - 0.0085*sin(2*M) - 0.0141*cos(2*M));

  return JED;
}

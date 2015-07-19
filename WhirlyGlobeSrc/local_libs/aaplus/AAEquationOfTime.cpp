/*
Module : AAEQUATIONOFTIME.CPP
Purpose: Implementation for the algorithms to calculate the "Equation of Time"
Created: PJN / 29-12-2003
History: PJN / 05-07-2005 1. Fix for a bug to ensure that values returned from CAAEquationOfTime::Calculate
                          does not return discontinuities. Instead it now returns negative values when
                          required.

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include "AAEquationOfTime.h"
#include "AA2DCoordinate.h"
#include "AACoordinateTransformation.h"
#include "AASun.h"
#include "AANutation.h"
#include <cmath>
using namespace std;


///////////////////////// Implementation //////////////////////////////////////

double CAAEquationOfTime::Calculate(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;
  double rho5 = rho4*rho;

  //Calculate the Suns mean longitude
  double L0 = CAACoordinateTransformation::MapTo0To360Range(280.4664567 + 360007.6982779*rho + 0.03032028*rhosquared +   
                                                            rhocubed / 49931 - rho4 / 15300 - rho5 / 2000000);

  //Calculate the Suns apparent right ascension
  double SunLong = CAASun::ApparentEclipticLongitude(JD);
  double SunLat = CAASun::ApparentEclipticLatitude(JD);
  double epsilon = CAANutation::TrueObliquityOfEcliptic(JD);
  CAA2DCoordinate Equatorial = CAACoordinateTransformation::Ecliptic2Equatorial(SunLong, SunLat, epsilon);

  epsilon = CAACoordinateTransformation::DegreesToRadians(epsilon);
  double E = L0 - 0.0057183 - Equatorial.X*15 + CAACoordinateTransformation::DMSToDegrees(0, 0, CAANutation::NutationInLongitude(JD))*cos(epsilon);
  if (E > 180)
    E = -(360 - E);
  E *= 4; //Convert to minutes of time
              
  return E;
}

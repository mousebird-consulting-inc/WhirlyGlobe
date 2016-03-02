/*
Module : AAPHYSICALMOON.H
Purpose: Implementation for the algorithms which obtain the physical parameters of the Moon
Created: PJN / 17-01-2004

Copyright (c) 2004 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////// Macros / Defines //////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __AAPHYSICALMOON_H_
#define __AAPHYSICALMOON_H_

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Includes //////////////////////////////////////////////

#include "AACoordinateTransformation.h"


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAPhysicalMoonDetails
{
public:
//Constructors / Destructors
  CAAPhysicalMoonDetails() : ldash(0), 
                             bdash(0), 
                             ldash2(0),
                             bdash2(0), 
                             l(0), 
                             b(0), 
                             P(0) 
  {
  };

//Member variables
  double ldash;
  double bdash;
  double ldash2;
  double bdash2;
  double l;
  double b;
  double P;
};

class AAPLUS_EXT_CLASS CAASelenographicMoonDetails
{
public:
//Constructors / Destructors
  CAASelenographicMoonDetails() : l0(0), 
                                  b0(0), 
                                  c0(0) 
  {
  };

//Member variables
  double l0;
  double b0;
  double c0;
};

class AAPLUS_EXT_CLASS CAAPhysicalMoon
{
public:
//Static methods
  static CAAPhysicalMoonDetails CalculateGeocentric(double JD);
  static CAAPhysicalMoonDetails CalculateTopocentric(double JD, double Longitude, double Latitude);
  static CAASelenographicMoonDetails CalculateSelenographicPositionOfSun(double JD);
  static double AltitudeOfSun(double JD, double Longitude, double Latitude);
  static double TimeOfSunrise(double JD, double Longitude, double Latitude);
  static double TimeOfSunset(double JD, double Longitude, double Latitude);

protected:
  static double SunriseSunsetHelper(double JD, double Longitude, double Latitude, bool bSunrise);
  static CAAPhysicalMoonDetails CalculateHelper(double JD, double& Lambda, double& Beta, double& epsilon, CAA2DCoordinate& Equatorial);
  static void CalculateOpticalLibration(double JD, double Lambda, double Beta, double& ldash, double& bdash, double& ldash2, double& bdash2, double& epsilon, double& omega, double& DeltaU, double& sigma, double& I, double& rho);
};

#endif //__AAPHYSICALMOON_H_

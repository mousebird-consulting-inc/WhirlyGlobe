/*
Module : AAPARALLAX.H
Purpose: Implementation for the algorithms which convert a geocentric set of coordinates to their topocentric equivalent
Created: PJN / 29-12-2003

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

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

#ifndef __AAPARALLAX_H__
#define __AAPARALLAX_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Includes //////////////////////////////////////////////

#include "AA2DCoordinate.h"


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAATopocentricEclipticDetails
{
public:
//Constructors / Destructors
  CAATopocentricEclipticDetails() : Lambda(0), 
                                    Beta(0), 
                                    Semidiameter(0) 
  {
  };

//Member variables
  double Lambda;
  double Beta;
  double Semidiameter;
};

class AAPLUS_EXT_CLASS CAAParallax
{
public:
//Conversion functions
  static CAA2DCoordinate Equatorial2TopocentricDelta(double Alpha, double Delta, double Distance, double Longitude, double Latitude, double Height, double JD);
  static CAA2DCoordinate Equatorial2Topocentric(double Alpha, double Delta, double Distance, double Longitude, double Latitude, double Height, double JD);
  static CAATopocentricEclipticDetails Ecliptic2Topocentric(double Lambda, double Beta, double Semidiameter, double Distance, double Epsilon, double Latitude, double Height, double JD);

  static double ParallaxToDistance(double Parallax);
  static double DistanceToParallax(double Distance);
};

#endif //__AAPARALLAX_H__

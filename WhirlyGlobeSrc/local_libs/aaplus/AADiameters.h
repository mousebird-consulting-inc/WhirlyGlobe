/*
Module : AADIAMETERS.H
Purpose: Implementation for the algorithms for the semi diameters of the Sun, Moon, Planets, and Asteroids
Created: PJN / 15-01-2004

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

#ifndef __AADIAMETERS_H__
#define __AADIAMETERS_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAADiameters
{
public:
//Static methods
  static double SunSemidiameterA(double Delta);
  static double MercurySemidiameterA(double Delta);
  static double VenusSemidiameterA(double Delta);
  static double MarsSemidiameterA(double Delta);
  static double JupiterEquatorialSemidiameterA(double Delta);
  static double JupiterPolarSemidiameterA(double Delta);
  static double SaturnEquatorialSemidiameterA(double Delta);
  static double SaturnPolarSemidiameterA(double Delta);
  static double ApparentSaturnPolarSemidiameterA(double Delta, double B);
  static double UranusSemidiameterA(double Delta);
  static double NeptuneSemidiameterA(double Delta);
  static double MercurySemidiameterB(double Delta);
  static double VenusSemidiameterB(double Delta);
  static double MarsSemidiameterB(double Delta);
  static double JupiterEquatorialSemidiameterB(double Delta);
  static double JupiterPolarSemidiameterB(double Delta);
  static double SaturnEquatorialSemidiameterB(double Delta);
  static double SaturnPolarSemidiameterB(double Delta);
  static double ApparentSaturnPolarSemidiameterB(double Delta, double B);
  static double UranusSemidiameterB(double Delta);
  static double NeptuneSemidiameterB(double Delta);
  static double PlutoSemidiameterB(double Delta);
  static double GeocentricMoonSemidiameter(double Delta);
  static double TopocentricMoonSemidiameter(double DistanceDelta, double Delta, double H, double Latitude, double Height);
  static double AsteroidDiameter(double H, double A);
  static double ApparentAsteroidDiameter(double H, double A);
};

#endif //__AADIAMETERS_H__

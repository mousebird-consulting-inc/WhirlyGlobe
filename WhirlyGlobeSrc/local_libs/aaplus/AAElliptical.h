/*
Module : AAELLIPTICAL.H
Purpose: Implementation for the algorithms for an elliptical orbit
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

#ifndef __AAELLIPTICAL_H__
#define __AAELLIPTICAL_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Includes //////////////////////////////////////////////

#include "AA3DCoordinate.h"


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAEllipticalObjectElements
{
public:
//Constructors / Destructors
  CAAEllipticalObjectElements() : a(0), 
                                  e(0), 
                                  i(0), 
                                  w(0), 
                                  omega(0), 
                                  JDEquinox(0), 
                                  T(0) 
  {
  };

//member variables
  double a; 
  double e;
  double i;
  double w;
  double omega;
  double JDEquinox;
  double T;
};

class AAPLUS_EXT_CLASS CAAEllipticalPlanetaryDetails
{
public:
//Constructors / Destructors
  CAAEllipticalPlanetaryDetails() : ApparentGeocentricLongitude(0), 
                                    ApparentGeocentricLatitude(0),
                                    ApparentGeocentricDistance(0), 
                                    ApparentLightTime(0),
                                    ApparentGeocentricRA(0), 
                                    ApparentGeocentricDeclination(0) 
  {
  };

//Member variables
  double ApparentGeocentricLongitude;
  double ApparentGeocentricLatitude;
  double ApparentGeocentricDistance;
  double ApparentLightTime;
  double ApparentGeocentricRA;
  double ApparentGeocentricDeclination;
};

class AAPLUS_EXT_CLASS CAAEllipticalObjectDetails
{
public:
//Constructors / Destructors
  CAAEllipticalObjectDetails() : HeliocentricEclipticLongitude(0), 
                                 HeliocentricEclipticLatitude(0), 
                                 TrueGeocentricRA(0), 
                                 TrueGeocentricDeclination(0),
                                 TrueGeocentricDistance(0), 
                                 TrueGeocentricLightTime(0),
                                 AstrometricGeocentricRA(0), 
                                 AstrometricGeocentricDeclination(0),
                                 AstrometricGeocentricDistance(0), 
                                 AstrometricGeocentricLightTime(0),
                                 Elongation(0), 
                                 PhaseAngle(0) 
  {
  };

//Member variables
  CAA3DCoordinate HeliocentricRectangularEquatorial;
  CAA3DCoordinate HeliocentricRectangularEcliptical;
  double HeliocentricEclipticLongitude; 
  double HeliocentricEclipticLatitude;
  double TrueGeocentricRA;
  double TrueGeocentricDeclination;
  double TrueGeocentricDistance;
  double TrueGeocentricLightTime;
  double AstrometricGeocentricRA;
  double AstrometricGeocentricDeclination;
  double AstrometricGeocentricDistance;
  double AstrometricGeocentricLightTime;
  double Elongation;
  double PhaseAngle;
};

class AAPLUS_EXT_CLASS CAAElliptical
{
public:
//Enums
  enum EllipticalObject
  {
    SUN,
    MERCURY,
    VENUS,
    MARS,
    JUPITER,
    SATURN,
    URANUS,
    NEPTUNE,
    PLUTO
  };

//Static methods
  static double DistanceToLightTime(double Distance);
  static CAAEllipticalPlanetaryDetails Calculate(double JD, EllipticalObject object);
  static double SemiMajorAxisFromPerihelionDistance(double q, double e);
  static double MeanMotionFromSemiMajorAxis(double a);
  static CAAEllipticalObjectDetails Calculate(double JD, const CAAEllipticalObjectElements& elements);
  static double InstantaneousVelocity(double r, double a);
  static double VelocityAtPerihelion(double e, double a);
  static double VelocityAtAphelion(double e, double a);
  static double LengthOfEllipse(double e, double a);
  static double CometMagnitude(double g, double delta, double k, double r);
  static double MinorPlanetMagnitude(double H, double delta, double G, double r, double PhaseAngle);
};

#endif //__AAELLIPTICAL_H__

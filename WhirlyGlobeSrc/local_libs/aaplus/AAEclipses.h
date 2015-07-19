/*
Module : AAECLIPSES.H
Purpose: Implementation for the algorithms which obtain the principal characteristics of an eclipse of the Sun or the Moon
Created: PJN / 21-01-2004

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

#ifndef __AAECLIPSES_H__
#define __AAECLIPSES_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAASolarEclipseDetails
{
public:
//Constructors / Destructors
  CAASolarEclipseDetails() : bEclipse(false), 
                             TimeOfMaximumEclipse(0), 
                             F(0), 
                             u(0), 
                             gamma(0), 
                             GreatestMagnitude(0) 
  {
  };

//Member variables
  bool   bEclipse;
  double TimeOfMaximumEclipse;
  double F;
  double u;
  double gamma;
  double GreatestMagnitude;
};


class AAPLUS_EXT_CLASS CAALunarEclipseDetails
{
public:
//Constructors / Destructors
  CAALunarEclipseDetails() : bEclipse(false), 
                             TimeOfMaximumEclipse(0),
                             F(0), 
                             u(0), 
                             gamma(0), 
                             PenumbralRadii(0),
                             UmbralRadii(0), 
                             PenumbralMagnitude(0), 
                             UmbralMagnitude(0), 
                             PartialPhaseSemiDuration(0),
                             TotalPhaseSemiDuration(0), 
                             PartialPhasePenumbraSemiDuration(0) 
  {
  };

//Member variables
  bool   bEclipse;
  double TimeOfMaximumEclipse;
  double F;
  double u;
  double gamma;
  double PenumbralRadii;
  double UmbralRadii;
  double PenumbralMagnitude;
  double UmbralMagnitude;
  double PartialPhaseSemiDuration;
  double TotalPhaseSemiDuration;
  double PartialPhasePenumbraSemiDuration;
};

class AAPLUS_EXT_CLASS CAAEclipses
{
public:
//Static methods
  static CAASolarEclipseDetails CalculateSolar(double k);
  static CAALunarEclipseDetails CalculateLunar(double k);

protected:
  static CAASolarEclipseDetails Calculate(double k, double& Mdash);
};

#endif //__AAECLIPSES_H__

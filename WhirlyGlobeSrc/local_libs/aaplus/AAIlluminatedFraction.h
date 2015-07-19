/*
Module : AAILLUMINATEDFRACTION.H
Purpose: Implementation for the algorithms for a Planet's Phase Angle, Illuminated Fraction and Magnitude
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

#ifndef __AAILLUMINATEDFRACTION_H__
#define __AAILLUMINATEDFRACTION_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAIlluminatedFraction
{
public:
//Static methods
  static double PhaseAngle(double r, double R, double Delta);
  static double PhaseAngle(double R, double R0, double B, double L, double L0, double Delta);
  static double PhaseAngleRectangular(double x, double y, double z, double B, double L, double Delta);
  static double IlluminatedFraction(double PhaseAngle);
  static double IlluminatedFraction(double r, double R, double Delta);
  static double MercuryMagnitudeMuller(double r, double Delta, double i);
  static double VenusMagnitudeMuller(double r, double Delta, double i);
  static double MarsMagnitudeMuller(double r, double Delta, double i);
  static double JupiterMagnitudeMuller(double r, double Delta);
  static double SaturnMagnitudeMuller(double r, double Delta, double DeltaU, double B);
  static double UranusMagnitudeMuller(double r, double Delta);
  static double NeptuneMagnitudeMuller(double r, double Delta);
  static double MercuryMagnitudeAA(double r, double Delta, double i);
  static double VenusMagnitudeAA(double r, double Delta, double i);
  static double MarsMagnitudeAA(double r, double Delta, double i);
  static double JupiterMagnitudeAA(double r, double Delta, double i);
  static double SaturnMagnitudeAA(double r, double Delta, double DeltaU, double B);
  static double UranusMagnitudeAA(double r, double Delta);
  static double NeptuneMagnitudeAA(double r, double Delta);
  static double PlutoMagnitudeAA(double r, double Delta);
};

#endif //__AAILLUMINATEDFRACTION_H__

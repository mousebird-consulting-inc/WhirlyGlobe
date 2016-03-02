/*
Module : AAGALILEANMOONS.H
Purpose: Implementation for the algorithms which obtain the positions of the 4 great moons of Jupiter
Created: PJN / 06-01-2004

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

#ifndef __AAGALILEANMOONS_H_
#define __AAGALILEANMOONS_H_

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Includes //////////////////////////////////////////////

#include "AA3DCoordinate.h"


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAGalileanMoonDetail
{
public:
//Constructors / Destructors
  CAAGalileanMoonDetail() : MeanLongitude(0), 
                            TrueLongitude(0), 
                            TropicalLongitude(0), 
                            EquatorialLatitude(0),
                            r(0), 
                            bInTransit(false), 
                            bInOccultation(false),
                            bInEclipse(false), 
                            bInShadowTransit(false) 
  {
  };

//Member variables
  double          MeanLongitude;
  double          TrueLongitude;
  double          TropicalLongitude;
  double          EquatorialLatitude;
  double          r;
  CAA3DCoordinate TrueRectangularCoordinates;
  CAA3DCoordinate ApparentRectangularCoordinates;
  bool            bInTransit;
  bool            bInOccultation;
  bool            bInEclipse;
  bool            bInShadowTransit;
};

class AAPLUS_EXT_CLASS CAAGalileanMoonsDetails
{
public:
//Member variables
  CAAGalileanMoonDetail Satellite1;
  CAAGalileanMoonDetail Satellite2;
  CAAGalileanMoonDetail Satellite3;
  CAAGalileanMoonDetail Satellite4;
};

class AAPLUS_EXT_CLASS CAAGalileanMoons
{
public:
//Static methods
  static CAAGalileanMoonsDetails Calculate(double JD);

protected:
  static CAAGalileanMoonsDetails CalculateHelper(double JD, double sunlongrad, double betarad, double R);
  static void Rotations(double X, double Y, double Z, double I, double psi, double i, double omega, double lambda0, double beta0, double& A6, double& B6, double& C6);
  static void FillInPhenomenaDetails(CAAGalileanMoonDetail& detail);
};

#endif //__AAGALILEANMOONS_H_

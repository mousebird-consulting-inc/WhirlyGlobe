/*
Module : AARISETRANSITSET.H
Purpose: Implementation for the algorithms which obtain the Rise, Transit and Set times
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

#ifndef __AARISETRANSITSET_H__
#define __AARISETRANSITSET_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAARiseTransitSetDetails
{
public:
//Constructors / Destructors
  CAARiseTransitSetDetails() : bRiseValid(false),
                               Rise(0),
                               bTransitAboveHorizon(false),
                               Transit(0),
                               bSetValid(false),
                               Set(0) 
  {
  };

//Member variables
  bool   bRiseValid;
  double Rise;
  bool   bTransitAboveHorizon;
  double Transit;
  bool   bSetValid;
  double Set;
};

class AAPLUS_EXT_CLASS CAARiseTransitSet
{
public:
//Static methods
  static CAARiseTransitSetDetails Calculate(double JD, double Alpha1, double Delta1, double Alpha2, double Delta2, double Alpha3, double Delta3, double Longitude, double Latitude, double h0);

protected:
//Static methods  
  static double CalculateTransit(double Alpha2, double theta0, double Longitude);
  static void CalculateRiseSet(double M0, double cosH0, CAARiseTransitSetDetails& details, double& M1, double& M2);
  static void CorrectRAValuesForInterpolation(double& Alpha1, double& Alpha2, double& Alpha3);
  static void CalculateRiseHelper(const CAARiseTransitSetDetails& details, double theta0, double deltaT, double Alpha1, double Delta1, double Alpha2, double Delta2, double Alpha3, 
                                  double Delta3, double Longitude, double Latitude, double LatitudeRad, double h0, double& M1);
  static void CalculateSetHelper(const CAARiseTransitSetDetails& details, double theta0, double deltaT, double Alpha1, double Delta1, double Alpha2, double Delta2, double Alpha3, 
                                 double Delta3, double Longitude, double Latitude, double LatitudeRad, double h0, double& M2);
  static void CalculateTransitHelper(double theta0, double deltaT, double Alpha1, double Alpha2, double Alpha3, double Longitude, double& M0);
  static void ConstraintM(double& M);
};

#endif //__AARISETRANSITSET_H__

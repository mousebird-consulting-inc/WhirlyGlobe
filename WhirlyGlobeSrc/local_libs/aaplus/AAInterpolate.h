/*
Module : AAINTERPOLATE.H
Purpose: Implementation for the algorithms for Interpolation
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

#ifndef __AAINTERPOLATE_H__
#define __AAINTERPOLATE_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAInterpolate
{
public:
//Static methods
  static double Interpolate(double n, double Y1, double Y2, double Y3);
  static double Interpolate(double n, double Y1, double Y2, double Y3, double Y4, double Y5);
  static double InterpolateToHalves(double Y1, double Y2, double Y3, double Y4);
  static double LagrangeInterpolate(double X, int n, double* pX, double* pY);
  static double Extremum(double Y1, double Y2, double Y3, double& nm);
  static double Extremum(double Y1, double Y2, double Y3, double Y4, double Y5, double& nm);
  static double Zero(double Y1, double Y2, double Y3);
  static double Zero(double Y1, double Y2, double Y3, double Y4, double Y5);
  static double Zero2(double Y1, double Y2, double Y3);
  static double Zero2(double Y1, double Y2, double Y3, double Y4, double Y5);
};

#endif //__AAINTERPOLATE_H__

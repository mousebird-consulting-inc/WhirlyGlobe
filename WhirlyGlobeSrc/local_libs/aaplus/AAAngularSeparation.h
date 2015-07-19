/*
Module : AAANGULARSEPARATION.H
Purpose: Implementation for the algorithms which obtain various separation distances between celestial objects
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

#ifndef __AAANGULARSEPARATION_H__
#define __AAANGULARSEPARATION_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


////////////////////// Classes ////////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAAngularSeparation
{
public:
//Static methods
  static double Separation(double Alpha1, double Delta1, double Alpha2, double Delta2);
  static double PositionAngle(double Alpha1, double Delta1, double Alpha2, double Delta2);
  static double DistanceFromGreatArc(double Alpha1, double Delta1, double Alpha2, double Delta2, double Alpha3, double Delta3);
  static double SmallestCircle(double Alpha1, double Delta1, double Alpha2, double Delta2, double Alpha3, double Delta3, bool& bType1);
};

#endif //__AAANGULARSEPARATION_H__

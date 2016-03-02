/*
Module : AAPHYSICALSUN.H
Purpose: Implementation for the algorithms which obtain the physical parameters of the Sun
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

#ifndef __AAPHYSICALSUN_H_
#define __AAPHYSICALSUN_H_

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


//////////////////////////// Classes //////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAPhysicalSunDetails
{
public:
//Constructors / Destructors
  CAAPhysicalSunDetails() : P(0), 
                            B0(0), 
                            L0(0) 
  {
  };

//Member variables
  double P;
  double B0;
  double L0;
};

class AAPLUS_EXT_CLASS CAAPhysicalSun
{
public:
//Static methods
  static CAAPhysicalSunDetails Calculate(double JD);
  static double TimeOfStartOfRotation(long C);
};

#endif //__AAPHYSICALSUN_H_

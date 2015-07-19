/*
Module : AASATURNRINGS.H
Purpose: Implementation for the algorithms which calculate various parameters related to the Rings of Saturn
Created: PJN / 08-01-2004

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

#ifndef __AASATURNRINGS_H_
#define __AASATURNRINGS_H_

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAASaturnRingDetails
{
public:
//Constructors / Destructors
  CAASaturnRingDetails() : B(0), 
                           Bdash(0), 
                           P(0),
                           a(0), 
                           b(0), 
                           DeltaU(0) 
  {
  };   

//Member variables
  double B;
  double Bdash;
  double P;
  double a;
  double b;
  double DeltaU;
};

class AAPLUS_EXT_CLASS CAASaturnRings
{
public:
//Static methods
  static CAASaturnRingDetails Calculate(double JD);
};

#endif //__AASATURNRINGS_H_

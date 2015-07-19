/*
Module : AABINARYSTAR.H
Purpose: Implementation for the algorithms for an binary stars system
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

#ifndef __AABINARYSTAR_H__
#define __AABINARYSTAR_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAABinaryStarDetails
{
public:
//Constructors / Destructors
  CAABinaryStarDetails() : r(0), 
                           Theta(0), 
                           Rho(0) 
  {
  };
  
//Member variables
  double r;
  double Theta;
  double Rho;
};

class AAPLUS_EXT_CLASS CAABinaryStar
{
public:
//Static methods
  static CAABinaryStarDetails Calculate(double t, double P, double T, double e, double a, double i, double omega, double w);
  static double ApparentEccentricity(double e, double i, double w);
};

#endif //__AABINARYSTAR_H_

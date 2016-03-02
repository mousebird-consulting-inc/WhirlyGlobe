/*
Module : AAMOONPERIGEEAPOGEE.H
Purpose: Implementation for the algorithms which obtain the dates of Lunar Apogee and Perigee
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

#ifndef __AAMOONPERIGEEAPOGEE_H__
#define __AAMOONPERIGEEAPOGEE_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAMoonPerigeeApogee
{
public:
//Static methods
  static double K(double Year);
  static double MeanPerigee(double k);
  static double MeanApogee(double k);
  static double TruePerigee(double k);
  static double TrueApogee(double k);
  static double PerigeeParallax(double k);
  static double ApogeeParallax(double k);
};

#endif //__AAMOONPERIGEEAPOGEE_H__

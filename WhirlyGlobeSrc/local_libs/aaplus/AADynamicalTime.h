/*
Module : AADYNAMICALTIME.H
Purpose: Implementation for the algorithms which provides for conversion between Universal Time (both UT1 and UTC) 
         and Terrestrial Time (TT) aka Terrestrial Dynamical Time (TDT) aka Ephemeris Time (ET)
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

#ifndef __AADYNAMICALTIME_H__
#define __AADYNAMICALTIME_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAADynamicalTime
{
public:
//Static methods
  static double DeltaT(double JD);
  static double CumulativeLeapSeconds(double JD);
  static double TT2UTC(double JD);
  static double UTC2TT(double JD);
  static double TT2TAI(double JD);
  static double TAI2TT(double JD);
  static double TT2UT1(double JD);
  static double UT12TT(double JD);
  static double UT1MinusUTC(double JD);
};

#endif //__AADYNAMICALTIME_H__

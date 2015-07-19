/*
Module : AAPLANETPERIHELIONAPHELION.H
Purpose: Implementation for the algorithms which obtain the dates of Perihelion and Aphelion of the planets
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

#ifndef __AAPLANETPERIHELIONAPHELION_H__
#define __AAPLANETPERIHELIONAPHELION_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAPlanetPerihelionAphelion
{
public:
//Static methods
  static long   MercuryK(double Year);
  static double MercuryPerihelion(long k);
  static double MercuryAphelion(long k);

  static long   VenusK(double Year);
  static double VenusPerihelion(long k);
  static double VenusAphelion(long k);

  static long   EarthK(double Year);
  static double EarthPerihelion(long k, bool bBarycentric = false);
  static double EarthAphelion(long k, bool bBarycentric = false);

  static long   MarsK(double Year);
  static double MarsPerihelion(long k);
  static double MarsAphelion(long k);

  static long   JupiterK(double Year);
  static double JupiterPerihelion(long k);
  static double JupiterAphelion(long k);

  static long   SaturnK(double Year);
  static double SaturnPerihelion(long k);
  static double SaturnAphelion(long k);

  static long   UranusK(double Year);
  static double UranusPerihelion(long k);
  static double UranusAphelion(long k);

  static long   NeptuneK(double Year);
  static double NeptunePerihelion(long k);
  static double NeptuneAphelion(long k);
};

#endif //__AAPLANETPERIHELIONAPHELION_H__

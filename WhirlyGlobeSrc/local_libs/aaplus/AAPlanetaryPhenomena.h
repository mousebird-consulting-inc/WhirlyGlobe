/*
Module : AAPLANETARYPHENOMENA.H
Purpose: Implementation for the algorithms which obtain the dates of various planetary phenomena
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

#ifndef __AAPLANETARYPHENOMENA_H__
#define __AAPLANETARYPHENOMENA_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAPlanetaryPhenomena
{
public:
//Enums
  enum PlanetaryObject
  {
    MERCURY,
    VENUS,
    MARS,
    JUPITER,
    SATURN,
    URANUS,
    NEPTUNE
  };

  enum EventType
  {
    INFERIOR_CONJUNCTION,
    SUPERIOR_CONJUNCTION,
    OPPOSITION,
    CONJUNCTION,
    EASTERN_ELONGATION,
    WESTERN_ELONGATION,
    STATION1,
    STATION2
  };

//Static methods
  static double K(double Year, PlanetaryObject object, EventType type);
  static double Mean(double k, PlanetaryObject object, EventType type);
  static double True(double k, PlanetaryObject object, EventType type);
  static double ElongationValue(double k, PlanetaryObject object, bool bEastern);
};

#endif //__AAPLANETARYPHENOMENA_H__

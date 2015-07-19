/*
Module : AAJEWISHCALENDAR.H
Purpose: Implementation for the algorithms which convert between the Gregorian, Julian and the Jewish calendar
Created: PJN / 04-02-2004

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

#ifndef __AAJEWISHDATE_H__
#define __AAJEWISHDATE_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Includes //////////////////////////////////////////////

#include "AADate.h"


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAJewishCalendar
{
public:
//Static methods
  static CAACalendarDate DateOfPesach(long Year, bool bGregorianCalendar = true);
  static bool IsLeap(long Year);
  static long DaysInYear(long Year);
};

#endif //__AAJEWISHCALENDAR_H__

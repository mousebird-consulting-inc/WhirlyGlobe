/*
Module : AAJEWISHCALENDAR.CPP
Purpose: Implementation for the algorithms which convert between the Gregorian and Julian calendars and the Jewish calendar
Created: PJN / 04-02-2004
History: PJN / 28-01-2007 1. Minor updates to fit in with new layout of CAADate class

Copyright (c) 2004 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


//////////////////////////// Includes /////////////////////////////////////////

#include "stdafx.h"
#include "AAJewishCalendar.h"
#include <cmath>
using namespace std;


//////////////////////////// Implementation ///////////////////////////////////

CAACalendarDate CAAJewishCalendar::DateOfPesach(long Year, bool bGregorianCalendar)
{
  //What will be the return value
  CAACalendarDate Pesach;

  long C = CAADate::INT(Year / 100.0);
  long S = CAADate::INT((3*C - 5) / 4.0);
  if (bGregorianCalendar == false)
    S = 0;
  long A = Year + 3760;
  long a = (12*Year + 12) % 19;
  long b = Year % 4;
  double Q = -1.904412361576 + 1.554241796621*a + 0.25*b - 0.003177794022*Year + S;
  long INTQ = CAADate::INT(Q);
  long j = (INTQ + 3*Year + 5*b+ 2 - S) % 7; 
  double r = Q - INTQ; 

  if ((j == 2) || (j == 4) || (j == 6))
    Pesach.Day = INTQ + 23;
  else if ((j == 1) && (a > 6) && (r >= 0.632870370))
    Pesach.Day = INTQ + 24;
  else if ((j == 0) && (a > 11) && (r >= 0.897723765))
    Pesach.Day = INTQ + 23;
  else
    Pesach.Day = INTQ + 22;

  if (Pesach.Day > 31)
  {
    Pesach.Month = 4;
    Pesach.Day -= 31;
  }
  else
    Pesach.Month = 3;

  Pesach.Year = A;

  return Pesach;
}

bool CAAJewishCalendar::IsLeap(long Year)
{
  long ymod19 = Year % 19;

  return (ymod19 == 0) || (ymod19 == 3) || (ymod19 == 6) || (ymod19 == 8) || (ymod19 == 11) || (ymod19 == 14) || (ymod19 == 17);
}

long CAAJewishCalendar::DaysInYear(long Year)
{
  //Find the previous civil year corresponding to the specified jewish year
  long CivilYear = Year - 3761;
  
  //Find the date of the next Jewish Year in that civil year
  CAACalendarDate CurrentPesach = DateOfPesach(CivilYear);
  bool bGregorian = CAADate::AfterPapalReform(CivilYear, CurrentPesach.Month, CurrentPesach.Day);
  CAADate CurrentYear(CivilYear, CurrentPesach.Month, CurrentPesach.Day, bGregorian);

  CAACalendarDate NextPesach = DateOfPesach(CivilYear+1);
  CAADate NextYear(CivilYear+1, NextPesach.Month, NextPesach.Day, bGregorian);

  return static_cast<long>(NextYear - CurrentYear);
}

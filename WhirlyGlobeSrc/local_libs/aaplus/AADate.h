/*
Module : AADATE.H
Purpose: Implementation for the algorithms which convert between the Gregorian and Julian calendars and the Julian Day
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

#ifndef __AADATE_H__
#define __AADATE_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAACalendarDate
{
public:
//Constructors / Destructors
  CAACalendarDate() : Year(0), 
                      Month(0), 
                      Day(0) 
  {
  };

//Member variables
  long Year;
  long Month;
  long Day;
};


class AAPLUS_EXT_CLASS CAADate
{
public:
//Enums
  enum DAY_OF_WEEK
  {	
    SUNDAY	  = 0,
    MONDAY	  = 1,
    TUESDAY	  = 2,
    WEDNESDAY	= 3,
    THURSDAY	= 4,
    FRIDAY	  = 5,
    SATURDAY	= 6
  };

//Constructors / Destructors
  CAADate();
  CAADate(long Year, long Month, double Day, bool bGregorianCalendar);
  CAADate(long Year, long Month, double Day, double Hour, double Minute, double Second, bool bGregorianCalendar);
  CAADate(double JD, bool bGregorianCalendar);

//Static Methods
  static double          DateToJD(long Year, long Month, double Day, bool bGregorianCalendar);
  static bool            IsLeap(long Year, bool bGregorianCalendar);
  static void            DayOfYearToDayAndMonth(long DayOfYear, bool bLeap, long& DayOfMonth, long& Month);
  static CAACalendarDate JulianToGregorian(long Year, long Month, long Day);
  static CAACalendarDate GregorianToJulian(long Year, long Month, long Day);
  static long            INT(double value);
  static bool            AfterPapalReform(long Year, long Month, double Day);
  static bool            AfterPapalReform(double JD);
  static double          DayOfYear(double JD, long Year, bool bGregorianCalendar);
  static long            DaysInMonth(long Month, bool bLeap);

//Non Static methods
  double      Julian() const { return m_dblJulian; };
  operator    double() const { return m_dblJulian; };
  long        Day() const;
  long        Month() const;
  long        Year() const;
  long        Hour() const;
  long        Minute() const;
  double      Second() const;
  void        Set(long Year, long Month, double Day, double Hour, double Minute, double Second, bool bGregorianCalendar);
  void        Set(double JD, bool bGregorianCalendar);
  void        SetInGregorianCalendar(bool bGregorianCalendar);
  void        Get(long& Year, long& Month, long& Day, long& Hour, long& Minute, double& Second) const;
  DAY_OF_WEEK DayOfWeek() const;
  double      DayOfYear() const;
  long        DaysInMonth() const;
  long        DaysInYear() const;
  bool        Leap() const;
  bool        InGregorianCalendar() const { return m_bGregorianCalendar; };
  double      FractionalYear() const;

protected:
//Member variables
  double m_dblJulian;  //Julian Day number for this date
  bool   m_bGregorianCalendar; //Is this date in the Gregorian calendar
};

#endif //__AADATE_H__

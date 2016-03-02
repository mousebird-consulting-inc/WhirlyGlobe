/*
Module : AAEASTER.CPP
Purpose: Implementation for the algorithms which calculate the date of Easter
Created: PJN / 29-12-2003
History: None

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////////// Includes //////////////////////////////////

#include "stdafx.h"
#include "AAEaster.h"


/////////////////////////////////// Implementation ////////////////////////////

CAAEasterDetails CAAEaster::Calculate(long nYear, bool GregorianCalendar)
{
  CAAEasterDetails details;

  if (GregorianCalendar)
  {                      
    int a = nYear % 19;
    int b = nYear / 100;
    int c = nYear % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b+8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19*a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2*e + 2*i - h -k) % 7;
    int m = (a + 11*h +22*l) / 451;
    int n = (h + l - 7*m + 114) / 31;
    int p = (h + l - 7*m + 114) % 31;
    details.Month = n;
    details.Day = p + 1;
  }
  else
  {                          
    int a = nYear % 4;
    int b = nYear % 7;
    int c = nYear % 19;
    int d = (19*c + 15) % 30;
    int e = (2*a + 4*b - d + 34) % 7;
    int f = (d + e + 114) / 31;
    int g = (d + e + 114) % 31;
    details.Month = f;
    details.Day = g + 1;
  }  

  return details;
}

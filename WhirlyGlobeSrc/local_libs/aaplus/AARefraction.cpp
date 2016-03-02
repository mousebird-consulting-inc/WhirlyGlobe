/*
Module : AAREFRACTION.CPP
Purpose: Implementation for the algorithms which model Atmospheric Refraction
Created: PJN / 29-12-2003
History: PJN / 26-11-2010 1. CAARefraction::RefractionFromApparent now returns a constant refraction value for all 
                          altitudes below a certain limit. Thanks to mehrzad khoddam for prompting this update.
                          2. CAARefraction::RefractionFromTrue now returns a constant refraction value for all 
                          altitudes below a certain limit. Thanks to mehrzad khoddam for prompting this update.

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


//////////////////////////////////// Includes /////////////////////////////////

#include "stdafx.h"
#include "AARefraction.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


/////////////////////////////////// Implementation ////////////////////////////

double CAARefraction::RefractionFromApparent(double Altitude, double Pressure, double Temperature)
{
  //return a constant value from this method if the altitude is below a specific value
  if (Altitude <= -1.6962987799993996)
    Altitude = -1.6962987799993996;

  double value = 1 / (tan(CAACoordinateTransformation::DegreesToRadians(Altitude + 7.31/(Altitude + 4.4)))) + 0.0013515;
  value *= (Pressure/1010 * 283/(273+Temperature));
  value /= 60;
  return value;
}

double CAARefraction::RefractionFromTrue(double Altitude, double Pressure, double Temperature)
{
  //return a constant value from this method if the altitude is below a specific value
  if (Altitude <= -1.9006387000003735)
    Altitude = -1.9006387000003735;

  double value = 1.02 / (tan(CAACoordinateTransformation::DegreesToRadians(Altitude + 10.3/(Altitude + 5.11)))) + 0.0019279;
  value *= (Pressure/1010 * 283/(273+Temperature));
  value /= 60;
  return value;
}

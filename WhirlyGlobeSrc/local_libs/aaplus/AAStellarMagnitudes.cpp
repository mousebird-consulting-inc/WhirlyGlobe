/*
Module : AASTELLARMAGNITUDES.CPP
Purpose: Implementation for the algorithms which operate on the stellar magntidue system
Created: PJN / 29-12-2003
History: PJN / 12-02-2004 1. Fixed a number of level 4 warnings when the code is compiled in VC.Net 2003

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


////////////////////////// Includes ///////////////////////////////////////////

#include "stdafx.h"
#include "AAStellarMagnitudes.h"
#include <cmath>
using namespace std;


////////////////////////// Implementation /////////////////////////////////////

double CAAStellarMagnitudes::CombinedMagnitude(double m1, double m2)
{
  double x = 0.4*(m2 - m1);
  return m2 - 2.5*log10(pow(10.0, x) + 1);
}

double CAAStellarMagnitudes::CombinedMagnitude(int Magnitudes, const double* pMagnitudes)
{
  double value = 0;
  for (int i=0; i<Magnitudes; i++)
    value += pow(10.0, -0.4*pMagnitudes[i]);

  return -2.5 * log10(value);
}

double CAAStellarMagnitudes::BrightnessRatio(double m1, double m2)
{
  double x = 0.4*(m2 - m1);
  return pow(10.0, x);
}

double CAAStellarMagnitudes::MagnitudeDifference(double brightnessRatio)
{
  return 2.5*log10(brightnessRatio);
}

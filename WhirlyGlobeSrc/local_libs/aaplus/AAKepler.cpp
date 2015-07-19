/*
Module : AAKEPLER.CPP
Purpose: Implementation for the algorithms which solve Kepler's equation
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


//////////////////// Includes /////////////////////////////////////////////////

#include "stdafx.h"
#include "AAKepler.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////// Implementation ///////////////////////////////////////////

double CAAKepler::Calculate(double M, double e, int nIterations)
{ 
  //Convert from degrees to radians
  M = CAACoordinateTransformation::DegreesToRadians(M);
  double PI = CAACoordinateTransformation::PI();

  double F = 1;
  if (M < 0)
    F = -1;
  M = fabs(M) / ( 2 * PI);
  M = (M - static_cast<int>(M))*2*PI*F;
  if (M < 0)
    M += 2*PI;
  F = 1;
  if (M > PI)
    F = -1;
  if (M > PI)
    M = 2*PI - M;

  double E = PI / 2; 
  double scale = PI / 4;
  for (int i=0; i<nIterations; i++)
  {
    double R = E - e*sin(E);
    if (M > R)
      E += scale;
    else
      E -= scale;
    scale /= 2; 
  }

  //Convert the result back to degrees
  return CAACoordinateTransformation::RadiansToDegrees(E) * F;
}

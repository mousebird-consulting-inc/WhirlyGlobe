/*
Module : AABINARYSTAR.CPP
Purpose: Implementation for the algorithms for an binary stars system
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
#include "AABinaryStar.h"
#include "AAKepler.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


////////////////////////////////// Implementation /////////////////////////////

CAABinaryStarDetails CAABinaryStar::Calculate(double t, double P, double T, double e, double a, double i, double omega, double w)
{
  double n = 360 / P;
  double M = CAACoordinateTransformation::MapTo0To360Range(n*(t - T));
  double E = CAAKepler::Calculate(M, e);
  E = CAACoordinateTransformation::DegreesToRadians(E);
  i = CAACoordinateTransformation::DegreesToRadians(i);
  w = CAACoordinateTransformation::DegreesToRadians(w);
  omega = CAACoordinateTransformation::DegreesToRadians(omega);

  CAABinaryStarDetails details;

  details.r = a*(1 - e*cos(E));

  double v = atan(sqrt((1 + e) / (1 - e)) * tan(E/2)) * 2;
  details.Theta = atan2(sin(v + w) * cos(i), cos(v + w)) + omega;
  details.Theta = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(details.Theta));

  double sinvw = sin(v + w);
  double cosvw = cos(v + w);
  double cosi = cos(i);
  details.Rho = details.r * sqrt((sinvw*sinvw*cosi*cosi) + (cosvw*cosvw));

  return details;
}

double CAABinaryStar::ApparentEccentricity(double e, double i, double w)
{
  i = CAACoordinateTransformation::DegreesToRadians(i);
  w = CAACoordinateTransformation::DegreesToRadians(w);

  double cosi = cos(i);
  double cosw = cos(w);
  double sinw = sin(w);
  double esquared = e*e;
  double A = (1 - esquared*cosw*cosw)*cosi*cosi;
  double B =  esquared*sinw*cosw*cosi;
  double C = 1 - esquared*sinw*sinw;
  double D = (A - C)*(A - C) + 4*B*B;

  double sqrtD = sqrt(D);
  return sqrt(2*sqrtD / (A + C + sqrtD));
}

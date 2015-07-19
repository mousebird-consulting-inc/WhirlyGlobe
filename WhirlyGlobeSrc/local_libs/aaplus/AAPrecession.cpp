/*
Module : AARPRECESSION.CPP
Purpose: Implementation for the algorithms for Precession
Created: PJN / 29-12-2003
History: PJN / 12-11-2014 1. Fixed two transcription bugs in the CAAPrecession::PrecessEquatorial method. The "0.000344*T" term was 
                          incorrectly using "0.0000344*T" when calculating "sigma" and the "0.000139*Tsquared" term was 
                          incorrectly using "0.000138*Tsquared" when calculating "zeta". Thanks to Erik Grosse for reporting this bug. 
                          The errors were so small that the values from the worked example of 21.b from the book ended up giving the 
                          same results. If a longer timespan was used for the example instead of the 28 years then the errors would 
                          have been easier to spot from the incorrect terms.

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////////////////// Includes ////////////////////////////////////

#include "stdafx.h"
#include "AAPrecession.h"
#include <cmath>
using namespace std;


///////////////////////////////// Implementation //////////////////////////////

CAA2DCoordinate CAAPrecession::AdjustPositionUsingUniformProperMotion(double t, double Alpha, double Delta, double PMAlpha, double PMDelta)
{
  CAA2DCoordinate value;
  value.X = Alpha + (PMAlpha * t / 3600);
  value.Y = Delta + (PMDelta * t / 3600);

  return value;
}

CAA2DCoordinate CAAPrecession::AdjustPositionUsingMotionInSpace(double r, double DeltaR, double t, double Alpha, double Delta, double PMAlpha, double PMDelta)
{
  //Convert DeltaR from km/s to Parsecs / Year
  DeltaR /= 977792;

  //Convert from seconds of time to Radians / Year
  PMAlpha /= 13751;

  //Convert from seconds of arc to Radians / Year
  PMDelta /= 206265;

  //Now convert to radians
  Alpha = CAACoordinateTransformation::HoursToRadians(Alpha);
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);

  double x = r * cos(Delta) * cos(Alpha);
  double y = r * cos(Delta) * sin(Alpha);
  double z = r * sin(Delta);

  double DeltaX = x/r*DeltaR - z*PMDelta*cos(Alpha) - y*PMAlpha;
  double DeltaY = y/r*DeltaR - z*PMDelta*sin(Alpha) + x*PMAlpha;
  double DeltaZ = z/r*DeltaR + r*PMDelta*cos(Delta);

  x += t*DeltaX;
  y += t*DeltaY;
  z += t*DeltaZ;

  CAA2DCoordinate value;
  value.X = CAACoordinateTransformation::RadiansToHours(atan2(y, x));
  if (value.X < 0)
    value.X += 24;
 
  value.Y = CAACoordinateTransformation::RadiansToDegrees(atan2(z, sqrt(x*x + y*y)));

  return value;
}

CAA2DCoordinate CAAPrecession::PrecessEquatorial(double Alpha, double Delta, double JD0, double JD)
{
  double T = (JD0 - 2451545.0) / 36525;
  double Tsquared = T*T;
  double t = (JD - JD0) / 36525;
  double tsquared = t*t;
  double tcubed  = tsquared * t;

  //Now convert to radians
  Alpha = CAACoordinateTransformation::HoursToRadians(Alpha);
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);

  double sigma = (2306.2181 + 1.39656*T - 0.000139*Tsquared)*t + (0.30188 - 0.000344*T)*tsquared + 0.017988*tcubed;
  sigma = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, sigma));

  double zeta = (2306.2181 + 1.39656*T - 0.000139*Tsquared)*t + (1.09468 + 0.000066*T)*tsquared + 0.018203*tcubed;
  zeta = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, zeta));

  double phi = (2004.3109 - 0.8533*T - 0.000217*Tsquared)*t -  (0.42665 + 0.000217*T)*tsquared - 0.041833*tcubed;
  phi = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, phi));

  double A = cos(Delta) * sin(Alpha + sigma);
  double B = cos(phi)*cos(Delta)*cos(Alpha + sigma) - sin(phi)*sin(Delta);
  double C = sin(phi)*cos(Delta)*cos(Alpha + sigma) + cos(phi)*sin(Delta);

  CAA2DCoordinate value;
  value.X = CAACoordinateTransformation::RadiansToHours(atan2(A, B) + zeta);  
  if (value.X < 0)
    value.X += 24;
  value.Y = CAACoordinateTransformation::RadiansToDegrees(asin(C));

  return value;
}

CAA2DCoordinate CAAPrecession::PrecessEquatorialFK4(double Alpha, double Delta, double JD0, double JD)
{
  double T = (JD0 - 2415020.3135) / 36524.2199;
  double t = (JD - JD0) / 36524.2199;
  double tsquared = t*t;
  double tcubed  = tsquared * t;

  //Now convert to radians
  Alpha = CAACoordinateTransformation::HoursToRadians(Alpha);
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);

  double sigma = (2304.250 + 1.396*T)*t + 0.302*tsquared + 0.018*tcubed;
  sigma = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, sigma));

  double zeta = 0.791*tsquared + 0.001*tcubed;
  zeta = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, zeta));
  zeta += sigma;

  double phi = (2004.682 - 0.853*T)*t - 0.426*tsquared - 0.042*tcubed;
  phi = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, phi));

  double A = cos(Delta) * sin(Alpha + sigma);
  double B = cos(phi)*cos(Delta)*cos(Alpha + sigma) - sin(phi)*sin(Delta);
  double C = sin(phi)*cos(Delta)*cos(Alpha + sigma) + cos(phi)*sin(Delta);

  CAA2DCoordinate value;
  value.X = CAACoordinateTransformation::RadiansToHours(atan2(A, B) + zeta);  
  if (value.X < 0)
    value.X += 24;
  value.Y = CAACoordinateTransformation::RadiansToDegrees(asin(C));

  return value;
}

CAA2DCoordinate CAAPrecession::PrecessEcliptic(double Lambda, double Beta, double JD0, double JD)
{
  double T = (JD0 - 2451545.0) / 36525;
  double Tsquared = T*T;
  double t = (JD - JD0) / 36525;
  double tsquared = t*t;
  double tcubed  = tsquared * t;

  //Now convert to radians
  Lambda = CAACoordinateTransformation::DegreesToRadians(Lambda);
  Beta = CAACoordinateTransformation::DegreesToRadians(Beta);

  double eta = (47.0029 - 0.06603*T + 0.000598*Tsquared)*t + (-0.03302 + 0.000598*T)*tsquared + 0.00006*tcubed;
  eta = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, eta));

  double pi = 174.876384*3600 + 3289.4789*T + 0.60622*Tsquared - (869.8089 + 0.50491*T)*t + 0.03536*tsquared;
  pi = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, pi));

  double p = (5029.0966 + 2.22226*T - 0.000042*Tsquared)*t + (1.11113 - 0.000042*T)*tsquared - 0.000006*tcubed;
  p = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::DMSToDegrees(0, 0, p));

  double A = cos(eta)*cos(Beta)*sin(pi - Lambda) - sin(eta)*sin(Beta);
  double B = cos(Beta)*cos(pi - Lambda);
  double C = cos(eta)*sin(Beta) + sin(eta)*cos(Beta)*sin(pi - Lambda);

  CAA2DCoordinate value;
  value.X = CAACoordinateTransformation::RadiansToDegrees(p + pi - atan2(A, B));  
  if (value.X < 0)
    value.X += 360;
  value.Y = CAACoordinateTransformation::RadiansToDegrees(asin(C));

  return value;
}

CAA2DCoordinate CAAPrecession::EquatorialPMToEcliptic(double Alpha, double Delta, double Beta, double PMAlpha, double PMDelta, double Epsilon)
{
  //Convert to radians
  Epsilon = CAACoordinateTransformation::DegreesToRadians(Epsilon);
  Alpha = CAACoordinateTransformation::HoursToRadians(Alpha);
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);
  Beta = CAACoordinateTransformation::DegreesToRadians(Beta);

  double cosb = cos(Beta);
  double sinEpsilon = sin(Epsilon);

  CAA2DCoordinate value;
  value.X = (PMDelta*sinEpsilon*cos(Alpha) + PMAlpha*cos(Delta)*(cos(Epsilon)*cos(Delta) + sinEpsilon*sin(Delta)*sin(Alpha)))/(cosb*cosb);
  value.Y = (PMDelta*(cos(Epsilon)*cos(Delta) + sinEpsilon*sin(Delta)*sin(Alpha)) - PMAlpha*sinEpsilon*cos(Alpha)*cos(Delta))/cosb;

  return value;
}

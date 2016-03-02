/*
Module : AAABERRATION.CPP
Purpose: Implementation for the algorithms for Aberration
Created: PJN / 29-12-2003
History: PJN / 21-04-2005 1. Renamed "AAAberation.cpp" to "AAAberration.cpp" so that all source code filenames 
                          match their corresponding header files. Thanks to Jürgen Schuck for suggesting this 
                          update.
         PJN / 18-03-2012 1. All global "g_*" tables are now const. Thanks to Roger Dahl for reporting this 
                          issue when compiling AA+ on ARM.

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////// Includes //////////////////////////////////////

#include "stdafx.h"
#include "AAAberration.h"
#include "AACoordinateTransformation.h"
#include "AAEarth.h"
#include "AASun.h"
#include <cmath>
using namespace std;


////////////////////////////// Macros / Defines ///////////////////////////////

struct AberrationCoefficient
{
  int L2;
  int L3;
  int L4;
  int L5;
  int L6;
  int L7;
  int L8;
  int Ldash;
  int D;
  int Mdash;
  int F;
  int xsin;
  int xsint;
  int xcos;
  int xcost;
  int ysin;
  int ysint;
  int ycos;
  int ycost;
  int zsin;
  int zsint;
  int zcos;
  int zcost;
};

const AberrationCoefficient g_AberrationCoefficients[] =
{ 
  //L2   L3   L4  L5  L6  L7  L8  Ldash D   Mdash F   xsin      xsint xcos    xcost ysin   ysint ycos     ycost zsin   zsint zcos    zcost
  {  0,  1,   0,  0,  0,  0,  0,  0,    0,  0,    0,  -1719914, -2,   -25,    0,    25,    -13,  1578089, 156,  10,    32,   684185, -358 },
  {  0,  2,   0,  0,  0,  0,  0,  0,    0,  0,    0,  6434,     141,  28007,  -107, 25697, -95,  -5904,   -130, 11141, -48,  -2559,  -55  },
  {  0,  0,   0,  1,  0,  0,  0,  0,    0,  0,    0,  715,      0,    0,      0,    6,     0,    -657,    0,    -15,   0,    -282,   0    },
  {  0,  0,   0,  0,  0,  0,  0,  1,    0,  0,    0,  715,      0,    0,      0,    0,     0,    -656,    0,    0,     0,    -285,   0    },
  {  0,  3,   0,  0,  0,  0,  0,  0,    0,  0,    0,  486,      -5,   -236,   -4,   -216,  -4,   -446,    5,    -94,   0,    -193,   0    },
  {  0,  0,   0,  0,  1,  0,  0,  0,    0,  0,    0,  159,      0,    0,      0,    2,     0,    -147,    0,    -6,    0,    -61,    0    },
  {  0,  0,   0,  0,  0,  0,  0,  0,    0,  0,    1,  0,        0,    0,      0,    0,     0,    26,      0,    0,     0,    -59,    0    },
  {  0,  0,   0,  0,  0,  0,  0,  1,    0,  1,    0,  39,       0,    0,      0,    0,     0,    -36,     0,    0,     0,    -16,    0    },
  {  0,  0,   0,  2,  0,  0,  0,  0,    0,  0,    0,  33,       0,    -10,    0,    -9,    0,    -30,     0,    -5,    0,    -13,    0    },
  {  0,  2,   0,  -1, 0,  0,  0,  0,    0,  0,    0,  31,       0,    1,      0,    1,     0,    -28,     0,    0,     0,    -12,    0    },
  {  0,  3,   -8, 3,  0,  0,  0,  0,    0,  0,    0,  8,        0,    -28,    0,    25,    0,    8,       0,    11,    0,    3,      0    },
  {  0,  5,   -8, 3,  0,  0,  0,  0,    0,  0,    0,  8,        0,    -28,    0,    -25,   0,    -8,      0,    -11,   0,    -3,     0    },
  {  2,  -1,  0,  0,  0,  0,  0,  0,    0,  0,    0,  21,       0,    0,      0,    0,     0,    -19,     0,    0,     0,    -8,     0    },
  {  1,  0,   0,  0,  0,  0,  0,  0,    0,  0,    0,  -19,      0,    0,      0,    0,     0,    17,      0,    0,     0,    8,      0    },
  {  0,  0,   0,  0,  0,  1,  0,  0,    0,  0,    0,  17,       0,    0,      0,    0,     0,    -16,     0,    0,     0,    -7,     0    },
  {  0,  1,   0,  -2, 0,  0,  0,  0,    0,  0,    0,  16,       0,    0,      0,    0,     0,    15,      0,    1,     0,    7,      0    },
  {  0,  0,   0,  0,  0,  0,  1,  0,    0,  0,    0,  16,       0,    0,      0,    1,     0,    -15,     0,    -3,    0,    -6,     0    },
  {  0,  1,   0,  1,  0,  0,  0,  0,    0,  0,    0,  11,       0,    -1,     0,    -1,    0,    -10,     0,    -1,    0,    -5,     0    },
  {  2,  -2,  0,  0,  0,  0,  0,  0,    0,  0,    0,  0,        0,    -11,    0,    -10,   0,    0,       0,    -4,    0,    0,      0    },
  {  0,  1,   0,  -1, 0,  0,  0,  0,    0,  0,    0,  -11,      0,    -2,     0,    -2,    0,    9,       0,    -1,    0,    4,      0    },
  {  0,  4,   0,  0,  0,  0,  0,  0,    0,  0,    0,  -7,       0,    -8,     0,    -8,    0,    6,       0,    -3,    0,    3,      0    },
  {  0,  3,   0,  -2, 0,  0,  0,  0,    0,  0,    0,  -10,      0,    0,      0,    0,     0,    9,       0,    0,     0,    4,      0    },
  {  1,  -2,  0,  0,  0,  0,  0,  0,    0,  0,    0,  -9,       0,    0,      0,    0,     0,    -9,      0,    0,     0,    -4,     0    },
  {  2,  -3,  0,  0,  0,  0,  0,  0,    0,  0,    0,  -9,       0,    0,      0,    0,     0,    -8,      0,    0,     0,    -4,     0    },
  {  0,  0,   0,  0,  2,  0,  0,  0,    0,  0,    0,  0,        0,    -9,     0,    -8,    0,    0,       0,    -3,    0,    0,      0    },
  {  2,  -4,  0,  0,  0,  0,  0,  0,    0,  0,    0,  0,        0,    -9,     0,    8,     0,    0,       0,    3,     0,    0,      0    },
  {  0,  3,   -2, 0,  0,  0,  0,  0,    0,  0,    0,  8,        0,    0,      0,    0,     0,    -8,      0,    0,     0,    -3,     0    },
  {  0,  0,   0,  0,  0,  0,  0,  1,    2,  -1,   0,  8,        0,    0,      0,    0,     0,    -7,      0,    0,     0,    -3,     0    },
  {  8,  -12, 0,  0,  0,  0,  0,  0,    0,  0,    0,  -4,       0,    -7,     0,    -6,    0,    4,       0,    -3,    0,    2,      0    },
  {  8,  -14, 0,  0,  0,  0,  0,  0,    0,  0,    0,  -4,       0,    -7,     0,    6,     0,    -4,      0,    3,     0,    -2,     0    },
  {  0,  0,   2,  0,  0,  0,  0,  0,    0,  0,    0,  -6,       0,    -5,     0,    -4,    0,    5,       0,    -2,    0,    2,      0    },
  {  3,  -4,  0,  0,  0,  0,  0,  0,    0,  0,    0,  -1,       0,    -1,     0,    -2,    0,    -7,      0,    1,     0,    -4,     0    },
  {  0,  2,   0,  -2, 0,  0,  0,  0,    0,  0,    0,  4,        0,    -6,     0,    -5,    0,    -4,      0,    -2,    0,    -2,     0    },
  {  3,  -3,  0,  0,  0,  0,  0,  0,    0,  0,    0,  0,        0,    -7,     0,    -6,    0,    0,       0,    -3,    0,    0,      0    },
  {  0,  2,   -2, 0,  0,  0,  0,  0,    0,  0,    0,  5,        0,    -5,     0,    -4,    0,    -5,      0,    -2,    0,    -2,     0    },
  {  0,  0,   0,  0,  0,  0,  0,  1,    -2, 0,    0,  5,        0,    0,      0,    0,     0,    -5,      0,    0,     0,    -2,     0    },
};


//////////////////////////////// Implementation ///////////////////////////////

CAA3DCoordinate CAAAberration::EarthVelocity(double JD)
{
  double T = (JD - 2451545) / 36525;
  double L2 = 3.1761467 + 1021.3285546 * T;
  double L3 = 1.7534703 + 628.3075849 * T;
  double L4 = 6.2034809 + 334.0612431 * T;
  double L5 = 0.5995465 + 52.9690965 * T;
  double L6 = 0.8740168 + 21.3299095 * T;
  double L7 = 5.4812939 + 7.4781599 * T;
  double L8 = 5.3118863 + 3.8133036 * T;
  double Ldash = 3.8103444 + 8399.6847337 * T;
  double D = 5.1984667 + 7771.3771486 * T;
  double Mdash = 2.3555559 + 8328.6914289 * T;
  double F = 1.6279052 + 8433.4661601 * T;

  CAA3DCoordinate velocity;

  int nAberrationCoefficients = sizeof(g_AberrationCoefficients) / sizeof(AberrationCoefficient);
  for (int i=0; i<nAberrationCoefficients; i++)
  {
    double Argument = g_AberrationCoefficients[i].L2*L2 + g_AberrationCoefficients[i].L3*L3 + 
                      g_AberrationCoefficients[i].L4*L4 + g_AberrationCoefficients[i].L5*L5 + 
                      g_AberrationCoefficients[i].L6*L6 + g_AberrationCoefficients[i].L7*L7 +  
                      g_AberrationCoefficients[i].L8*L8 + g_AberrationCoefficients[i].Ldash*Ldash + 
                      g_AberrationCoefficients[i].D*D + g_AberrationCoefficients[i].Mdash*Mdash + 
                      g_AberrationCoefficients[i].F*F;
    velocity.X += (g_AberrationCoefficients[i].xsin + g_AberrationCoefficients[i].xsint * T) * sin(Argument);
    velocity.X += (g_AberrationCoefficients[i].xcos + g_AberrationCoefficients[i].xcost * T) * cos(Argument);

    velocity.Y += (g_AberrationCoefficients[i].ysin + g_AberrationCoefficients[i].ysint * T) * sin(Argument);
    velocity.Y += (g_AberrationCoefficients[i].ycos + g_AberrationCoefficients[i].ycost * T) * cos(Argument);

    velocity.Z += (g_AberrationCoefficients[i].zsin + g_AberrationCoefficients[i].zsint * T) * sin(Argument);
    velocity.Z += (g_AberrationCoefficients[i].zcos + g_AberrationCoefficients[i].zcost * T) * cos(Argument);
  }

  return velocity;
}

CAA2DCoordinate CAAAberration::EquatorialAberration(double Alpha, double Delta, double JD)
{
  //Convert to radians
  Alpha = CAACoordinateTransformation::DegreesToRadians(Alpha*15);
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);

  double cosAlpha = cos(Alpha);
  double sinAlpha = sin(Alpha);
  double cosDelta = cos(Delta);
  double sinDelta = sin(Delta);

  CAA3DCoordinate velocity = EarthVelocity(JD);

  //What is the return value
  CAA2DCoordinate aberration;

  aberration.X = CAACoordinateTransformation::RadiansToHours((velocity.Y * cosAlpha - velocity.X * sinAlpha) / ( 17314463350.0 * cosDelta));
  aberration.Y = CAACoordinateTransformation::RadiansToDegrees(- (((velocity.X * cosAlpha + velocity.Y * sinAlpha) * sinDelta - velocity.Z * cosDelta) / 17314463350.0));

  return aberration;
}

CAA2DCoordinate CAAAberration::EclipticAberration(double Lambda, double Beta, double JD)
{
  //What is the return value
  CAA2DCoordinate aberration;

  double T = (JD - 2451545) / 36525;
  double Tsquared = T*T;
  double e = 0.016708634 - 0.000042037*T - 0.0000001267*Tsquared;
  double pi = 102.93735 + 1.71946*T + 0.00046*Tsquared;
  double k = 20.49552;
  double SunLongitude = CAASun::GeometricEclipticLongitude(JD);

  //Convert to radians
  pi = CAACoordinateTransformation::DegreesToRadians(pi);
  Lambda = CAACoordinateTransformation::DegreesToRadians(Lambda);
  Beta = CAACoordinateTransformation::DegreesToRadians(Beta);
  SunLongitude = CAACoordinateTransformation::DegreesToRadians(SunLongitude);

  aberration.X = (-k*cos(SunLongitude - Lambda) + e*k*cos(pi - Lambda)) / cos(Beta) / 3600;
  aberration.Y = -k*sin(Beta)*(sin(SunLongitude - Lambda) - e*sin(pi - Lambda)) / 3600;

  return aberration;
}

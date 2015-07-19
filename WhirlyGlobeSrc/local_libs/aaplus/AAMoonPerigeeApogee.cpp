/*
Module : AAMOONPERIGEEAPOGEE.CPP
Purpose: Implementation for the algorithms which obtain the dates of Lunar Apogee and Perigee
Created: PJN / 29-12-2003
History: PJN / 07-02-2009 1. Fixed a seemingly copy and paste bug in CAAMoonPerigeeApogee::TruePerigee. The 
                          layout of the code to accumulate the "Sigma" value was incorrect. The terms involving
                          T (e.g. +0.00019*T, -0.00013*T etc were adding these terms to the argument of the sin 
                          incorrectly. With the bug fixed the worked example 50.a from the book gives: 2447442.3543003569 
                          JDE or 1988 October 7 at 20h:30m:11.5 seconds. The previous buggy code was giving the same value 
                          of 2447442.3543003569, but this would be the case because T was a small value in the example. 
                          You would expect the error in the calculated to be bigger as the date departs from the Epoch 
                          2000.0. Thanks to Neoklis Kyriazis for reporting this bug.
         PJN / 03-07-2010 1. Fixed a bug in the g_MoonPerigeeApogeeCoefficients3 table. The "+0.013*cos(4D - 2F)" term was 
                          incorrectly using "+0.013*cos(4D - 20F)". The error in the lunar distance because of this coding 
                          error is of the order of 1 to 2 KM. Thanks to Thomas Meyer for reporting this bug.
                          2. Fixed a bug in the g_MoonPerigeeApogeeCoefficients1 table. The "D+2M-0.0010" term was 
                          incorrectly using "D+2M-0.0011". Thanks to Thomas Meyer for reporting this bug.
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


////////////////////////////////// Includes ///////////////////////////////////

#include "stdafx.h"
#include "AAMoonPerigeeApogee.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


////////////////////////////////// Macros / Defines ///////////////////////////

struct MoonPerigeeApogeeCoefficient
{
  int D;
  int M;
  int F;
  double C;
  double T;
};

const MoonPerigeeApogeeCoefficient g_MoonPerigeeApogeeCoefficients1[] =
{ 
  { 2,  0,  0,  -1.6769,  0 },
  { 4,  0,  0,  0.4589,   0 },
  { 6,  0,  0,  -0.1856,  0 },
  { 8,  0,  0,  0.0883,   0 },
  { 2,  -1, 0,  -0.0773,  0.00019 },
  { 0,  1,  0,  0.0502,   -0.00013 },
  { 10, 0,  0,  -0.0460,  0 },
  { 4,  -1, 0,  0.0422,   -0.00011 },
  { 6,  -1, 0,  -0.0256,  0 },
  { 12, 0,  0,  0.0253,   0 },
  { 1,  0,  0,  0.0237,   0 },
  { 8,  -1, 0,  0.0162,   0 },
  { 14, 0,  0,  -0.0145,  0 },
  { 0,  0,  2,  0.0129,   0 },
  { 3,  0,  0,  -0.0112,  0 },
  { 10, -1, 0,  -0.0104,  0 },
  { 16, 0,  0,  0.0086,   0 },
  { 12, -1, 0,  0.0069,   0 },
  { 5,  0,  0,  0.0066,   0 },
  { 2,  0,  2,  -0.0053,  0 },
  { 18, 0,  0,  -0.0052,  0 },
  { 14, -1, 0,  -0.0046,  0 },
  { 7,  0,  0,  -0.0041,  0 },
  { 2,  1,  0,  0.0040,   0 },
  { 20, 0,  0,  0.0032,   0 },
  { 1,  1,  0,  -0.0032,  0 },
  { 16, -1, 0,  0.0031,   0 },
  { 4,  1,  0,  -0.0029,  0 },
  { 9,  0,  0,  0.0027,   0 },
  { 4,  0,  2,  0.0027,   0 },
  { 2,  -2, 0,  -0.0027,  0 },
  { 4,  -2, 0,  0.0024,   0 },
  { 6,  -2, 0,  -0.0021,  0 },
  { 22, 0,  0,  -0.0021,  0 },
  { 18, -1, 0,  -0.0021,  0 },
  { 6,  1,  0,  0.0019,   0 },
  { 11, 0,  0,  -0.0018,  0 },
  { 8,  1,  0,  -0.0014,  0 },
  { 4,  0,  -2, -0.0014,  0 },
  { 6,  0,  2,  -0.0014,  0 },
  { 3,  1,  0,  0.0014,   0 },
  { 5,  1,  0,  -0.0014,  0 },
  { 13, 0,  0,  0.0013,   0 },
  { 20, -1, 0,  0.0013,   0 },
  { 3,  2,  0,  0.0011,   0 },
  { 4,  -2, 2,  -0.0011,  0 },
  { 1,  2,  0,  -0.0010,  0 },
  { 22, -1, 0,  -0.0009,  0 },
  { 0,  0,  4,  -0.0008,  0 },
  { 6,  0,  -2, 0.0008,   0 },
  { 2,  1,  -2, 0.0008,   0 },
  { 0,  2,  0 , 0.0007,   0 },
  { 0,  -1, 2,  0.0007,   0 },
  { 2,  0,  4,  0.0007,   0 },
  { 0,  -2, 2,  -0.0006,  0 },
  { 2,  2,  -2, -0.0006,  0 },
  { 24, 0,  0,  0.0006,   0 },
  { 4,  0,  -4, 0.0005,   0 },
  { 2,  2,  0,  0.0005,   0 },
  { 1,  -1, 0,  -0.0004,  0 }
}; 

const MoonPerigeeApogeeCoefficient g_MoonPerigeeApogeeCoefficients2[] =
{ 
  { 2,  0,  0,  0.4392,   0 },
  { 4,  0,  0,  0.0684,   0 },
  { 0,  1,  0,  0.0456,   -0.00011 },
  { 2,  -1, 0,  0.0426,   -0.00011 },
  { 0,  0,  2,  0.0212,   0 },
  { 1,  0,  0,  -0.0189,  0 },
  { 6,  0,  0,  0.0144,   0 },
  { 4,  -1, 0,  0.0113,   0 },
  { 2,  0,  2,  0.0047,   0 },
  { 1,  1,  0,  0.0036,   0 },
  { 8,  0,  0,  0.0035,   0 },
  { 6,  -1, 0,  0.0034,   0 },
  { 2,  0,  -2, -0.0034,  0 },
  { 2,  -2, 0,  0.0022,   0 },
  { 3,  0,  0,  -0.0017,  0 },
  { 4,  0,  2,  0.0013,   0 },
  { 8,  -1, 0,  0.0011,   0 },
  { 4,  -2, 0,  0.0010,   0 },
  { 10, 0,  0,  0.0009,   0 },
  { 3,  1,  0,  0.0007,   0 },
  { 0,  2,  0,  0.0006,   0 },
  { 2,  1,  0,  0.0005,   0 },
  { 2,  2,  0,  0.0005,   0 },
  { 6,  0,  2,  0.0004,   0 },
  { 6,  -2, 0,  0.0004,   0 },
  { 10, -1, 0,  0.0004,   0 },
  { 5,  0,  0,  -0.0004,  0 },
  { 4,  0,  -2, -0.0004,  0 },
  { 0,  1,  2,  0.0003,   0 },
  { 12, 0,  0,  0.0003,   0 },
  { 2,  -1, 2,  0.0003,   0 },
  { 1,  -1, 0,  -0.0003,  0 }    
};

const MoonPerigeeApogeeCoefficient g_MoonPerigeeApogeeCoefficients3[] =
{ 
  { 2,  0,  0,   63.224,   0        },
  { 4,  0,  0,   -6.990,   0        },
  { 2,  -1, 0,   2.834,    0        },
  { 2,  -1, 0,   0,        -0.0071  },
  { 6,  0,  0,   1.927,    0        },
  { 1,  0,  0,   -1.263,   0        },
  { 8,  0,  0,   -0.702,   0        },
  { 0,  1,  0,   0.696,    0        },
  { 0,  1,  0,   0,        -0.0017  },
  { 0,  0,  2,   -0.690,   0        },
  { 4,  -1, 0,   -0.629,   0        },
  { 4,  -1, 0,   0,        0.0016   },
  { 2,  0,  -2,  -0.392,   0        },
  { 10, 0,  0,   0.297,    0        },
  { 6,  -1, 0,   0.260,    0        },
  { 3,  0,  0,   0.201,    0        },
  { 2,  1,  0,   -0.161,   0        },
  { 1,  1,  0,   0.157,    0        },
  { 12, 0,  0,   -0.138,   0        },
  { 8,  -1, 0,   -0.127,   0        },
  { 2,  0,  2,   0.104,    0        },
  { 2,  -2, 0,   0.104,    0        },
  { 5,  0,  0,   -0.079,   0        },
  { 14, 0,  0,   0.068,    0        },
  { 10, -1, 0,   0.067,    0        },
  { 4,  1,  0,   0.054,    0        },
  { 12, -1, 0,   -0.038,   0        },
  { 4,  -2, 0,   -0.038,   0        },
  { 7,  0,  0,   0.037,    0        },
  { 4,  0,  2,   -0.037,   0        },
  { 16, 0,  0,   -0.035,   0        },
  { 3,  1,  0,   -0.030,   0        },
  { 1,  -1, 0,   0.029,    0        },
  { 6,  1,  0,   -0.025,   0        },
  { 0,  2,  0,   0.023,    0        },
  { 14, -1, 0,   0.023,    0        },
  { 2,  2,  0,   -0.023,   0        },
  { 6,  -2, 0,   0.022,    0        },
  { 2,  -1, -2,  -0.021,   0        },
  { 9,   0, 0,   -0.020,   0        },
  { 18, 0,  0,   0.019,    0        },
  { 6,  0,  2,   0.017,    0        },
  { 0,  -1, 2,   0.014,    0        },
  { 16, -1, 0,   -0.014,   0        },
  { 4,  0,  -2,  0.013,    0        }, 
  { 8,  1,  0,   0.012,    0        },
  { 11, 0,  0,   0.011,    0        },
  { 5,  1,  0,   0.010,    0        },
  { 20, 0,  0,   -0.010,   0        }
};

const MoonPerigeeApogeeCoefficient g_MoonPerigeeApogeeCoefficients4[] =
{
  { 2,  0,  0,  -9.147,     0      },
  { 1,  0,  0,  -0.841,     0      },
  { 0,  0,  2,  0.697,      0      },
  { 0,  1,  0,  -0.656,     0.0016 },
  { 4,  0,  0,  0.355,      0      },
  { 2,  -1, 0,  0.159,      0      },
  { 1,  1,  0,  0.127,      0      },
  { 4,  -1, 0,  0.065,      0      },
  { 6,  0,  0,  0.052,      0      },
  { 2,  1,  0,  0.043,      0      },
  { 2,  0,  2,  0.031,      0      },
  { 2,  0,  -2, -0.023,     0      },
  { 2,  -2, 0,  0.022,      0      },
  { 2,  2,  0,  0.019,      0      },
  { 0,  2,  0,  -0.016,     0      },
  { 6,  -1, 0,  0.014,      0      },
  { 8,  0,  0,  0.010,      0      }   
};


//////////////////////////////// Implementation ///////////////////////////////

double CAAMoonPerigeeApogee::K(double Year)
{
  return 13.2555*(Year - 1999.97);
}

double CAAMoonPerigeeApogee::MeanPerigee(double k)
{
  //convert from K to T
  double T = k/1325.55;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;
  double T4 = Tcubed*T;

  return 2451534.6698 + 27.55454989*k - 0.0006691*Tsquared - 0.000001098*Tcubed + 0.0000000052*T4;
}

double CAAMoonPerigeeApogee::MeanApogee(double k)
{
  //Uses the same formula as MeanPerigee
  return MeanPerigee(k);
}

double CAAMoonPerigeeApogee::TruePerigee(double k)
{
  double MeanJD = MeanPerigee(k);

  //convert from K to T
  double T = k/1325.55;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;
  double T4 = Tcubed*T;

  double D = CAACoordinateTransformation::MapTo0To360Range(171.9179 + 335.9106046*k - 0.0100383*Tsquared - 0.00001156*Tcubed + 0.000000055*T4);
  D = CAACoordinateTransformation::DegreesToRadians(D);
  double M = CAACoordinateTransformation::MapTo0To360Range(347.3477 + 27.1577721*k - 0.0008130*Tsquared - 0.0000010*Tcubed);
  M = CAACoordinateTransformation::DegreesToRadians(M);
  double F = CAACoordinateTransformation::MapTo0To360Range(316.6109 + 364.5287911*k - 0.0125053*Tsquared - 0.0000148*Tcubed);
  F = CAACoordinateTransformation::DegreesToRadians(F);

  int nPerigeeCoefficients = sizeof(g_MoonPerigeeApogeeCoefficients1) / sizeof(MoonPerigeeApogeeCoefficient);
  double Sigma = 0;
  for (int i=0; i<nPerigeeCoefficients; i++)
  {
    Sigma += (g_MoonPerigeeApogeeCoefficients1[i].C + T*g_MoonPerigeeApogeeCoefficients1[i].T) * sin(D*g_MoonPerigeeApogeeCoefficients1[i].D + M*g_MoonPerigeeApogeeCoefficients1[i].M +
                                                                                                     F*g_MoonPerigeeApogeeCoefficients1[i].F);
  }

  return MeanJD + Sigma;
}

double CAAMoonPerigeeApogee::PerigeeParallax(double k)
{
  //convert from K to T
  double T = k/1325.55;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;
  double T4 = Tcubed*T;

  double D = CAACoordinateTransformation::MapTo0To360Range(171.9179 + 335.9106046*k - 0.0100383*Tsquared - 0.00001156*Tcubed + 0.000000055*T4);
  D = CAACoordinateTransformation::DegreesToRadians(D);
  double M = CAACoordinateTransformation::MapTo0To360Range(347.3477 + 27.1577721*k - 0.0008130*Tsquared - 0.0000010*Tcubed);
  M = CAACoordinateTransformation::DegreesToRadians(M);
  double F = CAACoordinateTransformation::MapTo0To360Range(316.6109 + 364.5287911*k - 0.0125053*Tsquared - 0.0000148*Tcubed);
  F = CAACoordinateTransformation::DegreesToRadians(F);

  int nPerigeeCoefficients = sizeof(g_MoonPerigeeApogeeCoefficients3) / sizeof(MoonPerigeeApogeeCoefficient);
  double Parallax = 3629.215;
  for (int i=0; i<nPerigeeCoefficients; i++)
  {
    Parallax += (g_MoonPerigeeApogeeCoefficients3[i].C + T*g_MoonPerigeeApogeeCoefficients3[i].T) * cos(D*g_MoonPerigeeApogeeCoefficients3[i].D + M*g_MoonPerigeeApogeeCoefficients3[i].M +
                                                                                                        F*g_MoonPerigeeApogeeCoefficients3[i].F);
  }

  return Parallax/3600;
}

double CAAMoonPerigeeApogee::TrueApogee(double k)
{
  double MeanJD = MeanApogee(k);

  //convert from K to T
  double T = k/1325.55;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;
  double T4 = Tcubed*T;

  double D = CAACoordinateTransformation::MapTo0To360Range(171.9179 + 335.9106046*k - 0.0100383*Tsquared - 0.00001156*Tcubed + 0.000000055*T4);
  D = CAACoordinateTransformation::DegreesToRadians(D);
  double M = CAACoordinateTransformation::MapTo0To360Range(347.3477 + 27.1577721*k - 0.0008130*Tsquared - 0.0000010*Tcubed);
  M = CAACoordinateTransformation::DegreesToRadians(M);
  double F = CAACoordinateTransformation::MapTo0To360Range(316.6109 + 364.5287911*k - 0.0125053*Tsquared - 0.0000148*Tcubed);
  F = CAACoordinateTransformation::DegreesToRadians(F);

  int nApogeeCoefficients = sizeof(g_MoonPerigeeApogeeCoefficients2) / sizeof(MoonPerigeeApogeeCoefficient);
  double Sigma = 0;
  for (int i=0; i<nApogeeCoefficients; i++)
  {
    Sigma += (g_MoonPerigeeApogeeCoefficients2[i].C + T*g_MoonPerigeeApogeeCoefficients2[i].T) * sin(D*g_MoonPerigeeApogeeCoefficients2[i].D + M*g_MoonPerigeeApogeeCoefficients2[i].M +
                                                                                                     F*g_MoonPerigeeApogeeCoefficients2[i].F);
  }

  return MeanJD + Sigma;
}

double CAAMoonPerigeeApogee::ApogeeParallax(double k)
{
  //convert from K to T
  double T = k/1325.55;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;
  double T4 = Tcubed*T;

  double D = CAACoordinateTransformation::MapTo0To360Range(171.9179 + 335.9106046*k - 0.0100383*Tsquared - 0.00001156*Tcubed + 0.000000055*T4);
  D = CAACoordinateTransformation::DegreesToRadians(D);
  double M = CAACoordinateTransformation::MapTo0To360Range(347.3477 + 27.1577721*k - 0.0008130*Tsquared - 0.0000010*Tcubed);
  M = CAACoordinateTransformation::DegreesToRadians(M);
  double F = CAACoordinateTransformation::MapTo0To360Range(316.6109 + 364.5287911*k - 0.0125053*Tsquared - 0.0000148*Tcubed);
  F = CAACoordinateTransformation::DegreesToRadians(F);

  int nApogeeCoefficients = sizeof(g_MoonPerigeeApogeeCoefficients4) / sizeof(MoonPerigeeApogeeCoefficient);
  double Parallax = 3245.251;
  for (int i=0; i<nApogeeCoefficients; i++)
  {
    Parallax += (g_MoonPerigeeApogeeCoefficients4[i].C + T*g_MoonPerigeeApogeeCoefficients4[i].T) * cos(D*g_MoonPerigeeApogeeCoefficients4[i].D + M*g_MoonPerigeeApogeeCoefficients4[i].M +
                                                                                                        F*g_MoonPerigeeApogeeCoefficients4[i].F);
  }

  return Parallax / 3600;
}

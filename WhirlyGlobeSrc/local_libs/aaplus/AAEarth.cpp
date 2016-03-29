/*
Module : AAEARTH.CPP
Purpose: Implementation for the algorithms which calculate the position of Earth
Created: PJN / 29-12-2003
History: PJN / 03-10-2009 1. Fixed a copy and paste gremlin in the CAAEarth::EclipticLatitude method where it incorrectly
                          used B2, B3 & B4 coefficient terms for Venus. Due to how this bug occurred, the magnitude of the 
                          error from it would increase as the date deviated from the year 2000. Thanks to Isaac Salzman 
                          for reporting this bug.   
         PJN / 18-03-2012 1. All global "g_*" tables are now const. Thanks to Roger Dahl for reporting this 
                          issue when compiling AA+ on ARM.
         PJN / 04-08-2013 1. Fixed a transcription error in the twenty first coefficient used to calculate
                          the L0 term for the ecliptic longitude of Earth. Thanks to Isaac Clark for
                          reporting this issue. 
                          2. Fixed a transcription error in the sixteenth coefficient used to calculate the L1 term for 
                          the ecliptic longitude of Earth. Thanks to Isaac Clark for reporting this issue. 
                          3. Spot tests indicate that these two changes only affected the the ecliptic longitude in the 
                          eight decimal place.
                          4. Updated copyright details

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


///////////////////////////// Includes ////////////////////////////////////////

#include "stdafx.h"
#include "AAEarth.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////////////// Macros / Defines /////////////////////////////////

struct VSOP87Coefficient
{
  double A;
  double B;
  double C;
};

const VSOP87Coefficient g_L0EarthCoefficients[] =
{ 
  { 175347046, 0,         0 },
  { 3341656,   4.6692568, 6283.0758500 },
  { 34894,     4.62610,   12566.15170 },
  { 3497,      2.7441,    5753.3849 },
  { 3418,      2.8289,    3.5231 },
  { 3136,      3.6277,    77713.7715 },
  { 2676,      4.4181,    7860.4194 },
  { 2343,      6.1352,    3930.2097 },
  { 1324,      0.7425,    11506.7698 },
  { 1273,      2.0371,    529.6910 },
  { 1199,      1.1096,    1577.3435 },
  { 990,       5.233,     5884.927 },
  { 902,       2.045,     26.298 },
  { 857,       3.508,     398.149 },
  { 780,       1.179,     5223.694 },
  { 753,       2.533,     5507.553 },
  { 505,       4.583,     18849.228 },
  { 492,       4.205,     775.523 },
  { 357,       2.920,     0.067 },
  { 317,       5.849,     11790.629 },
  { 284,       1.899,     796.298 },
  { 271,       0.315,     10977.079 },
  { 243,       0.345,     5486.778 },
  { 206,       4.806,     2544.314 },
  { 205,       1.869,     5573.143 },
  { 202,       2.458,     6069.777 },
  { 156,       0.833,     213.299 },
  { 132,       3.411,     2942.463 },
  { 126,       1.083,     20.775 },
  { 115,       0.645,     0.980 },
  { 103,       0.636,     4694.003 },
  { 102,       0.976,     15720.839 },
  { 102,       4.267,     7.114 },
  { 99,        6.21,      2146.17 },
  { 98,        0.68,      155.42 },
  { 86,        5.98,      161000.69 },
  { 85,        1.30,      6275.96 },
  { 85,        3.67,      71430.70 },
  { 80,        1.81,      17260.15 },
  { 79,        3.04,      12036.46 },
  { 75,        1.76,      5088.63 },
  { 74,        3.50,      3154.69 },
  { 74,        4.68,      801.82 },
  { 70,        0.83,      9437.76 },
  { 62,        3.98,      8827.39 },
  { 61,        1.82,      7084.90 },
  { 57,        2.78,      6286.60 },
  { 56,        4.39,      14143.50 },
  { 56,        3.47,      6279.55 },
  { 52,        0.19,      12139.55 },
  { 52,        1.33,      1748.02 },
  { 51,        0.28,      5856.48 },
  { 49,        0.49,      1194.45 },
  { 41,        5.37,      8429.24 },
  { 41,        2.40,      19651.05 },
  { 39,        6.17,      10447.39 },
  { 37,        6.04,      10213.29 },
  { 37,        2.57,      1059.38 },
  { 36,        1.71,      2352.87 },
  { 36,        1.78,      6812.77 },
  { 33,        0.59,      17789.85 },
  { 30,        0.44,      83996.85 },
  { 30,        2.74,      1349.87 },
  { 25,        3.16,      4690.48 }
};

const VSOP87Coefficient g_L1EarthCoefficients[] =
{ 
  { 628331966747.0, 0,          0 },
  { 206059,         2.678235,   6283.075850 },
  { 4303,           2.6351,     12566.1517 },
  { 425,            1.590,      3.523 },
  { 119,            5.796,      26.298 },
  { 109,            2.966,      1577.344 },
  { 93,             2.59,       18849.23 },
  { 72,             1.14,       529.69 },
  { 68,             1.87,       398.15 },
  { 67,             4.41,       5507.55 },
  { 59,             2.89,       5223.69 },
  { 56,             2.17,       155.42 },
  { 45,             0.40,       796.30 },
  { 36,             0.47,       775.52 },
  { 29,             2.65,       7.11 },
  { 21,             5.34,       0.98 },
  { 19,             1.85,       5486.78 },
  { 19,             4.97,       213.30 },
  { 17,             2.99,       6275.96 },
  { 16,             0.03,       2544.31 },
  { 16,             1.43,       2146.17 },
  { 15,             1.21,       10977.08 },
  { 12,             2.83,       1748.02 },
  { 12,             3.26,       5088.63 },
  { 12,             5.27,       1194.45 },
  { 12,             2.08,       4694.00 },
  { 11,             0.77,       553.57 },
  { 10,             1.30,       6286.60 },
  { 10,             4.24,       1349.87 },
  { 9,              2.70,       242.73 },
  { 9,              5.64,       951.72 },
  { 8,              5.30,       2352.87 },
  { 6,              2.65,       9437.76 },
  { 6,              4.67,       4690.48 }             
};

const VSOP87Coefficient g_L2EarthCoefficients[] =
{ 
  { 52919,  0,      0 },
  { 8720,   1.0721, 6283.0758 },
  { 309,    0.867,  12566.152 },
  { 27,     0.05,   3.52 },
  { 16,     5.19,   26.30 },
  { 16,     3.68,   155.42 },
  { 10,     0.76,   18849.23 },
  { 9,      2.06,   77713.77 },
  { 7,      0.83,   775.52 },
  { 5,      4.66,   1577.34 },
  { 4,      1.03,   7.11 },
  { 4,      3.44,   5573.14 },
  { 3,      5.14,   796.30 },
  { 3,      6.05,   5507.55 },
  { 3,      1.19,   242.73 },
  { 3,      6.12,   529.69 },
  { 3,      0.31,   398.15 },
  { 3,      2.28,   553.57 },
  { 2,      4.38,   5223.69 },
  { 2,      3.75,   0.98 }     
};

const VSOP87Coefficient g_L3EarthCoefficients[] =
{ 
  { 289, 5.844, 6283.076 },
  { 35,  0,     0 },
  { 17,  5.49,  12566.15 },
  { 3,   5.20,  155.42 },
  { 1,   4.72,  3.52 },
  { 1,   5.30,  18849.23 },
  { 1,   5.97,  242.73 }
};

const VSOP87Coefficient g_L4EarthCoefficients[] =
{ 
  { 114, 3.142,  0 }, 
  { 8,   4.13,   6283.08 },
  { 1,   3.84,   12566.15 }
};

const VSOP87Coefficient g_L5EarthCoefficients[] =
{ 
  { 1, 3.14, 0 },
};

const VSOP87Coefficient g_B0EarthCoefficients[] =
{ 
  { 280, 3.199, 84334.662 },
  { 102, 5.422, 5507.553 },
  { 80,  3.88,  5223.69},
  { 44,  3.70,  2352.87 },
  { 32,  4.00,  1577.34 }
};

const VSOP87Coefficient g_B1EarthCoefficients[] =
{ 
  { 9, 3.90, 5507.55 },
  { 6, 1.73, 5223.69}
};

const VSOP87Coefficient g_R0EarthCoefficients[] =
{ 
  { 100013989,  0,          0 },
  { 1670700,    3.0984635,  6283.0758500 },
  { 13956,      3.05525,    12566.15170 },
  { 3084,       5.1985,     77713.7715 },
  { 1628,       1.1739,     5753.3849 },
  { 1576,       2.8469,     7860.4194 },
  { 925,        5.453,      11506.770 },
  { 542,        4.564,      3930.210 },
  { 472,        3.661,      5884.927 },
  { 346,        0.964,      5507.553 },
  { 329,        5.900,      5223.694 },
  { 307,        0.299,      5573.143 },
  { 243,        4.273,      11790.629 },
  { 212,        5.847,      1577.344 },
  { 186,        5.022,      10977.079 },
  { 175,        3.012,      18849.228 },
  { 110,        5.055,      5486.778 },
  { 98,         0.89,       6069.78 },
  { 86,         5.69,       15720.84 },
  { 86,         1.27,       161000.69},
  { 65,         0.27,       17260.15 },
  { 63,         0.92,       529.69 },
  { 57,         2.01,       83996.85 },
  { 56,         5.24,       71430.70 },
  { 49,         3.25,       2544.31 },
  { 47,         2.58,       775.52 },
  { 45,         5.54,       9437.76 },
  { 43,         6.01,       6275.96 },
  { 39,         5.36,       4694.00 },
  { 38,         2.39,       8827.39 },
  { 37,         0.83,       19651.05 },
  { 37,         4.90,       12139.55 },
  { 36,         1.67,       12036.46 },
  { 35,         1.84,       2942.46 },
  { 33,         0.24,       7084.90 },
  { 32,         0.18,       5088.63 },
  { 32,         1.78,       398.15 },
  { 28,         1.21,       6286.60 },
  { 28,         1.90,       6279.55 },
  { 26,         4.59,       10447.39 }
};

const VSOP87Coefficient g_R1EarthCoefficients[] =
{ 
  { 103019, 1.107490, 6283.075850 },
  { 1721,   1.0644,   12566.1517 },
  { 702,    3.142,    0 },
  { 32,     1.02,     18849.23 },
  { 31,     2.84,     5507.55 },
  { 25,     1.32,     5223.69 },
  { 18,     1.42,     1577.34 },
  { 10,     5.91,     10977.08 },
  { 9,      1.42,     6275.96 },
  { 9,      0.27,     5486.78 } 
};

const VSOP87Coefficient g_R2EarthCoefficients[] =
{ 
  { 4359, 5.7846, 6283.0758 },
  { 124,  5.579,  12566.152 },
  { 12,   3.14,   0 },
  { 9,    3.63,   77713.77 },
  { 6,    1.87,   5573.14 },
  { 3,    5.47,   18849.23 } 
};

const VSOP87Coefficient g_R3EarthCoefficients[] =
{ 
  { 145,  4.273,  6283.076 },
  { 7,    3.92,   12566.15 }
};

const VSOP87Coefficient g_R4EarthCoefficients[] =
{ 
  { 4, 2.56, 6283.08 }
};

const VSOP87Coefficient g_L1EarthCoefficientsJ2000[] =
{ 
  { 628307584999.0, 0,          0 },
  { 206059,         2.678235,   6283.075850 },
  { 4303,           2.6351,     12566.1517 },
  { 425,            1.590,      3.523 },
  { 119,            5.796,      26.298 },
  { 109,            2.966,      1577.344 },
  { 93,             2.59,       18849.23 },
  { 72,             1.14,       529.69 },
  { 68,             1.87,       398.15 },
  { 67,             4.41,       5507.55 },
  { 59,             2.89,       5223.69 },
  { 56,             2.17,       155.42 },
  { 45,             0.40,       796.30 },
  { 36,             0.47,       775.52 },
  { 29,             2.65,       7.11 },
  { 21,             5.43,       0.98 },
  { 19,             1.85,       5486.78 },
  { 19,             4.97,       213.30 },
  { 17,             2.99,       6275.96 },
  { 16,             0.03,       2544.31 },
  { 16,             1.43,       2146.17 },
  { 15,             1.21,       10977.08 },
  { 12,             2.83,       1748.02 },
  { 12,             3.26,       5088.63 },
  { 12,             5.27,       1194.45 },
  { 12,             2.08,       4694.00 },
  { 11,             0.77,       553.57 },
  { 10,             1.30,       6286.60 },
  { 10,             4.24,       1349.87 },
  { 9,              2.70,       242.73 },
  { 9,              5.64,       951.72 },
  { 8,              5.30,       2352.87 },
  { 6,              2.65,       9437.76 },
  { 6,              4.67,       4690.48 }             
};

const VSOP87Coefficient g_L2EarthCoefficientsJ2000[] =
{ 
  { 8722, 1.0725, 6283.0758 },
  { 991,  3.1416, 0 },
  { 295,  0.437,  12566.152 },
  { 27,   0.05,   3.52 },
  { 16,   5.19,   26.30 },
  { 16,   3.69,   155.42 },
  { 9,    0.30,   18849.23 },
  { 9,    2.06,   77713.77 },
  { 7,    0.83,   775.52 },
  { 5,    4.66,   1577.34 },
  { 4,    1.03,   7.11 },
  { 4,    3.44,   5573.14 },
  { 3,    5.14,   796.30 },
  { 3,    6.05,   5507.55 },
  { 3,    1.19,   242.73 },
  { 3,    6.12,   529.69 },
  { 3,    0.30,   398.15 },
  { 3,    2.28,   553.57 },
  { 2,    4.38,   5223.69 },
  { 2,    3.75,   0.98 }
};

const VSOP87Coefficient g_L3EarthCoefficientsJ2000[] =
{ 
  { 289,  5.842,  6283.076 },
  { 21,   6.05,   12566.15 },
  { 3,    5.20,   155.42 },
  { 3,    3.14,   0 },
  { 1,    4.72,   3.52 },
  { 1,    5.97,   242.73 },
  { 1,    5.54,   18849.23 }
};

const VSOP87Coefficient g_L4EarthCoefficientsJ2000[] =
{ 
  { 8,  4.14, 6283.08 },
  { 1,  3.28, 12566.15 }
};

const VSOP87Coefficient g_B1EarthCoefficientsJ2000[] =
{ 
  { 227778, 3.413766, 6283.075850 },
  { 3806,   3.3706,   12566.1517 },
  { 3620,   0,        0 },
  { 72,     3.33,     18849.23 },
  { 8,      3.89,     5507.55 },
  { 8,      1.79,     5223.69 },
  { 6,      5.20,     2352.87 }
};

const VSOP87Coefficient g_B2EarthCoefficientsJ2000[] =
{ 
  { 9721, 5.1519, 6283.07585 },
  { 233,  3.1416, 0 },
  { 134,  0.644,  12566.152 },
  { 7,    1.07,   18849.23 }
};

const VSOP87Coefficient g_B3EarthCoefficientsJ2000[] =
{ 
  { 276,  0.595,  6283.076 },
  { 17,   3.14,   0 },
  { 4,    0.12,   12566.15 }
};

const VSOP87Coefficient g_B4EarthCoefficientsJ2000[] =
{ 
  { 6,  2.27, 6283.08 },
  { 1,  0,    0 }
};


//////////////////////////////// Implementation ///////////////////////////////

double CAAEarth::EclipticLongitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;
  double rho5 = rho4*rho;

  //Calculate L0
  int nL0Coefficients = sizeof(g_L0EarthCoefficients) / sizeof(VSOP87Coefficient);
  double L0 = 0;
  int i;
  for (i=0; i<nL0Coefficients; i++)
    L0 += g_L0EarthCoefficients[i].A * cos(g_L0EarthCoefficients[i].B + g_L0EarthCoefficients[i].C*rho);

  //Calculate L1
  int nL1Coefficients = sizeof(g_L1EarthCoefficients) / sizeof(VSOP87Coefficient);
  double L1 = 0;
  for (i=0; i<nL1Coefficients; i++)
    L1 += g_L1EarthCoefficients[i].A * cos(g_L1EarthCoefficients[i].B + g_L1EarthCoefficients[i].C*rho);

  //Calculate L2
  int nL2Coefficients = sizeof(g_L2EarthCoefficients) / sizeof(VSOP87Coefficient);
  double L2 = 0;
  for (i=0; i<nL2Coefficients; i++)
    L2 += g_L2EarthCoefficients[i].A * cos(g_L2EarthCoefficients[i].B + g_L2EarthCoefficients[i].C*rho);

  //Calculate L3
  int nL3Coefficients = sizeof(g_L3EarthCoefficients) / sizeof(VSOP87Coefficient);
  double L3 = 0;
  for (i=0; i<nL3Coefficients; i++)
    L3 += g_L3EarthCoefficients[i].A * cos(g_L3EarthCoefficients[i].B + g_L3EarthCoefficients[i].C*rho);

  //Calculate L4
  int nL4Coefficients = sizeof(g_L4EarthCoefficients) / sizeof(VSOP87Coefficient);
  double L4 = 0;
  for (i=0; i<nL4Coefficients; i++)
    L4 += g_L4EarthCoefficients[i].A * cos(g_L4EarthCoefficients[i].B + g_L4EarthCoefficients[i].C*rho);

  //Calculate L5
  int nL5Coefficients = sizeof(g_L5EarthCoefficients) / sizeof(VSOP87Coefficient);
  double L5 = 0;
  for (i=0; i<nL5Coefficients; i++)
    L5 += g_L5EarthCoefficients[i].A * cos(g_L5EarthCoefficients[i].B + g_L5EarthCoefficients[i].C*rho);

  double value = (L0 + L1*rho + L2*rhosquared + L3*rhocubed + L4*rho4 + L5*rho5) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(value));
  return value;
}

double CAAEarth::EclipticLatitude(double JD)
{
  double rho = (JD - 2451545) / 365250;

  //Calculate B0
  int nB0Coefficients = sizeof(g_B0EarthCoefficients) / sizeof(VSOP87Coefficient);
  double B0 = 0;
  int i;
  for (i=0; i<nB0Coefficients; i++)
    B0 += g_B0EarthCoefficients[i].A * cos(g_B0EarthCoefficients[i].B + g_B0EarthCoefficients[i].C*rho);

  //Calculate B1
  int nB1Coefficients = sizeof(g_B1EarthCoefficients) / sizeof(VSOP87Coefficient);
  double B1 = 0;
  for (i=0; i<nB1Coefficients; i++)
    B1 += g_B1EarthCoefficients[i].A * cos(g_B1EarthCoefficients[i].B + g_B1EarthCoefficients[i].C*rho);

  //Note for Earth there are no B2, B3 or B4 coefficients to calculate

  double value = (B0 + B1*rho) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  return value;
}

double CAAEarth::RadiusVector(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate R0
  int nR0Coefficients = sizeof(g_R0EarthCoefficients) / sizeof(VSOP87Coefficient);
  double R0 = 0;
  int i;
  for (i=0; i<nR0Coefficients; i++)
    R0 += g_R0EarthCoefficients[i].A * cos(g_R0EarthCoefficients[i].B + g_R0EarthCoefficients[i].C*rho);

  //Calculate R1
  int nR1Coefficients = sizeof(g_R1EarthCoefficients) / sizeof(VSOP87Coefficient);
  double R1 = 0;
  for (i=0; i<nR1Coefficients; i++)
    R1 += g_R1EarthCoefficients[i].A * cos(g_R1EarthCoefficients[i].B + g_R1EarthCoefficients[i].C*rho);

  //Calculate R2
  int nR2Coefficients = sizeof(g_R2EarthCoefficients) / sizeof(VSOP87Coefficient);
  double R2 = 0;
  for (i=0; i<nR2Coefficients; i++)
    R2 += g_R2EarthCoefficients[i].A * cos(g_R2EarthCoefficients[i].B + g_R2EarthCoefficients[i].C*rho);

  //Calculate R3
  int nR3Coefficients = sizeof(g_R3EarthCoefficients) / sizeof(VSOP87Coefficient);
  double R3 = 0;
  for (i=0; i<nR3Coefficients; i++)
    R3 += g_R3EarthCoefficients[i].A * cos(g_R3EarthCoefficients[i].B + g_R3EarthCoefficients[i].C*rho);

  //Calculate R4
  int nR4Coefficients = sizeof(g_R4EarthCoefficients) / sizeof(VSOP87Coefficient);
  double R4 = 0;
  for (i=0; i<nR4Coefficients; i++)
    R4 += g_R4EarthCoefficients[i].A * cos(g_R4EarthCoefficients[i].B + g_R4EarthCoefficients[i].C*rho);

  return (R0 + R1*rho + R2*rhosquared + R3*rhocubed + R4*rho4) / 100000000;
}


double CAAEarth::EclipticLongitudeJ2000(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate L0
  int nL0Coefficients = sizeof(g_L0EarthCoefficients) / sizeof(VSOP87Coefficient);
  double L0 = 0;
  int i;
  for (i=0; i<nL0Coefficients; i++)
    L0 += g_L0EarthCoefficients[i].A * cos(g_L0EarthCoefficients[i].B + g_L0EarthCoefficients[i].C*rho);

  //Calculate L1
  int nL1Coefficients = sizeof(g_L1EarthCoefficientsJ2000) / sizeof(VSOP87Coefficient);
  double L1 = 0;
  for (i=0; i<nL1Coefficients; i++)
    L1 += g_L1EarthCoefficientsJ2000[i].A * cos(g_L1EarthCoefficientsJ2000[i].B + g_L1EarthCoefficientsJ2000[i].C*rho);

  //Calculate L2
  int nL2Coefficients = sizeof(g_L2EarthCoefficientsJ2000) / sizeof(VSOP87Coefficient);
  double L2 = 0;
  for (i=0; i<nL2Coefficients; i++)
    L2 += g_L2EarthCoefficientsJ2000[i].A * cos(g_L2EarthCoefficientsJ2000[i].B + g_L2EarthCoefficientsJ2000[i].C*rho);

  //Calculate L3
  int nL3Coefficients = sizeof(g_L3EarthCoefficientsJ2000) / sizeof(VSOP87Coefficient);
  double L3 = 0;
  for (i=0; i<nL3Coefficients; i++)
    L3 += g_L3EarthCoefficientsJ2000[i].A * cos(g_L3EarthCoefficientsJ2000[i].B + g_L3EarthCoefficientsJ2000[i].C*rho);

  //Calculate L4
  int nL4Coefficients = sizeof(g_L4EarthCoefficientsJ2000) / sizeof(VSOP87Coefficient);
  double L4 = 0;
  for (i=0; i<nL4Coefficients; i++)
    L4 += g_L4EarthCoefficientsJ2000[i].A * cos(g_L4EarthCoefficientsJ2000[i].B + g_L4EarthCoefficientsJ2000[i].C*rho);

  double value = (L0 + L1*rho + L2*rhosquared + L3*rhocubed + L4*rho4) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(value));
  return value;
}

double CAAEarth::EclipticLatitudeJ2000(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate B0
  int nB0Coefficients = sizeof(g_B0EarthCoefficients) / sizeof(VSOP87Coefficient);
  double B0 = 0;
  int i;
  for (i=0; i<nB0Coefficients; i++)
    B0 += g_B0EarthCoefficients[i].A * cos(g_B0EarthCoefficients[i].B + g_B0EarthCoefficients[i].C*rho);

  //Calculate B1
  int nB1Coefficients = sizeof(g_B1EarthCoefficientsJ2000) / sizeof(VSOP87Coefficient);
  double B1 = 0;
  for (i=0; i<nB1Coefficients; i++)
    B1 += g_B1EarthCoefficientsJ2000[i].A * cos(g_B1EarthCoefficientsJ2000[i].B + g_B1EarthCoefficientsJ2000[i].C*rho);

  //Calculate B2
  int nB2Coefficients = sizeof(g_B2EarthCoefficientsJ2000) / sizeof(VSOP87Coefficient);
  double B2 = 0;
  for (i=0; i<nB2Coefficients; i++)
    B2 += g_B2EarthCoefficientsJ2000[i].A * cos(g_B2EarthCoefficientsJ2000[i].B + g_B2EarthCoefficientsJ2000[i].C*rho);

  //Calculate B3
  int nB3Coefficients = sizeof(g_B3EarthCoefficientsJ2000) / sizeof(VSOP87Coefficient);
  double B3 = 0;
  for (i=0; i<nB3Coefficients; i++)
    B3 += g_B3EarthCoefficientsJ2000[i].A * cos(g_B3EarthCoefficientsJ2000[i].B + g_B3EarthCoefficientsJ2000[i].C*rho);

  //Calculate B4
  int nB4Coefficients = sizeof(g_B4EarthCoefficientsJ2000) / sizeof(VSOP87Coefficient);
  double B4 = 0;
  for (i=0; i<nB4Coefficients; i++)
    B4 += g_B4EarthCoefficientsJ2000[i].A * cos(g_B4EarthCoefficientsJ2000[i].B + g_B4EarthCoefficientsJ2000[i].C*rho);

  double value = (B0 + B1*rho + B2*rhosquared + B3*rhocubed + B4*rho4) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  return value;
}

double CAAEarth::SunMeanAnomaly(double JD)
{
  double T = (JD - 2451545) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;
  return CAACoordinateTransformation::MapTo0To360Range(357.5291092 + 35999.0502909*T - 0.0001536*Tsquared + Tcubed/24490000);
}

double CAAEarth::Eccentricity(double JD)
{
  double T = (JD - 2451545) / 36525;
  double Tsquared = T*T;
  return 1 - 0.002516*T - 0.0000074*Tsquared;
}

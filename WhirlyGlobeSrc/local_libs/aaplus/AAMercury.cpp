/*
Module : AAMERCURY.CPP
Purpose: Implementation for the algorithms which obtain the heliocentric position of Mercury
Created: PJN / 29-12-2003
History: PJN / 16-11-2005 1. Fixed a transcription error in the second coefficient used to calculate 
                          the longitude of Mercury. Thanks to "Maurizio" for reporting this bug.
         PJN / 12-05-2006 1. Fixed a transcription error in the third coefficient used to calculate
                          the R0 term for the radius vector of Mercury. Thanks to John Kruso for 
                          reporting this issue.
                          2. Fixed a transcription error in the third coefficient used to calculate
                          the R1 term for the radius vector of Mercury. Thanks to John Kruso for 
                          reporting this issue.  
                          3. Updated copyright details.
         PJN / 18-03-2012 1. All global "g_*" tables are now const. Thanks to Roger Dahl for reporting this 
                          issue when compiling AA+ on ARM.
         PJN / 04-08-2013 1. Fixed a transcription error in the third coefficient used to calculate
                          the L0 term for the ecliptic longitude of Mercury. Thanks to Isaac Clark for
                          reporting this issue.
                          2. Fixed a transcription error in the fifth coefficient used to calculate
                          the L2 term for the ecliptic longitude of Mercury. Thanks to Isaac Clark for
                          reporting this issue.
                          3. Fixed a transcription error in the second coefficient used to calculate
                          the L4 term for the ecliptic longitude of Mercury. Thanks to Isaac Clark for
                          reporting this issue.
                          4. Fixed a transcription error in the ninth coefficient used to calculate
                          the B0 term for the ecliptic latitude of Mercury. Thanks to Isaac Clark for
                          reporting this issue.
                          5. Spot tests indicate that these 4 changes only affected the ecliptic longitude
                          in the sixth decimal place and the ecliptic latitude in the eight decimal place.
                          6. Updated copyright details

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


//////////////////////////// Includes /////////////////////////////////////////

#include "stdafx.h"
#include "AAMercury.h"
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

const VSOP87Coefficient g_L0MercuryCoefficients[] =
{ 
  { 440250710, 0,          0 },
  { 40989415,  1.48302034, 26087.90314157 },
  { 5046294,   4.4778549,  52175.8062831 },
  { 855347,    1.165203,   78263.709425 },
  { 165590,    4.119692,   104351.612566 },
  { 34562,     0.77931,    130439.51571 },
  { 7583,      3.7135,     156527.4188 },
  { 3560,      1.5120,     1109.3786 },
  { 1803,      4.1033,     5661.3320 },
  { 1726,      0.3583,     182615.3220 },
  { 1590,      2.9951,     25028.5212 },
  { 1365,      4.5992,     27197.2817 },
  { 1017,      0.8803,     31749.2352 },
  { 714,       1.541,      24978.525 }, 
  { 644,       5.303,      21535.950 },
  { 451,       6.050,      51116.424 },
  { 404,       3.282,      208703.225 },
  { 352,       5.242,      20426.571 },
  { 345,       2.792,      15874.618 },
  { 343,       5.765,      955.600 },
  { 339,       5.863,      25558.212 },
  { 325,       1.337,      53285.185 },
  { 273,       2.495,      529.691 },
  { 264,       3.917,      57837.138 },
  { 260,       0.987,      4551.953 },
  { 239,       0.113,      1059.382 },
  { 235,       0.267,      11322.664 },
  { 217,       0.660,      13521.751 },
  { 209,       2.092,      47623.853 },
  { 183,       2.629,      27043.503 },
  { 182,       2.434,      25661.305 },
  { 176,       4.536,      51066.428 },
  { 173,       2.452,      24498.830 },
  { 142,       3.360,      37410.567 },
  { 138,       0.291,      10213.286 },
  { 125,       3.721,      39609.655 },
  { 118,       2.781,      77204.327 },
  { 106,       4.206,      19804.827 }
};

const VSOP87Coefficient g_L1MercuryCoefficients[] =
{ 
  { 2608814706223.0, 0,         0 },
  { 1126008,         6.2170397, 26087.9031416 },
  { 303471,          3.055655,  52175.806283 },
  { 80538,           6.10455,   78263.70942 },
  { 21245,           2.83532,   104351.61257 },
  { 5592,            5.8268,    130439.5157 },
  { 1472,            2.5185,    156527.4188 },
  { 388,             5.480,     182615.322 },
  { 352,             3.052,     1109.379 },
  { 103,             2.149,     208703.225 },
  { 94,              6.12,      27197.28 },
  { 91,              0.00,      24978.52 },
  { 52,              5.62,      5661.33 },
  { 44,              4.57,      25028.52 },
  { 28,              3.04,      51066.43 },
  { 27,              5.09,      234791.13 }
};

const VSOP87Coefficient g_L2MercuryCoefficients[] =
{ 
  { 53050, 0,       0 },
  { 16904, 4.69072, 26087.90314 },
  { 7397,  1.3474,  52175.8063 },
  { 3018,  4.4564,  78263.7094 },
  { 1107,  1.2623,  104351.6126 },
  { 378,   4.320,   130439.516 },
  { 123,   1.069,   156527.419 },
  { 39,    4.08,    182615.32 },
  { 15,    4.63,    1109.38 },
  { 12,    0.79,    208703.23 }
};

const VSOP87Coefficient g_L3MercuryCoefficients[] =
{ 
  { 188, 0.035, 52175.806 },
  { 142, 3.125, 26087.903 },
  { 97,  3.00,  78263.71 },
  { 44,  6.02,  104351.61 }, 
  { 35,  0,     0 },
  { 18,  2.78,  130439.52 },
  { 7,   5.82,  156527.42 },
  { 3,   2.57,  182615.32 }
};

const VSOP87Coefficient g_L4MercuryCoefficients[] =
{ 
  { 114, 3.1416, 0 },
  { 3,   2.03,   26087.90 },
  { 2,   1.42,   78263.71 },
  { 2,   4.50,   52175.81 },
  { 1,   4.50,   104351.61 },
  { 1,   1.27,   130439.52 }
};

const VSOP87Coefficient g_L5MercuryCoefficients[] =
{ 
  { 1, 3.14, 0 }
};

const VSOP87Coefficient g_B0MercuryCoefficients[] =
{ 
  { 11737529, 1.98357499, 26087.90314157 },
  { 2388077,  5.0373896,  52175.8062831 },
  { 1222840,  3.1415927,  0 },
  { 543252,   1.796444,   78263.709425 },
  { 129779,   4.832325,   104351.612566 },
  { 31867,    1.58088,    130439.51571 },
  { 7963,     4.6097,     156527.4188 },
  { 2014,     1.3532,     182615.3220 },
  { 514,      4.378,      208703.225 },
  { 209,      2.020,      24978.525 },
  { 208,      4.918,      27197.282 },
  { 132,      1.119,      234791.128 },
  { 121,      1.813,      53285.185 },
  { 100,      5.657,      20426.571 }
};

const VSOP87Coefficient g_B1MercuryCoefficients[] =
{ 
  { 429151, 3.501698, 26087.903142 },
  { 146234, 3.141593, 0 },
  { 22675,  0.01515,  52175.80628 },
  { 10895,  0.48540,  78263.70942 },
  { 6353,   3.4294,   104351.6126 },
  { 2496,   0.1605,   130439.5157 },
  { 860,    3.185,    156527.419 },
  { 278,    6.210,    182615.322 },
  { 86,     2.95,     208703.23 },
  { 28,     0.29,     27197.28 },
  { 26,     5.98,     234791.13 }
};

const VSOP87Coefficient g_B2MercuryCoefficients[] =
{ 
  { 11831, 4.79066, 26087.90314 },
  { 1914,  0,       0 },
  { 1045,  1.2122,  52175.8063 },
  { 266,   4.434,   78263.709 },
  { 170,   1.623,   104351.613 },
  { 96,    4.80,    130439.52 },
  { 45,    1.61,    156527.42 },
  { 18,    4.67,    182615.32 },
  { 7,     1.43,    208703.23 }
};

const VSOP87Coefficient g_B3MercuryCoefficients[] =
{ 
  { 235, 0.354, 26087.903 },
  { 161, 0,     0 },
  { 19,  4.36,  52175.81 },
  { 6,   2.51,  78263.71 },
  { 5,   6.14,  104351.61 },
  { 3,   3.12,  130439.52 }, 
  { 2,   6.27,  156527.42 }
};

const VSOP87Coefficient g_B4MercuryCoefficients[] =
{ 
  { 4, 1.75, 26087.90 },
  { 1, 3.14, 0 }
};

const VSOP87Coefficient g_R0MercuryCoefficients[] =
{ 
  { 39528272, 0,         0 },
  { 7834132,  6.1923372, 26087.9031416 },
  { 795526,   2.959897,  52175.806283 },
  { 121282,   6.010642,  78263.709425 },
  { 21922,    2.77820,   104351.61257 },
  { 4354,     5.8289,    130439.5157 },
  { 918,      2.597,     156527.419 },
  { 290,      1.424,     25028.521 },
  { 260,      3.028,     27197.282 },
  { 202,      5.647,     182615.322 },
  { 201,      5.592,     31749.235 },
  { 142,      6.253,     24978.525 },
  { 100,      3.734,     21535.950 }
};

const VSOP87Coefficient g_R1MercuryCoefficients[] =
{ 
  { 217348, 4.656172, 26087.903142 },
  { 44142,  1.42386,  52175.80628 },
  { 10094,  4.47466,  78263.70942 },
  { 2433,   1.2423,   104351.6126 },
  { 1624,   0,        0 },
  { 604,    4.293,    130439.516 },
  { 153,    1.061,    156527.419 },
  { 39,     4.11,     182615.32 }
};

const VSOP87Coefficient g_R2MercuryCoefficients[] =
{ 
  { 3118, 3.0823, 26087.9031 },
  { 1245, 6.1518, 52175.8063 },
  { 425,  2.926,  78263.709 },
  { 136,  5.980,  104351.613 },
  { 42,   2.75,   130439.52 },
  { 22,   3.14,   0 },
  { 13,   5.80,   156527.42 }
};

const VSOP87Coefficient g_R3MercuryCoefficients[] =
{ 
  { 33, 1.68, 26087.90 },
  { 24, 4.63, 52175.81 },
  { 12, 1.39, 78263.71 },
  { 5,  4.44, 104351.61 },
  { 2,  1.21, 130439.52 } 
};


///////////////////////////// Implementation //////////////////////////////////

double CAAMercury::EclipticLongitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;
  double rho5 = rho4*rho;

  //Calculate L0
  int nL0Coefficients = sizeof(g_L0MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double L0 = 0;
  int i;
  for (i=0; i<nL0Coefficients; i++)
    L0 += g_L0MercuryCoefficients[i].A * cos(g_L0MercuryCoefficients[i].B + g_L0MercuryCoefficients[i].C*rho);

  //Calculate L1
  int nL1Coefficients = sizeof(g_L1MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double L1 = 0;
  for (i=0; i<nL1Coefficients; i++)
    L1 += g_L1MercuryCoefficients[i].A * cos(g_L1MercuryCoefficients[i].B + g_L1MercuryCoefficients[i].C*rho);

  //Calculate L2
  int nL2Coefficients = sizeof(g_L2MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double L2 = 0;
  for (i=0; i<nL2Coefficients; i++)
    L2 += g_L2MercuryCoefficients[i].A * cos(g_L2MercuryCoefficients[i].B + g_L2MercuryCoefficients[i].C*rho);

  //Calculate L3
  int nL3Coefficients = sizeof(g_L3MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double L3 = 0;
  for (i=0; i<nL3Coefficients; i++)
    L3 += g_L3MercuryCoefficients[i].A * cos(g_L3MercuryCoefficients[i].B + g_L3MercuryCoefficients[i].C*rho);

  //Calculate L4
  int nL4Coefficients = sizeof(g_L4MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double L4 = 0;
  for (i=0; i<nL4Coefficients; i++)
    L4 += g_L4MercuryCoefficients[i].A * cos(g_L4MercuryCoefficients[i].B + g_L4MercuryCoefficients[i].C*rho);

  //Calculate L5
  int nL5Coefficients = sizeof(g_L5MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double L5 = 0;
  for (i=0; i<nL5Coefficients; i++)
    L5 += g_L5MercuryCoefficients[i].A * cos(g_L5MercuryCoefficients[i].B + g_L5MercuryCoefficients[i].C*rho);

  double value = (L0 + L1*rho + L2*rhosquared + L3*rhocubed + L4*rho4 + L5*rho5) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(value));
  return value;
}

double CAAMercury::EclipticLatitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate B0
  int nB0Coefficients = sizeof(g_B0MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double B0 = 0;
  int i;
  for (i=0; i<nB0Coefficients; i++)
    B0 += g_B0MercuryCoefficients[i].A * cos(g_B0MercuryCoefficients[i].B + g_B0MercuryCoefficients[i].C*rho);

  //Calculate B1
  int nB1Coefficients = sizeof(g_B1MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double B1 = 0;
  for (i=0; i<nB1Coefficients; i++)
    B1 += g_B1MercuryCoefficients[i].A * cos(g_B1MercuryCoefficients[i].B + g_B1MercuryCoefficients[i].C*rho);

  //Calculate B2
  int nB2Coefficients = sizeof(g_B2MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double B2 = 0;
  for (i=0; i<nB2Coefficients; i++)
    B2 += g_B2MercuryCoefficients[i].A * cos(g_B2MercuryCoefficients[i].B + g_B2MercuryCoefficients[i].C*rho);

  //Calculate B3
  int nB3Coefficients = sizeof(g_B3MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double B3 = 0;
  for (i=0; i<nB3Coefficients; i++)
    B3 += g_B3MercuryCoefficients[i].A * cos(g_B3MercuryCoefficients[i].B + g_B3MercuryCoefficients[i].C*rho);

  //Calculate B4
  int nB4Coefficients = sizeof(g_B4MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double B4 = 0;
  for (i=0; i<nB4Coefficients; i++)
    B4 += g_B4MercuryCoefficients[i].A * cos(g_B4MercuryCoefficients[i].B + g_B4MercuryCoefficients[i].C*rho);

  double value = (B0 + B1*rho + B2*rhosquared + B3*rhocubed + B4*rho4) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  return value;
}

double CAAMercury::RadiusVector(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;

  //Calculate R0
  int nR0Coefficients = sizeof(g_R0MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double R0 = 0;
  int i;
  for (i=0; i<nR0Coefficients; i++)
    R0 += g_R0MercuryCoefficients[i].A * cos(g_R0MercuryCoefficients[i].B + g_R0MercuryCoefficients[i].C*rho);

  //Calculate R1
  int nR1Coefficients = sizeof(g_R1MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double R1 = 0;
  for (i=0; i<nR1Coefficients; i++)
    R1 += g_R1MercuryCoefficients[i].A * cos(g_R1MercuryCoefficients[i].B + g_R1MercuryCoefficients[i].C*rho);

  //Calculate R2
  int nR2Coefficients = sizeof(g_R2MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double R2 = 0;
  for (i=0; i<nR2Coefficients; i++)
    R2 += g_R2MercuryCoefficients[i].A * cos(g_R2MercuryCoefficients[i].B + g_R2MercuryCoefficients[i].C*rho);

  //Calculate R3
  int nR3Coefficients = sizeof(g_R3MercuryCoefficients) / sizeof(VSOP87Coefficient);
  double R3 = 0;
  for (i=0; i<nR3Coefficients; i++)
    R3 += g_R3MercuryCoefficients[i].A * cos(g_R3MercuryCoefficients[i].B + g_R3MercuryCoefficients[i].C*rho);

  return (R0 + R1*rho + R2*rhosquared + R3*rhocubed) / 100000000;
}

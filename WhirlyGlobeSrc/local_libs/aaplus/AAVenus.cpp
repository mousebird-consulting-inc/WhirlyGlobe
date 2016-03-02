/*
Module : AAVENUS.CPP
Purpose: Implementation for the algorithms which obtain the heliocentric position of Venus
Created: PJN / 29-12-2003
History: PJN / 18-03-2012 1. All global "g_*" tables are now const. Thanks to Roger Dahl for reporting this 
                          issue when compiling AA+ on ARM.
         PJN / 04-08-2013 1. Fixed a transcription error in the third coefficient used to calculate
                          the B0 term for the ecliptic latitude of Venus. Thanks to Isaac Clark for
                          reporting this issue. Spot tests indicate that this change only affected the 
                          ecliptic latitude in the sixth decimal place.
                          3. Updated copyright details

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


//////////////////////////////// Includes /////////////////////////////////////////////////

#include "stdafx.h"
#include "AAVenus.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////////////////// Macros / Defines /////////////////////////////////////////

struct VSOP87Coefficient
{
  double A;
  double B;
  double C;
};

const VSOP87Coefficient g_L0VenusCoefficients[] =
{ 
  { 317614667, 0,         0 },
  { 1353968,   5.5931332, 10213.2855462 },
  { 89892,     5.30650,   20426.57109 },
  { 5477,      4.4163,    7860.4194 },
  { 3456,      2.6996,    11790.6291 },
  { 2372,      2.9938,    3930.2097 },
  { 1664,      4.2502,    1577.3435 },
  { 1438,      4.1575,    9683.5946 },
  { 1317,      5.1867,    26.2983 },
  { 1201,      6.1536,    30639.8566 },
  { 769,       0.816,     9437.763 },
  { 761,       1.950,     529.691 },
  { 708,       1.065,     775.523 },
  { 585,       3.998,     191.448 },
  { 500,       4.123,     15720.839 },
  { 429,       3.586,     19367.189 },
  { 327,       5.677,     5507.553 },
  { 326,       4.591,     10404.734 },
  { 232,       3.163,     9153.904 },
  { 180,       4.653,     1109.379 },
  { 155,       5.570,     19651.048 },
  { 128,       4.226,     20.775 },
  { 128,       0.962,     5661.332 },
  { 106,       1.537,     801.821 }
};

const VSOP87Coefficient g_L1VenusCoefficients[] =
{ 
  { 1021352943053.0, 0,       0 },
  { 95708,           2.46424, 10213.28555 },
  { 14445,           0.51625, 20426.57109 },
  { 213,             1.795,   30639.857 },
  { 174,             2.655,   26.298 },
  { 152,             6.106,   1577.344 },
  { 82,              5.70,    191.45 },
  { 70,              2.68,    9437.76 },  
  { 52,              3.60,    775.52 },   
  { 38,              1.03,    529.69 },
  { 30,              1.25,    5507.55 },
  { 25,              6.11,    10404.73 }
};

const VSOP87Coefficient g_L2VenusCoefficients[] =
{ 
  { 54127, 0,      0 },
  { 3891,  0.3451, 10213.2855 },
  { 1338,  2.0201, 20426.5711 },
  { 24,    2.05,   26.30 },
  { 19,    3.54,   30639.86 },
  { 10,    3.97,   775.52 },
  { 7,     1.52,   1577.34 },
  { 6,     1.00,   191.45 }
};

const VSOP87Coefficient g_L3VenusCoefficients[] =
{ 
  { 136, 4.804, 10213.286 },
  { 78,  3.67,  20426.57},
  { 26,  0,     0 }
};

const VSOP87Coefficient g_L4VenusCoefficients[] =
{ 
  { 114, 3.1416, 0 },
  { 3,   5.21,   20426.57 },
  { 2,   2.51,   10213.29 }
};

const VSOP87Coefficient g_L5VenusCoefficients[] =
{ 
  { 1, 3.14, 0 },
};

const VSOP87Coefficient g_B0VenusCoefficients[] =
{ 
  { 5923638, 0.2670278, 10213.2855462 },
  { 40108,   1.14737,   20426.57109 },
  { 32815,   3.14159,   0 },
  { 1011,    1.0895,    30639.8566 },
  { 149,     6.254,     18073.705 },
  { 138,     0.860,     1577.344 },
  { 130,     3.672,     9437.763 },
  { 120,     3.705,     2352.866 },
  { 108,     4.539,     22003.915 }
};

const VSOP87Coefficient g_B1VenusCoefficients[] =
{ 
  { 513348, 1.803643, 10213.285546 },
  { 4380,   3.3862,   20426.5711 }, 
  { 199,    0,        0 },
  { 197,    2.530,    30639.857 }
};

const VSOP87Coefficient g_B2VenusCoefficients[] =
{ 
  { 22378, 3.38509, 10213.28555 },
  { 282,   0,       0 },
  { 173,   5.256,   20426.571 },
  { 27,    3.87,    30639.86 }
};

const VSOP87Coefficient g_B3VenusCoefficients[] =
{ 
  { 647, 4.992, 10213.286 },
  { 20,  3.14,  0 },
  { 6,   0.77,  20426.57 },
  { 3,   5.44,  30639.86 }
};

const VSOP87Coefficient g_B4VenusCoefficients[] =
{ 
  { 14, 0.32, 10213.29 }
};

const VSOP87Coefficient g_R0VenusCoefficients[] =
{ 
  { 72334821, 0,        0 },
  { 489824,   4.021518, 10213.285546 },
  { 1658,     4.9021,   20426.5711 },
  { 1632,     2.8455,   7860.4194 },
  { 1378,     1.1285,   11790.6291 },
  { 498,      2.587,    9683.595 },
  { 374,      1.423,    3930.210 },
  { 264,      5.529,    9437.763 },
  { 237,      2.551,    15720.839 },
  { 222,      2.013,    19367.189 },
  { 126,      2.728,    1577.344 },
  { 119,      3.020,    10404.734 }
};

const VSOP87Coefficient g_R1VenusCoefficients[] =
{ 
  { 34551, 0.89199, 10213.28555 },
  { 234,   1.772,   20426.571 },
  { 234,   3.142,   0 }  
};

const VSOP87Coefficient g_R2VenusCoefficients[] =
{ 
  { 1407, 5.0637, 10213.2855 },
  { 16,   5.47,   20426.57 },
  { 13,   0,      0 }  
};

const VSOP87Coefficient g_R3VenusCoefficients[] =
{ 
  { 50, 3.22, 10213.29 }
};

const VSOP87Coefficient g_R4VenusCoefficients[] =
{ 
  { 1, 0.92, 10213.29 }
};


//////////////////////////////// Implementation ///////////////////////////////////////////

double CAAVenus::EclipticLongitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;
  double rho5 = rho4*rho;

  //Calculate L0
  int nL0Coefficients = sizeof(g_L0VenusCoefficients) / sizeof(VSOP87Coefficient);
  double L0 = 0;
  int i;
  for (i=0; i<nL0Coefficients; i++)
    L0 += g_L0VenusCoefficients[i].A * cos(g_L0VenusCoefficients[i].B + g_L0VenusCoefficients[i].C*rho);

  //Calculate L1
  int nL1Coefficients = sizeof(g_L1VenusCoefficients) / sizeof(VSOP87Coefficient);
  double L1 = 0;
  for (i=0; i<nL1Coefficients; i++)
    L1 += g_L1VenusCoefficients[i].A * cos(g_L1VenusCoefficients[i].B + g_L1VenusCoefficients[i].C*rho);

  //Calculate L2
  int nL2Coefficients = sizeof(g_L2VenusCoefficients) / sizeof(VSOP87Coefficient);
  double L2 = 0;
  for (i=0; i<nL2Coefficients; i++)
    L2 += g_L2VenusCoefficients[i].A * cos(g_L2VenusCoefficients[i].B + g_L2VenusCoefficients[i].C*rho);

  //Calculate L3
  int nL3Coefficients = sizeof(g_L3VenusCoefficients) / sizeof(VSOP87Coefficient);
  double L3 = 0;
  for (i=0; i<nL3Coefficients; i++)
    L3 += g_L3VenusCoefficients[i].A * cos(g_L3VenusCoefficients[i].B + g_L3VenusCoefficients[i].C*rho);

  //Calculate L4
  int nL4Coefficients = sizeof(g_L4VenusCoefficients) / sizeof(VSOP87Coefficient);
  double L4 = 0;
  for (i=0; i<nL4Coefficients; i++)
    L4 += g_L4VenusCoefficients[i].A * cos(g_L4VenusCoefficients[i].B + g_L4VenusCoefficients[i].C*rho);

  //Calculate L5
  int nL5Coefficients = sizeof(g_L5VenusCoefficients) / sizeof(VSOP87Coefficient);
  double L5 = 0;
  for (i=0; i<nL5Coefficients; i++)
    L5 += g_L5VenusCoefficients[i].A * cos(g_L5VenusCoefficients[i].B + g_L5VenusCoefficients[i].C*rho);

  double value = (L0 + L1*rho + L2*rhosquared + L3*rhocubed + L4*rho4 + L5*rho5) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(value));
  return value;
}

double CAAVenus::EclipticLatitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate B0
  int nB0Coefficients = sizeof(g_B0VenusCoefficients) / sizeof(VSOP87Coefficient);
  double B0 = 0;
  int i;
  for (i=0; i<nB0Coefficients; i++)
    B0 += g_B0VenusCoefficients[i].A * cos(g_B0VenusCoefficients[i].B + g_B0VenusCoefficients[i].C*rho);

  //Calculate B1
  int nB1Coefficients = sizeof(g_B1VenusCoefficients) / sizeof(VSOP87Coefficient);
  double B1 = 0;
  for (i=0; i<nB1Coefficients; i++)
    B1 += g_B1VenusCoefficients[i].A * cos(g_B1VenusCoefficients[i].B + g_B1VenusCoefficients[i].C*rho);

  //Calculate B2
  int nB2Coefficients = sizeof(g_B2VenusCoefficients) / sizeof(VSOP87Coefficient);
  double B2 = 0;
  for (i=0; i<nB2Coefficients; i++)
    B2 += g_B2VenusCoefficients[i].A * cos(g_B2VenusCoefficients[i].B + g_B2VenusCoefficients[i].C*rho);

  //Calculate B3
  int nB3Coefficients = sizeof(g_B3VenusCoefficients) / sizeof(VSOP87Coefficient);
  double B3 = 0;
  for (i=0; i<nB3Coefficients; i++)
    B3 += g_B3VenusCoefficients[i].A * cos(g_B3VenusCoefficients[i].B + g_B3VenusCoefficients[i].C*rho);

  //Calculate B4
  int nB4Coefficients = sizeof(g_B4VenusCoefficients) / sizeof(VSOP87Coefficient);
  double B4 = 0;
  for (i=0; i<nB4Coefficients; i++)
    B4 += g_B4VenusCoefficients[i].A * cos(g_B4VenusCoefficients[i].B + g_B4VenusCoefficients[i].C*rho);

  double value = (B0 + B1*rho + B2*rhosquared + B3*rhocubed + B4*rho4) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  return value;
}

double CAAVenus::RadiusVector(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate R0
  int nR0Coefficients = sizeof(g_R0VenusCoefficients) / sizeof(VSOP87Coefficient);
  double R0 = 0;
  int i;
  for (i=0; i<nR0Coefficients; i++)
    R0 += g_R0VenusCoefficients[i].A * cos(g_R0VenusCoefficients[i].B + g_R0VenusCoefficients[i].C*rho);

  //Calculate R1
  int nR1Coefficients = sizeof(g_R1VenusCoefficients) / sizeof(VSOP87Coefficient);
  double R1 = 0;
  for (i=0; i<nR1Coefficients; i++)
    R1 += g_R1VenusCoefficients[i].A * cos(g_R1VenusCoefficients[i].B + g_R1VenusCoefficients[i].C*rho);

  //Calculate R2
  int nR2Coefficients = sizeof(g_R2VenusCoefficients) / sizeof(VSOP87Coefficient);
  double R2 = 0;
  for (i=0; i<nR2Coefficients; i++)
    R2 += g_R2VenusCoefficients[i].A * cos(g_R2VenusCoefficients[i].B + g_R2VenusCoefficients[i].C*rho);

  //Calculate R3
  int nR3Coefficients = sizeof(g_R3VenusCoefficients) / sizeof(VSOP87Coefficient);
  double R3 = 0;
  for (i=0; i<nR3Coefficients; i++)
    R3 += g_R3VenusCoefficients[i].A * cos(g_R3VenusCoefficients[i].B + g_R3VenusCoefficients[i].C*rho);

  //Calculate R4
  int nR4Coefficients = sizeof(g_R4VenusCoefficients) / sizeof(VSOP87Coefficient);
  double R4 = 0;
  for (i=0; i<nR4Coefficients; i++)
    R4 += g_R4VenusCoefficients[i].A * cos(g_R4VenusCoefficients[i].B + g_R4VenusCoefficients[i].C*rho);

  return (R0 + R1*rho + R2*rhosquared + R3*rhocubed + R4*rho4) / 100000000;
}

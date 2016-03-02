/*
Module : AAPLUTO.CPP
Purpose: Implementation for the algorithms which obtain the heliocentric position of Pluto
Created: PJN / 29-12-2003
History: PJN / 07-02-2009 1. Optimized the layout of the PlutoCoefficient1 structure by making all elements
                          integers instead of doubles.
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


//////////////////////// Includes /////////////////////////////////////////////

#include "stdafx.h"
#include "AAPluto.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////////// Macros / Defines /////////////////////////////////////

struct PlutoCoefficient1
{
  int J;
  int S;
  int P;
};

struct PlutoCoefficient2
{
  double A;
  double B;
};

const PlutoCoefficient1 g_PlutoArgumentCoefficients[] =
{ 
  { 0,   0,    1 },
  { 0,   0,    2 },
  { 0, 	 0,    3 },
  { 0,	 0,    4 },
  { 0,	 0,    5 },
  { 0,	 0,    6 },
  { 0,	 1,   -1 },
  { 0,	 1,    0 },
  { 0,	 1,    1 },
  { 0,	 1,    2 },
  { 0,	 1,    3 },
  { 0,	 2,   -2 },
  { 0,	 2,   -1 },
  { 0,	 2,    0 },
  { 1,	-1,    0 },
  { 1,  -1,    1 },
  { 1,   0,   -3 },
  { 1,   0,   -2 },
  { 1,   0,   -1 },
  { 1,   0,    0 },
  { 1,   0,    1 },
  { 1,   0,    2 }, 
  { 1,   0,    3 },
  { 1,   0,    4 },
  { 1,   1,   -3 },
  { 1,   1,   -2 },
  { 1,   1,   -1 },
  { 1,   1,    0 },
  { 1,   1,    1 },
  { 1,   1,    3 },
  { 2,	 0,   -6 },
  { 2,	 0,   -5 },
  { 2,	 0,   -4 },
  { 2,	 0,   -3 },
  { 2,	 0,   -2 },
  { 2,	 0,   -1 },
  { 2,	 0,    0 },
  { 2,	 0,    1 },
  { 2,	 0,    2 },
  { 2,	 0,    3 },
  { 3,	 0,   -2 },
  { 3,	 0,   -1 },
  { 3,	 0,    0 }
};

const PlutoCoefficient2 g_PlutoLongitudeCoefficients[] =
{ 
  { -19799805, 19850055 },
  {  897144,  -4954829 },
  {  611149,	 1211027 },
  { -341243,	-189585 },
  {  129287,	-34992 },
  { -38164,	   30893 },
  {  20442,	  -9987 },
  { -4063,  	-5071 },
  { -6016,  	-3336 },
  { -3956,	   3039 }, 
  { -667,	     3572 }, 
  {  1276,	   501},
  {  1152,	  -917 }, 
  {  630,	    -1277 },
  {  2571,	  -459 },
  {  899,	    -1449 },
  { -1016,	   1043 },
  { -2343,   	-1012 },
  {  7042,	   788 },
  {  1199,	  -338 }, 
  {  418,	    -67 },
  {  120,	    -274 },
  { -60,	    -159 },
  { -82,	    -29 },
  { -36,	    -29 },
  { -40,	     7 },
  { -14,	     22 },
  {  4,	       13 },
  {  5,	       2 },
  { -1,	       0 },
  {  2,	       0 },
  { -4,        5 },
  {  4,	      -7 },
  {  14,	     24 },
  { -49,	    -34 },
  {  163,	    -48 },
  {  9,	      -24 },
  { -4,	       1 },
  { -3,	       1 },
  {  1,	       3 },
  { -3,	      -1 },
  {  5,	      -3 },
  {  0,	       0 },
};

const PlutoCoefficient2 g_PlutoLatitudeCoefficients[] =
{ 
  { -5452852,	-14974862 },
  {  3527812,	 1672790 }, 
  { -1050748,	 327647 }, 
  {  178690,	-292153 }, 
  {  18650,	   100340 },
  { -30697,	  -25823 }, 
  {  4878,	   11248 }, 
  {  226,	    -64 }, 
  {  2030,  	-836 },  
  {  69,	    -604 }, 
  { -247,	    -567 }, 
  { -57,	     1 }, 
  { -122,	     175 }, 
  { -49,	    -164 }, 
  { -197,	     199 }, 
  { -25,	     217 }, 
  {  589,	    -248 }, 
  { -269,	     711 }, 
  {  185,	     193 }, 
  {  315,	     807 }, 
  { -130,	    -43 }, 
  {  5,	       3 }, 
  {  2,	       17 }, 
  {  2,	       5 }, 
  {  2,	       3 }, 
  {  3,	       1 }, 
  {  2,	      -1 },  
  {  1,	      -1 }, 
  {  0,       -1 }, 
  {  0,	       0 }, 
  {  0,	      -2 }, 
  {  2,	       2 }, 
  { -7,	       0 },  
  {  10,		  -8 }, 
  { -3,	       20 }, 
  {  6,	       5 }, 
  {  14,	     17 }, 
  { -2,	       0 }, 
  {  0,	       0 }, 
  {  0,	       0 }, 
  {  0,	       1 }, 
  {  0,	       0 },
  {  1,	       0 }, 
};

const PlutoCoefficient2 g_PlutoRadiusCoefficients[] =
{ 
  {  66865439,	 68951812 },
  { -11827535,	-332538 },
  {  1593179,	  -1438890 }, 
  { -18444,	     483220 },
  { -65977,	    -85431 },
  {  31174,	    -6032 },
  { -5794,	     22161 },
  {  4601,	     4032 },
  { -1729,	     234 },
  { -415,	       702 },
  {  239,	       723 },
  {  67,	      -67 },
  {  1034,	    -451 },
  { -129,	       504 },
  {  480,	      -231 },
  {  2,	        -441 },
  { -3359,	     265 },
  {  7856,	    -7832 },
  {  36,	       45763 },
  {  8663,	     8547 },
  { -809,	      -769 },
  {  263,	      -144 }, 
  { -126,	       32 },
  { -35,	      -16 },
  { -19,	      -4 },
  { -15,	       8 },
  { -4,	         12 },
  {  5,	         6 },
  {  3,	         1 },
  {  6,	        -2 }, 
  {  2,	         2 },
  { -2,	        -2 }, 
  {  14,	       13 },
  { -63,	       13 },
  {  136,	      -236 }, 
  {  273,	       1065 },
  {  251,        149 },
  { -25,	      -9 }, 
  {  9,	        -2 },
  { -8,          7 },
  {  2,	        -10 },
  {  19,	       35 },
  {  10,	       3 },
};


/////////////////////////////////// Implementation ////////////////////////////

double CAAPluto::EclipticLongitude(double JD)
{
  double T = (JD - 2451545) / 36525;
  double J = 34.35 + 3034.9057*T;
  double S = 50.08 + 1222.1138*T;
  double P = 238.96 + 144.9600*T;

  //Calculate Longitude
  double L = 0;
  int nPlutoCoefficients = sizeof(g_PlutoArgumentCoefficients) / sizeof(PlutoCoefficient1);
  for (int i=0; i<nPlutoCoefficients; i++)
  {
    double Alpha = g_PlutoArgumentCoefficients[i].J * J +  g_PlutoArgumentCoefficients[i].S * S + g_PlutoArgumentCoefficients[i].P * P;
    Alpha = CAACoordinateTransformation::DegreesToRadians(Alpha);
    L += ((g_PlutoLongitudeCoefficients[i].A * sin(Alpha)) + (g_PlutoLongitudeCoefficients[i].B * cos(Alpha)));
  }
  L = L / 1000000;
  L += (238.958116 + 144.96*T);
  L = CAACoordinateTransformation::MapTo0To360Range(L);

  return L;
}

double CAAPluto::EclipticLatitude(double JD)
{
  double T = (JD - 2451545) / 36525;
  double J = 34.35 + 3034.9057*T;
  double S = 50.08 + 1222.1138*T;
  double P = 238.96 + 144.9600*T;

  //Calculate Latitude
  double L = 0;
  int nPlutoCoefficients = sizeof(g_PlutoArgumentCoefficients) / sizeof(PlutoCoefficient1);
  for (int i=0; i<nPlutoCoefficients; i++)
  {
    double Alpha = g_PlutoArgumentCoefficients[i].J * J +  g_PlutoArgumentCoefficients[i].S * S + g_PlutoArgumentCoefficients[i].P * P;
    Alpha = CAACoordinateTransformation::DegreesToRadians(Alpha);
    L += ((g_PlutoLatitudeCoefficients[i].A * sin(Alpha)) + (g_PlutoLatitudeCoefficients[i].B * cos(Alpha)));
  }
  L = L / 1000000;
  L += -3.908239;

  return L;
}

double CAAPluto::RadiusVector(double JD)
{
  double T = (JD - 2451545) / 36525;
  double J = 34.35 + 3034.9057*T;
  double S = 50.08 + 1222.1138*T;
  double P = 238.96 + 144.9600*T;

  //Calculate Radius
  double R = 0;
  int nPlutoCoefficients = sizeof(g_PlutoArgumentCoefficients) / sizeof(PlutoCoefficient1);
  for (int i=0; i<nPlutoCoefficients; i++)
  {
    double Alpha = g_PlutoArgumentCoefficients[i].J * J +  g_PlutoArgumentCoefficients[i].S * S + g_PlutoArgumentCoefficients[i].P * P;
    Alpha = CAACoordinateTransformation::DegreesToRadians(Alpha);
    R += ((g_PlutoRadiusCoefficients[i].A * sin(Alpha)) + (g_PlutoRadiusCoefficients[i].B * cos(Alpha)));
  }
  R = R / 10000000;
  R += 40.7241346;

  return R;
}

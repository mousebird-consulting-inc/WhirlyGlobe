/*
Module : AAMOONPHASES.CPP
Purpose: Implementation for the algorithms which obtain the dates for the phases of the Moon
Created: PJN / 11-01-2004
History: PJN / 22-02-2004 1. Fixed a bug in the calculation of the phase type from the k value in
                          CAAMoonPhases::TruePhase.
         PJN / 10-09-2011 1. Fixed a bug in the calculation of the "F" local variable which represents
                          the Moon's argument of latitude in the CAAMoonPhases::TruePhase method. 
                          Thanks to Andrew Hammond for reporting this bug.
         PJN / 29-03-2015 1. Fixed up some variable initializations around the use of modf. Thanks to Arnaud Cueille for
                          reporting this issue.

Copyright (c) 2004 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

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
#include "AAMoonPhases.h"
#include "AACoordinateTransformation.h"
#include <cmath>
#include <cassert>
using namespace std;


//////////////////////////// Implementation ///////////////////////////////////

double CAAMoonPhases::K(double Year)
{
  return 12.3685*(Year - 2000);
}

double CAAMoonPhases::MeanPhase(double k)
{
  //convert from K to T
  double T = k/1236.85;
  double T2 = T*T;
  double T3 = T2*T;
  double T4 = T3*T;

  return 2451550.09766 + 29.530588861*k + 0.00015437*T2 - 0.000000150*T3 + 0.00000000073*T4;
}

double CAAMoonPhases::TruePhase(double k)
{
  //What will be the return value
  double JD = MeanPhase(k);

  //convert from K to T
  double T = k/1236.85;
  double T2 = T*T;
  double T3 = T2*T;
  double T4 = T3*T;

  double E = 1 - 0.002516*T - 0.0000074*T2;
  double E2 = E*E;

  double M = CAACoordinateTransformation::MapTo0To360Range(2.5534 + 29.10535670*k - 0.0000014*T2 - 0.00000011*T3);
  M = CAACoordinateTransformation::DegreesToRadians(M);
  double Mdash = CAACoordinateTransformation::MapTo0To360Range(201.5643 + 385.81693528*k + 0.0107582*T2 + 0.00001238*T3 - 0.000000058*T4); 
  Mdash = CAACoordinateTransformation::DegreesToRadians(Mdash);
  double F = CAACoordinateTransformation::MapTo0To360Range(160.7108 + 390.67050284*k - 0.0016118*T2 - 0.00000227*T3 + 0.000000011*T4);
  F = CAACoordinateTransformation::DegreesToRadians(F);
  double omega = CAACoordinateTransformation::MapTo0To360Range(124.7746 - 1.56375588*k + 0.0020672*T2 + 0.00000215*T3);
  omega = CAACoordinateTransformation::DegreesToRadians(omega);
  double A1 = CAACoordinateTransformation::MapTo0To360Range(299.77 + 0.107408*k - 0.009173*T2);
  A1 = CAACoordinateTransformation::DegreesToRadians(A1);
  double A2 = CAACoordinateTransformation::MapTo0To360Range(251.88 + 0.016321*k);
  A2 = CAACoordinateTransformation::DegreesToRadians(A2);
  double A3 = CAACoordinateTransformation::MapTo0To360Range(251.83 + 26.651886*k);
  A3 = CAACoordinateTransformation::DegreesToRadians(A3);
  double A4 = CAACoordinateTransformation::MapTo0To360Range(349.42 + 36.412478*k);
  A4 = CAACoordinateTransformation::DegreesToRadians(A4);
  double A5 = CAACoordinateTransformation::MapTo0To360Range(84.66 + 18.206239*k);
  A5 = CAACoordinateTransformation::DegreesToRadians(A5);
  double A6 = CAACoordinateTransformation::MapTo0To360Range(141.74 + 53.303771*k);
  A6 = CAACoordinateTransformation::DegreesToRadians(A6);
  double A7 = CAACoordinateTransformation::MapTo0To360Range(207.14 + 2.453732*k);
  A7 = CAACoordinateTransformation::DegreesToRadians(A7);
  double A8 = CAACoordinateTransformation::MapTo0To360Range(154.84 + 7.306860*k);
  A8 = CAACoordinateTransformation::DegreesToRadians(A8);
  double A9 = CAACoordinateTransformation::MapTo0To360Range(34.52 + 27.261239*k);
  A9 = CAACoordinateTransformation::DegreesToRadians(A9);
  double A10 = CAACoordinateTransformation::MapTo0To360Range(207.19 + 0.121824*k);
  A10 = CAACoordinateTransformation::DegreesToRadians(A10);
  double A11 = CAACoordinateTransformation::MapTo0To360Range(291.34 + 1.844379*k);
  A11 = CAACoordinateTransformation::DegreesToRadians(A11);
  double A12 = CAACoordinateTransformation::MapTo0To360Range(161.72 + 24.198154*k);
  A12 = CAACoordinateTransformation::DegreesToRadians(A12);
  double A13 = CAACoordinateTransformation::MapTo0To360Range(239.56 + 25.513099*k);
  A13 = CAACoordinateTransformation::DegreesToRadians(A13);
  double A14 = CAACoordinateTransformation::MapTo0To360Range(331.55 + 3.592518*k);
  A14 = CAACoordinateTransformation::DegreesToRadians(A14);

  //convert to radians
  double kint = 0;
  double kfrac = modf(k, &kint);
  if (kfrac < 0)
    kfrac = 1 + kfrac;
  if (kfrac == 0) //New Moon
  {
    double DeltaJD = -0.40720*sin(Mdash) +
          0.17241*E*sin(M) +
          0.01608*sin(2*Mdash) +
          0.01039*sin(2*F) +
          0.00739*E*sin(Mdash - M) +
          -0.00514*E*sin(Mdash + M) +
          0.00208*E2*sin(2*M) +
          -0.00111*sin(Mdash - 2*F) +
          -0.00057*sin(Mdash + 2*F) +
          0.00056*E*sin(2*Mdash + M) +
          -0.00042*sin(3*Mdash) +
          0.00042*E*sin(M + 2*F) +
          0.00038*E*sin(M - 2*F) +
          -0.00024*E*sin(2*Mdash - M) +
          -0.00017*sin(omega) +
          -0.00007*sin(Mdash + 2*M) +
          0.00004*sin(2*Mdash - 2*F) +
          0.00004*sin(3*M) +
          0.00003*sin(Mdash + M - 2*F) +
          0.00003*sin(2*Mdash + 2*F) +
          -0.00003*sin(Mdash + M + 2*F) +
          0.00003*sin(Mdash - M + 2*F) +
          -0.00002*sin(Mdash - M - 2*F) +
          -0.00002*sin(3*Mdash + M) +
          0.00002*sin(4*Mdash);
    JD += DeltaJD;
  }
  else if ((kfrac == 0.25) || (kfrac == 0.75)) //First Quarter or Last Quarter
  {
    double DeltaJD = -0.62801*sin(Mdash) +
          0.17172*E*sin(M) +
          -0.01183*E*sin(Mdash + M) +
          0.00862*sin(2*Mdash) +
          0.00804*sin(2*F) +
          0.00454*E*sin(Mdash - M) +
          0.00204*E2*sin(2*M) +
          -0.00180*sin(Mdash - 2*F) +
          -0.00070*sin(Mdash + 2*F) +
          -0.00040*sin(3*Mdash) +
          -0.00034*E*sin(2*Mdash - M) +
          0.00032*E*sin(M + 2*F) +
          0.00032*E*sin(M - 2*F) +
          -0.00028*E2*sin(Mdash + 2*M) +
          0.00027*E*sin(2*Mdash + M) +
          -0.00017*sin(omega) +
          -0.00005*sin(Mdash - M - 2*F) +
          0.00004*sin(2*Mdash + 2*F) +
          -0.00004*sin(Mdash + M + 2*F) +
          0.00004*sin(Mdash - 2*M) +
          0.00003*sin(Mdash + M - 2*F) +
          0.00003*sin(3*M) +
          0.00002*sin(2*Mdash - 2*F) +
          0.00002*sin(Mdash - M + 2*F) +
          -0.00002*sin(3*Mdash + M);
    JD += DeltaJD;
          
    double W = 0.00306 - 0.00038*E*cos(M) + 0.00026*cos(Mdash) - 0.00002*cos(Mdash - M) + 0.00002*cos(Mdash + M) + 0.00002*cos(2*F);
    if (kfrac == 0.25) //First quarter
      JD += W;
    else
      JD -= W;          
  }
  else if (kfrac == 0.5) //Full Moon
  {
    double DeltaJD = -0.40614*sin(Mdash) +
          0.17302*E*sin(M) +
          0.01614*sin(2*Mdash) +
          0.01043*sin(2*F) +
          0.00734*E*sin(Mdash - M) +
          -0.00514*E*sin(Mdash + M) +
          0.00209*E2*sin(2*M) +
          -0.00111*sin(Mdash - 2*F) +
          -0.00057*sin(Mdash + 2*F) +
          0.00056*E*sin(2*Mdash + M) +
          -0.00042*sin(3*Mdash) +
          0.00042*E*sin(M + 2*F) +
          0.00038*E*sin(M - 2*F) +
          -0.00024*E*sin(2*Mdash - M) +
          -0.00017*sin(omega) +
          -0.00007*sin(Mdash + 2*M) +
          0.00004*sin(2*Mdash - 2*F) +
          0.00004*sin(3*M) +
          0.00003*sin(Mdash + M - 2*F) +
          0.00003*sin(2*Mdash + 2*F) +
          -0.00003*sin(Mdash + M + 2*F) +
          0.00003*sin(Mdash - M + 2*F) +
          -0.00002*sin(Mdash - M - 2*F) +
          -0.00002*sin(3*Mdash + M) +
          0.00002*sin(4*Mdash);
    JD += DeltaJD;
  }
  else
  {
    assert(false);
  }

  //Additional corrections for all phases
  double DeltaJD2 = 0.000325*sin(A1) +
        0.000165*sin(A2) +
        0.000164*sin(A3) +
        0.000126*sin(A4) +
        0.000110*sin(A5) +
        0.000062*sin(A6) +
        0.000060*sin(A7) +
        0.000056*sin(A8) +
        0.000047*sin(A9) +
        0.000042*sin(A10) +
        0.000040*sin(A11) +
        0.000037*sin(A12) +
        0.000035*sin(A13) +
        0.000023*sin(A14);
  JD += DeltaJD2;

  return JD;
}

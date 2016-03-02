/*
Module : AAMOONMAXDECLINATIONS.CPP
Purpose: Implementation for the algorithms which obtain the dates and values for Maximum declination of the Moon
Created: PJN / 13-01-2004
History: None

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
#include "AAMoonMaxDeclinations.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////////////// Implementation ///////////////////////////////////

double CAAMoonMaxDeclinations::K(double Year)
{
  return 13.3686*(Year - 2000.03);
}

double CAAMoonMaxDeclinations::MeanGreatestDeclination(double k, bool bNortherly)
{
  //convert from K to T
  double T = k/1336.86;
  double T2 = T*T;
  double T3 = T2*T;

  double value = bNortherly ? 2451562.5897 : 2451548.9289;
  return value + 27.321582247*k + 0.000119804*T2 - 0.000000141*T3;
}

double CAAMoonMaxDeclinations::MeanGreatestDeclinationValue(double k)
{
  //convert from K to T
  double T = k/1336.86;
  return 23.6961 - 0.013004*T;
}

double CAAMoonMaxDeclinations::TrueGreatestDeclination(double k, bool bNortherly)
{
  //convert from K to T
  double T = k/1336.86;
  double T2 = T*T;
  double T3 = T2*T;

  double D = bNortherly ? 152.2029 : 345.6676;
  D = CAACoordinateTransformation::MapTo0To360Range(D + 333.0705546*k - 0.0004214*T2 + 0.00000011*T3);
  double M = bNortherly ? 14.8591 : 1.3951;
  M = CAACoordinateTransformation::MapTo0To360Range(M + 26.9281592*k - 0.0000355*T2 - 0.00000010*T3);
  double Mdash = bNortherly ? 4.6881 : 186.2100;
  Mdash = CAACoordinateTransformation::MapTo0To360Range(Mdash + 356.9562794*k + 0.0103066*T2 + 0.00001251*T3); 
  double F = bNortherly ? 325.8867 : 145.1633;
  F = CAACoordinateTransformation::MapTo0To360Range(F + 1.4467807*k - 0.0020690*T2 - 0.00000215*T3);
  double E = 1 - 0.002516*T - 0.0000074*T2;

  //convert to radians
  D = CAACoordinateTransformation::DegreesToRadians(D);
  M = CAACoordinateTransformation::DegreesToRadians(M);
  Mdash = CAACoordinateTransformation::DegreesToRadians(Mdash);
  F = CAACoordinateTransformation::DegreesToRadians(F);


  double DeltaJD;
  if (bNortherly)
  {
    DeltaJD = 0.8975*cos(F) +
              -0.4726*sin(Mdash) +
              -0.1030*sin(2*F) +
              -0.0976*sin(2*D - Mdash) +
              -0.0462*cos(Mdash - F) +
              -0.0461*cos(Mdash + F) +
              -0.0438*sin(2*D) +
              0.0162*E*sin(M) +
              -0.0157*cos(3*F) +
              0.0145*sin(Mdash + 2*F) +
              0.0136*cos(2*D - F) +
              -0.0095*cos(2*D - Mdash - F) +
              -0.0091*cos(2*D - Mdash + F) +
              -0.0089*cos(2*D + F) +
              0.0075*sin(2*Mdash) +
              -0.0068*sin(Mdash - 2*F) +
              0.0061*cos(2*Mdash - F) +
              -0.0047*sin(Mdash + 3*F) +
              -0.0043*E*sin(2*D - M - Mdash) +
              -0.0040*cos(Mdash - 2*F) +
              -0.0037*sin(2*D - 2*Mdash) +
              0.0031*sin(F) +
              0.0030*sin(2*D + Mdash) +
              -0.0029*cos(Mdash + 2*F) +
              -0.0029*E*sin(2*D - M) +
              -0.0027*sin(Mdash + F) +
              0.0024*E*sin(M - Mdash) +
              -0.0021*sin(Mdash - 3*F) +
              0.0019*sin(2*Mdash + F) +
              0.0018*cos(2*D - 2*Mdash - F) +
              0.0018*sin(3*F) +
              0.0017*cos(Mdash + 3*F) +
              0.0017*cos(2*Mdash) +
              -0.0014*cos(2*D - Mdash) +
              0.0013*cos(2*D + Mdash + F) +
              0.0013*cos(Mdash) +
              0.0012*sin(3*Mdash + F) +
              0.0011*sin(2*D - Mdash + F) +
              -0.0011*cos(2*D - 2*Mdash) +
              0.0010*cos(D + F) + 
              0.0010*E*sin(M + Mdash) +
              -0.0009*sin(2*D - 2*F) +
              0.0007*cos(2*Mdash + F) +
              -0.0007*cos(3*Mdash + F);
  }
  else
  {
    DeltaJD = -0.8975*cos(F) +
              -0.4726*sin(Mdash) +
              -0.1030*sin(2*F) +
              -0.0976*sin(2*D - Mdash) +
              0.0541*cos(Mdash - F) +
              0.0516*cos(Mdash + F) +
              -0.0438*sin(2*D) +
              0.0112*E*sin(M) +
              0.0157*cos(3*F) +
              0.0023*sin(Mdash + 2*F) +
              -0.0136*cos(2*D - F) +
              0.0110*cos(2*D - Mdash - F) +
              0.0091*cos(2*D - Mdash + F) +
              0.0089*cos(2*D + F) +
              0.0075*sin(2*Mdash) +
              -0.0030*sin(Mdash - 2*F) +
              -0.0061*cos(2*Mdash - F) +
              -0.0047*sin(Mdash + 3*F) +
              -0.0043*E*sin(2*D - M - Mdash) +
              0.0040*cos(Mdash - 2*F) +
              -0.0037*sin(2*D - 2*Mdash) +
              -0.0031*sin(F) +
              0.0030*sin(2*D + Mdash) +
              0.0029*cos(Mdash + 2*F) +
              -0.0029*E*sin(2*D - M) +
              -0.0027*sin(Mdash + F) +
              0.0024*E*sin(M - Mdash) +
              -0.0021*sin(Mdash - 3*F) +
              -0.0019*sin(2*Mdash + F) +
              -0.0006*cos(2*D - 2*Mdash - F) +
              -0.0018*sin(3*F) +
              -0.0017*cos(Mdash + 3*F) +
              0.0017*cos(2*Mdash) +
              0.0014*cos(2*D - Mdash) +
              -0.0013*cos(2*D + Mdash + F) +
              -0.0013*cos(Mdash) +
              0.0012*sin(3*Mdash + F) +
              0.0011*sin(2*D - Mdash + F) +
              0.0011*cos(2*D - 2*Mdash) +
              0.0010*cos(D + F) + 
              0.0010*E*sin(M + Mdash) +
              -0.0009*sin(2*D - 2*F) +
              -0.0007*cos(2*Mdash + F) +
              -0.0007*cos(3*Mdash + F);
  }

  return MeanGreatestDeclination(k, bNortherly) + DeltaJD;
}

double CAAMoonMaxDeclinations::TrueGreatestDeclinationValue(double k, bool bNortherly)
{
  //convert from K to T
  double T = k/1336.86;
  double T2 = T*T;
  double T3 = T2*T;

  double D = bNortherly ? 152.2029 : 345.6676;
  D = CAACoordinateTransformation::MapTo0To360Range(D + 333.0705546*k - 0.0004214*T2 + 0.00000011*T3);
  double M = bNortherly ? 14.8591 : 1.3951;
  M = CAACoordinateTransformation::MapTo0To360Range(M + 26.9281592*k - 0.0000355*T2 - 0.00000010*T3);
  double Mdash = bNortherly ? 4.6881 : 186.2100;
  Mdash = CAACoordinateTransformation::MapTo0To360Range(Mdash + 356.9562794*k + 0.0103066*T2 + 0.00001251*T3); 
  double F = bNortherly ? 325.8867 : 145.1633;
  F = CAACoordinateTransformation::MapTo0To360Range(F + 1.4467807*k - 0.0020690*T2 - 0.00000215*T3);
  double E = 1 - 0.002516*T - 0.0000074*T2;

  //convert to radians
  D = CAACoordinateTransformation::DegreesToRadians(D);
  M = CAACoordinateTransformation::DegreesToRadians(M);
  Mdash = CAACoordinateTransformation::DegreesToRadians(Mdash);
  F = CAACoordinateTransformation::DegreesToRadians(F);

  double DeltaValue;
  if (bNortherly)
  {
    DeltaValue = 5.1093*sin(F) +
                 0.2658*cos(2*F) +
                 0.1448*sin(2*D - F) +
                 -0.0322*sin(3*F) +
                 0.0133*cos(2*D - 2*F) +
                 0.0125*cos(2*D) +
                 -0.0124*sin(Mdash - F) +
                 -0.0101*sin(Mdash + 2*F) +
                 0.0097*cos(F) +
                 -0.0087*E*sin(2*D + M - F) +
                 0.0074*sin(Mdash + 3*F) +
                 0.0067*sin(D + F) +
                 0.0063*sin(Mdash - 2*F) +
                 0.0060*E*sin(2*D - M - F) +
                 -0.0057*sin(2*D - Mdash - F) +
                 -0.0056*cos(Mdash + F) +
                 0.0052*cos(Mdash + 2*F) +
                 0.0041*cos(2*Mdash + F) +
                 -0.0040*cos(Mdash - 3*F) +
                 0.0038*cos(2*Mdash - F) +
                 -0.0034*cos(Mdash - 2*F) +
                 -0.0029*sin(2*Mdash) +
                 0.0029*sin(3*Mdash + F) +
                 -0.0028*E*cos(2*D + M - F) +
                 -0.0028*cos(Mdash - F) +
                 -0.0023*cos(3*F) +
                 -0.0021*sin(2*D + F) +
                 0.0019*cos(Mdash + 3*F) +
                 0.0018*cos(D + F) +
                 0.0017*sin(2*Mdash - F) +
                 0.0015*cos(3*Mdash + F) +
                 0.0014*cos(2*D + 2*Mdash + F) +
                 -0.0012*sin(2*D - 2*Mdash - F) +
                 -0.0012*cos(2*Mdash) +
                 -0.0010*cos(Mdash) +
                 -0.0010*sin(2*F) +
                 0.0006*sin(Mdash + F);
  }
  else
  {
    DeltaValue = -5.1093*sin(F) +
                 0.2658*cos(2*F) +
                 -0.1448*sin(2*D - F) +
                 0.0322*sin(3*F) +
                 0.0133*cos(2*D - 2*F) +
                 0.0125*cos(2*D) +
                 -0.0015*sin(Mdash - F) +
                 0.0101*sin(Mdash + 2*F) +
                 -0.0097*cos(F) +
                 0.0087*E*sin(2*D + M - F) +
                 0.0074*sin(Mdash + 3*F) +
                 0.0067*sin(D + F) +
                 -0.0063*sin(Mdash - 2*F) +
                 -0.0060*E*sin(2*D - M - F) +
                 0.0057*sin(2*D - Mdash - F) +
                 -0.0056*cos(Mdash + F) +
                 -0.0052*cos(Mdash + 2*F) +
                 -0.0041*cos(2*Mdash + F) +
                 -0.0040*cos(Mdash - 3*F) +
                 -0.0038*cos(2*Mdash - F) +
                 0.0034*cos(Mdash - 2*F) +
                 -0.0029*sin(2*Mdash) +
                 0.0029*sin(3*Mdash + F) +
                 0.0028*E*cos(2*D + M - F) +
                 -0.0028*cos(Mdash - F) +
                 0.0023*cos(3*F) +
                 0.0021*sin(2*D + F) +
                 0.0019*cos(Mdash + 3*F) +
                 0.0018*cos(D + F) +
                 -0.0017*sin(2*Mdash - F) +
                 0.0015*cos(3*Mdash + F) +
                 0.0014*cos(2*D + 2*Mdash + F) +
                 0.0012*sin(2*D - 2*Mdash - F) +
                 -0.0012*cos(2*Mdash) +
                 0.0010*cos(Mdash) +
                 -0.0010*sin(2*F) +
                 0.0037*sin(Mdash + F);
  }

  return MeanGreatestDeclinationValue(k) + DeltaValue;
}

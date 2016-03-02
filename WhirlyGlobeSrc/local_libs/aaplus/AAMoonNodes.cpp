/*
Module : AAMOONNODES.CPP
Purpose: Implementation for the algorithms which obtain the dates when the Moon passes thro its nodes
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


//////////////////////////// Includes /////////////////////////////////////////

#include "stdafx.h"
#include "AAMoonNodes.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////////////// Implementation ///////////////////////////////////

double CAAMoonNodes::K(double Year)
{
  return 13.4223*(Year - 2000.05);
}

double CAAMoonNodes::PassageThroNode(double k)
{
  //convert from K to T
  double T = k/1342.23;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;
  double T4 = Tcubed*T;

  double D = CAACoordinateTransformation::MapTo0To360Range(183.6380 + 331.73735682*k + 0.0014852*Tsquared + 0.00000209*Tcubed - 0.000000010*T4);
  double M = CAACoordinateTransformation::MapTo0To360Range(17.4006 + 26.82037250*k + 0.0001186*Tsquared + 0.00000006*Tcubed);
  double Mdash = CAACoordinateTransformation::MapTo0To360Range(38.3776 + 355.52747313*k + 0.0123499*Tsquared + 0.000014627*Tcubed - 0.000000069*T4); 
  double omega = CAACoordinateTransformation::MapTo0To360Range(123.9767 - 1.44098956*k + 0.0020608*Tsquared + 0.00000214*Tcubed - 0.000000016*T4);
  double V = CAACoordinateTransformation::MapTo0To360Range(299.75 + 132.85*T - 0.009173*Tsquared);
  double P = CAACoordinateTransformation::MapTo0To360Range(omega + 272.75 - 2.3*T);
  double E = 1 - 0.002516*T - 0.0000074*Tsquared;

  //convert to radians
  D = CAACoordinateTransformation::DegreesToRadians(D);
  double D2 = 2*D;
  double D4 = D2*D2;
  M = CAACoordinateTransformation::DegreesToRadians(M);
  Mdash = CAACoordinateTransformation::DegreesToRadians(Mdash);
  double Mdash2 = 2*Mdash;
  omega = CAACoordinateTransformation::DegreesToRadians(omega);
  V = CAACoordinateTransformation::DegreesToRadians(V);
  P = CAACoordinateTransformation::DegreesToRadians(P);

  double JD = 2451565.1619 + 27.212220817*k 
              + 0.0002762*Tsquared 
              + 0.000000021*Tcubed 
              - 0.000000000088*T4 
              - 0.4721*sin(Mdash) 
              - 0.1649*sin(D2) 
              - 0.0868*sin(D2 - Mdash) 
              + 0.0084*sin(D2 + Mdash) 
              - E*0.0083*sin(D2 - M) 
              - E*0.0039*sin(D2 - M - Mdash) 
              + 0.0034*sin(Mdash2) 
              - 0.0031*sin(D2 - Mdash2) 
              + E*0.0030*sin(D2 + M) 
              + E*0.0028*sin(M - Mdash) 
              + E*0.0026*sin(M) 
              + 0.0025*sin(D4) 
              + 0.0024*sin(D) 
              + E*0.0022*sin(M + Mdash) 
              + 0.0017*sin(omega) 
              + 0.0014*sin(D4 - Mdash) 
              + E*0.0005*sin(D2 + M - Mdash) 
              + E*0.0004*sin(D2 - M + Mdash) 
              - E*0.0003*sin(D2 - M*M) 
              + E*0.0003*sin(D4 - M) 
              + 0.0003*sin(V) 
              + 0.0003*sin(P);

  return JD;
}

/*
Module : AAPHYSICALJUPITER.CPP
Purpose: Implementation for the algorithms which obtain the physical parameters of the Jupiter
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


///////////////////////////////// Includes ////////////////////////////////////

#include "stdafx.h"
#include "AAPhysicalJupiter.h"
#include "AAJupiter.h"
#include "AAEarth.h"
#include "AANutation.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


//////////////////////////////// Implementation ///////////////////////////////

CAAPhysicalJupiterDetails CAAPhysicalJupiter::Calculate(double JD)
{
  //What will be the return value
  CAAPhysicalJupiterDetails details;

  //Step 1
  double d = JD - 2433282.5;
  double T1 = d/36525;
  double alpha0 = 268.00 + 0.1061*T1;
  double alpha0rad = CAACoordinateTransformation::DegreesToRadians(alpha0);
  double delta0 = 64.50 - 0.0164*T1;
  double delta0rad = CAACoordinateTransformation::DegreesToRadians(delta0);

  //Step 2
  double W1 = CAACoordinateTransformation::MapTo0To360Range(17.710 + 877.90003539*d);
  double W2 = CAACoordinateTransformation::MapTo0To360Range(16.838 + 870.27003539*d);
  
  //Step 3
  double l0 = CAAEarth::EclipticLongitude(JD);
  double l0rad = CAACoordinateTransformation::DegreesToRadians(l0);
  double b0 = CAAEarth::EclipticLatitude(JD);
  double b0rad = CAACoordinateTransformation::DegreesToRadians(b0);
  double R = CAAEarth::RadiusVector(JD);

  //Step 4
  double l = CAAJupiter::EclipticLongitude(JD);
  double lrad = CAACoordinateTransformation::DegreesToRadians(l);
  double b = CAAJupiter::EclipticLatitude(JD);
  double brad = CAACoordinateTransformation::DegreesToRadians(b);
  double r = CAAJupiter::RadiusVector(JD);

  //Step 5
  double x = r*cos(brad)*cos(lrad) - R*cos(l0rad);
  double y = r*cos(brad)*sin(lrad) - R*sin(l0rad);
  double z = r*sin(brad) - R*sin(b0rad);
  double DELTA = sqrt(x*x + y*y + z*z);

  //Step 6
  l -= 0.012990*DELTA/(r*r);
  lrad = CAACoordinateTransformation::DegreesToRadians(l);

  //Step 7
  x = r*cos(brad)*cos(lrad) - R*cos(l0rad);
  y = r*cos(brad)*sin(lrad) - R*sin(l0rad);
  z = r*sin(brad) - R*sin(b0rad);
  DELTA = sqrt(x*x + y*y + z*z);

  //Step 8
  double e0 = CAANutation::MeanObliquityOfEcliptic(JD);
  double e0rad = CAACoordinateTransformation::DegreesToRadians(e0);

  //Step 9
  double alphas = atan2(cos(e0rad)*sin(lrad) - sin(e0rad)*tan(brad), cos(lrad));
  double deltas = asin(cos(e0rad)*sin(brad) + sin(e0rad)*cos(brad)*sin(lrad));

  //Step 10
  details.DS = CAACoordinateTransformation::RadiansToDegrees(asin(-sin(delta0rad)*sin(deltas) - cos(delta0rad)*cos(deltas)*cos(alpha0rad - alphas)));

  //Step 11
  double u = y*cos(e0rad) - z*sin(e0rad);
  double v = y*sin(e0rad) + z*cos(e0rad);
  double alpharad = atan2(u, x);
  double alpha = CAACoordinateTransformation::RadiansToDegrees(alpharad);
  double deltarad = atan2(v, sqrt(x*x + u*u));
  double delta = CAACoordinateTransformation::RadiansToDegrees(deltarad);
  double xi = atan2(sin(delta0rad)*cos(deltarad)*cos(alpha0rad - alpharad) - sin(deltarad)*cos(delta0rad), cos(deltarad)*sin(alpha0rad - alpharad));

  //Step 12
  details.DE = CAACoordinateTransformation::RadiansToDegrees(asin(-sin(delta0rad)*sin(deltarad) - cos(delta0rad)*cos(deltarad)*cos(alpha0rad - alpharad)));

  //Step 13
  details.Geometricw1 = CAACoordinateTransformation::MapTo0To360Range(W1 - CAACoordinateTransformation::RadiansToDegrees(xi) - 5.07033*DELTA);
  details.Geometricw2 = CAACoordinateTransformation::MapTo0To360Range(W2 - CAACoordinateTransformation::RadiansToDegrees(xi) - 5.02626*DELTA);

  //Step 14
  double C = 57.2958 * (2*r*DELTA + R*R - r*r - DELTA*DELTA)/(4*r*DELTA);
  if (sin(lrad - l0rad) > 0)
  {
    details.Apparentw1 = CAACoordinateTransformation::MapTo0To360Range(details.Geometricw1 + C);
    details.Apparentw2 = CAACoordinateTransformation::MapTo0To360Range(details.Geometricw2 + C);
  }
  else
  {
    details.Apparentw1 = CAACoordinateTransformation::MapTo0To360Range(details.Geometricw1 - C);
    details.Apparentw2 = CAACoordinateTransformation::MapTo0To360Range(details.Geometricw2 - C);
  }

  //Step 15
  double NutationInLongitude = CAANutation::NutationInLongitude(JD);
  double NutationInObliquity = CAANutation::NutationInObliquity(JD);
  e0 += NutationInObliquity/3600;
  e0rad = CAACoordinateTransformation::DegreesToRadians(e0);

  //Step 16
  alpha += 0.005693*(cos(alpharad)*cos(l0rad)*cos(e0rad) + sin(alpharad)*sin(l0rad))/cos(deltarad);
  alpha = CAACoordinateTransformation::MapTo0To360Range(alpha);
  alpharad = CAACoordinateTransformation::DegreesToRadians(alpha);
  delta += 0.005693*(cos(l0rad)*cos(e0rad)*(tan(e0rad)*cos(deltarad) - sin(alpharad)*sin(deltarad)) + cos(alpharad)*sin(deltarad)*sin(l0rad));

  //Step 17
  double NutationRA = CAANutation::NutationInRightAscension(alpha/15, delta, e0, NutationInLongitude, NutationInObliquity);
  double alphadash = alpha + NutationRA/3600;
  double alphadashrad = CAACoordinateTransformation::DegreesToRadians(alphadash);
  double NutationDec = CAANutation::NutationInDeclination(alpha/15, e0, NutationInLongitude, NutationInObliquity);
  double deltadash = delta + NutationDec/3600;
  double deltadashrad = CAACoordinateTransformation::DegreesToRadians(deltadash);
  NutationRA = CAANutation::NutationInRightAscension(alpha0/15, delta0, e0, NutationInLongitude, NutationInObliquity);
  double alpha0dash = alpha0 + NutationRA/3600;
  double alpha0dashrad = CAACoordinateTransformation::DegreesToRadians(alpha0dash);
  NutationDec = CAANutation::NutationInDeclination(alpha0/15, e0, NutationInLongitude, NutationInObliquity);
  double delta0dash = delta0 + NutationDec/3600;
  double delta0dashrad = CAACoordinateTransformation::DegreesToRadians(delta0dash);

  //Step 18
  details.P = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(atan2(cos(delta0dashrad)*sin(alpha0dashrad - alphadashrad), sin(delta0dashrad)*cos(deltadashrad) - cos(delta0dashrad)*sin(deltadashrad)*cos(alpha0dashrad - alphadashrad))));

  return details;
}

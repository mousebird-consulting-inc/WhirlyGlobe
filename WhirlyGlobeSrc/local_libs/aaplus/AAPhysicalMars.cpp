/*
Module : AAPHYSICALMARS.CPP
Purpose: Implementation for the algorithms which obtain the physical parameters of Mars
Created: PJN / 04-01-2004
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
#include "AAPhysicalMars.h"
#include "AAMars.h"
#include "AAEarth.h"
#include "AASun.h"
#include "AANutation.h"
#include "AACoordinateTransformation.h"
#include "AAElliptical.h"
#include "AAMoonIlluminatedFraction.h"
#include "AAIlluminatedFraction.h"
#include <cmath>
using namespace std;


//////////////////////////////// Implementation ///////////////////////////////

CAAPhysicalMarsDetails CAAPhysicalMars::Calculate(double JD)
{
  //What will be the return value
  CAAPhysicalMarsDetails details;

  //Step 1
  double T = (JD - 2451545) / 36525;
  double Lambda0 = 352.9065 + 1.17330*T;
  double Lambda0rad = CAACoordinateTransformation::DegreesToRadians(Lambda0);
  double Beta0 = 63.2818 - 0.00394*T;
  double Beta0rad = CAACoordinateTransformation::DegreesToRadians(Beta0);

  //Step 2
  double l0 = CAAEarth::EclipticLongitude(JD);
  double l0rad = CAACoordinateTransformation::DegreesToRadians(l0);
  double b0 = CAAEarth::EclipticLatitude(JD);
  double b0rad = CAACoordinateTransformation::DegreesToRadians(b0);
  double R = CAAEarth::RadiusVector(JD);

  double PreviousLightTravelTime = 0;
  double LightTravelTime = 0;
  double x = 0;
  double y = 0;
  double z = 0;
  bool   bIterate = true;
  double DELTA = 0;
  double l = 0;
  double lrad = 0;
  double b = 0;
  double r = 0;
  while (bIterate)
  {
    double JD2 = JD - LightTravelTime;

    //Step 3
    l = CAAMars::EclipticLongitude(JD2);
    lrad = CAACoordinateTransformation::DegreesToRadians(l);
    b = CAAMars::EclipticLatitude(JD2);
    double brad = CAACoordinateTransformation::DegreesToRadians(b);
    r = CAAMars::RadiusVector(JD2);

    //Step 4
    x = r*cos(brad)*cos(lrad) - R*cos(l0rad);
    y = r*cos(brad)*sin(lrad) - R*sin(l0rad);
    z = r*sin(brad) - R*sin(b0rad);
    DELTA = sqrt(x*x + y*y + z*z);
    LightTravelTime = CAAElliptical::DistanceToLightTime(DELTA);

    //Prepare for the next loop around
    bIterate = (fabs(LightTravelTime - PreviousLightTravelTime) > 2E-6); //2E-6 correponds to 0.17 of a second
    if (bIterate)
      PreviousLightTravelTime = LightTravelTime;
  }

  //Step 5
  double lambdarad = atan2(y, x);
  double lambda = CAACoordinateTransformation::RadiansToDegrees(lambdarad);
  double betarad = atan2(z, sqrt(x*x + y*y));
  double beta = CAACoordinateTransformation::RadiansToDegrees(betarad);

  //Step 6
  details.DE = CAACoordinateTransformation::RadiansToDegrees(asin(-sin(Beta0rad)*sin(betarad) - cos(Beta0rad)*cos(betarad)*cos(Lambda0rad - lambdarad)));

  //Step 7
  double N = 49.5581 + 0.7721*T;
  double Nrad = CAACoordinateTransformation::DegreesToRadians(N);

  double ldash = l - 0.00697/r;
  double ldashrad = CAACoordinateTransformation::DegreesToRadians(ldash);
  double bdash = b - 0.000225*(cos(lrad - Nrad)/r);
  double bdashrad = CAACoordinateTransformation::DegreesToRadians(bdash);

  //Step 8
  details.DS = CAACoordinateTransformation::RadiansToDegrees(asin(-sin(Beta0rad)*sin(bdashrad) - cos(Beta0rad)*cos(bdashrad)*cos(Lambda0rad - ldashrad)));

  //Step 9
  double W = CAACoordinateTransformation::MapTo0To360Range(11.504 + 350.89200025*(JD - LightTravelTime - 2433282.5));

  //Step 10
  double e0 = CAANutation::MeanObliquityOfEcliptic(JD);
  double e0rad = CAACoordinateTransformation::DegreesToRadians(e0);
  CAA2DCoordinate PoleEquatorial = CAACoordinateTransformation::Ecliptic2Equatorial(Lambda0, Beta0, e0);
  double alpha0rad = CAACoordinateTransformation::HoursToRadians(PoleEquatorial.X);
  double delta0rad = CAACoordinateTransformation::DegreesToRadians(PoleEquatorial.Y);

  //Step 11
  double u = y*cos(e0rad) - z*sin(e0rad);
  double v = y*sin(e0rad) + z*cos(e0rad);
  double alpharad = atan2(u, x);
  double alpha = CAACoordinateTransformation::RadiansToHours(alpharad);
  double deltarad = atan2(v, sqrt(x*x + u*u));
  double delta = CAACoordinateTransformation::RadiansToDegrees(deltarad);
  double xi = atan2(sin(delta0rad)*cos(deltarad)*cos(alpha0rad - alpharad) - sin(deltarad)*cos(delta0rad), cos(deltarad)*sin(alpha0rad - alpharad));

  //Step 12
  details.w = CAACoordinateTransformation::MapTo0To360Range(W - CAACoordinateTransformation::RadiansToDegrees(xi));

  //Step 13
  double NutationInLongitude = CAANutation::NutationInLongitude(JD);
  double NutationInObliquity = CAANutation::NutationInObliquity(JD);

  //Step 14
  lambda += 0.005693*cos(l0rad - lambdarad)/cos(betarad);
  beta += 0.005693*sin(l0rad - lambdarad)*sin(betarad);

  //Step 15
  Lambda0 += NutationInLongitude/3600;
  lambda += NutationInLongitude/3600;
  e0 += NutationInObliquity/3600;

  //Step 16
  CAA2DCoordinate ApparentPoleEquatorial = CAACoordinateTransformation::Ecliptic2Equatorial(Lambda0, Beta0, e0);  
  double alpha0dash = CAACoordinateTransformation::HoursToRadians(ApparentPoleEquatorial.X);
  double delta0dash = CAACoordinateTransformation::DegreesToRadians(ApparentPoleEquatorial.Y);
  CAA2DCoordinate ApparentMars = CAACoordinateTransformation::Ecliptic2Equatorial(lambda, beta, e0);  
  double alphadash = CAACoordinateTransformation::HoursToRadians(ApparentMars.X);
  double deltadash = CAACoordinateTransformation::DegreesToRadians(ApparentMars.Y);

  //Step 17
  details.P = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(atan2(cos(delta0dash)*sin(alpha0dash - alphadash), sin(delta0dash)*cos(deltadash) - cos(delta0dash)*sin(deltadash)*cos(alpha0dash - alphadash))));

  //Step 18
  double SunLambda = CAASun::GeometricEclipticLongitude(JD);
  double SunBeta = CAASun::GeometricEclipticLatitude(JD);
  CAA2DCoordinate SunEquatorial = CAACoordinateTransformation::Ecliptic2Equatorial(SunLambda, SunBeta, e0);
  details.X = CAAMoonIlluminatedFraction::PositionAngle(SunEquatorial.X, SunEquatorial.Y, alpha, delta);

  //Step 19
  details.d = 9.36 / DELTA;
  details.k = CAAIlluminatedFraction::IlluminatedFraction(r, R, DELTA);
  details.q = (1 - details.k)*details.d;

  return details;
}

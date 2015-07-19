/*
Module : AASATURNRINGS.CPP
Purpose: Implementation for the algorithms which calculate various parameters related to the Rings of Saturn
Created: PJN / 08-01-2004
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


///////////////////////////////// Includes ////////////////////////////////////

#include "stdafx.h"
#include "AASaturnRings.h"
#include "AASaturn.h"
#include "AAEarth.h"
#include "AAFK5.h"
#include "AANutation.h"
#include "AACoordinateTransformation.h"
#include "AAElliptical.h"
#include <cmath>
using namespace std;


//////////////////////////////// Implementation ///////////////////////////////

CAASaturnRingDetails CAASaturnRings::Calculate(double JD)
{
  //What will be the return value
  CAASaturnRingDetails details;

  double T = (JD - 2451545) / 36525;
  double T2 = T*T;

  //Step 1. Calculate the inclination of the plane of the ring and the longitude of the ascending node referred to the ecliptic and mean equinox of the date
  double i = 28.075216 - 0.012998*T + 0.000004*T2;
  double irad = CAACoordinateTransformation::DegreesToRadians(i);
  double omega = 169.508470 + 1.394681*T + 0.000412*T2;
  double omegarad = CAACoordinateTransformation::DegreesToRadians(omega);

  //Step 2. Calculate the heliocentric longitude, latitude and radius vector of the Earth in the FK5 system
  double l0 = CAAEarth::EclipticLongitude(JD);
  double b0 = CAAEarth::EclipticLatitude(JD);
  l0 += CAAFK5::CorrectionInLongitude(l0, b0, JD);
  double l0rad = CAACoordinateTransformation::DegreesToRadians(l0);
  b0 += CAAFK5::CorrectionInLatitude(l0, JD);
  double b0rad = CAACoordinateTransformation::DegreesToRadians(b0);
  double R = CAAEarth::RadiusVector(JD);

  //Step 3. Calculate the corresponding coordinates l,b,r for Saturn but for the instance t-lightraveltime
  double DELTA = 9;
  double PreviousEarthLightTravelTime = 0;
  double EarthLightTravelTime = CAAElliptical::DistanceToLightTime(DELTA);
  double JD1 = JD - EarthLightTravelTime;
  bool   bIterate = true;
  double x = 0;
  double y = 0;
  double z = 0;
  double l = 0;
  double b = 0;
  double r = 0;
  while (bIterate)
  {
    //Calculate the position of Saturn
    l = CAASaturn::EclipticLongitude(JD1);
    b = CAASaturn::EclipticLatitude(JD1);
    l += CAAFK5::CorrectionInLongitude(l, b, JD1);
    b += CAAFK5::CorrectionInLatitude(l, JD1);

    double lrad = CAACoordinateTransformation::DegreesToRadians(l);
    double brad = CAACoordinateTransformation::DegreesToRadians(b);
    r = CAASaturn::RadiusVector(JD1);

    //Step 4
    x = r*cos(brad)*cos(lrad) - R*cos(l0rad);
    y = r*cos(brad)*sin(lrad) - R*sin(l0rad);
    z = r*sin(brad) - R*sin(b0rad);
    DELTA = sqrt(x*x + y*y + z*z);
    EarthLightTravelTime = CAAElliptical::DistanceToLightTime(DELTA);

    //Prepare for the next loop around
    bIterate = (fabs(EarthLightTravelTime - PreviousEarthLightTravelTime) > 2E-6); //2E-6 corresponds to 0.17 of a second
    if (bIterate)
    {
      JD1 = JD - EarthLightTravelTime;
      PreviousEarthLightTravelTime = EarthLightTravelTime;
    }
  }

  //Step 5. Calculate Saturn's geocentric Longitude and Latitude
  double lambda = atan2(y, x);
  double beta = atan2(z, sqrt(x*x + y*y));

  //Step 6. Calculate B, a and b
  details.B = asin(sin(irad)*cos(beta)*sin(lambda - omegarad) - cos(irad)*sin(beta));
  details.a = 375.35 / DELTA;
  details.b = details.a * sin(fabs(details.B));
  details.B = CAACoordinateTransformation::RadiansToDegrees(details.B);

  //Step 7. Calculate the longitude of the ascending node of Saturn's orbit
  double N = 113.6655 + 0.8771*T;
  double Nrad = CAACoordinateTransformation::DegreesToRadians(N);
  double ldash = l - 0.01759/r;
  double ldashrad = CAACoordinateTransformation::DegreesToRadians(ldash);
  double bdash = b - 0.000764*cos(ldashrad - Nrad)/r;
  double bdashrad = CAACoordinateTransformation::DegreesToRadians(bdash);

  //Step 8. Calculate Bdash
  details.Bdash = CAACoordinateTransformation::RadiansToDegrees(asin(sin(irad)*cos(bdashrad)*sin(ldashrad - omegarad) - cos(irad)*sin(bdashrad)));

  //Step 9. Calculate DeltaU
  double U1 = atan2(sin(irad)*sin(bdashrad) + cos(irad)*cos(bdashrad)*sin(ldashrad - omegarad), cos(bdashrad)*cos(ldashrad - omegarad));
  double U2 = atan2(sin(irad)*sin(beta) + cos(irad)*cos(beta)*sin(lambda - omegarad), cos(beta)*cos(lambda - omegarad));
  details.DeltaU = CAACoordinateTransformation::RadiansToDegrees(fabs(U1 - U2));

  //Step 10. Calculate the Nutations 
  double Obliquity = CAANutation::TrueObliquityOfEcliptic(JD);
  double NutationInLongitude = CAANutation::NutationInLongitude(JD);

  //Step 11. Calculate the Ecliptical longitude and latitude of the northern pole of the ring plane
  double lambda0 = omega - 90;
  double beta0 = 90 - i;

  //Step 12. Correct lambda and beta for the aberration of Saturn
  lambda += CAACoordinateTransformation::DegreesToRadians(0.005693*cos(l0rad - lambda)/cos(beta));
  beta += CAACoordinateTransformation::DegreesToRadians(0.005693*sin(l0rad - lambda)*sin(beta));

  //Step 13. Add nutation in longitude to lambda0 and lambda
  //double NLrad = CAACoordinateTransformation::DegreesToRadians(NutationInLongitude/3600);
  lambda = CAACoordinateTransformation::RadiansToDegrees(lambda);
  lambda += NutationInLongitude/3600;
  lambda = CAACoordinateTransformation::MapTo0To360Range(lambda);
  lambda0 += NutationInLongitude/3600;
  lambda0 = CAACoordinateTransformation::MapTo0To360Range(lambda0);

  //Step 14. Convert to equatorial coordinates
  beta = CAACoordinateTransformation::RadiansToDegrees(beta);
  CAA2DCoordinate GeocentricEclipticSaturn = CAACoordinateTransformation::Ecliptic2Equatorial(lambda, beta, Obliquity);  
  double alpha = CAACoordinateTransformation::HoursToRadians(GeocentricEclipticSaturn.X);
  double delta = CAACoordinateTransformation::DegreesToRadians(GeocentricEclipticSaturn.Y);
  CAA2DCoordinate GeocentricEclipticNorthPole = CAACoordinateTransformation::Ecliptic2Equatorial(lambda0, beta0, Obliquity);  
  double alpha0 = CAACoordinateTransformation::HoursToRadians(GeocentricEclipticNorthPole.X);
  double delta0 = CAACoordinateTransformation::DegreesToRadians(GeocentricEclipticNorthPole.Y);

  //Step 15. Calculate the Position angle
  details.P = CAACoordinateTransformation::RadiansToDegrees(atan2(cos(delta0)*sin(alpha0 - alpha), sin(delta0)*cos(delta) - cos(delta0)*sin(delta)*cos(alpha0 - alpha)));

  return details;
}

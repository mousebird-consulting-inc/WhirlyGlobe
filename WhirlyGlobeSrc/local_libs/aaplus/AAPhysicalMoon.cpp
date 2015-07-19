/*
Module : AAPHYSICALMOON.CPP
Purpose: Implementation for the algorithms which obtain the physical parameters of the Moon
Created: PJN / 17-01-2004
History: PJN / 19-02-2004 1. The optical libration in longitude is now returned in the range -180 - 180 degrees
         PJN / 10-05-2010 1. Minor update to CAAPhysicalMoon::CalculateTopocentric to put a value in a variable 
                          for easier debugging 

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
#include "AAPhysicalMoon.h"
#include "AASun.h"
#include "AAMoon.h"
#include "AAEarth.h"
#include "AANutation.h"
#include "AASidereal.h"
#include <cmath>
using namespace std;


//////////////////////////////// Implementation ///////////////////////////////

void CAAPhysicalMoon::CalculateOpticalLibration(double JD, double Lambda, double Beta, double& ldash, double& bdash, double& ldash2, double& bdash2, double& epsilon, double& omega, double& DeltaU, double& sigma, double& I, double& rho)
{
  //Calculate the initial quantities
  double Lambdarad = CAACoordinateTransformation::DegreesToRadians(Lambda);
  double Betarad = CAACoordinateTransformation::DegreesToRadians(Beta);
  I = CAACoordinateTransformation::DegreesToRadians(1.54242);
  DeltaU = CAACoordinateTransformation::DegreesToRadians(CAANutation::NutationInLongitude(JD)/3600);
  double F = CAACoordinateTransformation::DegreesToRadians(CAAMoon::ArgumentOfLatitude(JD));
  omega = CAACoordinateTransformation::DegreesToRadians(CAAMoon::MeanLongitudeAscendingNode(JD));
  epsilon = CAANutation::MeanObliquityOfEcliptic(JD) + CAANutation::NutationInObliquity(JD)/3600;

  //Calculate the optical librations
  double W = Lambdarad - DeltaU/3600 - omega;
  double A = atan2(sin(W)*cos(Betarad)*cos(I) - sin(Betarad)*sin(I), cos(W)*cos(Betarad));
  ldash = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(A) - CAACoordinateTransformation::RadiansToDegrees(F));
  if (ldash > 180)
    ldash -= 360;
  bdash = asin(-sin(W)*cos(Betarad)*sin(I) - sin(Betarad)*cos(I));

  //Calculate the physical librations
  double T = (JD - 2451545.0)/36525;
  double K1 = 119.75 + 131.849*T;
  K1 = CAACoordinateTransformation::DegreesToRadians(K1);
  double K2 = 72.56 + 20.186*T;
  K2 = CAACoordinateTransformation::DegreesToRadians(K2);

  double M = CAAEarth::SunMeanAnomaly(JD);
  M = CAACoordinateTransformation::DegreesToRadians(M);
  double Mdash = CAAMoon::MeanAnomaly(JD);
  Mdash = CAACoordinateTransformation::DegreesToRadians(Mdash);
  double D = CAAMoon::MeanElongation(JD);
  D = CAACoordinateTransformation::DegreesToRadians(D);
  double E = CAAEarth::Eccentricity(JD);

  rho = -0.02752*cos(Mdash) +
        -0.02245*sin(F) +
        0.00684*cos(Mdash - 2*F) +
        -0.00293*cos(2*F) +
        -0.00085*cos(2*F - 2*D) +
        -0.00054*cos(Mdash - 2*D) +
        -0.00020*sin(Mdash + F) +
        -0.00020*cos(Mdash + 2*F) +
        -0.00020*cos(Mdash - F) +
        0.00014*cos(Mdash + 2*F - 2*D);

  sigma = -0.02816*sin(Mdash) +
          0.02244*cos(F) +
          -0.00682*sin(Mdash - 2*F) +
          -0.00279*sin(2*F) +
          -0.00083*sin(2*F - 2*D) +
          0.00069*sin(Mdash - 2*D) +
          0.00040*cos(Mdash + F) +
          -0.00025*sin(2*Mdash) +
          -0.00023*sin(Mdash + 2*F) +
          0.00020*cos(Mdash - F) +
          0.00019*sin(Mdash - F) +
          0.00013*sin(Mdash + 2*F - 2*D) +
          -0.00010*cos(Mdash - 3*F);

  double tau = 0.02520*E*sin(M) +
               0.00473*sin(2*Mdash - 2*F) +
               -0.00467*sin(Mdash) +
               0.00396*sin(K1) +
               0.00276*sin(2*Mdash - 2*D) +
               0.00196*sin(omega) +
               -0.00183*cos(Mdash - F) +
               0.00115*sin(Mdash - 2*D) +
               -0.00096*sin(Mdash - D) +
               0.00046*sin(2*F - 2*D) +
               -0.00039*sin(Mdash - F) +
               -0.00032*sin(Mdash - M - D) +
               0.00027*sin(2*Mdash - M - 2*D) +
               0.00023*sin(K2) +
               -0.00014*sin(2*D) +
               0.00014*cos(2*Mdash - 2*F) +
               -0.00012*sin(Mdash - 2*F) +
               -0.00012*sin(2*Mdash) +
               0.00011*sin(2*Mdash - 2*M - 2*D);

  ldash2 = -tau + (rho*cos(A) + sigma*sin(A))*tan(bdash);
  bdash = CAACoordinateTransformation::RadiansToDegrees(bdash);
  bdash2 = sigma*cos(A) - rho*sin(A);
}

CAAPhysicalMoonDetails CAAPhysicalMoon::CalculateHelper(double JD, double& Lambda, double& Beta, double& epsilon, CAA2DCoordinate& Equatorial)
{
  //What will be the return value
  CAAPhysicalMoonDetails details;

  //Calculate the initial quantities
  Lambda = CAAMoon::EclipticLongitude(JD);
  Beta = CAAMoon::EclipticLatitude(JD);

  //Calculate the optical libration
  double omega;
  double DeltaU;
  double sigma;
  double I;
  double rho;
  CalculateOpticalLibration(JD, Lambda, Beta, details.ldash, details.bdash, details.ldash2, details.bdash2, epsilon, omega, DeltaU, sigma, I, rho);
  double epsilonrad = CAACoordinateTransformation::DegreesToRadians(epsilon);

  //Calculate the total libration
  details.l = details.ldash + details.ldash2;
  details.b = details.bdash + details.bdash2;
  double b = CAACoordinateTransformation::DegreesToRadians(details.b);

  //Calculate the position angle
  double V = omega + DeltaU + CAACoordinateTransformation::DegreesToRadians(sigma)/sin(I);
  double I_rho = I + CAACoordinateTransformation::DegreesToRadians(rho);
  double X = sin(I_rho)*sin(V);
  double Y = sin(I_rho)*cos(V)*cos(epsilonrad) - cos(I_rho)*sin(epsilonrad);
  double w = atan2(X, Y);

  Equatorial = CAACoordinateTransformation::Ecliptic2Equatorial(Lambda, Beta, epsilon);
  double Alpha = CAACoordinateTransformation::HoursToRadians(Equatorial.X);

  details.P = CAACoordinateTransformation::RadiansToDegrees(asin(sqrt(X*X + Y*Y)*cos(Alpha - w)/(cos(b))));

  return details;
}

CAAPhysicalMoonDetails CAAPhysicalMoon::CalculateGeocentric(double JD)
{
  double Lambda;
  double Beta;
  double epsilon;
  CAA2DCoordinate Equatorial;
  return CalculateHelper(JD, Lambda, Beta, epsilon, Equatorial);
}

CAAPhysicalMoonDetails CAAPhysicalMoon::CalculateTopocentric(double JD, double Longitude, double Latitude)
{
  //First convert to radians
  Longitude = CAACoordinateTransformation::DegreesToRadians(Longitude);
  Latitude = CAACoordinateTransformation::DegreesToRadians(Latitude);

  double Lambda;
  double Beta;
  double epsilon;
  CAA2DCoordinate Equatorial;
  CAAPhysicalMoonDetails details = CalculateHelper(JD, Lambda, Beta, epsilon, Equatorial);

  double R = CAAMoon::RadiusVector(JD);
  double pi = CAAMoon::RadiusVectorToHorizontalParallax(R);
  double Alpha = CAACoordinateTransformation::HoursToRadians(Equatorial.X);
  double Delta = CAACoordinateTransformation::DegreesToRadians(Equatorial.Y);  

  double AST = CAASidereal::ApparentGreenwichSiderealTime(JD);
  double H = CAACoordinateTransformation::HoursToRadians(AST) - Longitude - Alpha;

  double Q = atan2(cos(Latitude)*sin(H), cos(Delta)*sin(Latitude) - sin(Delta)*cos(Latitude)*cos(H));
  double Z = acos(sin(Delta)*sin(Latitude) + cos(Delta)*cos(Latitude)*cos(H));
  double pidash = pi*(sin(Z) + 0.0084*sin(2*Z));

  double Prad = CAACoordinateTransformation::DegreesToRadians(details.P);

  double DeltaL = -pidash*sin(Q - Prad)/cos(CAACoordinateTransformation::DegreesToRadians(details.b));
  details.l += DeltaL;
  double DeltaB = pidash*cos(Q - Prad);
  details.b += DeltaB;
  double DeltaP = DeltaL*sin(CAACoordinateTransformation::DegreesToRadians(details.b)) - pidash*sin(Q)*tan(Delta);
  details.P += DeltaP;

  return details;
}

CAASelenographicMoonDetails CAAPhysicalMoon::CalculateSelenographicPositionOfSun(double JD)
{
  double R = CAAEarth::RadiusVector(JD)*149597970;
  double Delta = CAAMoon::RadiusVector(JD);
  double lambda0 = CAASun::ApparentEclipticLongitude(JD);
  double lambda = CAAMoon::EclipticLongitude(JD);
  double beta = CAAMoon::EclipticLatitude(JD);

  double lambdah = CAACoordinateTransformation::MapTo0To360Range(lambda0 + 180 + Delta/R*57.296*cos(CAACoordinateTransformation::DegreesToRadians(beta))*sin(CAACoordinateTransformation::DegreesToRadians(lambda0 - lambda)));
  double betah = Delta/R*beta;

  //What will be the return value
  CAASelenographicMoonDetails details;

  //Calculate the optical libration
  double omega;
  double DeltaU;
  double sigma;
  double I;
  double rho;
  double ldash0;
  double bdash0;
  double ldash20;
  double bdash20;
  double epsilon;
  CalculateOpticalLibration(JD, lambdah, betah, ldash0, bdash0, ldash20, bdash20, epsilon, omega, DeltaU, sigma, I, rho);

  details.l0 = ldash0 + ldash20;
  details.b0 = bdash0 + bdash20;
  details.c0 = CAACoordinateTransformation::MapTo0To360Range(450 - details.l0);
  return details;
}

double CAAPhysicalMoon::AltitudeOfSun(double JD, double Longitude, double Latitude)
{
  //Calculate the selenographic details
  CAASelenographicMoonDetails selenographicDetails = CalculateSelenographicPositionOfSun(JD);

  //convert to radians
  Latitude = CAACoordinateTransformation::DegreesToRadians(Latitude);
  Longitude = CAACoordinateTransformation::DegreesToRadians(Longitude);
  selenographicDetails.b0 = CAACoordinateTransformation::DegreesToRadians(selenographicDetails.b0);
  selenographicDetails.c0 = CAACoordinateTransformation::DegreesToRadians(selenographicDetails.c0);

  return CAACoordinateTransformation::RadiansToDegrees(asin(sin(selenographicDetails.b0)*sin(Latitude) + cos(selenographicDetails.b0)*cos(Latitude)*sin(selenographicDetails.c0 + Longitude)));
}

double CAAPhysicalMoon::SunriseSunsetHelper(double JD, double Longitude, double Latitude, bool bSunrise)
{
  double JDResult = JD;
  double Latituderad = CAACoordinateTransformation::DegreesToRadians(Latitude);
  double h;
  do
  {
    h = AltitudeOfSun(JDResult, Longitude, Latitude);
    double DeltaJD = h/(12.19075*cos(Latituderad));
    if (bSunrise)
      JDResult -= DeltaJD;
    else
      JDResult += DeltaJD;
  }
  while (fabs(h) > 0.001);

  return JDResult;
}

double CAAPhysicalMoon::TimeOfSunrise(double JD, double Longitude, double Latitude)
{
  return SunriseSunsetHelper(JD, Longitude, Latitude, true);
}

double CAAPhysicalMoon::TimeOfSunset(double JD, double Longitude, double Latitude)
{
  return SunriseSunsetHelper(JD, Longitude, Latitude, false);
}

/*
Module : AAGLOBE.CPP
Purpose: Implementation for the algorithms for the Earth's Globe
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


/////////////////////////// Includes //////////////////////////////////////////

#include "stdafx.h"
#include "AAGlobe.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


/////////////////////////// Implementation ////////////////////////////////////

double CAAGlobe::RhoSinThetaPrime(double GeographicalLatitude, double Height)
{
  GeographicalLatitude = CAACoordinateTransformation::DegreesToRadians(GeographicalLatitude);

  double U = atan(0.99664719 * tan(GeographicalLatitude));
  return 0.99664719 * sin(U) + (Height/6378149 * sin(GeographicalLatitude));
}

double CAAGlobe::RhoCosThetaPrime(double GeographicalLatitude, double Height)
{
  //Convert from degress to radians
  GeographicalLatitude = CAACoordinateTransformation::DegreesToRadians(GeographicalLatitude);

  double U = atan(0.99664719 * tan(GeographicalLatitude));
  return cos(U) + (Height/6378149 * cos(GeographicalLatitude));
}

double CAAGlobe::RadiusOfParallelOfLatitude(double GeographicalLatitude)
{
  //Convert from degress to radians
  GeographicalLatitude = CAACoordinateTransformation::DegreesToRadians(GeographicalLatitude);

  double sinGeo = sin(GeographicalLatitude);
  return (6378.14 * cos(GeographicalLatitude)) / (sqrt(1 - 0.0066943847614084*sinGeo*sinGeo));
}

double CAAGlobe::RadiusOfCurvature(double GeographicalLatitude)
{
  //Convert from degress to radians
  GeographicalLatitude = CAACoordinateTransformation::DegreesToRadians(GeographicalLatitude);

  double sinGeo = sin(GeographicalLatitude);
  return (6378.14 * (1 - 0.0066943847614084)) / pow((1 - 0.0066943847614084 * sinGeo * sinGeo), 1.5);
}

double CAAGlobe::DistanceBetweenPoints(double GeographicalLatitude1, double GeographicalLongitude1, double GeographicalLatitude2, double GeographicalLongitude2)
{
  //Convert from degress to radians
  GeographicalLatitude1 = CAACoordinateTransformation::DegreesToRadians(GeographicalLatitude1);
  GeographicalLatitude2 = CAACoordinateTransformation::DegreesToRadians(GeographicalLatitude2);
  GeographicalLongitude1 = CAACoordinateTransformation::DegreesToRadians(GeographicalLongitude1);
  GeographicalLongitude2 = CAACoordinateTransformation::DegreesToRadians(GeographicalLongitude2);

  double F = (GeographicalLatitude1 + GeographicalLatitude2) / 2;
  double G = (GeographicalLatitude1 - GeographicalLatitude2) / 2;
  double lambda = (GeographicalLongitude1 - GeographicalLongitude2) / 2;
  double sinG = sin(G);
  double cosG = cos(G);
  double cosF = cos(F);
  double sinF = sin(F);
  double sinLambda = sin(lambda);
  double cosLambda = cos(lambda);
  double S = (sinG*sinG*cosLambda*cosLambda) + (cosF*cosF*sinLambda*sinLambda);
  double C = (cosG*cosG*cosLambda*cosLambda) + (sinF*sinF*sinLambda*sinLambda);
  double w = atan(sqrt(S/C));
  double R = sqrt(S*C)/w;
  double D = 2*w*6378.14;
  double Hprime = (3*R - 1) / (2*C);
  double Hprime2 = (3*R + 1) / (2*S);
  double f = 0.0033528131778969144060323814696721;

  return D * (1 + (f*Hprime*sinF*sinF*cosG*cosG) - (f*Hprime2*cosF*cosF*sinG*sinG));
}

/*
Module : AADIAMETERS.CPP
Purpose: Implementation for the algorithms for the semi diameters of the Sun, Moon, Planets and Asteroids
Created: PJN / 15-01-2004
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


//////////////////// Includes /////////////////////////////////////////////////

#include "stdafx.h"
#include "AADiameters.h"
#include "AACoordinateTransformation.h"
#include "AAGlobe.h"
#include <cmath>
using namespace std;


//////////////////// Implementation ///////////////////////////////////////////

double CAADiameters::SunSemidiameterA(double Delta)
{
  return 959.63/Delta;
}

double CAADiameters::MercurySemidiameterA(double Delta)
{
  return 3.34/Delta;
}

double CAADiameters::VenusSemidiameterA(double Delta)
{
  return 8.41/Delta;
}

double CAADiameters::MarsSemidiameterA(double Delta)
{
  return 4.68/Delta;
}

double CAADiameters::JupiterEquatorialSemidiameterA(double Delta)
{
  return 98.47/Delta;
}

double CAADiameters::JupiterPolarSemidiameterA(double Delta)
{
  return 91.91/Delta;
}

double CAADiameters::SaturnEquatorialSemidiameterA(double Delta)
{
  return 83.33/Delta;
}

double CAADiameters::SaturnPolarSemidiameterA(double Delta)
{
  return 74.57/Delta;
}

double CAADiameters::ApparentSaturnPolarSemidiameterA(double Delta, double B)
{
  double cosB = cos(CAACoordinateTransformation::DegreesToRadians(B));
  return SaturnPolarSemidiameterA(Delta)*sqrt(1 - 0.199197*cosB*cosB);
}

double CAADiameters::UranusSemidiameterA(double Delta)
{
  return 34.28/Delta;
}

double CAADiameters::NeptuneSemidiameterA(double Delta)
{
  return 36.56/Delta;
}

double CAADiameters::MercurySemidiameterB(double Delta)
{
  return 3.36/Delta;
}

double CAADiameters::VenusSemidiameterB(double Delta)
{
  return 8.34/Delta;
}

double CAADiameters::MarsSemidiameterB(double Delta)
{
  return 4.68/Delta;
}

double CAADiameters::JupiterEquatorialSemidiameterB(double Delta)
{
  return 98.44/Delta;
}

double CAADiameters::JupiterPolarSemidiameterB(double Delta)
{
  return 92.06/Delta;
}

double CAADiameters::SaturnEquatorialSemidiameterB(double Delta)
{
  return 82.73/Delta;
}

double CAADiameters::SaturnPolarSemidiameterB(double Delta)
{
  return 73.82/Delta;
}

double CAADiameters::ApparentSaturnPolarSemidiameterB(double Delta, double B)
{
  double cosB = cos(CAACoordinateTransformation::DegreesToRadians(B));
  return SaturnPolarSemidiameterB(Delta)*sqrt(1 - 0.203800*cosB*cosB);
}

double CAADiameters::UranusSemidiameterB(double Delta)
{
  return 35.02/Delta;
}

double CAADiameters::NeptuneSemidiameterB(double Delta)
{
  return 33.50/Delta;
}

double CAADiameters::PlutoSemidiameterB(double Delta)
{
  return 2.07/Delta;
}

double CAADiameters::GeocentricMoonSemidiameter(double Delta)
{
  return CAACoordinateTransformation::RadiansToDegrees(0.272481*6378.14/Delta)*3600;
}

double CAADiameters::TopocentricMoonSemidiameter(double DistanceDelta, double Delta, double H, double Latitude, double Height)
{
  //Convert to radians
  H = CAACoordinateTransformation::HoursToRadians(H);
  Delta = CAACoordinateTransformation::DegreesToRadians(Delta);

  double pi = asin(6378.14/DistanceDelta);
  double A = cos(Delta)*sin(H);
  double B = cos(Delta)*cos(H) - CAAGlobe::RhoCosThetaPrime(Latitude, Height)*sin(pi);
  double C = sin(Delta) - CAAGlobe::RhoSinThetaPrime(Latitude, Height)*sin(pi);
  double q = sqrt(A*A + B*B + C*C);

  double s = CAACoordinateTransformation::DegreesToRadians(GeocentricMoonSemidiameter(DistanceDelta)/3600);
  return CAACoordinateTransformation::RadiansToDegrees(asin(sin(s)/q))*3600;
}

double CAADiameters::AsteroidDiameter(double H, double A)
{
  double x = 3.12 - H/5 - 0.217147*log(A);
  return pow(10.0, x);
}

double CAADiameters::ApparentAsteroidDiameter(double Delta, double d)
{
  return 0.0013788*d/Delta;
}

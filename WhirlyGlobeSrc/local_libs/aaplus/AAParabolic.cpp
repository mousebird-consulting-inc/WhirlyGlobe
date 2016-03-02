/*
Module : AAPARABOLIC.CPP
Purpose: Implementation for the algorithms for a parabolic orbit
Created: PJN / 29-12-2003
History: PJN / 31-01-2005 1. Fixed a bug in CAAParabolic::Calculate where the JD value was being used incorrectly
                          in the loop. Thanks to Mika Heiskanen for reporting this problem.
         PJN / 16-03-2008 1. Fixed a bug in CAAParabolic::Calculate(double JD, 
                          const CAAParabolicObjectElements& elements) in the calculation of the 
                          heliocentric rectangular ecliptical, the heliocentric ecliptical latitude and 
                          the heliocentric ecliptical longitude coordinates. The code incorrectly used the 
                          value "omega" instead of "w" in its calculation of the value "u". Unfortunately 
                          there is no worked examples in Jean Meeus's book for these particular values, 
                          hence resulting in my coding errors. Thanks to Jay Borseth for reporting this bug.                 
         PJN / 08-09-2013 1. Fixed a bug in the calculation of HeliocentricEclipticLongitude and 
                          HeliocentricEclipticLatitude in CAAParabolic::Calculate. Thanks to Joe Novak for 
                          reporting this problem.

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


////////////////////////////// Includes ///////////////////////////////////////

#include "stdafx.h"
#include "AAParabolic.h"
#include "AACoordinateTransformation.h"
#include "AASun.h"
#include "AANutation.h"
#include "AAElliptical.h"
#include <cmath>
using namespace std;


////////////////////////////// Implementation /////////////////////////////////

double CAAParabolic::CalculateBarkers(double W)
{
  double S = W / 3; 
  bool bRecalc = true;
  while (bRecalc)
  {
    double S2 = S*S;
    double NextS = (2*S2*S + W) / (3 * (S2 + 1));

    //Prepare for the next loop around
    bRecalc = (fabs(NextS - S) > 0.000001);
    S = NextS;
  }

  return S;
}

CAAParabolicObjectDetails CAAParabolic::Calculate(double JD, const CAAParabolicObjectElements& elements)
{
  double Epsilon = CAANutation::MeanObliquityOfEcliptic(elements.JDEquinox);

  double JD0 = JD;

  //What will be the return value
  CAAParabolicObjectDetails details;

  Epsilon = CAACoordinateTransformation::DegreesToRadians(Epsilon);
  double omega = CAACoordinateTransformation::DegreesToRadians(elements.omega);
  double w = CAACoordinateTransformation::DegreesToRadians(elements.w);
  double i = CAACoordinateTransformation::DegreesToRadians(elements.i);

  double sinEpsilon = sin(Epsilon);
  double cosEpsilon = cos(Epsilon);
  double sinOmega = sin(omega);
  double cosOmega = cos(omega);
  double cosi = cos(i);
  double sini = sin(i);

  double F = cosOmega;
  double G = sinOmega * cosEpsilon;
  double H = sinOmega * sinEpsilon;
  double P = -sinOmega * cosi;
  double Q = cosOmega*cosi*cosEpsilon - sini*sinEpsilon;
  double R = cosOmega*cosi*sinEpsilon + sini*cosEpsilon;
  double a = sqrt(F*F + P*P);
  double b = sqrt(G*G + Q*Q);
  double c = sqrt(H*H + R*R);
  double A = atan2(F, P);
  double B = atan2(G, Q);
  double C = atan2(H, R);

  CAA3DCoordinate SunCoord = CAASun::EquatorialRectangularCoordinatesAnyEquinox(JD, elements.JDEquinox);

  for (int j=0; j<2; j++)
  {
    double W = 0.03649116245/(elements.q * sqrt(elements.q)) * (JD0 - elements.T);
    double s = CalculateBarkers(W);
    double v = 2*atan(s);
    double r = elements.q * (1 + s*s);
    double x = r * a * sin(A + w + v);
    double y = r * b * sin(B + w + v);
    double z = r * c * sin(C + w + v);

    if (j == 0)
    {
      details.HeliocentricRectangularEquatorial.X = x;
      details.HeliocentricRectangularEquatorial.Y = y;
      details.HeliocentricRectangularEquatorial.Z = z;

      //Calculate the heliocentric ecliptic coordinates also
      double u = w + v;
      double cosu = cos(u);
      double sinu = sin(u);

      details.HeliocentricRectangularEcliptical.X = r * (cosOmega*cosu - sinOmega*sinu*cosi);
      details.HeliocentricRectangularEcliptical.Y = r * (sinOmega*cosu + cosOmega*sinu*cosi);
      details.HeliocentricRectangularEcliptical.Z = r*sini*sinu;

      details.HeliocentricEclipticLongitude = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(atan2(details.HeliocentricRectangularEcliptical.Y, details.HeliocentricRectangularEcliptical.X))); 
      details.HeliocentricEclipticLatitude = CAACoordinateTransformation::RadiansToDegrees(asin(details.HeliocentricRectangularEcliptical.Z / r));
    }

    double psi = SunCoord.X + x;
    double nu = SunCoord.Y + y;
    double sigma = SunCoord.Z + z;

    double Alpha = atan2(nu, psi);
    Alpha = CAACoordinateTransformation::RadiansToDegrees(Alpha);
    double Delta = atan2(sigma, sqrt(psi*psi + nu*nu));
    Delta = CAACoordinateTransformation::RadiansToDegrees(Delta);
    double Distance = sqrt(psi*psi + nu*nu + sigma*sigma);

    if (j == 0)
    {
      details.TrueGeocentricRA = CAACoordinateTransformation::MapTo0To24Range(Alpha / 15);
      details.TrueGeocentricDeclination = Delta;
      details.TrueGeocentricDistance = Distance;
      details.TrueGeocentricLightTime = CAAElliptical::DistanceToLightTime(Distance);
    }
    else
    {
      details.AstrometricGeocenticRA = CAACoordinateTransformation::MapTo0To24Range(Alpha / 15);
      details.AstrometricGeocentricDeclination = Delta;
      details.AstrometricGeocentricDistance = Distance;
      details.AstrometricGeocentricLightTime = CAAElliptical::DistanceToLightTime(Distance);

      double RES = sqrt(SunCoord.X*SunCoord.X + SunCoord.Y*SunCoord.Y + SunCoord.Z*SunCoord.Z);

      details.Elongation = CAACoordinateTransformation::RadiansToDegrees(acos((RES*RES + Distance*Distance - r*r) / (2 * RES * Distance)));
      details.PhaseAngle = CAACoordinateTransformation::RadiansToDegrees(acos((r*r + Distance*Distance - RES*RES) / (2 * r * Distance)));
    }

    if (j == 0) //Prepare for the next loop around
      JD0 = JD - details.TrueGeocentricLightTime;
  }
    
  return details;
}

/*
Module : AAELLIPTICAL.CPP
Purpose: Implementation for the algorithms for an elliptical orbit
Created: PJN / 29-12-2003
History: PJN / 24-05-2004 1. Fixed a missing break statement in CAAElliptical::Calculate. Thanks to
                          Carsten A. Arnholm for reporting this bug. 
                          2. Also fixed an issue with the calculation of the apparent distance to 
                          the Sun.
         PJN / 31-12-2004 1. Fix for CAAElliptical::MinorPlanetMagnitude where the phase angle was
                          being incorrectly converted from Radians to Degress when it was already
                          in degrees. Thanks to Martin Burri for reporting this problem.
         PJN / 05-06-2006 1. Fixed a bug in CAAElliptical::Calculate(double JD, EllipticalObject object)
                          where the correction for nutation was incorrectly using the Mean obliquity of
                          the ecliptic instead of the true value. The results from the test program now 
                          agree much more closely with the example Meeus provides which is the position 
                          of Venus on 1992 Dec. 20 at 0h Dynamical Time. I've also checked the positions
                          against the JPL Horizons web site and the agreement is much better. Because the
                          True obliquity of the Ecliptic is defined as the mean obliquity of the ecliptic
                          plus the nutation in obliquity, it is relatively easy to determine the magnitude
                          of error this was causing. From the chapter on Nutation in the book, and 
                          specifically the table which gives the cosine coefficients for nutation in 
                          obliquity you can see that the absolute worst case error would be the sum of the 
                          absolute values of all of the coefficients and would have been c. 10 arc seconds 
                          of degree, which is not a small amount!. This value would be an absolute worst 
                          case and I would expect the average error value to be much much smaller 
                          (probably much less than an arc second). Anyway the bug has now been fixed. 
                          Thanks to Patrick Wong for pointing out this rather significant bug. 
         PJN / 10-11-2008 1. Fixed a bug in CAAElliptical::Calculate(double JD, 
                          const CAAEllipticalObjectElements& elements) in the calculation of the 
                          heliocentric rectangular ecliptical, the heliocentric ecliptical latitude and 
                          the heliocentric ecliptical longitude coordinates. The code incorrectly used the 
                          value "omega" instead of "w" in its calculation of the value "u". Unfortunately 
                          there is no worked examples in Jean Meeus's book for these particular values, 
                          hence resulting in my coding errors. Thanks to Carsten A. Arnholm for reporting 
                          this bug. 
         PJN / 10-05-2010 1. The CAAEllipticalObjectDetails::AstrometricGeocenticRA value is now known as
                          AstrometricGeocentricRA. Thanks to Scott Marley for reporting this spelling mistake

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
#include "AAElliptical.h"
#include "AAAberration.h"
#include "AACoordinateTransformation.h"
#include "AASun.h"
#include "AAMercury.h"
#include "AAVenus.h"
#include "AAEarth.h"
#include "AAMars.h"
#include "AAJupiter.h"
#include "AASaturn.h"
#include "AAUranus.h"
#include "AANeptune.h"
#include "AAPluto.h"
#include "AAFK5.h"
#include "AANutation.h"
#include "AAKepler.h"
#include <cmath>
#include <cassert>
using namespace std;


////////////////////////////// Implementation /////////////////////////////////

double CAAElliptical::DistanceToLightTime(double Distance)
{
  return Distance * 0.0057755183;
}

CAAEllipticalPlanetaryDetails CAAElliptical::Calculate(double JD, EllipticalObject object)
{
  //What will the the return value
  CAAEllipticalPlanetaryDetails details;

  double JD0 = JD;
  double L0 = 0;
  double B0 = 0;
  double R0 = 0;
  double cosB0 = 0;
  if (object != SUN)
  {
    L0 = CAAEarth::EclipticLongitude(JD0);
    B0 = CAAEarth::EclipticLatitude(JD0);
    R0 = CAAEarth::RadiusVector(JD0);
    L0 = CAACoordinateTransformation::DegreesToRadians(L0);
    B0 = CAACoordinateTransformation::DegreesToRadians(B0);
    cosB0 = cos(B0);
  }


  //Calculate the initial values
  double L = 0;
  double B = 0;
  double R = 0;
  switch (object)
  {
    case SUN:
    {
      L = CAASun::GeometricEclipticLongitude(JD0);
      B = CAASun::GeometricEclipticLatitude(JD0);
      R = CAAEarth::RadiusVector(JD0);
      break;
    }
    case MERCURY:
    {
      L = CAAMercury::EclipticLongitude(JD0);
      B = CAAMercury::EclipticLatitude(JD0);
      R = CAAMercury::RadiusVector(JD0);
      break;
    }
    case VENUS:
    {
      L = CAAVenus::EclipticLongitude(JD0);
      B = CAAVenus::EclipticLatitude(JD0);
      R = CAAVenus::RadiusVector(JD0);
      break;
    }
    case MARS:
    {
      L = CAAMars::EclipticLongitude(JD0);
      B = CAAMars::EclipticLatitude(JD0);
      R = CAAMars::RadiusVector(JD0);
      break;
    }
    case JUPITER:
    {
      L = CAAJupiter::EclipticLongitude(JD0);
      B = CAAJupiter::EclipticLatitude(JD0);
      R = CAAJupiter::RadiusVector(JD0);
      break;
    }
    case SATURN:
    {
      L = CAASaturn::EclipticLongitude(JD0);
      B = CAASaturn::EclipticLatitude(JD0);
      R = CAASaturn::RadiusVector(JD0);
      break;
    }
    case URANUS:
    {
      L = CAAUranus::EclipticLongitude(JD0);
      B = CAAUranus::EclipticLatitude(JD0);
      R = CAAUranus::RadiusVector(JD0);
      break;
    }
    case NEPTUNE:
    {
      L = CAANeptune::EclipticLongitude(JD0);
      B = CAANeptune::EclipticLatitude(JD0);
      R = CAANeptune::RadiusVector(JD0);
      break;
    }
    case PLUTO:
    {
      L = CAAPluto::EclipticLongitude(JD0);
      B = CAAPluto::EclipticLatitude(JD0);
      R = CAAPluto::RadiusVector(JD0);
      break;
    }
    default:
    {
      assert(false);
      break;
    }
  }

  bool bRecalc = true;
  bool bFirstRecalc = true;
  double LPrevious = 0;
  double BPrevious = 0;
  double RPrevious = 0;
  while (bRecalc)
  {
    switch (object)
    {
      case SUN:
      {
        L = CAASun::GeometricEclipticLongitude(JD0);
        B = CAASun::GeometricEclipticLatitude(JD0);
        R = CAAEarth::RadiusVector(JD0);
        break;
      }
      case MERCURY:
      {
        L = CAAMercury::EclipticLongitude(JD0);
        B = CAAMercury::EclipticLatitude(JD0);
        R = CAAMercury::RadiusVector(JD0);
        break;
      }
      case VENUS:
      {
        L = CAAVenus::EclipticLongitude(JD0);
        B = CAAVenus::EclipticLatitude(JD0);
        R = CAAVenus::RadiusVector(JD0);
        break;
      }
      case MARS:
      {
        L = CAAMars::EclipticLongitude(JD0);
        B = CAAMars::EclipticLatitude(JD0);
        R = CAAMars::RadiusVector(JD0);
        break;
      }
      case JUPITER:
      {
        L = CAAJupiter::EclipticLongitude(JD0);
        B = CAAJupiter::EclipticLatitude(JD0);
        R = CAAJupiter::RadiusVector(JD0);
        break;
      }
      case SATURN:
      {
        L = CAASaturn::EclipticLongitude(JD0);
        B = CAASaturn::EclipticLatitude(JD0);
        R = CAASaturn::RadiusVector(JD0);
        break;
      }
      case URANUS:
      {
        L = CAAUranus::EclipticLongitude(JD0);
        B = CAAUranus::EclipticLatitude(JD0);
        R = CAAUranus::RadiusVector(JD0);
        break;
      }
      case NEPTUNE:
      {
        L = CAANeptune::EclipticLongitude(JD0);
        B = CAANeptune::EclipticLatitude(JD0);
        R = CAANeptune::RadiusVector(JD0);
        break;
      }
      case PLUTO:
      {
        L = CAAPluto::EclipticLongitude(JD0);
        B = CAAPluto::EclipticLatitude(JD0);
        R = CAAPluto::RadiusVector(JD0);
        break;
      }
      default:
      {
        assert(false);
        break;
      }
    }

    if (!bFirstRecalc)
    {
      bRecalc = ((fabs(L - LPrevious) > 0.00001) || (fabs(B - BPrevious) > 0.00001) || (fabs(R - RPrevious) > 0.000001));
      LPrevious = L;
      BPrevious = B;
      RPrevious = R;
    }
    else
      bFirstRecalc = false;  

    //Calculate the new value
    if (bRecalc)
    {
      double distance;
      if (object != SUN)
      {
        double Lrad = CAACoordinateTransformation::DegreesToRadians(L);
        double Brad = CAACoordinateTransformation::DegreesToRadians(B);
        double cosB = cos(Brad);
        double cosL = cos(Lrad);
        double x = R * cosB * cosL - R0 * cosB0 * cos(L0);
        double y = R * cosB * sin(Lrad) - R0 * cosB0 * sin(L0);
        double z = R * sin(Brad) - R0 * sin(B0);
        distance = sqrt(x*x + y*y + z*z);
      }
      else
        distance = R; //Distance to the sun from the earth is in fact the radius vector

      //Prepare for the next loop around
      JD0 = JD - CAAElliptical::DistanceToLightTime(distance);
    }
  }

  double Lrad = CAACoordinateTransformation::DegreesToRadians(L);
  double Brad = CAACoordinateTransformation::DegreesToRadians(B);
  double cosB = cos(Brad);
  double cosL = cos(Lrad);
  double x = R * cosB * cosL - R0 * cosB0 * cos(L0);
  double y = R * cosB * sin(Lrad) - R0 * cosB0 * sin(L0);
  double z = R * sin(Brad) - R0 * sin(B0);
  double x2 = x*x;
  double y2 = y*y;

  details.ApparentGeocentricLatitude = CAACoordinateTransformation::RadiansToDegrees(atan2(z, sqrt(x2 + y2)));
  details.ApparentGeocentricDistance = sqrt(x2 + y2 + z*z);
  details.ApparentGeocentricLongitude = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(atan2(y, x)));
  details.ApparentLightTime = CAAElliptical::DistanceToLightTime(details.ApparentGeocentricDistance);

  //Adjust for Aberration
  CAA2DCoordinate Aberration = CAAAberration::EclipticAberration(details.ApparentGeocentricLongitude, details.ApparentGeocentricLatitude, JD);
  details.ApparentGeocentricLongitude += Aberration.X;
  details.ApparentGeocentricLatitude += Aberration.Y;

  //convert to the FK5 system
  double DeltaLong = CAAFK5::CorrectionInLongitude(details.ApparentGeocentricLongitude, details.ApparentGeocentricLatitude, JD);
  details.ApparentGeocentricLatitude += CAAFK5::CorrectionInLatitude(details.ApparentGeocentricLongitude, JD);
  details.ApparentGeocentricLongitude += DeltaLong;

  //Correct for nutation
  double NutationInLongitude = CAANutation::NutationInLongitude(JD);
  double Epsilon = CAANutation::TrueObliquityOfEcliptic(JD);
  details.ApparentGeocentricLongitude += CAACoordinateTransformation::DMSToDegrees(0, 0, NutationInLongitude);

  //Convert to RA and Dec
  CAA2DCoordinate ApparentEqu = CAACoordinateTransformation::Ecliptic2Equatorial(details.ApparentGeocentricLongitude, details.ApparentGeocentricLatitude, Epsilon);
  details.ApparentGeocentricRA = ApparentEqu.X;
  details.ApparentGeocentricDeclination = ApparentEqu.Y;

  return details;
}

double CAAElliptical::SemiMajorAxisFromPerihelionDistance(double q, double e)
{
  return q / (1 - e);
}

double CAAElliptical::MeanMotionFromSemiMajorAxis(double a)
{
  return 0.9856076686 / (a * sqrt(a));
}

CAAEllipticalObjectDetails CAAElliptical::Calculate(double JD, const CAAEllipticalObjectElements& elements)
{
  double Epsilon = CAANutation::MeanObliquityOfEcliptic(elements.JDEquinox);

  double JD0 = JD;

  //What will be the return value
  CAAEllipticalObjectDetails details;

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
  double n = CAAElliptical::MeanMotionFromSemiMajorAxis(elements.a);

  CAA3DCoordinate SunCoord = CAASun::EquatorialRectangularCoordinatesAnyEquinox(JD, elements.JDEquinox);

  for (int j=0; j<2; j++)
  {
    double M = n * (JD0 - elements.T);
    double E = CAAKepler::Calculate(M, elements.e);
    E = CAACoordinateTransformation::DegreesToRadians(E);
    double v = 2*atan(sqrt((1 + elements.e) / (1 - elements.e)) * tan(E/2));
    double r = elements.a * (1 - elements.e*cos(E));
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
      details.TrueGeocentricLightTime = DistanceToLightTime(Distance);
    }
    else
    {
      details.AstrometricGeocentricRA = CAACoordinateTransformation::MapTo0To24Range(Alpha / 15);
      details.AstrometricGeocentricDeclination = Delta;
      details.AstrometricGeocentricDistance = Distance;
      details.AstrometricGeocentricLightTime = DistanceToLightTime(Distance);

      double RES = sqrt(SunCoord.X*SunCoord.X + SunCoord.Y*SunCoord.Y + SunCoord.Z*SunCoord.Z);

      details.Elongation = acos((RES*RES + Distance*Distance - r*r) / (2 * RES * Distance));
      details.Elongation = CAACoordinateTransformation::RadiansToDegrees(details.Elongation);

      details.PhaseAngle = acos((r*r + Distance*Distance - RES*RES) / (2 * r * Distance));
      details.PhaseAngle = CAACoordinateTransformation::RadiansToDegrees(details.PhaseAngle);
    }

    if (j == 0) //Prepare for the next loop around
      JD0 = JD - details.TrueGeocentricLightTime;
  }
    
  return details;
}

double CAAElliptical::InstantaneousVelocity(double r, double a)
{
  return 42.1219 * sqrt((1/r) - (1/(2*a)));
}

double CAAElliptical::VelocityAtPerihelion(double e, double a)
{
  return 29.7847 / sqrt(a) * sqrt((1+e)/(1-e));
}

double CAAElliptical::VelocityAtAphelion(double e, double a)
{
  return 29.7847 / sqrt(a) * sqrt((1-e)/(1+e));
}

double CAAElliptical::LengthOfEllipse(double e, double a)
{
  double b = a * sqrt(1 - e*e);
  return CAACoordinateTransformation::PI() * (3 * (a+b) - sqrt((a+3*b)*(3*a + b)));
}

double CAAElliptical::CometMagnitude(double g, double delta, double k, double r)
{
  return g + 5*log10(delta) + k*log10(r);
}

double CAAElliptical::MinorPlanetMagnitude(double H, double delta, double G, double r, double PhaseAngle)
{
  //Convert from degrees to radians
  PhaseAngle = CAACoordinateTransformation::DegreesToRadians(PhaseAngle);

  double phi1 = exp(-3.33*pow(tan(PhaseAngle/2), 0.63));
  double phi2 = exp(-1.87*pow(tan(PhaseAngle/2), 1.22));

  return H + 5*log10(r*delta) - 2.5*log10((1 - G)*phi1 + G*phi2);
}

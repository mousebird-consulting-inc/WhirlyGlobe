/*
Module : AAPLANETPERIHELIONAPHELION.CPP
Purpose: Implementation for the algorithms which obtain the dates of Perihelion and Aphelion of the planets
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
#include "AAPlanetPerihelionAphelion.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


///////////////////////////////// Implementation //////////////////////////////

long CAAPlanetPerihelionAphelion::MercuryK(double Year)
{
  return static_cast<long>(4.15201*(Year - 2000.12));
}

double CAAPlanetPerihelionAphelion::MercuryPerihelion(long k)
{
  return 2451590.257 + 87.96934963*k;
}

double CAAPlanetPerihelionAphelion::MercuryAphelion(long k)
{
  double kdash = k + 0.5;
  return 2451590.257 + 87.96934963*kdash;
}

long CAAPlanetPerihelionAphelion::VenusK(double Year)
{
  return static_cast<long>(1.62549*(Year - 2000.53));
}

double CAAPlanetPerihelionAphelion::VenusPerihelion(long k)
{
  double kdash = k;
  double ksquared = kdash * kdash;
  return 2451738.233 + 224.7008188*kdash - 0.0000000327*ksquared;
}

double CAAPlanetPerihelionAphelion::VenusAphelion(long k)
{
  double kdash = k + 0.5;
  double ksquared = kdash * kdash;
  return 2451738.233 + 224.7008188*kdash - 0.0000000327*ksquared;
}

long CAAPlanetPerihelionAphelion::EarthK(double Year)
{
  return static_cast<long>(0.99997*(Year - 2000.01));
}

double CAAPlanetPerihelionAphelion::EarthPerihelion(long k, bool bBarycentric)
{
  double kdash = k;
  double ksquared = kdash * kdash;
  double JD = 2451547.507 + 365.2596358*kdash + 0.0000000156*ksquared;

  if (!bBarycentric)
  {
    //Apply the corrections
    double A1 = CAACoordinateTransformation::MapTo0To360Range(328.41 + 132.788585*k);
    A1 = CAACoordinateTransformation::DegreesToRadians(A1);
    double A2 = CAACoordinateTransformation::MapTo0To360Range(316.13 + 584.903153*k);
    A2 = CAACoordinateTransformation::DegreesToRadians(A2);
    double A3 = CAACoordinateTransformation::MapTo0To360Range(346.20 + 450.380738*k);
    A3 = CAACoordinateTransformation::DegreesToRadians(A3);
    double A4 = CAACoordinateTransformation::MapTo0To360Range(136.95 + 659.306737*k);
    A4 = CAACoordinateTransformation::DegreesToRadians(A4);
    double A5 = CAACoordinateTransformation::MapTo0To360Range(249.52 + 329.653368*k);
    A5 = CAACoordinateTransformation::DegreesToRadians(A5);

    JD += 1.278*sin(A1);
    JD -= 0.055*sin(A2);
    JD -= 0.091*sin(A3);
    JD -= 0.056*sin(A4);
    JD -= 0.045*sin(A5);
  }

  return JD;
}

double CAAPlanetPerihelionAphelion::EarthAphelion(long k, bool bBarycentric)
{
  double kdash = k + 0.5;
  double ksquared = kdash * kdash;
  double JD = 2451547.507 + 365.2596358*kdash + 0.0000000156*ksquared;

  if (!bBarycentric)
  {
    //Apply the corrections
    double A1 = CAACoordinateTransformation::MapTo0To360Range(328.41 + 132.788585*k);
    A1 = CAACoordinateTransformation::DegreesToRadians(A1);
    double A2 = CAACoordinateTransformation::MapTo0To360Range(316.13 + 584.903153*k);
    A2 = CAACoordinateTransformation::DegreesToRadians(A2);
    double A3 = CAACoordinateTransformation::MapTo0To360Range(346.20 + 450.380738*k);
    A3 = CAACoordinateTransformation::DegreesToRadians(A3);
    double A4 = CAACoordinateTransformation::MapTo0To360Range(136.95 + 659.306737*k);
    A4 = CAACoordinateTransformation::DegreesToRadians(A4);
    double A5 = CAACoordinateTransformation::MapTo0To360Range(249.52 + 329.653368*k);
    A5 = CAACoordinateTransformation::DegreesToRadians(A5);

    JD -= 1.352*sin(A1);
    JD += 0.061*sin(A2);
    JD += 0.062*sin(A3);
    JD += 0.029*sin(A4);
    JD += 0.031*sin(A5);
  }

  return JD;
}

long CAAPlanetPerihelionAphelion::MarsK(double Year)
{
  return static_cast<long>(0.53166*(Year - 2001.78));
}

double CAAPlanetPerihelionAphelion::MarsPerihelion(long k)
{
  double kdash = k;
  double ksquared = kdash * kdash;
  return 2452195.026 + 686.9957857*kdash - 0.0000001187*ksquared;
}

double CAAPlanetPerihelionAphelion::MarsAphelion(long k)
{
  double kdash = k + 0.5;
  double ksquared = kdash * kdash;
  return 2452195.026 + 686.9957857*kdash - 0.0000001187*ksquared;
}

long CAAPlanetPerihelionAphelion::JupiterK(double Year)
{
  return static_cast<long>(0.08430*(Year - 2011.20));
}

double CAAPlanetPerihelionAphelion::JupiterPerihelion(long k)
{
  double kdash = k;
  double ksquared = kdash * kdash;
  return 2455636.936 + 4332.897065*kdash + 0.0001367*ksquared;
}

double CAAPlanetPerihelionAphelion::JupiterAphelion(long k)
{
  double kdash = k + 0.5;
  double ksquared = kdash * kdash;
  return 2455636.936 + 4332.897065*kdash + 0.0001367*ksquared;
}

long CAAPlanetPerihelionAphelion::SaturnK(double Year)
{
  return static_cast<long>(0.03393*(Year - 2003.52));
}

double CAAPlanetPerihelionAphelion::SaturnPerihelion(long k)
{
  double kdash = k;
  double ksquared = kdash * kdash;
  return 2452830.12 + 10764.21676*kdash + 0.000827*ksquared;
}

double CAAPlanetPerihelionAphelion::SaturnAphelion(long k)
{
  double kdash = k + 0.5;
  double ksquared = kdash * kdash;
  return 2452830.12 + 10764.21676*kdash + 0.000827*ksquared;
}

long CAAPlanetPerihelionAphelion::UranusK(double Year)
{
  return static_cast<long>(0.01190*(Year - 2051.1));
}

double CAAPlanetPerihelionAphelion::UranusPerihelion(long k)
{
  double kdash = k;
  double ksquared = kdash * kdash;
  return 2470213.5 + 30694.8767*kdash - 0.00541*ksquared;
}

double CAAPlanetPerihelionAphelion::UranusAphelion(long k)
{
  double kdash = k + 0.5;
  double ksquared = kdash * kdash;
  return 2470213.5 + 30694.8767*kdash - 0.00541*ksquared;
}

long CAAPlanetPerihelionAphelion::NeptuneK(double Year)
{
  return static_cast<long>(0.00607*(Year - 2047.5));
}

double CAAPlanetPerihelionAphelion::NeptunePerihelion(long k)
{
  double kdash = k;
  double ksquared = kdash * kdash;
  return 2468895.1 + 60190.33*kdash + 0.03429*ksquared;
}

double CAAPlanetPerihelionAphelion::NeptuneAphelion(long k)
{
  double kdash = k + 0.5;
  double ksquared = kdash * kdash;
  return 2468895.1 + 60190.33*kdash + 0.03429*ksquared;
}

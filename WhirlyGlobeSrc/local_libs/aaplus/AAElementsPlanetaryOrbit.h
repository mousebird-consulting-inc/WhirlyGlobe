/*
Module : AAELEMENTSPLANETARYORBIT.H
Purpose: Implementation for the algorithms to calculate the elements of the planetary orbits
Created: PJN / 29-12-2003

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////// Macros / Defines //////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __AAELEMENTSPLANETARYORBIT_H__
#define __AAELEMENTSPLANETARYORBIT_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAElementsPlanetaryOrbit
{
public:
//Static methods
  static double MercuryMeanLongitude(double JD);
  static double MercurySemimajorAxis(double JD);
  static double MercuryEccentricity(double JD);
  static double MercuryInclination(double JD);
  static double MercuryLongitudeAscendingNode(double JD);
  static double MercuryLongitudePerihelion(double JD);

  static double VenusMeanLongitude(double JD);
  static double VenusSemimajorAxis(double JD);
  static double VenusEccentricity(double JD);
  static double VenusInclination(double JD);
  static double VenusLongitudeAscendingNode(double JD);
  static double VenusLongitudePerihelion(double JD);

  static double EarthMeanLongitude(double JD);
  static double EarthSemimajorAxis(double JD);
  static double EarthEccentricity(double JD);
  static double EarthInclination(double JD);
  static double EarthLongitudePerihelion(double JD);

  static double MarsMeanLongitude(double JD);
  static double MarsSemimajorAxis(double JD);
  static double MarsEccentricity(double JD);
  static double MarsInclination(double JD);
  static double MarsLongitudeAscendingNode(double JD);
  static double MarsLongitudePerihelion(double JD);

  static double JupiterMeanLongitude(double JD);
  static double JupiterSemimajorAxis(double JD);
  static double JupiterEccentricity(double JD);
  static double JupiterInclination(double JD);
  static double JupiterLongitudeAscendingNode(double JD);
  static double JupiterLongitudePerihelion(double JD);

  static double SaturnMeanLongitude(double JD);
  static double SaturnSemimajorAxis(double JD);
  static double SaturnEccentricity(double JD);
  static double SaturnInclination(double JD);
  static double SaturnLongitudeAscendingNode(double JD);
  static double SaturnLongitudePerihelion(double JD);

  static double UranusMeanLongitude(double JD);
  static double UranusSemimajorAxis(double JD);
  static double UranusEccentricity(double JD);
  static double UranusInclination(double JD);
  static double UranusLongitudeAscendingNode(double JD);
  static double UranusLongitudePerihelion(double JD);

  static double NeptuneMeanLongitude(double JD);
  static double NeptuneSemimajorAxis(double JD);
  static double NeptuneEccentricity(double JD);
  static double NeptuneInclination(double JD);
  static double NeptuneLongitudeAscendingNode(double JD);
  static double NeptuneLongitudePerihelion(double JD);

  static double MercuryMeanLongitudeJ2000(double JD);
  static double MercuryInclinationJ2000(double JD);
  static double MercuryLongitudeAscendingNodeJ2000(double JD);
  static double MercuryLongitudePerihelionJ2000(double JD);

  static double VenusMeanLongitudeJ2000(double JD);
  static double VenusInclinationJ2000(double JD);
  static double VenusLongitudeAscendingNodeJ2000(double JD);
  static double VenusLongitudePerihelionJ2000(double JD);

  static double EarthMeanLongitudeJ2000(double JD);
  static double EarthInclinationJ2000(double JD);
  static double EarthLongitudeAscendingNodeJ2000(double JD);
  static double EarthLongitudePerihelionJ2000(double JD);

  static double MarsMeanLongitudeJ2000(double JD);
  static double MarsInclinationJ2000(double JD);
  static double MarsLongitudeAscendingNodeJ2000(double JD);
  static double MarsLongitudePerihelionJ2000(double JD);

  static double JupiterMeanLongitudeJ2000(double JD);
  static double JupiterInclinationJ2000(double JD);
  static double JupiterLongitudeAscendingNodeJ2000(double JD);
  static double JupiterLongitudePerihelionJ2000(double JD);

  static double SaturnMeanLongitudeJ2000(double JD);
  static double SaturnInclinationJ2000(double JD);
  static double SaturnLongitudeAscendingNodeJ2000(double JD);
  static double SaturnLongitudePerihelionJ2000(double JD);

  static double UranusMeanLongitudeJ2000(double JD);
  static double UranusInclinationJ2000(double JD);
  static double UranusLongitudeAscendingNodeJ2000(double JD);
  static double UranusLongitudePerihelionJ2000(double JD);

  static double NeptuneMeanLongitudeJ2000(double JD);
  static double NeptuneInclinationJ2000(double JD);
  static double NeptuneLongitudeAscendingNodeJ2000(double JD);
  static double NeptuneLongitudePerihelionJ2000(double JD);
};

#endif //__AAELEMENTSPLANETARYORBIT_H__

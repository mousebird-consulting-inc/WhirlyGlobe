/*
Module : AAELEMENTSPLANETARYORBIT.CPP
Purpose: Implementation for the algorithms to calculate the elements of the planetary orbits
Created: PJN / 29-12-2003
History: PJN / 27-10-2012 1. Fixed a transcription bug in CAAElementsPlanetaryOrbit::SaturnLongitudePerihelion. The correct
                          coefficient for multiplying by T should have been 1.9637613 instead of 1.19637613. In the worked
                          example of 31.a from Meeus's book, he uses the date June 24 2065. With this bug now eliminated 
                          the value returned is now 94.34 degrees instead of 93.84 degrees for this date. Basically the 
                          incorrect coefficient would have returned a value too small by 0.77 degrees for every Julian 
                          century after the epoch J2000.0 = 2000 January 1.5 TD = JDE 2451545.0. Thanks to Sudhakar 
                          Gumparthi for reporting this bug.

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
#include "AAElementsPlanetaryOrbit.h"
#include "AACoordinateTransformation.h"


/////////////////////////// Implementation ////////////////////////////////////

double CAAElementsPlanetaryOrbit::MercuryMeanLongitude(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(252.250906 + 149474.0722491*T + 0.00030350*Tsquared + 0.000000018*Tcubed);
}

double CAAElementsPlanetaryOrbit::MercurySemimajorAxis(double /*JD*/)
{
  return 0.387098310;
}

double CAAElementsPlanetaryOrbit::MercuryEccentricity(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return 0.20563175 + 0.000020407*T - 0.0000000283*Tsquared - 0.00000000018*Tcubed;
}

double CAAElementsPlanetaryOrbit::MercuryInclination(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(7.004986 + 0.0018215*T - 0.00001810*Tsquared + 0.000000056*Tcubed);
}

double CAAElementsPlanetaryOrbit::MercuryLongitudeAscendingNode(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(48.330893 + 1.1861883*T + 0.00017542*Tsquared + 0.000000215*Tcubed);
}

double CAAElementsPlanetaryOrbit::MercuryLongitudePerihelion(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(77.456119 + 1.5564776*T + 0.00029544*Tsquared + 0.000000009*Tcubed);
}

double CAAElementsPlanetaryOrbit::VenusMeanLongitude(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(181.979801 + 58519.2130302*T + 0.00031014*Tsquared + 0.000000015*Tcubed);
}

double CAAElementsPlanetaryOrbit::VenusSemimajorAxis(double /*JD*/)
{
  return 0.723329820;
}

double CAAElementsPlanetaryOrbit::VenusEccentricity(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return 0.00677192 - 0.000047765*T + 0.0000000981*Tsquared + 0.00000000046*Tcubed;
}

double CAAElementsPlanetaryOrbit::VenusInclination(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(3.394662 + 0.0010037*T - 0.00000088*Tsquared - 0.000000007*Tcubed);
}

double CAAElementsPlanetaryOrbit::VenusLongitudeAscendingNode(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(76.679920 + 0.9011206*T + 0.00040618*Tsquared - 0.000000093*Tcubed);
}

double CAAElementsPlanetaryOrbit::VenusLongitudePerihelion(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(131.563703 + 1.4022288*T - 0.00107618*Tsquared - 0.000005678*Tcubed);
}

double CAAElementsPlanetaryOrbit::EarthMeanLongitude(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(100.466457 + 36000.7698278*T + 0.00030322*Tsquared + 0.000000020*Tcubed);
}

double CAAElementsPlanetaryOrbit::EarthSemimajorAxis(double /*JD*/)
{
  return 1.000001018;
}

double CAAElementsPlanetaryOrbit::EarthEccentricity(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return 0.01670863 - 0.000042037*T - 0.0000001267*Tsquared + 0.00000000014*Tcubed;
}

double CAAElementsPlanetaryOrbit::EarthInclination(double /*JD*/)
{
  return 0;
}

double CAAElementsPlanetaryOrbit::EarthLongitudePerihelion(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(102.937348 + 1.17195366*T + 0.00045688*Tsquared - 0.000000018*Tcubed);
}

double CAAElementsPlanetaryOrbit::MarsMeanLongitude(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(355.433000 + 19141.6964471*T + 0.00031052*Tsquared + 0.000000016*Tcubed);
}

double CAAElementsPlanetaryOrbit::MarsSemimajorAxis(double /*JD*/)
{
  return 1.523679342;
}

double CAAElementsPlanetaryOrbit::MarsEccentricity(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return 0.09340065 + 0.000090484*T - 0.0000000806*Tsquared - 0.00000000025*Tcubed;
}

double CAAElementsPlanetaryOrbit::MarsInclination(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(1.849726 - 0.0006011*T + 0.00001276*Tsquared - 0.000000007*Tcubed);
}

double CAAElementsPlanetaryOrbit::MarsLongitudeAscendingNode(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(49.588093 + 0.7720959*T + 0.00001557*Tsquared + 0.000002267*Tcubed);
}

double CAAElementsPlanetaryOrbit::MarsLongitudePerihelion(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(336.060234 + 1.8410449*T + 0.00013477*Tsquared + 0.000000536*Tcubed);
}

double CAAElementsPlanetaryOrbit::JupiterMeanLongitude(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(34.351519 + 3036.3027748*T + 0.00022330*Tsquared + 0.000000037*Tcubed);
}

double CAAElementsPlanetaryOrbit::JupiterSemimajorAxis(double JD)
{
  double T = (JD - 2451545.0) / 36525;

  return 5.202603209 + 0.0000001913*T;
}

double CAAElementsPlanetaryOrbit::JupiterEccentricity(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return 0.04849793 + 0.000163225*T - 0.0000004714*Tsquared - 0.00000000201*Tcubed;
}

double CAAElementsPlanetaryOrbit::JupiterInclination(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(1.303267 - 0.0054965*T + 0.00000466*Tsquared - 0.000000002*Tcubed);
}

double CAAElementsPlanetaryOrbit::JupiterLongitudeAscendingNode(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(100.464407 + 1.0209774*T + 0.00040315*Tsquared + 0.000000404*Tcubed);
}

double CAAElementsPlanetaryOrbit::JupiterLongitudePerihelion(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(14.331207 + 1.6126352*T + 0.00103042*Tsquared - 0.000004464*Tcubed);
}

double CAAElementsPlanetaryOrbit::SaturnMeanLongitude(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(50.077444 + 1223.5110686*T + 0.00051908*Tsquared - 0.000000030*Tcubed);
}

double CAAElementsPlanetaryOrbit::SaturnSemimajorAxis(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;

  return 9.554909192 - 0.0000021390*T + 0.000000004*Tsquared;
}

double CAAElementsPlanetaryOrbit::SaturnEccentricity(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return 0.05554814 - 0.0003446641*T - 0.0000006436*Tsquared + 0.00000000340*Tcubed;
}

double CAAElementsPlanetaryOrbit::SaturnInclination(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(2.488879 - 0.0037362*T - 0.00001519*Tsquared + 0.000000087*Tcubed);
}

double CAAElementsPlanetaryOrbit::SaturnLongitudeAscendingNode(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(113.665503 + 0.8770880*T - 0.00012176*Tsquared - 0.000002249*Tcubed);
}

double CAAElementsPlanetaryOrbit::SaturnLongitudePerihelion(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(93.057237 + 1.9637613*T + 0.00083753*Tsquared + 0.000004928*Tcubed);
}

double CAAElementsPlanetaryOrbit::UranusMeanLongitude(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(314.055005 + 429.8640561*T + 0.00030390*Tsquared + 0.000000026*Tcubed);
}

double CAAElementsPlanetaryOrbit::UranusSemimajorAxis(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;

  return 19.218446062 - 0.0000000372*T + 0.00000000098*Tsquared;
}

double CAAElementsPlanetaryOrbit::UranusEccentricity(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return 0.04638122 - 0.000027293*T + 0.0000000789*Tsquared + 0.00000000024*Tcubed;
}

double CAAElementsPlanetaryOrbit::UranusInclination(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(0.773197 + 0.0007744*T + 0.00003749*Tsquared - 0.000000092*Tcubed);
}

double CAAElementsPlanetaryOrbit::UranusLongitudeAscendingNode(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(74.005957 + 0.5211278*T + 0.00133947*Tsquared + 0.000018484*Tcubed);
}

double CAAElementsPlanetaryOrbit::UranusLongitudePerihelion(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(173.005291 + 1.4863790*T + 0.00021406*Tsquared + 0.000000434*Tcubed);
}

double CAAElementsPlanetaryOrbit::NeptuneMeanLongitude(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(304.348665 + 219.8833092*T + 0.00030882*Tsquared + 0.000000018*Tcubed);
}

double CAAElementsPlanetaryOrbit::NeptuneSemimajorAxis(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;

  return 30.110386869 - 0.0000001663*T + 0.00000000069*Tsquared;
}

double CAAElementsPlanetaryOrbit::NeptuneEccentricity(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tcubed = T*T*T;

  return 0.00945575 + 0.000006033*T - 0.00000000005*Tcubed;
}

double CAAElementsPlanetaryOrbit::NeptuneInclination(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(1.769953 - 0.0093082*T - 0.00000708*Tsquared + 0.000000027*Tcubed);
}

double CAAElementsPlanetaryOrbit::NeptuneLongitudeAscendingNode(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(131.784057 + 1.1022039*T + 0.00025952*Tsquared - 0.000000637*Tcubed);
}

double CAAElementsPlanetaryOrbit::NeptuneLongitudePerihelion(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(48.120276 + 1.4262957*T + 0.00038434*Tsquared + 0.000000020*Tcubed);
}

double CAAElementsPlanetaryOrbit::MercuryMeanLongitudeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(252.250906 + 149472.6746358*T - 0.00000536*Tsquared + 0.000000002*Tcubed);
}

double CAAElementsPlanetaryOrbit::MercuryInclinationJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(7.004986 - 0.0059516*T + 0.00000080*Tsquared + 0.000000043*Tcubed);
}

double CAAElementsPlanetaryOrbit::MercuryLongitudeAscendingNodeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(48.330893 - 0.1254227*T - 0.00008833*Tsquared - 0.000000200*Tcubed);
}

double CAAElementsPlanetaryOrbit::MercuryLongitudePerihelionJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(77.456119 + 0.1588643*T - 0.00001342*Tsquared - 0.000000007*Tcubed);
}

double CAAElementsPlanetaryOrbit::VenusMeanLongitudeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(181.979801 + 58517.8156760*T + 0.00000165*Tsquared - 0.000000002*Tcubed);
}

double CAAElementsPlanetaryOrbit::VenusInclinationJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(3.394662 - 0.0008568*T - 0.00003244*Tsquared + 0.000000009*Tcubed);
}

double CAAElementsPlanetaryOrbit::VenusLongitudeAscendingNodeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(76.679920 - 0.2780134*T - 0.00014257*Tsquared - 0.000000164*Tcubed);
}

double CAAElementsPlanetaryOrbit::VenusLongitudePerihelionJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(131.563703 + 0.0048746*T - 0.00138467*Tsquared - 0.000005695*Tcubed);
}

double CAAElementsPlanetaryOrbit::EarthMeanLongitudeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(100.466457 + 35999.3728565*T - 0.00000568*Tsquared - 0.000000001*Tcubed);
}

double CAAElementsPlanetaryOrbit::EarthInclinationJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return 0.0130548*T - 0.00000931*Tsquared - 0.000000034*Tcubed;
}

double CAAElementsPlanetaryOrbit::EarthLongitudeAscendingNodeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(174.873176 - 0.241098*T + 0.00004262*Tsquared + 0.000000001*Tcubed);
}

double CAAElementsPlanetaryOrbit::EarthLongitudePerihelionJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(102.937348 + 0.3225654*T + 0.00014799*Tsquared - 0.000000039*Tcubed);
}

double CAAElementsPlanetaryOrbit::MarsMeanLongitudeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(355.433000 + 19140.2993039*T + 0.00000262*Tsquared - 0.000000003*Tcubed);
}

double CAAElementsPlanetaryOrbit::MarsInclinationJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(1.849726 - 0.0081477*T - 0.00002255*Tsquared - 0.000000029*Tcubed);
}

double CAAElementsPlanetaryOrbit::MarsLongitudeAscendingNodeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(49.588093 - 0.2950250*T - 0.00064048*Tsquared - 0.000001964*Tcubed);
}

double CAAElementsPlanetaryOrbit::MarsLongitudePerihelionJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(336.060234 + 0.4439016*T - 0.00017313*Tsquared + 0.000000518*Tcubed);
}

double CAAElementsPlanetaryOrbit::JupiterMeanLongitudeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(34.351519 + 3034.9056606*T - 0.00008501*Tsquared + 0.000000016*Tcubed);
}

double CAAElementsPlanetaryOrbit::JupiterInclinationJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(1.303267 - 0.0019877*T + 0.00003320*Tsquared + 0.000000097*Tcubed);
}

double CAAElementsPlanetaryOrbit::JupiterLongitudeAscendingNodeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(100.464407 + 0.1767232*T + 0.00090700*Tsquared - 0.000007272*Tcubed);
}

double CAAElementsPlanetaryOrbit::JupiterLongitudePerihelionJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(14.331207 + 0.2155209*T + 0.00072211*Tsquared - 0.000004485*Tcubed);
}

double CAAElementsPlanetaryOrbit::SaturnMeanLongitudeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(50.077444 + 1222.1138488*T + 0.00021004*Tsquared - 0.000000046*Tcubed);
}

double CAAElementsPlanetaryOrbit::SaturnInclinationJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(2.488879 + 0.0025514*T - 0.00004906*Tsquared + 0.000000017*Tcubed);
}

double CAAElementsPlanetaryOrbit::SaturnLongitudeAscendingNodeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(113.665503 - 0.2566722*T - 0.00018399*Tsquared + 0.000000480*Tcubed);
}

double CAAElementsPlanetaryOrbit::SaturnLongitudePerihelionJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(93.057237 + 0.5665415*T + 0.00052850*Tsquared + 0.000004912*Tcubed);
}

double CAAElementsPlanetaryOrbit::UranusMeanLongitudeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(314.055005 + 428.4669983*T - 0.00000486*Tsquared + 0.000000006*Tcubed);
}

double CAAElementsPlanetaryOrbit::UranusInclinationJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(0.773197 - 0.0016869*T + 0.00000349*Tsquared + 0.000000016*Tcubed);
}

double CAAElementsPlanetaryOrbit::UranusLongitudeAscendingNodeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(74.005957 + 0.0741431*T + 0.00040539*Tsquared + 0.000000119*Tcubed);
}

double CAAElementsPlanetaryOrbit::UranusLongitudePerihelionJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(173.005291 + 0.0893212*T - 0.00009470*Tsquared + 0.000000414*Tcubed);
}

double CAAElementsPlanetaryOrbit::NeptuneMeanLongitudeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(304.348665 + 218.4862002*T + 0.00000059*Tsquared - 0.000000002*Tcubed);
}

double CAAElementsPlanetaryOrbit::NeptuneInclinationJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;

  return CAACoordinateTransformation::MapTo0To360Range(1.769953 + 0.0002256*T + 0.00000023*Tsquared);
}

double CAAElementsPlanetaryOrbit::NeptuneLongitudeAscendingNodeJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;
  double Tcubed = Tsquared*T;

  return CAACoordinateTransformation::MapTo0To360Range(131.784057 - 0.0061651*T - 0.00000219*Tsquared - 0.000000078*Tcubed);
}

double CAAElementsPlanetaryOrbit::NeptuneLongitudePerihelionJ2000(double JD)
{
  double T = (JD - 2451545.0) / 36525;
  double Tsquared = T*T;

  return CAACoordinateTransformation::MapTo0To360Range(48.120276 + 0.0291866*T + 0.00007610*Tsquared);
}

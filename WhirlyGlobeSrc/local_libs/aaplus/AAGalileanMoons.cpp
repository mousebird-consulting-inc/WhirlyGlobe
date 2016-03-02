/*
Module : AAGALILEANMOONS.CPP
Purpose: Implementation for the algorithms which obtain the positions of the 4 great moons of Jupiter
Created: PJN / 06-01-2004
History: PJN / 08-05-2011 1. Fixed a bug in CAAGalileanMoons::CalculateHelper where the periodic terms in longitude for
                          the four satellites (Sigma1 to Sigma4) were not being converted to radians prior to some
                          trigonometric calculations. Thanks to Thomas Meyer for reporting this bug.

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
#include "AAGalileanMoons.h"
#include "AAJupiter.h"
#include "AASun.h"
#include "AAEarth.h"
#include "AAElliptical.h"
#include "AACoordinateTransformation.h"
#include "AAElementsPlanetaryOrbit.h"
#include <cmath>
using namespace std;


//////////////////////////////// Implementation ///////////////////////////////

CAAGalileanMoonsDetails CAAGalileanMoons::CalculateHelper(double JD, double sunlongrad, double betarad, double R)
{
  //What will be the return value
  CAAGalileanMoonsDetails details;

  //Calculate the position of Jupiter decreased by the light travel time from Jupiter to the specified position
  double DELTA = 5;
  double PreviousLightTravelTime = 0;
  double LightTravelTime = CAAElliptical::DistanceToLightTime(DELTA);
  double x = 0;
  double y = 0;
  double z = 0;
  double l = 0;
  double lrad;
  double b = 0;
  double brad;
  double r;
  double JD1 = JD - LightTravelTime;
  bool bIterate = true;
  while (bIterate)
  {
    //Calculate the position of Jupiter
    l = CAAJupiter::EclipticLongitude(JD1);
    lrad = CAACoordinateTransformation::DegreesToRadians(l);
    b = CAAJupiter::EclipticLatitude(JD1);
    brad = CAACoordinateTransformation::DegreesToRadians(b);
    r = CAAJupiter::RadiusVector(JD1);

    x = r*cos(brad)*cos(lrad) + R*cos(sunlongrad);
    y = r*cos(brad)*sin(lrad) + R*sin(sunlongrad);
    z = r*sin(brad) + R*sin(betarad);
    DELTA = sqrt(x*x + y*y + z*z);
    LightTravelTime = CAAElliptical::DistanceToLightTime(DELTA);

    //Prepare for the next loop around
    bIterate = (fabs(LightTravelTime - PreviousLightTravelTime) > 2E-6); //2E-6 corresponds to 0.17 of a second
    if (bIterate)
    {
      JD1 = JD - LightTravelTime;
      PreviousLightTravelTime = LightTravelTime;
    }
  }

  //Calculate Jupiter's Longitude and Latitude
  double lambda0 = atan2(y, x);
  double beta0 = atan(z/sqrt(x*x + y*y));

  double t = JD - 2443000.5 - LightTravelTime;

  //Calculate the mean longitudes 
  double l1 = 106.07719 + 203.488955790*t;
  double l1rad = CAACoordinateTransformation::DegreesToRadians(l1);
  double l2 = 175.73161 + 101.374724735*t;
  double l2rad = CAACoordinateTransformation::DegreesToRadians(l2);
  double l3 = 120.55883 + 50.317609207*t;
  double l3rad = CAACoordinateTransformation::DegreesToRadians(l3);
  double l4 = 84.44459 + 21.571071177*t;
  double l4rad = CAACoordinateTransformation::DegreesToRadians(l4);

  //Calculate the perijoves
  double pi1 = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::MapTo0To360Range(97.0881 + 0.16138586*t));
  double pi2 = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::MapTo0To360Range(154.8663 + 0.04726307*t));
  double pi3 = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::MapTo0To360Range(188.1840 + 0.00712734*t));
  double pi4 = CAACoordinateTransformation::DegreesToRadians(CAACoordinateTransformation::MapTo0To360Range(335.2868 + 0.00184000*t));

  //Calculate the nodes on the equatorial plane of jupiter
  double w1 = 312.3346 - 0.13279386*t;
  double w1rad = CAACoordinateTransformation::DegreesToRadians(w1);
  double w2 = 100.4411 - 0.03263064*t;
  double w2rad = CAACoordinateTransformation::DegreesToRadians(w2);
  double w3 = 119.1942 - 0.00717703*t;
  double w3rad = CAACoordinateTransformation::DegreesToRadians(w3);
  double w4 = 322.6186 - 0.00175934*t;
  double w4rad = CAACoordinateTransformation::DegreesToRadians(w4);

  //Calculate the Principal inequality in the longitude of Jupiter
  double GAMMA = 0.33033*sin(CAACoordinateTransformation::DegreesToRadians(163.679 + 0.0010512*t)) +
                 0.03439*sin(CAACoordinateTransformation::DegreesToRadians(34.486 - 0.0161731*t));

  //Calculate the "phase of free libration"
  double philambda = CAACoordinateTransformation::DegreesToRadians(199.6766 + 0.17379190*t);

  //Calculate the longitude of the node of the equator of Jupiter on the ecliptic
  double psi = CAACoordinateTransformation::DegreesToRadians(316.5182 - 0.00000208*t);

  //Calculate the mean anomalies of Jupiter and Saturn
  double G = CAACoordinateTransformation::DegreesToRadians(30.23756 + 0.0830925701*t + GAMMA);
  double Gdash = CAACoordinateTransformation::DegreesToRadians(31.97853 + 0.0334597339*t);

  //Calculate the longitude of the perihelion of Jupiter
  double PI = CAACoordinateTransformation::DegreesToRadians(13.469942);

  //Calculate the periodic terms in the longitudes of the satellites
  double Sigma1 = 0.47259*sin(2*(l1rad - l2rad)) +
                  -0.03478*sin(pi3 - pi4) +
                  0.01081*sin(l2rad - 2*l3rad + pi3) +
                  0.00738*sin(philambda) +
                  0.00713*sin(l2rad - 2*l3rad + pi2) +
                  -0.00674*sin(pi1 + pi3 - 2*PI - 2*G) +
                  0.00666*sin(l2rad - 2*l3rad + pi4) +
                  0.00445*sin(l1rad - pi3) +
                  -0.00354*sin(l1rad - l2rad) +
                  -0.00317*sin(2*psi - 2*PI) +
                  0.00265*sin(l1rad - pi4) +
                  -0.00186*sin(G) +
                  0.00162*sin(pi2 - pi3) +
                  0.00158*sin(4*(l1rad - l2rad)) +
                  -0.00155*sin(l1rad - l3rad) +
                  -0.00138*sin(psi + w3rad - 2*PI - 2*G) +
                  -0.00115*sin(2*(l1rad - 2*l2rad + w2rad)) +
                  0.00089*sin(pi2 - pi4) +
                  0.00085*sin(l1rad + pi3 - 2*PI - 2*G) +
                  0.00083*sin(w2rad - w3rad) +
                  0.00053*sin(psi - w2rad);
  double Sigma1rad = CAACoordinateTransformation::DegreesToRadians(Sigma1);

  double Sigma2 = 1.06476*sin(2*(l2rad - l3rad)) +
                  0.04256*sin(l1rad - 2*l2rad + pi3) +
                  0.03581*sin(l2rad - pi3) +
                  0.02395*sin(l1rad - 2*l2rad + pi4) +
                  0.01984*sin(l2rad - pi4) +
                  -0.01778*sin(philambda) +
                  0.01654*sin(l2rad - pi2) +
                  0.01334*sin(l2rad - 2*l3rad + pi2) +
                  0.01294*sin(pi3 - pi4) +
                  -0.01142*sin(l2rad - l3rad) +
                  -0.01057*sin(G) +
                  -0.00775*sin(2*(psi - PI)) +
                  0.00524*sin(2*(l1rad - l2rad)) +
                  -0.00460*sin(l1rad - l3rad) +
                  0.00316*sin(psi - 2*G + w3rad - 2*PI) +
                  -0.00203*sin(pi1 + pi3 - 2*PI - 2*G) +
                  0.00146*sin(psi - w3rad) +
                  -0.00145*sin(2*G) +
                  0.00125*sin(psi - w4rad) +
                  -0.00115*sin(l1rad - 2*l3rad + pi3) +
                  -0.00094*sin(2*(l2rad - w2rad)) +
                  0.00086*sin(2*(l1rad - 2*l2rad + w2rad)) +
                  -0.00086*sin(5*Gdash - 2*G + CAACoordinateTransformation::DegreesToRadians(52.225)) +
                  -0.00078*sin(l2rad - l4rad) +
                  -0.00064*sin(3*l3rad - 7*l4rad + 4*pi4) +
                  0.00064*sin(pi1 - pi4) +
                  -0.00063*sin(l1rad - 2*l3rad + pi4) +
                  0.00058*sin(w3rad - w4rad) +
                  0.00056*sin(2*(psi - PI - G)) +
                  0.00056*sin(2*(l2rad - l4rad)) +
                  0.00055*sin(2*(l1rad - l3rad)) +
                  0.00052*sin(3*l3rad - 7*l4rad + pi3 + 3*pi4) +
                  -0.00043*sin(l1rad - pi3) +
                  0.00041*sin(5*(l2rad - l3rad)) +
                  0.00041*sin(pi4 - PI) +
                  0.00032*sin(w2rad - w3rad) +
                  0.00032*sin(2*(l3rad - G - PI));
  double Sigma2rad = CAACoordinateTransformation::DegreesToRadians(Sigma2);

  double Sigma3 = 0.16490*sin(l3rad - pi3) +
                  0.09081*sin(l3rad - pi4) +
                  -0.06907*sin(l2rad - l3rad) +
                  0.03784*sin(pi3 - pi4) +
                  0.01846*sin(2*(l3rad - l4rad)) +
                  -0.01340*sin(G) +
                  -0.01014*sin(2*(psi - PI)) +
                  0.00704*sin(l2rad - 2*l3rad + pi3) +
                  -0.00620*sin(l2rad - 2*l3rad + pi2) +
                  -0.00541*sin(l3rad - l4rad) +
                  0.00381*sin(l2rad - 2*l3rad + pi4) +
                  0.00235*sin(psi - w3rad) +
                  0.00198*sin(psi - w4rad) +
                  0.00176*sin(philambda) +
                  0.00130*sin(3*(l3rad - l4rad)) +
                  0.00125*sin(l1rad - l3rad) +
                  -0.00119*sin(5*Gdash - 2*G + CAACoordinateTransformation::DegreesToRadians(52.225)) +
                  0.00109*sin(l1rad - l2rad) +
                  -0.00100*sin(3*l3rad - 7*l4rad + 4*pi4) +
                  0.00091*sin(w3rad - w4rad) +
                  0.00080*sin(3*l3rad - 7*l4rad + pi3 + 3*pi4) +
                  -0.00075*sin(2*l2rad - 3*l3rad + pi3) +
                  0.00072*sin(pi1 + pi3 - 2*PI - 2*G) +
                  0.00069*sin(pi4 - PI) +
                  -0.00058*sin(2*l3rad - 3*l4rad + pi4) +
                  -0.00057*sin(l3rad - 2*l4rad + pi4) +
                  0.00056*sin(l3rad + pi3 - 2*PI - 2*G) +
                  -0.00052*sin(l2rad - 2*l3rad + pi1) +
                  -0.00050*sin(pi2 - pi3) +
                  0.00048*sin(l3rad - 2*l4rad + pi3) +
                  -0.00045*sin(2*l2rad - 3*l3rad + pi4) +
                  -0.00041*sin(pi2 - pi4) +
                  -0.00038*sin(2*G) +
                  -0.00037*sin(pi3 - pi4 + w3rad - w4rad) +
                  -0.00032*sin(3*l3rad - 7*l4rad + 2*pi3 + 2*pi4) +
                  0.00030*sin(4*(l3rad - l4rad)) +
                  0.00029*sin(l3rad + pi4 - 2*PI - 2*G) +
                  -0.00028*sin(w3rad + psi - 2*PI - 2*G) + 
                  0.00026*sin(l3rad - PI - G) +
                  0.00024*sin(l2rad - 3*l3rad + 2*l4rad) +
                  0.00021*sin(l3rad - PI - G) +
                  -0.00021*sin(l3rad - pi2) +
                  0.00017*sin(2*(l3rad - pi3));
  double Sigma3rad = CAACoordinateTransformation::DegreesToRadians(Sigma3);
                  
  double Sigma4 = 0.84287*sin(l4rad - pi4) +
                  0.03431*sin(pi4 - pi3) +
                  -0.03305*sin(2*(psi - PI)) +
                  -0.03211*sin(G) +
                  -0.01862*sin(l4rad - pi3) +
                  0.01186*sin(psi - w4rad) +
                  0.00623*sin(l4rad + pi4 - 2*G - 2*PI) +
                  0.00387*sin(2*(l4rad - pi4)) +
                  -0.00284*sin(5*Gdash - 2*G + CAACoordinateTransformation::DegreesToRadians(52.225)) +
                  -0.00234*sin(2*(psi - pi4)) +
                  -0.00223*sin(l3rad - l4rad) +
                  -0.00208*sin(l4rad - PI) +
                  0.00178*sin(psi + w4rad - 2*pi4) +
                  0.00134*sin(pi4 - PI) +
                  0.00125*sin(2*(l4rad - G - PI)) +
                  -0.00117*sin(2*G) +
                  -0.00112*sin(2*(l3rad - l4rad)) +
                  0.00107*sin(3*l3rad - 7*l4rad + 4*pi4) +
                  0.00102*sin(l4rad - G - PI) +
                  0.00096*sin(2*l4rad - psi - w4rad) +
                  0.00087*sin(2*(psi - w4rad)) +
                  -0.00085*sin(3*l3rad - 7*l4rad + pi3 + 3*pi4) +
                  0.00085*sin(l3rad - 2*l4rad + pi4) +
                  -0.00081*sin(2*(l4rad - psi)) +
                  0.00071*sin(l4rad + pi4 - 2*PI - 3*G) +
                  0.00061*sin(l1rad - l4rad) +
                  -0.00056*sin(psi - w3rad) +
                  -0.00054*sin(l3rad - 2*l4rad + pi3) +
                  0.00051*sin(l2rad - l4rad) +
                  0.00042*sin(2*(psi - G - PI)) +
                  0.00039*sin(2*(pi4 - w4rad)) +
                  0.00036*sin(psi + PI - pi4 - w4rad) +
                  0.00035*sin(2*Gdash - G + CAACoordinateTransformation::DegreesToRadians(188.37)) +
                  -0.00035*sin(l4rad - pi4 + 2*PI - 2*psi) +
                  -0.00032*sin(l4rad + pi4 - 2*PI - G) +
                  0.00030*sin(2*Gdash - 2*G + CAACoordinateTransformation::DegreesToRadians(149.15)) +
                  0.00029*sin(3*l3rad - 7*l4rad + 2*pi3 + 2*pi4) +
                  0.00028*sin(l4rad - pi4 + 2*psi - 2*PI) +
                  -0.00028*sin(2*(l4rad - w4rad)) +
                  -0.00027*sin(pi3 - pi4 + w3rad - w4rad) +
                  -0.00026*sin(5*Gdash - 3*G + CAACoordinateTransformation::DegreesToRadians(188.37)) +
                  0.00025*sin(w4rad - w3rad) +
                  -0.00025*sin(l2rad - 3*l3rad + 2*l4rad) +
                  -0.00023*sin(3*(l3rad - l4rad)) +
                  0.00021*sin(2*l4rad - 2*PI - 3*G) +
                  -0.00021*sin(2*l3rad - 3*l4rad + pi4) +
                  0.00019*sin(l4rad - pi4 - G) +
                  -0.00019*sin(2*l4rad - pi3 - pi4) +
                  -0.00018*sin(l4rad - pi4 + G) +
                  -0.00016*sin(l4rad + pi3 - 2*PI - 2*G);
  //There is no need to calculate a Sigma4rad as it is not used in any subsequent trignometric functions
                  
  details.Satellite1.MeanLongitude = CAACoordinateTransformation::MapTo0To360Range(l1);
  details.Satellite1.TrueLongitude = CAACoordinateTransformation::MapTo0To360Range(l1 + Sigma1);
  double L1 = CAACoordinateTransformation::DegreesToRadians(details.Satellite1.TrueLongitude);

  details.Satellite2.MeanLongitude = CAACoordinateTransformation::MapTo0To360Range(l2);
  details.Satellite2.TrueLongitude = CAACoordinateTransformation::MapTo0To360Range(l2 + Sigma2);
  double L2 = CAACoordinateTransformation::DegreesToRadians(details.Satellite2.TrueLongitude);

  details.Satellite3.MeanLongitude = CAACoordinateTransformation::MapTo0To360Range(l3);
  details.Satellite3.TrueLongitude = CAACoordinateTransformation::MapTo0To360Range(l3 + Sigma3);
  double L3 = CAACoordinateTransformation::DegreesToRadians(details.Satellite3.TrueLongitude);

  details.Satellite4.MeanLongitude = CAACoordinateTransformation::MapTo0To360Range(l4);
  details.Satellite4.TrueLongitude = CAACoordinateTransformation::MapTo0To360Range(l4 + Sigma4);
  double L4 = CAACoordinateTransformation::DegreesToRadians(details.Satellite4.TrueLongitude);

  //Calculate the periodic terms in the latitudes of the satellites
  double B1 = atan(0.0006393*sin(L1 - w1rad) +
                   0.0001825*sin(L1 - w2rad) +
                   0.0000329*sin(L1 - w3rad) +
                   -0.0000311*sin(L1 - psi) +
                   0.0000093*sin(L1 - w4rad) +
                   0.0000075*sin(3*L1 - 4*l2rad - 1.9927*Sigma1rad + w2rad) +
                   0.0000046*sin(L1 + psi - 2*PI - 2*G));
  details.Satellite1.EquatorialLatitude = CAACoordinateTransformation::RadiansToDegrees(B1);

  double B2 = atan(0.0081004*sin(L2 - w2rad) +
                   0.0004512*sin(L2 - w3rad) +
                   -0.0003284*sin(L2 - psi) +
                   0.0001160*sin(L2 - w4rad) + 
                   0.0000272*sin(l1rad - 2*l3rad + 1.0146*Sigma2rad + w2rad) +
                   -0.0000144*sin(L2 - w1rad) +
                   0.0000143*sin(L2 + psi - 2*PI - 2*G) +
                   0.0000035*sin(L2 - psi + G) +
                   -0.0000028*sin(l1rad - 2*l3rad + 1.0146*Sigma2rad + w3rad));
  details.Satellite2.EquatorialLatitude = CAACoordinateTransformation::RadiansToDegrees(B2);
  
  double B3 = atan(0.0032402*sin(L3 - w3rad) +
                   -0.0016911*sin(L3 - psi) +
                   0.0006847*sin(L3 - w4rad) +
                   -0.0002797*sin(L3 - w2rad) +
                   0.0000321*sin(L3 + psi - 2*PI - 2*G) +
                   0.0000051*sin(L3 - psi + G) +
                   -0.0000045*sin(L3 - psi - G) +
                   -0.0000045*sin(L3 + psi - 2*PI) +
                   0.0000037*sin(L3 + psi - 2*PI - 3*G) +
                   0.0000030*sin(2*l2rad - 3*L3 + 4.03*Sigma3rad + w2rad) +
                   -0.0000021*sin(2*l2rad - 3*L3 + 4.03*Sigma3rad + w3rad));
  details.Satellite3.EquatorialLatitude = CAACoordinateTransformation::RadiansToDegrees(B3);

  double B4 = atan(-0.0076579*sin(L4 - psi) +
                   0.0044134*sin(L4 - w4rad) +
                   -0.0005112*sin(L4 - w3rad) +
                   0.0000773*sin(L4 + psi - 2*PI - 2*G) +
                   0.0000104*sin(L4 - psi + G) +
                   -0.0000102*sin(L4 - psi - G) +
                   0.0000088*sin(L4 + psi - 2*PI - 3*G) +
                   -0.0000038*sin(L4 + psi - 2*PI - G));
  details.Satellite4.EquatorialLatitude = CAACoordinateTransformation::RadiansToDegrees(B4);

  //Calculate the periodic terms for the radius vector
  details.Satellite1.r = 5.90569 * (1 + (-0.0041339*cos(2*(l1rad - l2rad)) +
                                         -0.0000387*cos(l1rad - pi3) +
                                         -0.0000214*cos(l1rad - pi4) +
                                         0.0000170*cos(l1rad - l2rad) +
                                         -0.0000131*cos(4*(l1rad - l2rad)) +
                                         0.0000106*cos(l1rad - l3rad) +
                                         -0.0000066*cos(l1rad + pi3 - 2*PI - 2*G)));

  details.Satellite2.r = 9.39657 * (1 + (0.0093848*cos(l1rad - l2rad) +
                                         -0.0003116*cos(l2rad - pi3) +
                                         -0.0001744*cos(l2rad - pi4) +
                                         -0.0001442*cos(l2rad - pi2) +
                                         0.0000553*cos(l2rad - l3rad) +
                                         0.0000523*cos(l1rad - l3rad) +
                                         -0.0000290*cos(2*(l1rad - l2rad)) +
                                         0.0000164*cos(2*(l2rad - w2rad)) +
                                         0.0000107*cos(l1rad - 2*l3rad + pi3) +
                                         -0.0000102*cos(l2rad - pi1) +
                                         -0.0000091*cos(2*(l1rad - l3rad))));

  details.Satellite3.r = 14.98832 * (1 + (-0.0014388*cos(l3rad - pi3) +
                                          -0.0007919*cos(l3rad - pi4) +
                                          0.0006342*cos(l2rad - l3rad) +
                                          -0.0001761*cos(2*(l3rad - l4rad)) +
                                          0.0000294*cos(l3rad - l4rad) +
                                          -0.0000156*cos(3*(l3rad - l4rad)) +
                                          0.0000156*cos(l1rad - l3rad) +
                                          -0.0000153*cos(l1rad - l2rad) +
                                          0.0000070*cos(2*l2rad - 3*l3rad + pi3) +
                                          -0.0000051*cos(l3rad + pi3 - 2*PI - 2*G)));

  details.Satellite4.r = 26.36273 * (1 + (-0.0073546*cos(l4rad - pi4) +
                                          0.0001621*cos(l4rad - pi3) +
                                          0.0000974*cos(l3rad - l4rad) +
                                          -0.0000543*cos(l4rad + pi4 - 2*PI - 2*G) +
                                          -0.0000271*cos(2*(l4rad - pi4)) +
                                          0.0000182*cos(l4rad - PI) +
                                          0.0000177*cos(2*(l3rad - l4rad)) +
                                          -0.0000167*cos(2*l4rad - psi - w4rad) +
                                          0.0000167*cos(psi - w4rad) +
                                          -0.0000155*cos(2*(l4rad - PI - G)) +
                                          0.0000142*cos(2*(l4rad - psi)) +
                                          0.0000105*cos(l1rad - l4rad) +
                                          0.0000092*cos(l2rad - l4rad) +
                                          -0.0000089*cos(l4rad - PI - G) +
                                          -0.0000062*cos(l4rad + pi4 - 2*PI - 3*G) +
                                          0.0000048*cos(2*(l4rad - w4rad))));
  
  //Calculate T0
  double T0 = (JD - 2433282.423)/36525;

  //Calculate the precession in longitude from Epoch B1950 to the date
  double P = CAACoordinateTransformation::DegreesToRadians(1.3966626*T0 + 0.0003088*T0*T0);

  //Add it to L1 - L4 and psi
  L1 += P;
  details.Satellite1.TropicalLongitude = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(L1));
  L2 += P;
  details.Satellite2.TropicalLongitude = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(L2));
  L3 += P;
  details.Satellite3.TropicalLongitude = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(L3));
  L4 += P;
  details.Satellite4.TropicalLongitude = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(L4));
  psi += P;

  //Calculate the inclination of Jupiter's axis of rotation on the orbital plane
  double T = (JD - 2415020.5)/36525;
  double I = 3.120262 + 0.0006*T;
  double Irad = CAACoordinateTransformation::DegreesToRadians(I);

  double X1 = details.Satellite1.r*cos(L1 - psi)*cos(B1);
  double X2 = details.Satellite2.r*cos(L2 - psi)*cos(B2);
  double X3 = details.Satellite3.r*cos(L3 - psi)*cos(B3);
  double X4 = details.Satellite4.r*cos(L4 - psi)*cos(B4);
  double X5 = 0;

  double Y1 = details.Satellite1.r*sin(L1 - psi)*cos(B1);
  double Y2 = details.Satellite2.r*sin(L2 - psi)*cos(B2);
  double Y3 = details.Satellite3.r*sin(L3 - psi)*cos(B3);
  double Y4 = details.Satellite4.r*sin(L4 - psi)*cos(B4);
  double Y5 = 0;

  double Z1 = details.Satellite1.r*sin(B1);
  double Z2 = details.Satellite2.r*sin(B2);
  double Z3 = details.Satellite3.r*sin(B3);
  double Z4 = details.Satellite4.r*sin(B4);
  double Z5 = 1;

  //Now do the rotations, first for the ficticious 5th satellite, so that we can calculate D
  double omega = CAACoordinateTransformation::DegreesToRadians(CAAElementsPlanetaryOrbit::JupiterLongitudeAscendingNode(JD));
  double i = CAACoordinateTransformation::DegreesToRadians(CAAElementsPlanetaryOrbit::JupiterInclination(JD));
  double A6;
  double B6;
  double C6;
  Rotations(X5, Y5, Z5, Irad, psi, i, omega, lambda0, beta0, A6, B6, C6);
  double D = atan2(A6, C6);

  //Now calculate the values for satellite 1
  Rotations(X1, Y1, Z1, Irad, psi, i, omega, lambda0, beta0, A6, B6, C6);
  details.Satellite1.TrueRectangularCoordinates.X = A6*cos(D) - C6*sin(D);
  details.Satellite1.TrueRectangularCoordinates.Y = A6*sin(D) + C6*cos(D);
  details.Satellite1.TrueRectangularCoordinates.Z = B6;

  //Now calculate the values for satellite 2
  Rotations(X2, Y2, Z2, Irad, psi, i, omega, lambda0, beta0, A6, B6, C6);
  details.Satellite2.TrueRectangularCoordinates.X = A6*cos(D) - C6*sin(D);
  details.Satellite2.TrueRectangularCoordinates.Y = A6*sin(D) + C6*cos(D);
  details.Satellite2.TrueRectangularCoordinates.Z = B6;

  //Now calculate the values for satellite 3
  Rotations(X3, Y3, Z3, Irad, psi, i, omega, lambda0, beta0, A6, B6, C6);
  details.Satellite3.TrueRectangularCoordinates.X = A6*cos(D) - C6*sin(D);
  details.Satellite3.TrueRectangularCoordinates.Y = A6*sin(D) + C6*cos(D);
  details.Satellite3.TrueRectangularCoordinates.Z = B6;

  //And finally for satellite 4
  Rotations(X4, Y4, Z4, Irad, psi, i, omega, lambda0, beta0, A6, B6, C6);
  details.Satellite4.TrueRectangularCoordinates.X = A6*cos(D) - C6*sin(D);
  details.Satellite4.TrueRectangularCoordinates.Y = A6*sin(D) + C6*cos(D);
  details.Satellite4.TrueRectangularCoordinates.Z = B6;

  //apply the differential light-time correction
  details.Satellite1.ApparentRectangularCoordinates.X = details.Satellite1.TrueRectangularCoordinates.X + fabs(details.Satellite1.TrueRectangularCoordinates.Z)/17295*sqrt(1 - (details.Satellite1.TrueRectangularCoordinates.X/details.Satellite1.r)*(details.Satellite1.TrueRectangularCoordinates.X/details.Satellite1.r));
  details.Satellite1.ApparentRectangularCoordinates.Y = details.Satellite1.TrueRectangularCoordinates.Y;
  details.Satellite1.ApparentRectangularCoordinates.Z = details.Satellite1.TrueRectangularCoordinates.Z;

  details.Satellite2.ApparentRectangularCoordinates.X = details.Satellite2.TrueRectangularCoordinates.X + fabs(details.Satellite2.TrueRectangularCoordinates.Z)/21819*sqrt(1 - (details.Satellite2.TrueRectangularCoordinates.X/details.Satellite2.r)*(details.Satellite2.TrueRectangularCoordinates.X/details.Satellite2.r));
  details.Satellite2.ApparentRectangularCoordinates.Y = details.Satellite2.TrueRectangularCoordinates.Y;
  details.Satellite2.ApparentRectangularCoordinates.Z = details.Satellite2.TrueRectangularCoordinates.Z;

  details.Satellite3.ApparentRectangularCoordinates.X = details.Satellite3.TrueRectangularCoordinates.X + fabs(details.Satellite3.TrueRectangularCoordinates.Z)/27558*sqrt(1 - (details.Satellite3.TrueRectangularCoordinates.X/details.Satellite3.r)*(details.Satellite3.TrueRectangularCoordinates.X/details.Satellite3.r));
  details.Satellite3.ApparentRectangularCoordinates.Y = details.Satellite3.TrueRectangularCoordinates.Y;
  details.Satellite3.ApparentRectangularCoordinates.Z = details.Satellite3.TrueRectangularCoordinates.Z;

  details.Satellite4.ApparentRectangularCoordinates.X = details.Satellite4.TrueRectangularCoordinates.X + fabs(details.Satellite4.TrueRectangularCoordinates.Z)/36548*sqrt(1 - (details.Satellite4.TrueRectangularCoordinates.X/details.Satellite4.r)*(details.Satellite4.TrueRectangularCoordinates.X/details.Satellite4.r));
  details.Satellite4.ApparentRectangularCoordinates.Y = details.Satellite4.TrueRectangularCoordinates.Y;
  details.Satellite4.ApparentRectangularCoordinates.Z = details.Satellite4.TrueRectangularCoordinates.Z;

  //apply the perspective effect correction
  double W = DELTA/(DELTA + details.Satellite1.TrueRectangularCoordinates.Z/2095);
  details.Satellite1.ApparentRectangularCoordinates.X *= W;
  details.Satellite1.ApparentRectangularCoordinates.Y *= W;

  W = DELTA/(DELTA + details.Satellite2.TrueRectangularCoordinates.Z/2095);
  details.Satellite2.ApparentRectangularCoordinates.X *= W;
  details.Satellite2.ApparentRectangularCoordinates.Y *= W;

  W = DELTA/(DELTA + details.Satellite3.TrueRectangularCoordinates.Z/2095);
  details.Satellite3.ApparentRectangularCoordinates.X *= W;
  details.Satellite3.ApparentRectangularCoordinates.Y *= W;

  W = DELTA/(DELTA + details.Satellite4.TrueRectangularCoordinates.Z/2095);
  details.Satellite4.ApparentRectangularCoordinates.X *= W;
  details.Satellite4.ApparentRectangularCoordinates.Y *= W;

  return details;
}

CAAGalileanMoonsDetails CAAGalileanMoons::Calculate(double JD)
{
  //Calculate the position of the Sun
  double sunlong = CAASun::GeometricEclipticLongitude(JD);
  double sunlongrad = CAACoordinateTransformation::DegreesToRadians(sunlong);
  double beta = CAASun::GeometricEclipticLatitude(JD);
  double betarad = CAACoordinateTransformation::DegreesToRadians(beta);
  double R = CAAEarth::RadiusVector(JD);

  //Calculate the the light travel time from Jupiter to the Earth
  double DELTA = 5;
  double PreviousEarthLightTravelTime = 0;
  double EarthLightTravelTime = CAAElliptical::DistanceToLightTime(DELTA);
  double JD1 = JD - EarthLightTravelTime;
  bool bIterate = true;
  double x;
  double y;
  double z;
  while (bIterate)
  {
    //Calculate the position of Jupiter
    double l = CAAJupiter::EclipticLongitude(JD1);
    double lrad = CAACoordinateTransformation::DegreesToRadians(l);
    double b = CAAJupiter::EclipticLatitude(JD1);
    double brad = CAACoordinateTransformation::DegreesToRadians(b);
    double r = CAAJupiter::RadiusVector(JD1);

    x = r*cos(brad)*cos(lrad) + R*cos(sunlongrad);
    y = r*cos(brad)*sin(lrad) + R*sin(sunlongrad);
    z = r*sin(brad) + R*sin(betarad);
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

  //Calculate the details as seen from the earth
  CAAGalileanMoonsDetails details1 = CalculateHelper(JD, sunlongrad, betarad, R);
  FillInPhenomenaDetails(details1.Satellite1);
  FillInPhenomenaDetails(details1.Satellite2);
  FillInPhenomenaDetails(details1.Satellite3);
  FillInPhenomenaDetails(details1.Satellite4);

  //Calculate the the light travel time from Jupiter to the Sun
  JD1 = JD - EarthLightTravelTime;
  double l = CAAJupiter::EclipticLongitude(JD1);
  double lrad = CAACoordinateTransformation::DegreesToRadians(l);
  double b = CAAJupiter::EclipticLatitude(JD1);
  double brad = CAACoordinateTransformation::DegreesToRadians(b);
  double r = CAAJupiter::RadiusVector(JD1);
  x = r*cos(brad)*cos(lrad);
  y = r*cos(brad)*sin(lrad);
  z = r*sin(brad);
  DELTA = sqrt(x*x + y*y + z*z);
  double SunLightTravelTime = CAAElliptical::DistanceToLightTime(DELTA);

  //Calculate the details as seen from the Sun
  CAAGalileanMoonsDetails details2 = CalculateHelper(JD + SunLightTravelTime - EarthLightTravelTime, sunlongrad, betarad, 0);
  FillInPhenomenaDetails(details2.Satellite1);
  FillInPhenomenaDetails(details2.Satellite2);
  FillInPhenomenaDetails(details2.Satellite3);
  FillInPhenomenaDetails(details2.Satellite4);

  //Finally transfer the required values from details2 to details1
  details1.Satellite1.bInEclipse = details2.Satellite1.bInOccultation;
  details1.Satellite2.bInEclipse = details2.Satellite2.bInOccultation;
  details1.Satellite3.bInEclipse = details2.Satellite3.bInOccultation;
  details1.Satellite4.bInEclipse = details2.Satellite4.bInOccultation;
  details1.Satellite1.bInShadowTransit = details2.Satellite1.bInTransit;
  details1.Satellite2.bInShadowTransit = details2.Satellite2.bInTransit;
  details1.Satellite3.bInShadowTransit = details2.Satellite3.bInTransit;
  details1.Satellite4.bInShadowTransit = details2.Satellite4.bInTransit;

  return details1;
}

void CAAGalileanMoons::Rotations(double X, double Y, double Z, double I, double psi, double i, double omega, double lambda0, double beta0, double& A6, double& B6, double& C6)
{
  double phi = psi - omega;

  //Rotation towards Jupiter's orbital plane
  double A1 = X;
  double B1 = Y*cos(I) - Z*sin(I);
  double C1 = Y*sin(I) + Z*cos(I);
  
  //Rotation towards the ascending node of the orbit of jupiter
  double A2 = A1*cos(phi) - B1*sin(phi);
  double B2 = A1*sin(phi) + B1*cos(phi);
  double C2 = C1;

  //Rotation towards the plane of the ecliptic
  double A3 = A2;
  double B3 = B2*cos(i) - C2*sin(i);
  double C3 = B2*sin(i) + C2*cos(i);

  //Rotation towards the vernal equinox
  double A4 = A3*cos(omega) - B3*sin(omega);
  double B4 = A3*sin(omega) + B3*cos(omega);
  double C4 = C3;

  double A5 = A4*sin(lambda0) - B4*cos(lambda0);
  double B5 = A4*cos(lambda0) + B4*sin(lambda0);
  double C5 = C4;
 
  A6 = A5;
  B6 = C5*sin(beta0) + B5*cos(beta0);
  C6 = C5*cos(beta0) - B5*sin(beta0);
}

void CAAGalileanMoons::FillInPhenomenaDetails(CAAGalileanMoonDetail& detail)
{
  double Y1 = 1.071374 * detail.ApparentRectangularCoordinates.Y;

  double r = Y1*Y1 + detail.ApparentRectangularCoordinates.X*detail.ApparentRectangularCoordinates.X;

  if (r < 1)
  {
    if (detail.ApparentRectangularCoordinates.Z < 0)
    {
      //Satellite nearer to Earth than Jupiter, so it must be a transit not an occultation
      detail.bInTransit = true;
      detail.bInOccultation = false;
    }
    else
    {
      detail.bInTransit = false;
      detail.bInOccultation = true;
    }
  }
  else
  {
    detail.bInTransit = false;
    detail.bInOccultation = false;
  }
}

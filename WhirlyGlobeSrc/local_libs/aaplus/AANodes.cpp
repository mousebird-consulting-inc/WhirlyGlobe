/*
Module : AANODES.CPP
Purpose: Implementation for the algorithms which calculate passage thro the nodes
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


///////////////////////////// Includes ////////////////////////////////////////

#include "stdafx.h"
#include "AANodes.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


///////////////////////////// Implementation //////////////////////////////////

CAANodeObjectDetails CAANodes::PassageThroAscendingNode(const CAAEllipticalObjectElements& elements)
{
  double v = CAACoordinateTransformation::MapTo0To360Range(-elements.w);
  v = CAACoordinateTransformation::DegreesToRadians(v);
  double E = atan(sqrt((1 - elements.e) / (1 + elements.e)) * tan(v/2))*2;
  double M = E - elements.e*sin(E);
  M = CAACoordinateTransformation::RadiansToDegrees(M);
  double n = CAAElliptical::MeanMotionFromSemiMajorAxis(elements.a);

  CAANodeObjectDetails details;
  details.t = elements.T + M/n;
  details.radius = elements.a*(1 - elements.e*cos(E));

  return details;
}

CAANodeObjectDetails CAANodes::PassageThroDescendingNode(const CAAEllipticalObjectElements& elements)
{
  double v = CAACoordinateTransformation::MapTo0To360Range(180 - elements.w);
  v = CAACoordinateTransformation::DegreesToRadians(v);
  double E = atan(sqrt((1 - elements.e) / (1 + elements.e)) * tan(v/2))*2;
  double M = E - elements.e*sin(E);
  M = CAACoordinateTransformation::RadiansToDegrees(M);
  double n = CAAElliptical::MeanMotionFromSemiMajorAxis(elements.a);

  CAANodeObjectDetails details;
  details.t = elements.T + M/n;
  details.radius = elements.a*(1 - elements.e*cos(E));

  return details;
}

CAANodeObjectDetails CAANodes::PassageThroAscendingNode(const CAAParabolicObjectElements& elements)
{
  double v = CAACoordinateTransformation::MapTo0To360Range(-elements.w);
  v = CAACoordinateTransformation::DegreesToRadians(v);
  double s = tan(v / 2);
  double s2 = s*s;

  CAANodeObjectDetails details;
  details.t = elements.T + 27.403895*(s2*s + 3*s)*elements.q*sqrt(elements.q);
  details.radius = elements.q*(1 + s2);

  return details;
}

CAANodeObjectDetails CAANodes::PassageThroDescendingNode(const CAAParabolicObjectElements& elements)
{
  double v = CAACoordinateTransformation::MapTo0To360Range(180 - elements.w);
  v = CAACoordinateTransformation::DegreesToRadians(v);

  double s = tan(v / 2);
  double s2 = s*s;

  CAANodeObjectDetails details;
  details.t = elements.T + 27.403895*(s2*s + 3*s)*elements.q*sqrt(elements.q);
  details.radius = elements.q*(1 + s2);

  return details;
}

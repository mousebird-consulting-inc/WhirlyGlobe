/*
Module : AA+.H
Purpose: Main include file for AA+ framework
Created: PJN / 30-1-2005

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////// Macros / Defines //////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __AAPLUS_H__
#define __AAPLUS_H__

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////////////// Includes //////////////////////////////////////

#include "AA2DCoordinate.h"
#include "AA3DCoordinate.h"
#include "AAAberration.h"
#include "AAAngularSeparation.h"
#include "AABinaryStar.h"
#include "AACoordinateTransformation.h"
#include "AADate.h"
#include "AADiameters.h"
#include "AADynamicalTime.h"
#include "AAEarth.h"
#include "AAEaster.h"
#include "AAEclipses.h"
#include "AAEclipticalElements.h"
#include "AAElementsPlanetaryOrbit.h"
#include "AAElliptical.h"
#include "AAEquationOfTime.h"
#include "AAEquinoxesAndSolstices.h"
#include "AAFK5.h"
#include "AAGalileanMoons.h"
#include "AAGlobe.h"
#include "AAIlluminatedFraction.h"
#include "AAInterpolate.h"
#include "AAJewishCalendar.h"
#include "AAJupiter.h"
#include "AAKepler.h"
#include "AAMars.h"
#include "AAMercury.h"
#include "AAMoon.h"
#include "AAMoonIlluminatedFraction.h"
#include "AAMoonMaxDeclinations.h"
#include "AAMoonNodes.h"
#include "AAMoonPerigeeApogee.h"
#include "AAMoonPhases.h"
#include "AAMoslemCalendar.h"
#include "AANearParabolic.h"
#include "AANeptune.h"
#include "AANodes.h"
#include "AANutation.h"
#include "AAParabolic.h"
#include "AAParallactic.h"
#include "AAParallax.h"
#include "AAPhysicalJupiter.h"
#include "AAPhysicalMars.h"
#include "AAPhysicalMoon.h"
#include "AAPhysicalSun.h"
#include "AAPlanetaryPhenomena.h"
#include "AAPlanetPerihelionAphelion.h"
#include "AAPluto.h"
#include "AAPrecession.h"
#include "AARefraction.h"
#include "AARiseTransitSet.h"
#include "AASaturn.h"
#include "AASaturnMoons.h"
#include "AASaturnRings.h"
#include "AASidereal.h"
#include "AAStellarMagnitudes.h"
#include "AASun.h"
#include "AAUranus.h"
#include "AAVenus.h"

#endif //__AAPLUS_H__

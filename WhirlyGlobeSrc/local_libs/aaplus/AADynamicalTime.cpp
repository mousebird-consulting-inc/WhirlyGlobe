/*
Module : AADYNAMICALTIME.CPP
Purpose: Implementation for the algorithms which provides for conversion between Universal Time (both UT1 and UTC) and Terrestrial Time (TT) aka Terrestrial Dynamical Time (TDT) 
         aka Ephemeris Time (ET)
Created: PJN / 29-12-2003
History: PJN / 01-02-2005 1. Fixed a problem with the declaration of the variable "Index" in the function 
                          CAADynamicalTime::DeltaT. Thanks to Mika Heiskanen for reporting this problem.
         PJN / 26-01-2007 1. Update to fit in with new layout of CAADate class
         PJN / 28-01-2007 1. Further updates to fit in with new layout of CAADate class
         PJN / 08-05-2011 1. Fixed a compilation issue on GCC where size_t was undefined in various methods. Thanks to 
                          Carsten A. Arnholm and Andrew Hammond for reporting this bug.
         PJN / 01-05-2012 1. Updated CAADynamicalTime::DeltaT to use new polynomical expressions from Espenak & Meeus 2006.
                          References used: http://eclipse.gsfc.nasa.gov/SEcat5/deltatpoly.html and 
                          http://www.staff.science.uu.nl/~gent0113/deltat/deltat_old.htm (Espenak & Meeus 2006 section). 
                          Thanks to Murphy Chesney for prompting this update.
         PJN / 02-05-2012 1. To further improve the accuracy of the CAADynamicalTime::DeltaT method, the code now uses a 
                          lookup table between the dates of 1 February 1973 to 1 April 2012 (for observed values) and predicted 
                          values from April 2012 till April 2015. These values are as provided by IERS Rapid
                          Service/Prediction Center at http://maia.usno.navy.mil/ser7/deltat.data and 
                          http://maia.usno.navy.mil/ser7/deltat.preds. This lookup table will of course need to be kept up to 
                          date as IERS update this information. As currently coded there is a single discontinuity of c. one second 
                          in early April 2015. At this point http://maia.usno.navy.mil/ser7/deltat.preds indicates an error value 
                          for DeltaT of about 0.9 seconds anyway.
                          2. A new CAADynamicalTime::CumulativeLeapSeconds has been provided. This method takes as input the Julian
                          Day value and returns the cumulative total of Leap seconds which have been applied to this point. For more
                          information about leap seconds please see http://en.wikipedia.org/wiki/Leap_second. Using this method you 
                          can now implement code which converts from Terrestial Time to Coordinated Universal time as follows:
                          
                          double TerrestialTime = some calculation using AA+ algorithms(JD);
                          double DeltaT = CAADynamicalTime::DeltaT(JD);
                          double UniversalTime1 = TerrestialTime - DeltaT/86400.0; //The time of the event using the UT1 time scale
                          double TerrestialAtomicTime = TerrestialTime - (32.184/86400.0); //The time of the event using the TAI time scale
                          double CumulativeLeapSeconds = CAADynamicalTime::CumulativeLeapSeconds(JD);
                          double CoordinatedUniversalTime = (DeltaT - CumulativeLeapSeconds - 32.184)/86400.0 + UniversalTime1; //The time of the event using the UTC time scale
         PJN / 13-10-2012 1. Fixed a typo in the spelling of Coefficient throughout the AADynamicalTime.cpp module
         PJN / 04-08-2013 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st April 2013
                          2. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to 1st January 2023
         PJN / 28-10-2013 1. Addition of a TT2UTC method which converts from TT to UTC.
                          2. Addition of a UTC2TT method which converts from UTC to TT.
                          3. Addition of a TT2TAI method which converts from TT to TAI.
                          4. Addition of a TAI2TT method which converts from TAI to TT.
                          5. Addition of a TT2UT1 method which converts from TT to UT1.
                          6. Addition of a UT12TT method which converts from UT1 to TT.
                          7. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st September 2013
                          8. Addition of a UT1MinusUTC method which returns UT1 - UTC.
         PJN / 12-11-2014 1. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st October 2014
                          2. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to 1st January 2024
         PJN / 15-02-2015 1. Updated copyright details.
                          2. Updated the observed DeltaT values from http://maia.usno.navy.mil/ser7/deltat.data to 1st January 2015
                          3. Updated the predicted DeltaT values from http://maia.usno.navy.mil/ser7/deltat.preds to 1st January 2024
                          4. Updated the CumulativeLeapSeconds table from http://maia.usno.navy.mil/ser7/tai-utc.dat to 1st July 2015
  
Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


////////////////////////////////// Includes ///////////////////////////////////

#include "stdafx.h"
#include "AADynamicalTime.h"
#include "AADate.h"
#include <cassert>
#include <cstddef>
using namespace std;


////////////////////////////////// Macros / Defines ///////////////////////////

struct DeltaTValue
{
  double JD;
  double DeltaT;
};

const DeltaTValue g_DeltaTValues[] = 
{
//All the initial values are observed values from 1 February 1973 to 1 October 2014 as taken from http://maia.usno.navy.mil/ser7/deltat.data 
  { 2441714.5,	43.4724 },
  { 2441742.5,	43.5648 },
  { 2441773.5,	43.6737 },
  { 2441803.5,	43.7782 },
  { 2441834.5,	43.8763 },
  { 2441864.5,	43.9562 },
  { 2441895.5,	44.0315 },
  { 2441926.5,	44.1132 },
  { 2441956.5,	44.1982 },
  { 2441987.5,	44.2952 },
  { 2442017.5,	44.3936 },
  { 2442048.5,	44.4841 },
  { 2442079.5,	44.5646 },
  { 2442107.5,	44.6425 },
  { 2442138.5,	44.7386 },
  { 2442168.5,	44.8370 },
  { 2442199.5,	44.9302 },
  { 2442229.5,	44.9986 },
  { 2442260.5,	45.0584 },
  { 2442291.5,	45.1284 },
  { 2442321.5,	45.2064 },
  { 2442352.5,	45.2980 },
  { 2442382.5,	45.3897 },
  { 2442413.5,	45.4761 },
  { 2442444.5,	45.5633 },
  { 2442472.5,	45.6450 },
  { 2442503.5,	45.7375 },
  { 2442533.5,	45.8284 },
  { 2442564.5,	45.9133 },
  { 2442594.5,	45.9820 },
  { 2442625.5,	46.0408 },
  { 2442656.5,	46.1067 },
  { 2442686.5,	46.1825 },
  { 2442717.5,	46.2789 },
  { 2442747.5,	46.3713 },
  { 2442778.5,	46.4567 },
  { 2442809.5,	46.5445 },
  { 2442838.5,	46.6311 },
  { 2442869.5,	46.7302 },
  { 2442899.5,	46.8284 },
  { 2442930.5,	46.9247 },
  { 2442960.5,	46.9970 },
  { 2442991.5,	47.0709 },
  { 2443022.5,	47.1451 },
  { 2443052.5,	47.2362 },
  { 2443083.5,	47.3413 },
  { 2443113.5,	47.4319 },
  { 2443144.5,	47.5214 },
  { 2443175.5,	47.6049 },
  { 2443203.5,	47.6837 },
  { 2443234.5,	47.7781 },
  { 2443264.5,	47.8771 },
  { 2443295.5,	47.9687 },
  { 2443325.5,	48.0348 },
  { 2443356.5,	48.0942 },
  { 2443387.5,	48.1608 },
  { 2443417.5,	48.2460 },
  { 2443448.5,	48.3439 },
  { 2443478.5,	48.4355 },
  { 2443509.5,	48.5344 },
  { 2443540.5,	48.6325 },
  { 2443568.5,	48.7294 },
  { 2443599.5,	48.8365 },
  { 2443629.5,	48.9353 },
  { 2443660.5,	49.0319 },
  { 2443690.5,	49.1013 },
  { 2443721.5,	49.1591 },
  { 2443752.5,	49.2286 },
  { 2443782.5,	49.3070 },
  { 2443813.5,	49.4018 },
  { 2443843.5,	49.4945 },
  { 2443874.5,	49.5862 },
  { 2443905.5,	49.6805 },
  { 2443933.5,	49.7602 },
  { 2443964.5,	49.8556 },
  { 2443994.5,	49.9489 },
  { 2444025.5,	50.0347 },
  { 2444055.5,	50.1019 },
  { 2444086.5,	50.1622 },
  { 2444117.5,	50.2260 },
  { 2444147.5,	50.2968 },
  { 2444178.5,	50.3831 },
  { 2444208.5,	50.4599 },
  { 2444239.5,	50.5387 },
  { 2444270.5,	50.6161 },
  { 2444299.5,	50.6866 },
  { 2444330.5,	50.7658 },
  { 2444360.5,	50.8454 },
  { 2444391.5,	50.9187 },
  { 2444421.5,	50.9761 },
  { 2444452.5,	51.0278 },
  { 2444483.5,	51.0843 },
  { 2444513.5,	51.1538 },
  { 2444544.5,	51.2319 },
  { 2444574.5,	51.3063 },
  { 2444605.5,	51.3808 },
  { 2444636.5,	51.4526 },
  { 2444664.5,	51.5160 },
  { 2444695.5,	51.5985 },
  { 2444725.5,	51.6809 },
  { 2444756.5,	51.7573 },
  { 2444786.5,	51.8133 },
  { 2444817.5,	51.8532 },
  { 2444848.5,	51.9014 },
  { 2444878.5,	51.9603 },
  { 2444909.5,	52.0328 },
  { 2444939.5,	52.0985 },
  { 2444970.5,	52.1668 },
  { 2445001.5,	52.2316 },
  { 2445029.5,	52.2938 },
  { 2445060.5,	52.3680 },
  { 2445090.5,	52.4465 },
  { 2445121.5,	52.5180 },
  { 2445151.5,	52.5752 },
  { 2445182.5,	52.6178 },
  { 2445213.5,	52.6668 },
  { 2445243.5,	52.7340 },
  { 2445274.5,	52.8056 },
  { 2445304.5,	52.8792 },
  { 2445335.5,	52.9565 },
  { 2445366.5,	53.0445 },
  { 2445394.5,	53.1268 },
  { 2445425.5,	53.2197 },
  { 2445455.5,	53.3024 },
  { 2445486.5,	53.3747 },
  { 2445516.5,	53.4335 },
  { 2445547.5,	53.4778 },
  { 2445578.5,	53.5300 },
  { 2445608.5,	53.5845 },
  { 2445639.5,	53.6523 },
  { 2445669.5,	53.7256 },
  { 2445700.5,	53.7882 },
  { 2445731.5,	53.8367 },
  { 2445760.5,	53.8830 },
  { 2445791.5,	53.9443 },
  { 2445821.5,	54.0042 },
  { 2445852.5,	54.0536 },
  { 2445882.5,	54.0856 },
  { 2445913.5,	54.1084 },
  { 2445944.5,	54.1463 },
  { 2445974.5,	54.1914 },
  { 2446005.5,	54.2452 },
  { 2446035.5,	54.2958 },
  { 2446066.5,	54.3427 },
  { 2446097.5,	54.3911 },
  { 2446125.5,	54.4320 },
  { 2446156.5,	54.4898 },
  { 2446186.5,	54.5456 },
  { 2446217.5,	54.5977 },
  { 2446247.5,	54.6355 },
  { 2446278.5,	54.6532 },
  { 2446309.5,	54.6776 },
  { 2446339.5,	54.7174 },
  { 2446370.5,	54.7741 },
  { 2446400.5,	54.8253 },
  { 2446431.5,	54.8713 },
  { 2446462.5,	54.9161 },
  { 2446490.5,	54.9581 },
  { 2446521.5,	54.9997 },
  { 2446551.5,	55.0476 },
  { 2446582.5,	55.0912 },
  { 2446612.5,	55.1132 },
  { 2446643.5,	55.1328 },
  { 2446674.5,	55.1532 },
  { 2446704.5,	55.1898 },
  { 2446735.5,	55.2416 },
  { 2446765.5,	55.2838 },
  { 2446796.5,	55.3222 },
  { 2446827.5,	55.3613 },
  { 2446855.5,	55.4063 },
  { 2446886.5,	55.4629 },
  { 2446916.5,	55.5111 },
  { 2446947.5,	55.5524 },
  { 2446977.5,	55.5812 },
  { 2447008.5,	55.6004 },
  { 2447039.5,	55.6262 },
  { 2447069.5,	55.6656 },
  { 2447100.5,	55.7168 },
  { 2447130.5,	55.7698 },
  { 2447161.5,	55.8197 },
  { 2447192.5,	55.8615 },
  { 2447221.5,	55.9130 },
  { 2447252.5,	55.9663 },
  { 2447282.5,	56.0220 },
  { 2447313.5,	56.0700 },
  { 2447343.5,	56.0939 },
  { 2447374.5,	56.1105 },
  { 2447405.5,	56.1314 },
  { 2447435.5,	56.1611 },
  { 2447466.5,	56.2068 },
  { 2447496.5,	56.2583 },
  { 2447527.5,	56.3000 },
  { 2447558.5,	56.3399 },
  { 2447586.5,	56.3790 },
  { 2447617.5,	56.4283 },
  { 2447647.5,	56.4804 },
  { 2447678.5,	56.5352 },
  { 2447708.5,	56.5697 },
  { 2447739.5,	56.5983 },
  { 2447770.5,	56.6328 },
  { 2447800.5,	56.6739 },
  { 2447831.5,	56.7332 },
  { 2447861.5,	56.7972 },
  { 2447892.5,	56.8553 },
  { 2447923.5,	56.9111 },
  { 2447951.5,	56.9755 },
  { 2447982.5,	57.0471 },
  { 2448012.5,	57.1136 },
  { 2448043.5,	57.1738 },
  { 2448073.5,	57.2226 },
  { 2448104.5,	57.2597 },
  { 2448135.5,	57.3073 },
  { 2448165.5,	57.3643 },
  { 2448196.5,	57.4334 },
  { 2448226.5,	57.5016 },
  { 2448257.5,	57.5653 },
  { 2448288.5,	57.6333 },
  { 2448316.5,	57.6973 },
  { 2448347.5,	57.7711 },
  { 2448377.5,	57.8407 },
  { 2448408.5,	57.9058 },
  { 2448438.5,	57.9576 },
  { 2448469.5,	57.9975 },
  { 2448500.5,	58.0426 },
  { 2448530.5,	58.1043 },
  { 2448561.5,	58.1679 },
  { 2448591.5,	58.2389 },
  { 2448622.5,	58.3092 },
  { 2448653.5,	58.3833 },
  { 2448682.5,	58.4537 },
  { 2448713.5,	58.5401 },
  { 2448743.5,	58.6228 },
  { 2448774.5,	58.6917 },
  { 2448804.5,	58.7410 },
  { 2448835.5,	58.7836 },
  { 2448866.5,	58.8406 },
  { 2448896.5,	58.8986 },
  { 2448927.5,	58.9714 },
  { 2448957.5,	59.0438 },
  { 2448988.5,	59.1218 },
  { 2449019.5,	59.2003 },
  { 2449047.5,	59.2747 },
  { 2449078.5,	59.3574 },
  { 2449108.5,	59.4434 },
  { 2449139.5,	59.5242 },
  { 2449169.5,	59.5850 },
  { 2449200.5,	59.6344 },
  { 2449231.5,	59.6928 },
  { 2449261.5,	59.7588 },
  { 2449292.5,	59.8386 },
  { 2449322.5,	59.9111 },
  { 2449353.5,	59.9845 },
  { 2449384.5,	60.0564 },
  { 2449412.5,	60.1231 },
  { 2449443.5,	60.2042 },
  { 2449473.5,	60.2804 },
  { 2449504.5,	60.3530 },
  { 2449534.5,	60.4012 },
  { 2449565.5,	60.4440 },
  { 2449596.5,	60.4900 },
  { 2449626.5,	60.5578 },
  { 2449657.5,	60.6324 },
  { 2449687.5,	60.7059 },
  { 2449718.5,	60.7853 },
  { 2449749.5,	60.8664 },
  { 2449777.5,	60.9387 },
  { 2449808.5,	61.0277 },
  { 2449838.5,	61.1103 },
  { 2449869.5,	61.1870 },
  { 2449899.5,	61.2454 },
  { 2449930.5,	61.2881 },
  { 2449961.5,	61.3378 },
  { 2449991.5,	61.4036 },
  { 2450022.5,	61.4760 },
  { 2450052.5,	61.5525 },
  { 2450083.5,	61.6287 },
  { 2450114.5,	61.6846 },
  { 2450143.5,	61.7433 },
  { 2450174.5,	61.8132 },
  { 2450204.5,	61.8823 },
  { 2450235.5,	61.9497 },
  { 2450265.5,	61.9969 },
  { 2450296.5,	62.0343 },
  { 2450327.5,	62.0714 },
  { 2450357.5,	62.1202 },
  { 2450388.5,	62.1810 },
  { 2450418.5,	62.2382 },
  { 2450449.5,	62.2950 },
  { 2450480.5,	62.3506 },
  { 2450508.5,	62.3995 },
  { 2450539.5,	62.4754 },
  { 2450569.5,	62.5463 },
  { 2450600.5,	62.6136 },
  { 2450630.5,	62.6571 },
  { 2450661.5,	62.6942 },
  { 2450692.5,	62.7383 },
  { 2450722.5,	62.7926 },
  { 2450753.5,	62.8567 },
  { 2450783.5,	62.9146 },
  { 2450814.5,	62.9659 },
  { 2450845.5,	63.0217 },
  { 2450873.5,	63.0807 },
  { 2450904.5,	63.1462 },
  { 2450934.5,	63.2053 },
  { 2450965.5,	63.2599 },
  { 2450995.5,	63.2844 },
  { 2451026.5,	63.2961 },
  { 2451057.5,	63.3126 },
  { 2451087.5,	63.3422 },
  { 2451118.5,	63.3871 },
  { 2451148.5,	63.4339 },
  { 2451179.5,	63.4673 },
  { 2451210.5,	63.4979 },
  { 2451238.5,	63.5319 },
  { 2451269.5,	63.5679 },
  { 2451299.5,	63.6104 },
  { 2451330.5,	63.6444 },
  { 2451360.5,	63.6642 },
  { 2451391.5,	63.6739 },
  { 2451422.5,	63.6926 },
  { 2451452.5,	63.7147 },
  { 2451483.5,	63.7518 },
  { 2451513.5,	63.7927 },
  { 2451544.5,	63.8285 },
  { 2451575.5,	63.8557 },
  { 2451604.5,	63.8804 },
  { 2451635.5,	63.9075 },
  { 2451665.5,	63.9393 },
  { 2451696.5,	63.9691 },
  { 2451726.5,	63.9799 },
  { 2451757.5,	63.9833 },
  { 2451788.5,	63.9938 },
  { 2451818.5,	64.0093 },
  { 2451849.5,	64.0400 },
  { 2451879.5,	64.0670 },
  { 2451910.5,	64.0908 },
  { 2451941.5,	64.1068 },
  { 2451969.5,	64.1282 },
  { 2452000.5,	64.1584 },
  { 2452030.5,	64.1833 },
  { 2452061.5,	64.2094 },
  { 2452091.5,	64.2117 },
  { 2452122.5,	64.2073 },
  { 2452153.5,	64.2116 },
  { 2452183.5,	64.2223 },
  { 2452214.5,	64.2500 },
  { 2452244.5,	64.2761 },
  { 2452275.5,	64.2998 },
  { 2452306.5,	64.3192 },
  { 2452334.5,	64.3450 },
  { 2452365.5,	64.3735 },
  { 2452395.5,	64.3943 },
  { 2452426.5,	64.4151 },
  { 2452456.5,	64.4132 },
  { 2452487.5,	64.4118 },
  { 2452518.5,	64.4097 },
  { 2452548.5,	64.4168 },
  { 2452579.5,	64.4329 },
  { 2452609.5,	64.4511 },
  { 2452640.5,	64.4734 },
  { 2452671.5,	64.4893 },
  { 2452699.5,	64.5053 },
  { 2452730.5,	64.5269 },
  { 2452760.5,	64.5471 },
  { 2452791.5,	64.5597 },
  { 2452821.5,	64.5512 },
  { 2452852.5,	64.5371 },
  { 2452883.5,	64.5359 },
  { 2452913.5,	64.5415 },
  { 2452944.5,	64.5544 },
  { 2452974.5,	64.5654 },
  { 2453005.5,	64.5736 },
  { 2453036.5,	64.5891 },
  { 2453065.5,	64.6015 },
  { 2453096.5,	64.6176 },
  { 2453126.5,	64.6374 },
  { 2453157.5,	64.6549 },
  { 2453187.5,	64.6530 },
  { 2453218.5,	64.6379 },
  { 2453249.5,	64.6372 },
  { 2453279.5,	64.6400 },
  { 2453310.5,	64.6543 },
  { 2453340.5,	64.6723 },
  { 2453371.5,	64.6876 },
  { 2453402.5,	64.7052 },
  { 2453430.5,	64.7313 },
  { 2453461.5,	64.7575 },
  { 2453491.5,	64.7811 },
  { 2453522.5,	64.8001 },
  { 2453552.5,	64.7995 },
  { 2453583.5,	64.7876 },
  { 2453614.5,	64.7831 },
  { 2453644.5,	64.7921 },
  { 2453675.5,	64.8096 },
  { 2453705.5,	64.8311 },
  { 2453736.5,	64.8452 },
  { 2453767.5,	64.8597 },
  { 2453795.5,	64.8850 },
  { 2453826.5,	64.9175 },
  { 2453856.5,	64.9480 },
  { 2453887.5,	64.9794 },
  { 2453917.5,	64.9895 },
  { 2453948.5,	65.0028 },
  { 2453979.5,	65.0138 },
  { 2454009.5,	65.0371 },
  { 2454040.5,	65.0773 },
  { 2454070.5,	65.1122 },
  { 2454101.5,	65.1464 },
  { 2454132.5,	65.1833 },
  { 2454160.5,	65.2145 },
  { 2454191.5,	65.2494 },
  { 2454221.5,	65.2921 },
  { 2454252.5,	65.3279 },
  { 2454282.5,	65.3413 },
  { 2454313.5,	65.3452 },
  { 2454344.5,	65.3496 },
  { 2454374.5,	65.3711 },
  { 2454405.5,	65.3972 },
  { 2454435.5,	65.4296 },
  { 2454466.5,	65.4573 },
  { 2454497.5,	65.4868 },
  { 2454526.5,	65.5152 },
  { 2454557.5,	65.5450 },
  { 2454587.5,	65.5781 },
  { 2454618.5,	65.6127 },
  { 2454648.5,	65.6288 },
  { 2454679.5,	65.6370 },
  { 2454710.5,	65.6493 },
  { 2454740.5,	65.6760 },
  { 2454771.5,	65.7097 },
  { 2454801.5,	65.7461 },
  { 2454832.5,	65.7768 },
  { 2454863.5,	65.8025 },
  { 2454891.5,	65.8237 },
  { 2454922.5,	65.8595 },
  { 2454952.5,	65.8973 },
  { 2454983.5,	65.9323 },
  { 2455013.5,	65.9509 },
  { 2455044.5,	65.9534 },
  { 2455075.5,	65.9628 },
  { 2455105.5,	65.9839 },
  { 2455136.5,	66.0147 },
  { 2455166.5,	66.0420 },
  { 2455197.5,	66.0699 },
  { 2455228.5,	66.0961 },
  { 2455256.5,	66.1310 },
  { 2455287.5,	66.1683 },
  { 2455317.5,	66.2072 },
  { 2455348.5,	66.2356 },
  { 2455378.5,	66.2409 },
  { 2455409.5,	66.2335 },
  { 2455440.5,	66.2349 },
  { 2455470.5,	66.2441 },
  { 2455501.5,	66.2751 },
  { 2455531.5,	66.3054 },
  { 2455562.5,	66.3246 },
  { 2455593.5,	66.3406 },
  { 2455621.5,	66.3624 },
  { 2455652.5,	66.3957 },
  { 2455682.5,	66.4289 },
  { 2455713.5,	66.4619 },
  { 2455743.5,	66.4749 },
  { 2455774.5,	66.4751 },
  { 2455805.5,	66.4829 },
  { 2455835.5,	66.5056 },
  { 2455866.5,	66.5383 },
  { 2455896.5,	66.5706 },
  { 2455927.5,	66.6030 },
  { 2455958.5,	66.6340 },
  { 2455987.5,	66.6569 },
  { 2456018.5,  66.6925 }, //1 April 2012
  { 2456048.5,  66.7289 },
  { 2456079.5,  66.7579 },
  { 2456109.5,  66.7708 },
  { 2456140.5,  66.7740 },
  { 2456171.5,  66.7846 },
  { 2456201.5,  66.8103 },
  { 2456232.5,  66.8400 },
  { 2456262.5,  66.8779 },
  { 2456293.5,  66.9069 }, //1 January 2013
  { 2456324.5,  66.9443 }, //1 Februrary 2013
  { 2456352.5,  66.9763 }, //1 March 2013
  { 2456383.5,  67.0258 }, //1 April 2013
  { 2456413.5,  67.0716 }, //1 May 2013
  { 2456444.5,  67.1100 }, //1 June 2013
  { 2456474.5,  67.1266 }, //1 July 2013
  { 2456505.5,  67.1331 }, //1 August 2013
  { 2456536.5,  67.1458 }, //1 September 2013
  { 2456566.5,  67.1717 }, //1 October 2013
  { 2456597.5,  67.2091 }, //1 November 2013
  { 2456627.5,  67.2460 }, //1 December 2013
  { 2456658.5,  67.2810 }, //1 January 2014
  { 2456689.5,  67.3136 }, //1 February 2014
  { 2456717.5,  67.3457 }, //1 March 2014
  { 2456748.5,  67.3890 }, //1 April 2014
  { 2456778.5,  67.4318 }, //1 May 2014
  { 2456809.5,  67.4666 }, //1 June 2014
  { 2456839.5,  67.4858 }, //1 July 2014
  { 2456870.5,  67.4989 }, //1 August 2014
  { 2456901.5,  67.5111 }, //1 September 2014
  { 2456931.5,  67.5353 }, //1 October 2014
  { 2456962.5,  67.5711 }, //1 November 2014
  { 2456992.5,  67.6070 }, //1 December 2014
  { 2457023.5,  67.6439 }, //1 January 2015

//All these final values are predicted values from Year 2015.25 to Year 2024.0 are taken from http://maia.usno.navy.mil/ser7/deltat.preds
  { 2457114.75, 67.9    }, //2015.25
  { 2457206.00, 68.0    }, //2015.5
  { 2457297.25, 68.2    }, //2015.75
  { 2457388.50, 68.3    }, //2016.0
  { 2457480.00, 68.4    }, //2016.25
  { 2457571.50, 68.5    }, //2016.5
  { 2457663.00, 68.6    }, //2016.75
  { 2457754.50, 68.7    }, //2017.0
  { 2457845.75, 68.9    }, //2017.25
  { 2457937.00, 69      }, //2017.5
  { 2458210.75, 69      }, //2018.25
  { 2458302.00, 70      }, //2018.5
  { 2459032.50, 70      }, //2020.5
  { 2459124.00, 71      }, //2020.75
  { 2459763.00, 71      }, //2022.5
  { 2459854.25, 72      }, //2022.75
  { 2460310.50, 72      }, //2024.0

//Note as currently coded there is a single discontinuity of c. 1.87 seconds on 1 January 2024. At this point http://maia.usno.navy.mil/ser7/deltat.preds indicates an error value for DeltaT of about 5 seconds anyway.
};

struct LeapSecondCoefficient
{
  double JD;
  double LeapSeconds;
  double BaseMJD;
  double Coefficient;
};

const LeapSecondCoefficient g_LeapSecondCoefficients[] = //Cumulative leap second values from 1 Jan 1961 to 1 July 2015 as taken from http://maia.usno.navy.mil/ser7/tai-utc.dat
{
  { 2437300.5, 1.4228180, 37300, 0.001296  },
  { 2437512.5, 1.3728180, 37300, 0.001296  },
  { 2437665.5, 1.8458580, 37665, 0.0011232 },
  { 2438334.5, 1.9458580, 37665, 0.0011232 },
  { 2438395.5, 3.2401300, 38761, 0.001296  },
  { 2438486.5, 3.3401300, 38761, 0.001296  },
  { 2438639.5, 3.4401300, 38761, 0.001296  },
  { 2438761.5, 3.5401300, 38761, 0.001296  },
  { 2438820.5, 3.6401300, 38761, 0.001296  },
  { 2438942.5, 3.7401300, 38761, 0.001296  },
  { 2439004.5, 3.8401300, 38761, 0.001296  },
  { 2439126.5, 4.3131700, 39126, 0.002592  },
  { 2439887.5, 4.2131700, 39126, 0.002592  },
  { 2441317.5, 10.0,      41317, 0.0       },
  { 2441499.5, 11.0,      41317, 0.0       },
  { 2441683.5, 12.0,      41317, 0.0       },
  { 2442048.5, 13.0,      41317, 0.0       },
  { 2442413.5, 14.0,      41317, 0.0       },
  { 2442778.5, 15.0,      41317, 0.0       },
  { 2443144.5, 16.0,      41317, 0.0       }, 
  { 2443509.5, 17.0,      41317, 0.0       }, 
  { 2443874.5, 18.0,      41317, 0.0       }, 
  { 2444239.5, 19.0,      41317, 0.0       }, 
  { 2444786.5, 20.0,      41317, 0.0       }, 
  { 2445151.5, 21.0,      41317, 0.0       }, 
  { 2445516.5, 22.0,      41317, 0.0       }, 
  { 2446247.5, 23.0,      41317, 0.0       }, 
  { 2447161.5, 24.0,      41317, 0.0       }, 
  { 2447892.5, 25.0,      41317, 0.0       }, 
  { 2448257.5, 26.0,      41317, 0.0       }, 
  { 2448804.5, 27.0,      41317, 0.0       }, 
  { 2449169.5, 28.0,      41317, 0.0       }, 
  { 2449534.5, 29.0,      41317, 0.0       },
  { 2450083.5, 30.0,      41317, 0.0       }, 
  { 2450630.5, 31.0,      41317, 0.0       }, 
  { 2451179.5, 32.0,      41317, 0.0       }, 
  { 2453736.5, 33.0,      41317, 0.0       }, 
  { 2454832.5, 34.0,      41317, 0.0       }, 
  { 2456109.5, 35.0,      41317, 0.0       },
  { 2457204.5, 36.0,      41317, 0.0       }
};  


////////////////////////////////// Implementation /////////////////////////////

double CAADynamicalTime::DeltaT(double JD)
{
  //What will be the return value from the method
  double Delta = 0;

  //Determine if we can use the lookup table
  size_t nLookupElements = sizeof(g_DeltaTValues) / sizeof(DeltaTValue);
  if ((JD >= g_DeltaTValues[0].JD) && (JD < g_DeltaTValues[nLookupElements - 1].JD))
  {
    //Find the index in the lookup table which contains the JD value closest to the JD input parameter
    bool bFound = false;
    size_t nFoundIndex = 0;
    while (!bFound)
    {
      assert(nFoundIndex < nLookupElements);
      bFound = (g_DeltaTValues[nFoundIndex].JD > JD);
      
      //Prepare for the next loop
      if (!bFound)
        ++nFoundIndex;
      else
      {
        //Now do a simple linear interpolation of the DeltaT values from the lookup table
        Delta = (JD - g_DeltaTValues[nFoundIndex - 1].JD) / (g_DeltaTValues[nFoundIndex].JD - g_DeltaTValues[nFoundIndex - 1].JD) * (g_DeltaTValues[nFoundIndex].DeltaT - g_DeltaTValues[nFoundIndex - 1].DeltaT) + g_DeltaTValues[nFoundIndex - 1].DeltaT;
      }
    }
  }
  else
  {
    CAADate date(JD, CAADate::AfterPapalReform(JD));
    double y = date.FractionalYear();
  
    //Use the polynomial expressions from Espenak & Meeus 2006. References: http://eclipse.gsfc.nasa.gov/SEcat5/deltatpoly.html and
    //http://www.staff.science.uu.nl/~gent0113/deltat/deltat_old.htm (Espenak & Meeus 2006 section)
    if (y < -500)
    {
      double u = (y - 1820)/100.0;
      double u2 = u*u;
      Delta = -20 + (32*u2);
    }
    else if (y < 500)
    {
      double u = y/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      double u4 = u3*u;
      double u5 = u4*u;
      double u6 = u5*u;
      Delta = 10583.6 + (-1014.41*u) + (33.78311*u2) + (-5.952053*u3) + (-0.1798452*u4) + (0.022174192*u5) + (0.0090316521*u6);
    }
    else if (y < 1600)
    {
      double u = (y - 1000)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      double u4 = u3*u;
      double u5 = u4*u;
      double u6 = u5*u;
      Delta = 1574.2 + (-556.01*u) + (71.23472*u2) + (0.319781*u3) + (-0.8503463*u4) + (-0.005050998*u5) + (0.0083572073*u6);
    }
    else if (y < 1700)
    {
      double u = (y - 1600)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      Delta = 120 + (-98.08*u) + (-153.2*u2) + (u3/0.007129);
    }
    else if (y < 1800)
    {
      double u = (y - 1700)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      double u4 = u3*u;
      Delta = 8.83 + (16.03*u) + (-59.285*u2) + (133.36*u3) + (-u4/0.01174);
    }
    else if (y < 1860)
    {
      double u = (y - 1800)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      double u4 = u3*u;
      double u5 = u4*u;
      double u6 = u5*u;
      double u7 = u6*u;
      Delta = 13.72 + (-33.2447*u) + (68.612*u2) + (4111.6*u3) + (-37436*u4) + (121272*u5) + (-169900*u6) + (87500*u7);
    }
    else if (y < 1900)
    {
      double u = (y - 1860)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      double u4 = u3*u;
      double u5 = u4*u;
      Delta = 7.62 + (57.37*u) + (-2517.54*u2) + (16806.68*u3) + (-44736.24*u4) + (u5/0.0000233174);
    }
    else if (y < 1920)
    {
      double u = (y - 1900)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      double u4 = u3*u;
      Delta = -2.79 + (149.4119*u) + (-598.939*u2) + (6196.6*u3) + (-19700*u4);
    }
    else if (y < 1941)
    {
      double u = (y - 1920)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      Delta = 21.20 + (84.493*u) + (-761.00*u2) + (2093.6*u3);
    }
    else if (y < 1961)
    {
      double u = (y - 1950)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      Delta = 29.07 + (40.7*u) + (-u2/0.0233) + (u3/0.002547);
    }
    else if (y < 1986)
    {
      double u = (y - 1975)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      Delta = 45.45 + 106.7*u - u2/0.026 - u3/0.000718;
    }
    else if (y < 2005)
    {
      double u = (y - 2000)/100.0;
      double u2 = u*u;
      double u3 = u2*u;
      double u4 = u3*u;
      double u5 = u4*u;
      Delta = 63.86 + (33.45*u) + (-603.74*u2) + (1727.5*u3) + (65181.4*u4) + (237359.9*u5);
    }
    else if (y < 2050)
    {
      double u = (y - 2000)/100.0;
      double u2 = u*u;
      Delta = 62.92 + (32.217*u) + (55.89*u2);
    }
    else if (y < 2150)
    {
      double u = (y - 1820)/100.0;
      double u2 = u*u;
      Delta = -205.72 + (56.28*u) + (32*u2);
    }
    else
    {
      double u = (y - 1820)/100.0;
      double u2 = u*u;
      Delta = -20 + (32*u2);
    }
  }

  return Delta;
}

double CAADynamicalTime::CumulativeLeapSeconds(double JD)
{
  //What will be the return value from the method
  double LeapSeconds = 0;
 
  size_t nLookupElements = sizeof(g_LeapSecondCoefficients) / sizeof(LeapSecondCoefficient);
  if (JD >= g_LeapSecondCoefficients[0].JD)
  {
    //Find the index in the lookup table which contains the JD value closest to the JD input parameter
    bool bContinue = true;
    size_t nIndex = 1;
    while (bContinue)
    {
      if (nIndex >= nLookupElements)
      {
        LeapSeconds = g_LeapSecondCoefficients[nLookupElements - 1].LeapSeconds + (JD - 2400000.5 - g_LeapSecondCoefficients[nLookupElements - 1].BaseMJD) * g_LeapSecondCoefficients[nLookupElements - 1].Coefficient;
        bContinue = false;
      }
      else if (JD < g_LeapSecondCoefficients[nIndex].JD)
      {
        LeapSeconds = g_LeapSecondCoefficients[nIndex - 1].LeapSeconds + (JD - 2400000.5 - g_LeapSecondCoefficients[nIndex - 1].BaseMJD) * g_LeapSecondCoefficients[nIndex - 1].Coefficient;
        bContinue = false;
      }

      //Prepare for the next loop
      if (bContinue)
        ++nIndex;
    }
  }

  return LeapSeconds;
}

double CAADynamicalTime::TT2UTC(double JD)
{
  double DT = DeltaT(JD);
  double UT1 = JD - (DT / 86400.0);
  double LeapSeconds = CumulativeLeapSeconds(JD); 
  return ((DT - LeapSeconds - 32.184) / 86400.0) + UT1;
}

double CAADynamicalTime::UTC2TT(double JD)
{
  double DT = DeltaT(JD);
  double LeapSeconds = CumulativeLeapSeconds(JD); 
  double UT1 = JD - ((DT - LeapSeconds - 32.184) / 86400.0);
  return UT1 + (DT / 86400.0);
}

double CAADynamicalTime::TT2TAI(double JD)
{
  return JD - (32.184 / 86400.0);
}

double CAADynamicalTime::TAI2TT(double JD)
{
  return JD + (32.184 / 86400.0);
}

double CAADynamicalTime::TT2UT1(double JD)
{
  return JD - (DeltaT(JD) / 86400.0);
}

double CAADynamicalTime::UT12TT(double JD)
{
  return JD + (DeltaT(JD) / 86400.0);
}

double CAADynamicalTime::UT1MinusUTC(double JD)
{
  double JDUTC = JD + ((CAADynamicalTime::DeltaT(JD) - CumulativeLeapSeconds(JD) - 32.184) / 86400);
  return (JD - JDUTC) * 86400;
}
/*
Module : AAJUPITER.CPP
Purpose: Implementation for the algorithms which obtain the heliocentric position of Uranus
Created: PJN / 29-12-2003
History: PJN / 31-05-2004 1) Added a missing coefficient to g_L1JupiterCoefficients array as used by 
                          CAAJupiter::EclipticLongitude. Thanks to Brian Orme for reporting this problem.
                          2) Added missing g_B5JupiterCoefficients[] in CAAJupiter::EclipticLatitude. Again 
                          thanks to Brian Orme for reporting this problem.
         PJN / 18-03-2012 1. All global "g_*" tables are now const. Thanks to Roger Dahl for reporting this 
                          issue when compiling AA+ on ARM.
         PJN / 04-08-2013 1. Fixed a transcription error in the thirty ninth coefficient used to calculate 
                          the L0 term for the ecliptic longitude of Jupiter. Thanks to Isaac Clark for 
                          reporting this issue. 
                          2. Fixed a transcription error in the sixteenth coefficient used to calculate the 
                          R1 term for the ecliptic radius vector of Jupiter. Thanks to Isaac Clark for 
                          reporting this issue. 
                          3. Spot tests indicate that these two changes only affected the the ecliptic 
                          longitude in the seventh decimal place and the ecliptic radius vector in the 
                          seventh decimal place.
                          4. Updated copyright details

Copyright (c) 2003 - 2015 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////// Includes //////////////////////////////////////////////

#include "stdafx.h"
#include "AAJupiter.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


////////////////////// Macros / Defines ///////////////////////////////////////

struct VSOP87Coefficient
{
  double A;
  double B;
  double C;
};

const VSOP87Coefficient g_L0JupiterCoefficients[] =
{ 
  { 59954691, 0,          0 },
  { 9695899,  5.0619179,  529.6909651 },
  { 573610,   1.444062,   7.113547 },
  { 306389,   5.417347,   1059.381930 },
  { 97178,    4.14265,    632.78374 },
  { 72903,    3.64043,    522.57742 },
  { 64264,    3.41145,    103.09277 },
  { 39806,    2.29377,    419.48464 },
  { 38858,    1.27232,    316.39187 },
  { 27965,    1.78455,    536.80451 },
  { 13590,    5.77481,    1589.07290 },
  { 8769,     3.6300,     949.1756 },
  { 8246,     3.5823,     206.1855 },
  { 7368,     5.0810,     735.8765 },
  { 6263,     0.0250,     213.2991 },
  { 6114,     4.5132,     1162.4747 },
  { 5305,     4.1863,     1052.2684 },
  { 5305,     1.3067,     14.2271 },
  { 4905,     1.3208,     110.2063 },
  { 4647,     4.6996,     3.9322 },
  { 3045,     4.3168,     426.5982 },
  { 2610,     1.5667,     846.0828 },
  { 2028,     1.0638,     3.1814 },
  { 1921,     0.9717,     639.8973 },
  { 1765,     2.1415,     1066.4955 },
  { 1723,     3.8804,     1265.5675 },
  { 1633,     3.5820,     515.4639 },
  { 1432,     4.2968,     625.6702 },
  { 973,      4.098,      95.979 },
  { 884,      2.437,      412.371 },
  { 733,      6.085,      838.969 },
  { 731,      3.806,      1581.959 },
  { 709,      1.293,      742.990 },
  { 692,      6.134,      2118.764 },
  { 614,      4.109,      1478.867 },
  { 582,      4.540,      309.278 },
  { 495,      3.756,      323.505 },
  { 441,      2.958,      454.909 },
  { 417,      1.036,      2.448 },
  { 390,      4.897,      1692.166 },
  { 376,      4.703,      1368.660 },
  { 341,      5.715,      533.623 },
  { 330,      4.740,      0.048 },
  { 262,      1.877,      0.963 },
  { 261,      0.820,      380.128 },
  { 257,      3.724,      199.072 },
  { 244,      5.220,      728.763 },
  { 235,      1.227,      909.819 },
  { 220,      1.651,      543.918 },
  { 207,      1.855,      525.759 },
  { 202,      1.807,      1375.774 },
  { 197,      5.293,      1155.361 },
  { 175,      3.730,      942.062 },
  { 175,      3.226,      1898.351 },
  { 175,      5.910,      956.289 },
  { 158,      4.365,      1795.258 },
  { 151,      3.906,      74.782 },
  { 149,      4.377,      1685.052 },
  { 141,      3.136,      491.558 },
  { 138,      1.318,      1169.588 },
  { 131,      4.169,      1045.155 },
  { 117,      2.500,      1596.186 },
  { 117,      3.389,      0.521 },
  { 106,      4.554,      526.510 }
};

const VSOP87Coefficient g_L1JupiterCoefficients[] =
{ 
  { 52993480757.0, 0,          0 },
  { 489741,        4.220667,   529.690965 },
  { 228919,        6.026475,   7.113547 },
  { 27655,         4.57266,    1059.38193 },
  { 20721,         5.45939,    522.57742 },
  { 12106,         0.16986,    536.80451 },
  { 6068,          4.4242,     103.0928 },
  { 5434,          3.9848,     419.4846 },
  { 4238,          5.8901,     14.2271 },
  { 2212,          5.2677,     206.1855 },
  { 1746,          4.9267,     1589.0729 },
  { 1296,          5.5513,     3.1814 },
  { 1173,          5.8565,     1052.2684 },
  { 1163,          0.5145,     3.9322 },
  { 1099,          5.3070,     515.4639 },
  { 1007,          0.4648,     735.8765 },
  { 1004,          3.1504,     426.5982 },
  { 848,           5.758,      110.206 },
  { 827,           4.803,      213.299 },
  { 816,           0.586,      1066.495 },
  { 725,           5.518,      639.897 },
  { 568,           5.989,      625.670 },
  { 474,           4.132,      412.371 },
  { 413,           5.737,      95.979 },
  { 345,           4.242,      632.784 },
  { 336,           3.732,      1162.475 },
  { 234,           4.035,      949.176 },
  { 234,           6.243,      309.278 },
  { 199,           1.505,      838.969 },
  { 195,           2.219,      323.505 },
  { 187,           6.086,      742.990 },
  { 184,           6.280,      543.918 },
  { 171,           5.417,      199.072 },
  { 131,           0.626,      728.763 },
  { 115,           0.680,      846.083 },
  { 115,           5.286,      2118.764 },
  { 108,           4.493,      956.289 },
  { 80,            5.82,       1045.15 },
  { 72,            5.34,       942.06 },
  { 70,            5.97,       532.87 },
  { 67,            5.73,       21.34 },
  { 66,            0.13,       526.51 },
  { 65,            6.09,       1581.96 },
  { 59,            0.59,       1155.36 },
  { 58,            0.99,       1596.19 },
  { 57,            5.97,       1169.59 },
  { 57,            1.41,       533.62 },
  { 55,            5.43,       10.29 },
  { 52,            5.73,       117.32 },
  { 52,            0.23,       1368.66 },
  { 50,            6.08,       525.76 },
  { 47,            3.63,       1478.87 },
  { 47,            0.51,       1265.57 },
  { 40,            4.16,       1692.17 },
  { 34,            0.10,       302.16 },
  { 33,            5.04,       220.41 },
  { 32,            5.37,       508.35 },
  { 29,            5.42,       1272.68 },
  { 29,            3.36,       4.67 },
  { 29,            0.76,       88.87 },
  { 25,            1.61,       831.86 }
};

const VSOP87Coefficient g_L2JupiterCoefficients[] =
{ 
  { 47234,  4.32148,  7.11355 },
  { 38966,  0,        0 },
  { 30629,  2.93021,  529.69097 },
  { 3189,   1.0550,   522.5774 },
  { 2729,   4.8455,   536.8045 },
  { 2723,   3.4141,   1059.3819 },
  { 1721,   4.1873,   14.2271 },
  { 383,    5.768,    419.485 },
  { 378,    0.760,    515.464 },
  { 367,    6.055,    103.093 },
  { 337,    3.786,    3.181 },
  { 308,    0.694,    206.186 },
  { 218,    3.814,    1589.073 },
  { 199,    5.340,    1066.495 },
  { 197,    2.484,    3.932 },
  { 156,    1.406,    1052.268 },
  { 146,    3.814,    639.897 },
  { 142,    1.634,    426.598 },
  { 130,    5.837,    412.371 },
  { 117,    1.414,    625.670 },
  { 97,     4.03,     110.21 },
  { 91,     1.11,     95.98 },
  { 87,     2.52,     632.78 },
  { 79,     4.64,     543.92 },
  { 72,     2.22,     735.88 },
  { 58,     0.83,     199.07 },
  { 57,     3.12,     213.30 },
  { 49,     1.67,     309.28 },
  { 40,     4.02,     21.34 },
  { 40,     0.62,     323.51 },
  { 36,     2.33,     728.76 },
  { 29,     3.61,     10.29 },
  { 28,     3.24,     838.97 },
  { 26,     4.50,     742.99 },
  { 26,     2.51,     1162.47 },
  { 25,     1.22,     1045.15 },
  { 24,     3.01,     956.29 },
  { 19,     4.29,     532.87 },
  { 18,     0.81,     508.35 },
  { 17,     4.20,     2118.76 },
  { 17,     1.83,     526.51 },
  { 15,     5.81,     1596.19 },
  { 15,     0.68,     942.06 },
  { 15,     4.00,     117.32 },
  { 14,     5.95,     316.39 },
  { 14,     1.80,     302.16 },
  { 13,     2.52,     88.87 },
  { 13,     4.37,     1169.59 },
  { 11,     4.44,     525.76 },
  { 10,     1.72,     1581.96 },
  { 9,      2.18,     1155.36 },
  { 9,      3.29,     220.41 },
  { 9,      3.32,     831.86 },
  { 8,      5.76,     846.08 },
  { 8,      2.71,     533.62 },
  { 7,      2.18,     1265.57 },
  { 6,      0.50,     949.18 }     
};

const VSOP87Coefficient g_L3JupiterCoefficients[] =
{ 
  { 6502, 2.5986, 7.1135 },
  { 1357, 1.3464, 529.6910 },
  { 471,  2.475,  14.227 },
  { 417,  3.245,  536.805 },
  { 353,  2.974,  522.577 },
  { 155,  2.076,  1059.382 },
  { 87,   2.51,   515.46 },
  { 44,   0,      0 },
  { 34,   3.83,   1066.50 },
  { 28,   2.45,   206.19 },
  { 24,   1.28,   412.37 },
  { 23,   2.98,   543.92 },
  { 20,   2.10,   639.90 },
  { 20,   1.40,   419.48 },
  { 19,   1.59,   103.09 },
  { 17,   2.30,   21.34 },
  { 17,   2.60,   1589.07 },
  { 16,   3.15,   625.67 },
  { 16,   3.36,   1052.27 },
  { 13,   2.76,   95.98 },
  { 13,   2.54,   199.07 },
  { 13,   6.27,   426.60 },
  { 9,    1.76,   10.29 },
  { 9,    2.27,   110.21 },
  { 7,    3.43,   309.28 },
  { 7,    4.04,   728.76 },
  { 6,    2.52,   508.35 },
  { 5,    2.91,   1045.15 },
  { 5,    5.25,   323.51 },
  { 4,    4.30,   88.87 },
  { 4,    3.52,   302.16 },
  { 4,    4.09,   735.88 },
  { 3,    1.43,   956.29 },
  { 3,    4.36,   1596.19 },
  { 3,    1.25,   213.30 },
  { 3,    5.02,   838.97 },
  { 3,    2.24,   117.32 },
  { 2,    2.90,   742.99 },
  { 2,    2.36,   942.06 }
};

const VSOP87Coefficient g_L4JupiterCoefficients[] =
{ 
  { 669,  0.853,  7.114 },
  { 114,  3.142,  0 },
  { 100,  0.743,  14.227 },
  { 50,   1.65,   536.80 },
  { 44,   5.82,   529.69 },
  { 32,   4.86,   522.58 },
  { 15,   4.29,   515.46 },
  { 9,    0.71,   1059.38 },
  { 5,    1.30,   543.92 },
  { 4,    2.32,   1066.50 },
  { 4,    0.48,   21.34 },
  { 3,    3.00,   412.37 },
  { 2,    0.40,   639.90 },
  { 2,    4.26,   199.07 },
  { 2,    4.91,   625.67 },
  { 2,    4.26,   206.19 },
  { 1,    5.26,   1052.27 },
  { 1,    4.72,   95.98 },
  { 1,    1.29,   1589.07 }
};

const VSOP87Coefficient g_L5JupiterCoefficients[] =
{ 
  { 50, 5.26, 7.11 },
  { 16, 5.25, 14.23 },
  { 4,  0.01, 536.80 },
  { 2,  1.10, 522.58 },
  { 1,  3.14, 0 }
};

const VSOP87Coefficient g_B0JupiterCoefficients[] =
{ 
  { 2268616,  3.5585261,  529.6909651 },
  { 110090,   0,          0 },
  { 109972,   3.908093,   1059.381930 },
  { 8101,     3.6051,     522.5774 },
  { 6438,     0.3063,     536.8045 },
  { 6044,     4.2588,     1589.0729 },
  { 1107,     2.9853,     1162.4747 },
  { 944,      1.675,      426.598 },
  { 942,      2.936,      1052.268 },
  { 894,      1.754,      7.114 },
  { 836,      5.179,      103.093 },
  { 767,      2.155,      632.784 },
  { 684,      3.678,      213.299 },
  { 629,      0.643,      1066.495 },
  { 559,      0.014,      846.083 },
  { 532,      2.703,      110.206 },
  { 464,      1.173,      949.176 },
  { 431,      2.608,      419.485 },
  { 351,      4.611,      2118.764 },
  { 132,      4.778,      742.990 },
  { 123,      3.350,      1692.166 },
  { 116,      1.387,      323.505 },
  { 115,      5.049,      316.392 },
  { 104,      3.701,      515.464 },
  { 103,      2.319,      1478.867 },
  { 102,      3.153,      1581.959 }
};

const VSOP87Coefficient g_B1JupiterCoefficients[] =
{ 
  { 177352, 5.701665, 529.690965 },
  { 3230,   5.7794,   1059.3819 },
  { 3081,   5.4746,   522.5774 },
  { 2212,   4.7348,   536.8045 },
  { 1694,   3.1416,   0 },
  { 346,    4.746,    1052.268 },
  { 234,    5.189,    1066.495 },
  { 196,    6.186,    7.114 },
  { 150,    3.927,    1589.073 },
  { 114,    3.439,    632.784 },
  { 97,     2.91,     949.18 },
  { 82,     5.08,     1162.47 },
  { 77,     2.51,     103.09 },
  { 77,     0.61,     419.48 },
  { 74,     5.50,     515.46 },
  { 61,     5.45,     213.30 },
  { 50,     3.95,     735.88 },
  { 46,     0.54,     110.21 },
  { 45,     1.90,     846.08 },
  { 37,     4.70,     543.92 },
  { 36,     6.11,     316.39 },
  { 32,     4.92,     1581.96 }
};

const VSOP87Coefficient g_B2JupiterCoefficients[] =
{ 
  { 8094, 1.4632, 529.6910 },
  { 813,  3.1416, 0 },
  { 742,  0.957,  522.577 },
  { 399,  2.899,  536.805 },
  { 342,  1.447,  1059.382 },
  { 74,   0.41,   1052.27 },
  { 46,   3.48,   1066.50 },
  { 30,   1.93,   1589.07 },
  { 29,   0.99,   515.46 },
  { 23,   4.27,   7.11 },
  { 14,   2.92,   543.92 },
  { 12,   5.22,   632.78 },
  { 11,   4.88,   949.18 },
  { 6,    6.21,   1045.15 }
};

const VSOP87Coefficient g_B3JupiterCoefficients[] =
{ 
  { 252,  3.381,  529.691 },
  { 122,  2.733,  522.577 },
  { 49,   1.04,   536.80 },
  { 11,   2.31,   1052.27 },
  { 8,    2.77,   515.46 },
  { 7,    4.25,   1059.38 },
  { 6,    1.78,   1066.50 },
  { 4,    1.13,   543.92 },
  { 3,    3.14,   0 }
};

const VSOP87Coefficient g_B4JupiterCoefficients[] =
{ 
  { 15, 4.53, 522.58 },
  { 5,  4.47, 529.69 },
  { 4,  5.44, 536.80 },
  { 3,  0,    0 },
  { 2,  4.52, 515.46 },
  { 1,  4.20, 1052.27 }
};

const VSOP87Coefficient g_B5JupiterCoefficients[] =
{ 
  { 1,  0.09, 522.58 }
};

const VSOP87Coefficient g_R0JupiterCoefficients[] =
{ 
  { 520887429,  0,          0 },
  { 25209327,   3.49108640, 529.69096509 },
  { 610600,     3.841154,   1059.381930 },
  { 282029,     2.574199,   632.783739 },
  { 187647,     2.075904,   522.577418 },
  { 86793,      0.71001,    419.48464 },
  { 72063,      0.21466,    536.80451 },
  { 65517,      5.97996,    316.39187 },
  { 30135,      2.16132,    949.17561 },
  { 29135,      1.67759,    103.09277 },
  { 23947,      0.27458,    7.11355 },
  { 23453,      3.54023,    735.87651 },
  { 22284,      4.19363,    1589.07290 },
  { 13033,      2.96043,    1162.47470 },
  { 12749,      2.71550,    1052.26838 },
  { 9703,       1.9067,     206.1855 },
  { 9161,       4.4135,     213.2991 },
  { 7895,       2.4791,     426.5982 },
  { 7058,       2.1818,     1265.5675 },
  { 6138,       6.2642,     846.0828 },
  { 5477,       5.6573,     639.8973 },
  { 4170,       2.0161,     515.4639 },
  { 4137,       2.7222,     625.6702 },
  { 3503,       0.5653,     1066.4955 },
  { 2617,       2.0099,     1581.9593 },
  { 2500,       4.5518,     838.9693 },
  { 2128,       6.1275,     742.9901 },
  { 1912,       0.8562,     412.3711 },
  { 1611,       3.0887,     1368.6603 },
  { 1479,       2.6803,     1478.8666 },
  { 1231,       1.8904,     323.5054 },
  { 1217,       1.8017,     110.2063 },
  { 1015,       1.3867,     454.9094 },
  { 999,        2.872,      309.278 },
  { 961,        4.549,      2118.764 },
  { 886,        4.148,      533.623 },
  { 821,        1.593,      1898.351 },
  { 812,        5.941,      909.819 },
  { 777,        3.677,      728.763 },
  { 727,        3.988,      1155.361 },
  { 655,        2.791,      1685.052 },
  { 654,        3.382,      1692.166 },
  { 621,        4.823,      956.289 },
  { 615,        2.276,      942.062 },
  { 562,        0.081,      543.918 },
  { 542,        0.284,      525.759 }
};

const VSOP87Coefficient g_R1JupiterCoefficients[] =
{ 
  { 1271802,2.6493751,  529.6909651 },
  { 61662,  3.00076,    1059.38193 },
  { 53444,  3.89718,    522.57742 },
  { 41390,  0,          0 },
  { 31185,  4.88277,    536.80451 },
  { 11847,  2.41330,    419.48464 },
  { 9166,   4.7598,     7.1135 },
  { 3404,   3.3469,     1589.0729 },
  { 3203,   5.2108,     735.8765 },
  { 3176,   2.7930,     103.0928 },
  { 2806,   3.7422,     515.4639 },
  { 2677,   4.3305,     1052.2684 },
  { 2600,   3.6344,     206.1855 },
  { 2412,   1.4695,     426.5982 },
  { 2101,   3.9276,     639.8973 },
  { 1646,   5.3095,     1066.4955 },
  { 1641,   4.4163,     625.6702 },
  { 1050,   3.1611,     213.2991 },
  { 1025,   2.5543,     412.3711 },
  { 806,    2.678,      632.784 },
  { 741,    2.171,      1162.475 },
  { 677,    6.250,      838.969 },
  { 567,    4.577,      742.990 },
  { 485,    2.469,      949.176 },
  { 469,    4.710,      543.918 },
  { 445,    0.403,      323.505 },
  { 416,    5.368,      728.763 },
  { 402,    4.605,      309.278 },
  { 347,    4.681,      14.227 },
  { 338,    3.168,      956.289 },
  { 261,    5.343,      846.083 },
  { 247,    3.923,      942.062 },
  { 220,    4.842,      1368.660 },
  { 203,    5.600,      1155.361 },
  { 200,    4.439,      1045.155 },
  { 197,    3.706,      2118.764 },
  { 196,    3.759,      199.072 },
  { 184,    4.265,      95.979 },
  { 180,    4.402,      532.872 },
  { 170,    4.846,      526.510 },
  { 146,    6.130,      533.623 },
  { 133,    1.322,      110.206 },
  { 132,    4.512,      525.759 }
};

const VSOP87Coefficient g_R2JupiterCoefficients[] =
{ 
  { 79645,  1.35866,  529.69097 },
  { 8252,   5.7777,   522.5774 },
  { 7030,   3.2748,   536.8045 },
  { 5314,   1.8384,   1059.3819 },
  { 1861,   2.9768,   7.1135 },
  { 964,    5.480,    515.464 },
  { 836,    4.199,    419.485 },
  { 498,    3.142,    0 },
  { 427,    2.228,    639.897 },
  { 406,    3.783,    1066.495 },
  { 377,    2.242,    1589.073 },
  { 363,    5.368,    206.186 },
  { 342,    6.099,    1052.268 },
  { 339,    6.127,    625.670 },
  { 333,    0.003,    426.598 },
  { 280,    4.262,    412.371 },
  { 257,    0.963,    632.784 },
  { 230,    0.705,    735.877 },
  { 201,    3.069,    543.918 },
  { 200,    4.429,    103.093 },
  { 139,    2.932,    14.227 },
  { 114,    0.787,    728.763 },
  { 95,     1.70,     838.97 },
  { 86,     5.14,     323.51 },
  { 83,     0.06,     309.28 },
  { 80,     2.98,     742.99 },
  { 75,     1.60,     956.29 },
  { 70,     1.51,     213.30 },
  { 67,     5.47,     199.07 },
  { 62,     6.10,     1045.15 },
  { 56,     0.96,     1162.47 },
  { 52,     5.58,     942.06 },
  { 50,     2.72,     532.87 },
  { 45,     5.52,     508.35 },
  { 44,     0.27,     526.51 },
  { 40,     5.95,     95.98 }   
};

const VSOP87Coefficient g_R3JupiterCoefficients[] =
{ 
  { 3519, 6.0580, 529.6910 },
  { 1073, 1.6732, 536.8045 },
  { 916,  1.413,  522.577 },
  { 342,  0.523,  1059.382 },
  { 255,  1.196,  7.114 },
  { 222,  0.952,  515.464 },
  { 90,   3.14,   0 },
  { 69,   2.27,   1066.50 },
  { 58,   1.41,   543.92 },
  { 58,   0.53,   639.90 },
  { 51,   5.98,   412.37 },
  { 47,   1.58,   625.67 },
  { 43,   6.12,   419.48 },
  { 37,   1.18,   14.23 },
  { 34,   1.67,   1052.27 },
  { 34,   0.85,   206.19 },
  { 31,   1.04,   1589.07 },
  { 30,   4.63,   426.60 },
  { 21,   2.50,   728.76 },
  { 15,   0.89,   199.07 },
  { 14,   0.96,   508.35 },
  { 13,   1.50,   1045.15 },
  { 12,   2.61,   735.88 },
  { 12,   3.56,   323.51 },
  { 11,   1.79,   309.28 },
  { 11,   6.28,   956.29 },
  { 10,   6.26,   103.09 },
  { 9,    3.45,   838.97 }
};

const VSOP87Coefficient g_R4JupiterCoefficients[] =
{ 
  { 129,  0.084,  536.805 },
  { 113,  4.249,  529.691 },
  { 83,   3.30,   522.58 },
  { 38,   2.73,   515.46 }, 
  { 27,   5.69,   7.11 },
  { 18,   5.40,   1059.38 },
  { 13,   6.02,   543.92 },
  { 9,    0.77,   1066.50 },
  { 8,    5.68,   14.23 },
  { 7,    1.43,   412.37 },
  { 6,    5.12,   639.90 },
  { 5,    3.34,   625.67 },
  { 3,    3.40,   1052.27 },
  { 3,    4.16,   728.76 },
  { 3,    2.90,   426.60 }
};

const VSOP87Coefficient g_R5JupiterCoefficients[] =
{ 
  { 11, 4.75, 536.80 },
  { 4,  5.92, 522.58 },
  { 2,  5.57, 515.46 },
  { 2,  4.30, 543.92 },
  { 2,  3.69, 7.11 },
  { 2,  4.13, 1059.38 },
  { 2,  5.49, 1066.50 }
};


////////////////////////// Implementation /////////////////////////////////////

double CAAJupiter::EclipticLongitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;
  double rho5 = rho4*rho;

  //Calculate L0
  int nL0Coefficients = sizeof(g_L0JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double L0 = 0;
  int i;
  for (i=0; i<nL0Coefficients; i++)
    L0 += g_L0JupiterCoefficients[i].A * cos(g_L0JupiterCoefficients[i].B + g_L0JupiterCoefficients[i].C*rho);

  //Calculate L1
  int nL1Coefficients = sizeof(g_L1JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double L1 = 0;
  for (i=0; i<nL1Coefficients; i++)
    L1 += g_L1JupiterCoefficients[i].A * cos(g_L1JupiterCoefficients[i].B + g_L1JupiterCoefficients[i].C*rho);

  //Calculate L2
  int nL2Coefficients = sizeof(g_L2JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double L2 = 0;
  for (i=0; i<nL2Coefficients; i++)
    L2 += g_L2JupiterCoefficients[i].A * cos(g_L2JupiterCoefficients[i].B + g_L2JupiterCoefficients[i].C*rho);

  //Calculate L3
  int nL3Coefficients = sizeof(g_L3JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double L3 = 0;
  for (i=0; i<nL3Coefficients; i++)
    L3 += g_L3JupiterCoefficients[i].A * cos(g_L3JupiterCoefficients[i].B + g_L3JupiterCoefficients[i].C*rho);

  //Calculate L4
  int nL4Coefficients = sizeof(g_L4JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double L4 = 0;
  for (i=0; i<nL4Coefficients; i++)
    L4 += g_L4JupiterCoefficients[i].A * cos(g_L4JupiterCoefficients[i].B + g_L4JupiterCoefficients[i].C*rho);

  //Calculate L5
  int nL5Coefficients = sizeof(g_L5JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double L5 = 0;
  for (i=0; i<nL5Coefficients; i++)
    L5 += g_L5JupiterCoefficients[i].A * cos(g_L5JupiterCoefficients[i].B + g_L5JupiterCoefficients[i].C*rho);

  double value = (L0 + L1*rho + L2*rhosquared + L3*rhocubed + L4*rho4 + L5*rho5) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(value));
  return value;
}

double CAAJupiter::EclipticLatitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;
  double rho5 = rho4*rho;

  //Calculate B0
  int nB0Coefficients = sizeof(g_B0JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double B0 = 0;
  int i;
  for (i=0; i<nB0Coefficients; i++)
    B0 += g_B0JupiterCoefficients[i].A * cos(g_B0JupiterCoefficients[i].B + g_B0JupiterCoefficients[i].C*rho);

  //Calculate B1
  int nB1Coefficients = sizeof(g_B1JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double B1 = 0;
  for (i=0; i<nB1Coefficients; i++)
    B1 += g_B1JupiterCoefficients[i].A * cos(g_B1JupiterCoefficients[i].B + g_B1JupiterCoefficients[i].C*rho);

  //Calculate B2
  int nB2Coefficients = sizeof(g_B2JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double B2 = 0;
  for (i=0; i<nB2Coefficients; i++)
    B2 += g_B2JupiterCoefficients[i].A * cos(g_B2JupiterCoefficients[i].B + g_B2JupiterCoefficients[i].C*rho);

  //Calculate B3
  int nB3Coefficients = sizeof(g_B3JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double B3 = 0;
  for (i=0; i<nB3Coefficients; i++)
    B3 += g_B3JupiterCoefficients[i].A * cos(g_B3JupiterCoefficients[i].B + g_B3JupiterCoefficients[i].C*rho);

  //Calculate B4
  int nB4Coefficients = sizeof(g_B4JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double B4 = 0;
  for (i=0; i<nB4Coefficients; i++)
    B4 += g_B4JupiterCoefficients[i].A * cos(g_B4JupiterCoefficients[i].B + g_B4JupiterCoefficients[i].C*rho);

  //Calculate B5
  int nB5Coefficients = sizeof(g_B5JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double B5 = 0;
  for (i=0; i<nB5Coefficients; i++)
    B5 += g_B5JupiterCoefficients[i].A * cos(g_B5JupiterCoefficients[i].B + g_B5JupiterCoefficients[i].C*rho);

  double value = (B0 + B1*rho + B2*rhosquared + B3*rhocubed + B4*rho4 + B5*rho5) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  return value;
}

double CAAJupiter::RadiusVector(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;
  double rho5 = rho4*rho;

  //Calculate R0
  int nR0Coefficients = sizeof(g_R0JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double R0 = 0;
  int i;
  for (i=0; i<nR0Coefficients; i++)
    R0 += g_R0JupiterCoefficients[i].A * cos(g_R0JupiterCoefficients[i].B + g_R0JupiterCoefficients[i].C*rho);

  //Calculate R1
  int nR1Coefficients = sizeof(g_R1JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double R1 = 0;
  for (i=0; i<nR1Coefficients; i++)
    R1 += g_R1JupiterCoefficients[i].A * cos(g_R1JupiterCoefficients[i].B + g_R1JupiterCoefficients[i].C*rho);

  //Calculate R2
  int nR2Coefficients = sizeof(g_R2JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double R2 = 0;
  for (i=0; i<nR2Coefficients; i++)
    R2 += g_R2JupiterCoefficients[i].A * cos(g_R2JupiterCoefficients[i].B + g_R2JupiterCoefficients[i].C*rho);

  //Calculate R3
  int nR3Coefficients = sizeof(g_R3JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double R3 = 0;
  for (i=0; i<nR3Coefficients; i++)
    R3 += g_R3JupiterCoefficients[i].A * cos(g_R3JupiterCoefficients[i].B + g_R3JupiterCoefficients[i].C*rho);

  //Calculate R4
  int nR4Coefficients = sizeof(g_R4JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double R4 = 0;
  for (i=0; i<nR4Coefficients; i++)
    R4 += g_R4JupiterCoefficients[i].A * cos(g_R4JupiterCoefficients[i].B + g_R4JupiterCoefficients[i].C*rho);

//Calculate R5
  int nR5Coefficients = sizeof(g_R5JupiterCoefficients) / sizeof(VSOP87Coefficient);
  double R5 = 0;
  for (i=0; i<nR5Coefficients; i++)
    R5 += g_R5JupiterCoefficients[i].A * cos(g_R5JupiterCoefficients[i].B + g_R5JupiterCoefficients[i].C*rho);

  return (R0 + R1*rho + R2*rhosquared + R3*rhocubed + R4*rho4 + R5*rho5) / 100000000;
}

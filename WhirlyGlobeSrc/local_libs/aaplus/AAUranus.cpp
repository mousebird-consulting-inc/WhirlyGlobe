/*
Module : AAURANUS.CPP
Purpose: Implementation for the algorithms which obtain the heliocentric position of Uranus
Created: PJN / 29-12-2003
History: PJN / 18-03-2012 1. All global "g_*" tables are now const. Thanks to Roger Dahl for reporting this 
                          issue when compiling AA+ on ARM.

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
#include "AAUranus.h"
#include "AACoordinateTransformation.h"
#include <cmath>
using namespace std;


////////////////////////////////// Macros / Defines ///////////////////////////

struct VSOP87Coefficient
{
  double A;
  double B;
  double C;
};

const VSOP87Coefficient g_L0UranusCoefficients[] =
{ 
  { 548129294,	0,        	0 },
  { 9260408,	  0.8910642,	74.7815986 },
  { 1504248,    3.6271926,	1.4844727 },
  { 365982,   	1.899622,	  73.297126 },
  { 272328,   	3.358237,	  149.563197 },
  { 70328,  	  5.39254,	  63.73590 },
  { 68893, 	    6.09292,	  76.26607 },
  { 61999, 	    2.26952,	  2.96895 },
  { 61951, 	    2.85099,	  11.04570 },
  { 26469, 	    3.14152,	  71.81265 },
  { 25711, 	    6.11380,	  454.90937 },
  { 21079,	    4.36059,	  148.07872 },
  { 17819, 	    1.74437,	  36.64856 },
  { 14613,	    4.73732,	  3.93215 },
  { 11163, 	    5.82682,	  224.34480 },
  { 10998,	    0.48865,	  138.51750 },
  { 9527,	      2.9552,	    35.1641 },
  { 7546,	      5.2363,	    109.9457 },
  { 4220,	      3.2333,	    70.8494 },
  { 4052,	      2.2775,	    151.0477 },
  { 3490, 	    5.4831,	    146.5943 },
  { 3355,	      1.0655,	    4.4534 },
  { 3144,  	    4.7520,	    77.7505 },
  { 2927,	      4.6290,	    9.5612 },
  { 2922,	      5.3524,	    85.8273 },
  { 2273,  	    4.3660,	    70.3282 },
  { 2149,	      0.6075,	    38.1330 },
  { 2051,  	    1.5177,	    0.1119 },
  { 1992,  	    4.9244,	    277.0350 },
  { 1667,	      3.6274,	    380.1278 },
  { 1533,  	    2.5859,	    52.6902 },
  { 1376,	      2.0428,	    65.2204 },
  { 1372,	      4.1964,	    111.4302 }, 
  { 1284,  	    3.1135,	    202.2534 },
  { 1282,	      0.5427,	    222.8603 },
  { 1244,	      0.9161,	    2.4477 },
  { 1221,	      0.1990,	    108.4612 },
  { 1151,	      4.1790,	    33.6796 },
  { 1150,	      0.9334,	    3.1814 },
  { 1090,	      1.7750,	    12.5302 },
  { 1072,	      0.2356,	    62.2514 },
  { 946,	      1.192,	    127.472 },
  { 708,	      5.183,	    213.299 },
  { 653,	      0.966,	    78.714 },
  { 628,	      0.182,	    984.600 },
  { 607,	      5.432,	    529.691 },
  { 559,	      3.358,	    0.521 },
  { 524,	      2.013,	    299.126 },
  { 483,	      2.106,	    0.963 },
  { 471,	      1.407,	    184.727 },
  { 467,	      0.415,	    145.110 },
  { 434,	      5.521,	    183.243 },
  { 405,	      5.987,	    8.077 },
  { 399,	      0.338,	    415.552 },
  { 396,	      5.870,	    351.817 },
  { 379,	      2.350,	    56.622 },
  { 310,	      5.833,	    145.631 },
  { 300,	      5.644,	    22.091 },
  { 294,	      5.839,	    39.618 },
  { 252,	      1.637,	    221.376 },
  { 249,	      4.746,	    225.829 },
  { 239,	      2.350,	    137.033 },
  { 224,        0.516,	    84.343 },
  { 223,	      2.843,	    0.261 },
  { 220,	      1.922,	    67.668 },
  { 217,	      6.142,	    5.938 },
  { 216,	      4.778,	    340.771 },
  { 208,        5.580,	    68.844 },
  { 202,	      1.297,	    0.048 }, 
  { 199,	      0.956,	    152.532 },
  { 194,	      1.888,	    456.394 },
  { 193,	      0.916,	    453.425 } ,
  { 187,        1.319,	    0.160 },
  { 182,	      3.536,	    79.235 }, 
  { 173,	      1.539,	    160.609 },
  { 172,	      5.680,	    219.891 },
  { 170,	      3.677,    	5.417 },
  { 169,	      5.879,	    18.159 },
  { 165,	      1.424,	    106.977 },
  { 163,	      3.050,	    112.915 },
  { 158,	      0.738,	    54.175 },
  { 147,	      1.263,	    59.804 },
  { 143,	      1.300,	    35.425 },
  { 139,	      5.386,	    32.195 },
  { 139,	      4.260,	    909.819 },
  { 124,        1.374,	    7.114 },
  { 110,	      2.027,	    554.070 },
  { 109,	      5.706,	    77.963 },
  { 104,	      5.028,	    0.751 },
  { 104,	      1.458,	    24.379 },
  { 103,        0.681,	    14.978 }
};

const VSOP87Coefficient g_L1UranusCoefficients[] =
{ 
  { 7502543122.0,	0,	      0 },
  { 154458,   	  5.242017,	74.781599 },
  { 24456,	      1.71256,	1.48447 },
  { 9258,	        0.4284,	  11.0457 },
  { 8266,	        1.5022,	  63.7359 },
  { 7842,	        1.3198,	  149.5632 },
  { 3899,	        0.4648,	  3.9322 },
  { 2284,	        4.1737,	  76.2661 },
  { 1927,	        0.5301,	  2.9689 },
  { 1233,	        1.5863,	  70.8494 },
  { 791,	        5.436,	  3.181 }, 
  { 767,	        1.996,	  73.297  },
  { 482,	        2.984,	  85.827  },
  { 450,	        4.138,	  138.517  },
  { 446,	        3.723,	  224.345  },
  { 427,	        4.731,	  71.813  },
  { 354,	        2.583,	  148.079  },
  { 348,	        2.454,	  9.561  },
  { 317,	        5.579,	  52.690  },
  { 206,	        2.363,	  2.448  },
  { 189,	        4.202,	  56.622  },
  { 184,	        0.284,	  151.048  },
  { 180,	        5.684,	  12.530  },
  { 171,	        3.001,	  78.714  },
  { 158,	        2.909,	  0.963  },
  { 155,	        5.591,	  4.453  },
  { 154,	        4.652,	  35.164  },
  { 152,	        2.942,	  77.751  },
  { 143,	        2.590,	  62.251  },
  { 121,	        4.148,	  127.472  },
  { 116,	        3.732,	  65.220  },
  { 102,	        4.188,	  145.631  },
  { 102,	        6.034,	  0.112  },
  { 88,	          3.99,	    18.16  },
  { 88,	          6.16,	    202.25  },
  { 81,	          2.64,	    22.09  },
  { 72,	          6.05,	    70.33  },
  { 69,	          4.05,	    77.96  },
  { 59,	          3.70,	    67.67  },
  { 47,	          3.54,	    351.82  },
  { 44,	          5.91,	    7.11  },
  { 43,	          5.72,	    5.42  },
  { 39,	          4.92,	    222.86  },
  { 36,	          5.90,	    33.68  },
  { 36,	          3.29,	    8.08  },
  { 36,	          3.33,	    71.60  },
  { 35,	          5.08,	    38.13  },
  { 31,	          5.62,	    984.60  },
  { 31,	          5.50,	    59.80  },
  { 31,	          5.46,	    160.61  },
  { 30,	          1.66,	    447.80  },
  { 29,	          1.15,	    462.02  },
  { 29,	          4.52,	    84.34  },
  { 27,	          5.54,	    131.40  },
  { 27,	          6.15,	    299.13  },
  { 26,	          4.99,	    137.03  },
  { 25,	          5.74,	    380.13  }
};

const VSOP87Coefficient g_L2UranusCoefficients[] =
{ 
  { 53033,  0,      0 },
  { 2358,   2.2601, 74.7816 },
  { 769,    4.526,  11.046 },
  { 552,    3.258,  63.736 },
  { 542,    2.276,  3.932 },
  { 529,    4.923,  1.484 },
  { 258,    3.691,  3.181 },
  { 239,    5.858,  149.563 },
  { 182,    6.218,  70.849 },
  { 54,     1.44,   76.27 },
  { 49,     6.03,   56.62 },
  { 45,     3.91,   2.45 },
  { 45,     0.81,   85.83 },
  { 38,     1.78,   52.69 },
  { 37,     4.46,   2.97 },
  { 33,     0.86,   9.56 },
  { 29,     5.10,   73.30 },
  { 24,     2.11,   18.16 },
  { 22,     5.99,   138.52 },
  { 22,     4.82,   78.71 },
  { 21,     2.40,   77.96 },
  { 21,     2.17,   224.34 },
  { 17,     2.54,   145.63 },
  { 17,     3.47,   12.53 },
  { 12,     0.02,   22.09 },
  { 11,     0.08,   127.47 },
  { 10,     5.16,   71.60 },
  { 10,     4.46,   62.25 },
  { 9,      4.26,   7.11 },
  { 8,      5.50,   67.67 },
  { 7,      1.25,   5.42 },
  { 6,      3.36,   447.80 },
  { 6,      5.45,   65.22 },
  { 6,      4.52,   151.05 },
  { 6,      5.73,   462.02 }
};

const VSOP87Coefficient g_L3UranusCoefficients[] =
{ 
  { 121,  0.024,  74.782 },
  { 68,   4.12,   3.93 },
  { 53,   2.39,   11.05 },
  { 46,   0,      0 },
  { 45,   2.04,   3.18 },
  { 44,   2.96,   1.48 },
  { 25,   4.89,   63.74 },
  { 21,   4.55,   70.85 },
  { 20,   2.31,   149.56 },
  { 9,    1.58,   56.62 },
  { 4,    0.23,   18.16 },
  { 4,    5.39,   76.27 },
  { 4,    0.95,   77.96 },
  { 3,    4.98,   85.83 },
  { 3,    4.13,   52.69 },
  { 3,    0.37,   78.71 },
  { 2,    0.86,   145.63 },
  { 2,    5.66,   9.56 }
};

const VSOP87Coefficient g_L4UranusCoefficients[] =
{ 
  { 114,  3.142,  0 },
  { 6,    4.58,   74.78 },
  { 3,    0.35,   11.05 },
  { 1,    3.42,   56.62 }
};

const VSOP87Coefficient g_B0UranusCoefficients[] =
{ 
  { 1346278,  2.6187781,  74.7815986 },
  { 62341,    5.08111,    149.56320 },
  { 61601,    3.14159,    0 },
  { 9964,     1.6160,     76.2661 },
  { 9926,     0.5763,     73.2971 },
  { 3259,     1.2612,     224.3448 },
  { 2972,     2.2437,     1.4845 },
  { 2010,     6.0555,     148.0787 },
  { 1522,     0.2796,     63.7359 },
  { 924,      4.038,      151.048 },
  { 761,      6.140,      71.813 },
  { 522,      3.321,      138.517 },
  { 463,      0.743,      85.827 },
  { 437,      3.381,      529.691 },
  { 435,      0.341,      77.751 },
  { 431,      3.554,      213.299 },
  { 420,      5.213,      11.046 },
  { 245,      0.788,      2.969 },
  { 233,      2.257,      222.860 },
  { 216,      1.591,      38.133 },
  { 180,      3.725,      299.126 },
  { 175,      1.236,      146.594 },
  { 174,      1.937,      380.128 },
  { 160,      5.336,      111.430 },
  { 144,      5.962,      35.164 },
  { 116,      5.739,      70.849 },
  { 106,      0.941,      70.328 },
  { 102,      2.619,      78.714 }
};

const VSOP87Coefficient g_B1UranusCoefficients[] =
{ 
  { 206366, 4.123943, 74.781599 },
  { 8563,   0.3382,   149.5632 },
  { 1726,   2.1219,   73.2971 },
  { 1374,   0,        0 },
  { 1369,   3.0686,   76.2661 },
  { 451,    3.777,    1.484 },
  { 400,    2.848,    224.345 },
  { 307,    1.255,    148.079 },
  { 154,    3.786,    63.736 },
  { 112,    5.573,    151.048 },
  { 111,    5.329,    138.517 },
  { 83,     3.59,     71.81 },
  { 56,     3.40,     85.83 },
  { 54,     1.70,     77.75 },
  { 42,     1.21,     11.05 },
  { 41,     4.45,     78.71 },
  { 32,     3.77,     222.86 },
  { 30,     2.56,     2.97 },
  { 27,     5.34,     213.30 },
  { 26,     0.42,     380.13 }
};

const VSOP87Coefficient g_B2UranusCoefficients[] =
{ 
  { 9212, 5.8004, 74.7816 },
  { 557,  0,      0},
  { 286,  2.177,  149.563 },
  { 95,   3.84,   73.30 },
  { 45,   4.88,   76.27 },
  { 20,   5.46,   1.48 },
  { 15,   0.88,   138.52 },
  { 14,   2.85,   148.08 },
  { 14,   5.07,   63.74 },
  { 10,   5.00,   224.34 },
  { 8,    6.27,   78.71 }
};

const VSOP87Coefficient g_B3UranusCoefficients[] =
{ 
  { 268,  1.251,  74.782 },
  { 11,   3.14,   0 },
  { 6,    4.01,   149.56 },
  { 3,    5.78,   73.30 }
};

const VSOP87Coefficient g_B4UranusCoefficients[] =
{ 
  { 6,  2.85, 74.78 }
};

const VSOP87Coefficient g_R0UranusCoefficients[] =
{ 
  { 1921264848,   0,          0 },
  { 88784984,     5.60377527, 74.78159857 },
  { 3440836,      0.3283610,  73.2971259 },
  { 2055653,      1.7829517,  149.5631971 },
  { 649322,       4.522473,   76.266071 },
  { 602248,       3.860038,   63.735898 },
  { 496404,       1.401399,   454.909367 },
  { 338526,       1.580027,   138.517497 },
  { 243508,       1.570866,   71.812653 },
  { 190522,       1.998094,   1.484473 },
  { 161858,       2.791379,   148.078724 },
  { 143706,       1.383686,   11.045700 },
  { 93192,        0.17437,    36.64856 },
  { 89806,        3.66105,    109.94569 },
  { 71424,        4.24509,    224.34480 },
  { 46677,        1.39977,    35.16409 },
  { 39026,        3.36235,    277.03499 },
  { 39010,        1.66971,    70.84945 },
  { 36755,        3.88649,    146.59425 },
  { 30349,        0.70100,    151.04767 },
  { 29156,        3.18056,    77.75054 },
  { 25786,        3.78538,    85.82730 },
  { 25620,        5.25656,    380.12777 },
  { 22637,        0.72519,    529.69097 },
  { 20473,        2.79640,    70.32818 },
  { 20472,        1.55589,    202.25340 },
  { 17901,        0.55455,    2.96895 },
  { 15503,        5.35405,    38.13304 },
  { 14702,        4.90434,    108.46122 },
  { 12897,        2.62154,    111.43016 },
  { 12328,        5.96039,    127.47180 },
  { 11959,        1.75044,    984.60033 },
  { 11853,        0.99343,    52.69020 },
  { 11696,        3.29826,    3.93215 },
  { 11495,        0.43774,    65.22037 },
  { 10793,        1.42105,    213.29910 },
  { 9111,         4.9964,     62.2514 },
  { 8421,         5.2535,     222.8603 },
  { 8402,         5.0388,     415.5525 },
  { 7449,         0.7949,     351.8166 },
  { 7329,         3.9728,     183.2428 },
  { 6046,         5.6796,     78.7138 },
  { 5524,         3.1150,     9.5612 },
  { 5445,         5.1058,     145.1098 },
  { 5238,         2.6296,     33.6796 },
  { 4079,         3.2206,     340.7709 },
  { 3919,         4.2502,     39.6175 },
  { 3802,         6.1099,     184.7273 },
  { 3781,         3.4584,     456.3938 },
  { 3687,         2.4872,     453.4249 },
  { 3102,         4.1403,     219.8914 },
  { 2963,         0.8298,     56.6224 },
  { 2942,         0.4239,     299.1264 },
  { 2940,         2.1464,     137.0330 },
  { 2938,         3.6766,     140.0020 },
  { 2865,         0.3100,     12.5302 },
  { 2538,         4.8546,     131.4039 },
  { 2364,         0.4425,     554.0700 },
  { 2183,         2.9404,     305.3462 }
};

const VSOP87Coefficient g_R1UranusCoefficients[] =
{ 
  { 1479896,  3.6720571,  74.7815986 },
  { 71212,    6.22601,    63.73590 },
  { 68627,    6.13411,    149.56320 },
  { 24060,    3.14159,    0 },
  { 21468,    2.60177,    76.26607 },
  { 20857,    5.24625,    11.04570 },
  { 11405,    0.01848,    70.84945 },
  { 7497,     0.4236,     73.2971 },
  { 4244,     1.4169,     85.8273 },
  { 3927,     3.1551,     71.8127 },
  { 3578,     2.3116,     224.3448 },
  { 3506,     2.5835,     138.5175 },
  { 3229,     5.2550,     3.9322 },
  { 3060,     0.1532,     1.4845 },
  { 2564,     0.9808,     148.0787 },
  { 2429,     3.9944,     52.6902 },
  { 1645,     2.6535,     127.4718 },
  { 1584,     1.4305,     78.7138 },
  { 1508,     5.0600,     151.0477 },
  { 1490,     2.6756,     56.6224 },
  { 1413,     4.5746,     202.2534 },
  { 1403,     1.3699,     77.7505 },
  { 1228,     1.0470,     62.2514 },
  { 1033,     0.2646,     131.4039 },
  { 992,      2.172,      65.220 },
  { 862,      5.055,      351.817 },
  { 744,      3.076,      35.164 },
  { 687,      2.499,      77.963 },
  { 647,      4.473,      70.328 },
  { 624,      0.863,      9.561 },
  { 604,      0.907,      984.600 },
  { 575,      3.231,      447.796 },
  { 562,      2.718,      462.023 },
  { 530,      5.917,      213.299 },
  { 528,      5.151,      2.969 }
};

const VSOP87Coefficient g_R2UranusCoefficients[] =
{ 
  { 22440,  0.69953,  74.78160 },
  { 4727,   1.6990,   63.7359 },
  { 1682,   4.6483,   70.8494 },
  { 1650,   3.0966,   11.0457 },
  { 1434,   3.5212,   149.5632 },
  { 770,    0,        0 },
  { 500,    6.172,    76.266 },
  { 461,    0.767,    3.932 },
  { 390,    4.496,    56.622 },
  { 390,    5.527,    85.827 },
  { 292,    0.204,    52.690 },
  { 287,    3.534,    73.297 },
  { 273,    3.847,    138.517 },
  { 220,    1.964,    131.404 },
  { 216,    0.848,    77.963 },
  { 205,    3.248,    78.714 },
  { 149,    4.898,    127.472 },
  { 129,    2.081,    3.181 }
};

const VSOP87Coefficient g_R3UranusCoefficients[] =
{ 
  { 1164,   4.7345, 74.7816 },
  { 212,    3.343,  63.736 },
  { 196,    2.980,  70.849 },
  { 105,    0.958,  11.046 },
  { 73,     1.00,   149.56 },
  { 72,     0.03,   56.62 },
  { 55,     2.59,   3.93 },
  { 36,     5.65,   77.96 },
  { 34,     3.82,   76.27 },
  { 32,     3.60,   131.40 }
};

const VSOP87Coefficient g_R4UranusCoefficients[] =
{ 
  { 53, 3.01, 74.78 },
  { 10, 1.91, 56.62 }
};


/////////////////////////////// Implementation ////////////////////////////////

double CAAUranus::EclipticLongitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate L0
  int nL0Coefficients = sizeof(g_L0UranusCoefficients) / sizeof(VSOP87Coefficient);
  double L0 = 0;
  int i;
  for (i=0; i<nL0Coefficients; i++)
    L0 += g_L0UranusCoefficients[i].A * cos(g_L0UranusCoefficients[i].B + g_L0UranusCoefficients[i].C*rho);

  //Calculate L1
  int nL1Coefficients = sizeof(g_L1UranusCoefficients) / sizeof(VSOP87Coefficient);
  double L1 = 0;
  for (i=0; i<nL1Coefficients; i++)
    L1 += g_L1UranusCoefficients[i].A * cos(g_L1UranusCoefficients[i].B + g_L1UranusCoefficients[i].C*rho);

  //Calculate L2
  int nL2Coefficients = sizeof(g_L2UranusCoefficients) / sizeof(VSOP87Coefficient);
  double L2 = 0;
  for (i=0; i<nL2Coefficients; i++)
    L2 += g_L2UranusCoefficients[i].A * cos(g_L2UranusCoefficients[i].B + g_L2UranusCoefficients[i].C*rho);

  //Calculate L3
  int nL3Coefficients = sizeof(g_L3UranusCoefficients) / sizeof(VSOP87Coefficient);
  double L3 = 0;
  for (i=0; i<nL3Coefficients; i++)
    L3 += g_L3UranusCoefficients[i].A * cos(g_L3UranusCoefficients[i].B + g_L3UranusCoefficients[i].C*rho);

  //Calculate L4
  int nL4Coefficients = sizeof(g_L4UranusCoefficients) / sizeof(VSOP87Coefficient);
  double L4 = 0;
  for (i=0; i<nL4Coefficients; i++)
    L4 += g_L4UranusCoefficients[i].A * cos(g_L4UranusCoefficients[i].B + g_L4UranusCoefficients[i].C*rho);


  double value = (L0 + L1*rho + L2*rhosquared + L3*rhocubed + L4*rho4) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::MapTo0To360Range(CAACoordinateTransformation::RadiansToDegrees(value));
  return value;
}

double CAAUranus::EclipticLatitude(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate B0
  int nB0Coefficients = sizeof(g_B0UranusCoefficients) / sizeof(VSOP87Coefficient);
  double B0 = 0;
  int i;
  for (i=0; i<nB0Coefficients; i++)
    B0 += g_B0UranusCoefficients[i].A * cos(g_B0UranusCoefficients[i].B + g_B0UranusCoefficients[i].C*rho);

  //Calculate B1
  int nB1Coefficients = sizeof(g_B1UranusCoefficients) / sizeof(VSOP87Coefficient);
  double B1 = 0;
  for (i=0; i<nB1Coefficients; i++)
    B1 += g_B1UranusCoefficients[i].A * cos(g_B1UranusCoefficients[i].B + g_B1UranusCoefficients[i].C*rho);

  //Calculate B2
  int nB2Coefficients = sizeof(g_B2UranusCoefficients) / sizeof(VSOP87Coefficient);
  double B2 = 0;
  for (i=0; i<nB2Coefficients; i++)
    B2 += g_B2UranusCoefficients[i].A * cos(g_B2UranusCoefficients[i].B + g_B2UranusCoefficients[i].C*rho);

  //Calculate B3
  int nB3Coefficients = sizeof(g_B3UranusCoefficients) / sizeof(VSOP87Coefficient);
  double B3 = 0;
  for (i=0; i<nB3Coefficients; i++)
    B3 += g_B3UranusCoefficients[i].A * cos(g_B3UranusCoefficients[i].B + g_B3UranusCoefficients[i].C*rho);

  //Calculate B4
  int nB4Coefficients = sizeof(g_B4UranusCoefficients) / sizeof(VSOP87Coefficient);
  double B4 = 0;
  for (i=0; i<nB4Coefficients; i++)
    B4 += g_B4UranusCoefficients[i].A * cos(g_B4UranusCoefficients[i].B + g_B4UranusCoefficients[i].C*rho);

  double value = (B0 + B1*rho + B2*rhosquared + B3*rhocubed + B4*rho4) / 100000000;

  //convert results back to degrees
  value = CAACoordinateTransformation::RadiansToDegrees(value);
  return value;
}

double CAAUranus::RadiusVector(double JD)
{
  double rho = (JD - 2451545) / 365250;
  double rhosquared = rho*rho;
  double rhocubed = rhosquared*rho;
  double rho4 = rhocubed*rho;

  //Calculate R0
  int nR0Coefficients = sizeof(g_R0UranusCoefficients) / sizeof(VSOP87Coefficient);
  double R0 = 0;
  int i;
  for (i=0; i<nR0Coefficients; i++)
    R0 += g_R0UranusCoefficients[i].A * cos(g_R0UranusCoefficients[i].B + g_R0UranusCoefficients[i].C*rho);

  //Calculate R1
  int nR1Coefficients = sizeof(g_R1UranusCoefficients) / sizeof(VSOP87Coefficient);
  double R1 = 0;
  for (i=0; i<nR1Coefficients; i++)
    R1 += g_R1UranusCoefficients[i].A * cos(g_R1UranusCoefficients[i].B + g_R1UranusCoefficients[i].C*rho);

  //Calculate R2
  int nR2Coefficients = sizeof(g_R2UranusCoefficients) / sizeof(VSOP87Coefficient);
  double R2 = 0;
  for (i=0; i<nR2Coefficients; i++)
    R2 += g_R2UranusCoefficients[i].A * cos(g_R2UranusCoefficients[i].B + g_R2UranusCoefficients[i].C*rho);

  //Calculate R3
  int nR3Coefficients = sizeof(g_R3UranusCoefficients) / sizeof(VSOP87Coefficient);
  double R3 = 0;
  for (i=0; i<nR3Coefficients; i++)
    R3 += g_R3UranusCoefficients[i].A * cos(g_R3UranusCoefficients[i].B + g_R3UranusCoefficients[i].C*rho);

//Calculate R4
  int nR4Coefficients = sizeof(g_R4UranusCoefficients) / sizeof(VSOP87Coefficient);
  double R4 = 0;
  for (i=0; i<nR4Coefficients; i++)
    R4 += g_R4UranusCoefficients[i].A * cos(g_R4UranusCoefficients[i].B + g_R4UranusCoefficients[i].C*rho);
  
  return (R0 + R1*rho + R2*rhosquared + R3*rhocubed + R4*rho4) / 100000000;
}

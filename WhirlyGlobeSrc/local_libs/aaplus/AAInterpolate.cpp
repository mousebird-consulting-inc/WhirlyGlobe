/*
Module : AAINTERPOLATE.CPP
Purpose: Implementation for the algorithms for Interpolation
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


////////////////////// Includes ///////////////////////////////////////////////

#include "stdafx.h"
#include "AAInterpolate.h"
#include <cmath>
#include <cassert>
using namespace std;


////////////////////// Implementation /////////////////////////////////////////

double CAAInterpolate::Interpolate(double n, double Y1, double Y2, double Y3)
{
  double a = Y2 - Y1;
  double b = Y3 - Y2;
  double c = Y1 + Y3 - 2*Y2;

  return Y2 + n / 2 * (a + b + n*c);
}

double CAAInterpolate::Interpolate(double n, double Y1, double Y2, double Y3, double Y4, double Y5)
{
  double A = Y2 - Y1;
  double B = Y3 - Y2;
  double C = Y4 - Y3;
  double D = Y5 - Y4;
  double E = B - A;
  double F = C - B;
  double G = D - C;
  double H = F - E;
  double J = G - F;
  double K = J - H;

  double N2 = n*n;
  double N3 = N2*n;
  double N4 = N3*n;

  return Y3 + n*((B+C)/2 - (H+J)/12) + N2*(F/2 - K/24) + N3*((H+J)/12) + N4*(K/24);
}

double CAAInterpolate::InterpolateToHalves(double Y1, double Y2, double Y3, double Y4)
{
  return (9*(Y2 + Y3) - Y1 - Y4) / 16;
}

double CAAInterpolate::LagrangeInterpolate(double X, int n, double* pX, double* pY)
{
  //Validate our parameters
  assert(pX);
  assert(pY);
#ifdef __analysis_assume
  __analysis_assume(pX);
  __analysis_assume(pY);
#endif

  double V = 0;

  for (int i=1; i<=n; i++)
  {
    double C = 1;
    for (int j=1; j<=n; j++)
    {
      if (j != i)
        C = C*(X - pX[j-1]) / (pX[i-1] - pX[j-1]);
    }

    V += C*pY[i - 1];
  }

  return V;
}

double CAAInterpolate::Extremum(double Y1, double Y2, double Y3, double& nm)
{
  double a = Y2 - Y1;
  double b = Y3 - Y2;
  double c = Y1 + Y3 - 2*Y2;

  double ab = a + b;

  nm = -ab/(2*c);
  return (Y2 - ((ab*ab)/(8*c)));
}

double CAAInterpolate::Extremum(double Y1, double Y2, double Y3, double Y4, double Y5, double& nm)
{
  double A = Y2 - Y1;
  double B = Y3 - Y2;
  double C = Y4 - Y3;
  double D = Y5 - Y4;
  double E = B - A;
  double F = C - B;
  double G = D - C;
  double H = F - E;
  double J = G - F;
  double K = J - H;

  bool bRecalc = true;
  double nmprev = 0;
  nm = nmprev;
  while (bRecalc)
  {
    double NMprev2 = nmprev*nmprev;
    double NMprev3 = NMprev2*nmprev;
    nm = (6*B + 6*C - H - J +3*NMprev2*(H+J) + 2*NMprev3*K) / (K - 12*F);

    bRecalc = (fabs(nm - nmprev) > 1E-12);
    if (bRecalc)
      nmprev = nm;
  }

  return Interpolate(nm, Y1, Y2, Y3, Y4, Y5);
}

double CAAInterpolate::Zero(double Y1, double Y2, double Y3)
{
  double a = Y2 - Y1;
  double b = Y3 - Y2;
  double c = Y1 + Y3 - 2*Y2;

  bool bRecalc = true;
  double n0prev = 0;
  double n0 = n0prev;
  while (bRecalc)
  {
    n0 = -2*Y2/(a + b + c*n0prev);

    bRecalc = (fabs(n0 - n0prev) > 1E-12);
    if (bRecalc)
      n0prev = n0;
  }

  return n0;
}

double CAAInterpolate::Zero(double Y1, double Y2, double Y3, double Y4, double Y5)
{
  double A = Y2 - Y1;
  double B = Y3 - Y2;
  double C = Y4 - Y3;
  double D = Y5 - Y4;
  double E = B - A;
  double F = C - B;
  double G = D - C;
  double H = F - E;
  double J = G - F;
  double K = J - H;

  bool bRecalc = true;
  double n0prev = 0;
  double n0 = n0prev;
  while (bRecalc)
  {
    double n0prev2 = n0prev*n0prev;
    double n0prev3 = n0prev2*n0prev;
    double n0prev4 = n0prev3*n0prev;

    n0 = (-24*Y3 + n0prev2*(K - 12*F) - 2*n0prev3*(H+J) - n0prev4*K)/(2*(6*B + 6*C - H - J));

    bRecalc = (fabs(n0 - n0prev) > 1E-12);
    if (bRecalc)
      n0prev = n0;
  }

  return n0;
}


double CAAInterpolate::Zero2(double Y1, double Y2, double Y3)
{
  double a = Y2 - Y1;
  double b = Y3 - Y2;
  double c = Y1 + Y3 - 2*Y2;

  bool bRecalc = true;
  double n0prev = 0;
  double n0 = n0prev;
  while (bRecalc)
  {
    double deltan0 = - (2*Y2 + n0prev*(a + b + c*n0prev)) / (a + b + 2*c*n0prev);
    n0 = n0prev + deltan0;

    bRecalc = (fabs(deltan0) > 1E-12);
    if (bRecalc)
      n0prev = n0;
  }

  return n0;
}

double CAAInterpolate::Zero2(double Y1, double Y2, double Y3, double Y4, double Y5)
{
  double A = Y2 - Y1;
  double B = Y3 - Y2;
  double C = Y4 - Y3;
  double D = Y5 - Y4;
  double E = B - A;
  double F = C - B;
  double G = D - C;
  double H = F - E;
  double J = G - F;
  double K = J - H;
  double M = K / 24;
  double N = (H + J)/12;
  double P = F/2 - M;
  double Q = (B+C)/2 - N;

  bool bRecalc = true;
  double n0prev = 0;
  double n0 = n0prev;
  while (bRecalc)
  {
    double n0prev2 = n0prev*n0prev;
    double n0prev3 = n0prev2*n0prev;
    double n0prev4 = n0prev3*n0prev;

    double deltan0 = - (M * n0prev4 + N*n0prev3 + P*n0prev2 + Q*n0prev + Y3) / (4*M*n0prev3 + 3*N*n0prev2 + 2*P*n0prev + Q);
    n0 = n0prev + deltan0;

    bRecalc = (fabs(deltan0) > 1E-12);
    if (bRecalc)
      n0prev = n0;
  }

  return n0;
}

/*
Module : AAPHYSICALMARS.H
Purpose: Implementation for the algorithms which obtain the physical parameters of Mars
Created: PJN / 04-01-2004

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

#ifndef __AAPHYSICALMARS_H_
#define __AAPHYSICALMARS_H_

#ifndef AAPLUS_EXT_CLASS
#define AAPLUS_EXT_CLASS
#endif


/////////////////////// Classes ///////////////////////////////////////////////

class AAPLUS_EXT_CLASS CAAPhysicalMarsDetails
{
public:
  CAAPhysicalMarsDetails() : DE(0), 
                             DS(0), 
                             w(0), 
                             P(0),
                             X(0), 
                             k(0), 
                             q(0), 
                             d(0) 
  {
  };

//Member variables
  double DE;
  double DS;
  double w;
  double P;
  double X;
  double k;
  double q;
  double d;
};

class AAPLUS_EXT_CLASS CAAPhysicalMars
{
public:
//Static methods
  static CAAPhysicalMarsDetails Calculate(double JD);
};

#endif //__AAPHYSICALMARS_H_

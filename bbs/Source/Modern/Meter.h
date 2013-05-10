/*
  ==============================================================================

  Copyright 2007-2013 William Andrew Burnson. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY WILLIAM ANDREW BURNSON ''AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL WILLIAM ANDREW BURNSON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are
  those of the authors and should not be interpreted as representing official
  policies, either expressed or implied, of William Andrew Burnson.

  ------------------------------------------------------------------------------

  This file is part of Belle, Bonne, Sage --
    The 'Beautiful, Good, Wise' C++ Vector-Graphics Library for Music Notation 

  ==============================================================================
*/

#ifndef BELLEBONNESAGE_MODERN_METER_H
#define BELLEBONNESAGE_MODERN_METER_H

#include "Directory.h"

namespace bellebonnesage { namespace modern
{
  ///Algorithms for typesetting a meter.
  struct Meter
  {
    ///Engrave the different forms of meters.
    static void Engrave(Directory& d, Stamp& s, graph::MeterToken* mt, 
      bool isOnExtraStaff = false)
    {
      if(!mt || d.s.IsTabStaff() || 
        (d.s.IsStandardAndTabStaff() && isOnExtraStaff)) 
          return;
      
      mica::UUID m = mt->Value;
      if(m != mica::CommonTime && m != mica::CutTime)
      {
        prim::count Num = mica::M(mica::map(
          mica::Numerator, m)).Index(mica::Numbers);
        prim::count Den = mica::M(mica::map(
          mica::Denominator, m)).Index(mica::Numbers);
        s.Add().p2 = d.Symbol(0x0030 + Num);
        s.z().a = Affine::Translate(
            prim::planar::Vector(0.0, 0.0)) * Affine::Scale(4.0);
        s.z().n = mt;
        s.Add().p2 = d.Symbol(0x0030 + Den);
        s.z().a = Affine::Translate(
            prim::planar::Vector(0.0, -2.0)) * Affine::Scale(4.0);
        s.z().n = mt;
      }
      else
      {
        prim::count C = (m == mica::CommonTime ? 76 : 77);
        s.Add().p2 = d.Symbol(C);
        s.z().a = Affine::Scale(4.0);
        s.z().n = mt;
      }
    }
  };
}}
#endif

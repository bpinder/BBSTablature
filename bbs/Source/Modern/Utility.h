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

#ifndef BELLEBONNESAGE_MODERN_UTILITY_H
#define BELLEBONNESAGE_MODERN_UTILITY_H

#include "Cache.h"
#include "House.h"

namespace bellebonnesage { namespace modern
{
  ///Context-less static conversion methods
  struct Utility
  {
    ///Gets the base notehead value without dots applied.
    static prim::Ratio GetUndottedValue(prim::Ratio c)
    {
      //Round down to nearest power-of-two ratio or inverse ratio.
      prim::Ratio Value = 1;
      while(Value < c)
        Value *= 2;
      while(Value > c)
        Value /= 2;
      return Value;
    }
    
    ///Determines the number of dots for a rhythm.
    static prim::count CountDots(prim::Ratio Duration,
      prim::count MaxDotsToConsider)
    {
      prim::Ratio Base = GetUndottedValue(Duration);
      if(Base == Duration)
        return 0;
      prim::Ratio t = 1;
      for(prim::count i = 1; i <= MaxDotsToConsider; i++)
      {
        t *= prim::Ratio(1, 2);
        prim::Ratio c = Base * (prim::Ratio(2) - t);
        if(c == Duration)
          return i;
      }
      return -1; //Unexpected rhythm encountered
    }

    ///Determines the number of flags for a rhythm.
    static prim::count CountFlags(prim::Ratio Duration)
    {
      prim::Ratio Base = GetUndottedValue(Duration);
      prim::count Flags = 0;
      while(Base < prim::Ratio(1, 4))
      {
        Flags++;
        Base *= 2;
      }
      return Flags;
    }
    
    ///Gets the notehead for the given note and rhythm.
    static prim::count GetNotehead(prim::Ratio r)
    {
      prim::Ratio h = GetUndottedValue(r);
      if(h <= prim::Ratio(1, 4))
        return Cache::QuarterNoteNoStem;
      else if(h == prim::Ratio(1, 2))
        return Cache::HalfNoteNoStem;
      else
        return Cache::WholeNote;
    }
    
    static mica::UUID GetLineSpace(prim::count i)
    {
      return mica::M(mica::LineSpaces).Item(i, mica::LS0);
    }
    
    static prim::count GetLineSpaceIndex(mica::UUID LineSpace)
    {
      int p = index(mica::LineSpaces, LineSpace, mica::LS0);
      if(p == mica::Undefined)
        return 0;
      return p;
    }
    
    ///Gets the line/space of the top line for a given number of staff lines.
    static prim::count GetTopLine(prim::count StaffLines)
    {
      return StaffLines - 1;
    }
    
    ///Gets the line/space of the bottom line for a given number of staff lines.
    static prim::count GetBottomLine(prim::count StaffLines)
    {
      return -StaffLines + 1;
    }
    
    ///Rounds the current line-space up to the next space.
    static prim::count RoundUpToNextSpace(prim::count s, prim::count StaffLines)
    {
      if(StaffLines % 2)
        return (s % 2) ? s : s + 1;
      else
        return (s % 2) ? s + 1 : s;
    }
    
    /**Converts a line space into vertical position. One might think that staff
    lines were necessarily spaced equally, but it turns out that in some
    hand-engraved scores, notes on ledger lines are scrunched together slightly
    and there is a little extra space before the first ledger. The extent of
    these attributes is controlled by the house style.*/
    static prim::number GetLineSpacePosition(prim::count s,
      prim::count StaffLines, const House& h)
    {
      //For things notes that touch staff lines, use even spacing.
      if(s >= GetBottomLine(StaffLines) - 1 &&
        s <= GetTopLine(StaffLines) + 1)
          return (prim::number)s / 2.0;
      
      if(s > 0)
      {
        prim::number ds =
          (prim::number)(s - GetTopLine(StaffLines) - 2) / 2.0;
        return (GetTopLine(StaffLines) + 2) / 2.0 +
          ds * h.LedgerLineScrunch + h.LedgerLineGap;
      }
      else
      {
        prim::number ds =
          (prim::number)(s - GetBottomLine(StaffLines) + 2) / 2.0;
        return (GetBottomLine(StaffLines) - 2) / 2.0 +
          ds * h.LedgerLineScrunch - h.LedgerLineGap;
      }
      return 0;
    }
    
    static prim::count GetNumberOfAccidentals(mica::UUID KeySignature)
    {
      return prim::Abs(mica::index(mica::KeySignatures, KeySignature,
        mica::NoAccidentals));
    }
    
    static prim::count GetAccidentalPosition(mica::UUID KeySignature,
      mica::UUID Clef, prim::count i)
    {
      mica::UUID PositionSequence = mica::map(Clef,
        mica::map(KeySignature, mica::Accidental));
      return GetLineSpaceIndex(mica::M(PositionSequence).Item(i));
    }
  };
}}
#endif

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

    /**Converts the stringIndex of a TabbedNote to a LineSpace value that can be
    passed through GetLineSpacePosition() to get the position.*/
    static prim::number GetLineSpaceForTabbedNote (int stringIndex, 
      int numStrings)
    {
      if (numStrings % 2)
        return (((prim::number)numStrings / 2.0 + 0.5) - (stringIndex + 1)) * 2;
      else
        return ((numStrings / 2) - (stringIndex + 1)) * 2 + 1;
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

    /**Converts a note name to a midi note number. This function
    likely won't be needed once the new MICA library is incorporated*/
    static inline prim::count GetNoteNumberForNoteName (mica::UUID note)
    {
      mica::UUID octave = mica::map (note, mica::Octave);
      prim::count octaveIndex = mica::index (mica::Octaves, 
        octave, mica::Octave0);

      mica::UUID name = mica::map (note, mica::NoteName);
      prim::count noteIndex = 0;

      //All possible names for C Sharp
      if (name == mica::BDoubleSharp 
        || name == mica::CSharp
        || name == mica::DFlat
        || name == mica::ETripleFlat)  noteIndex = 1;

      //All possible names for D
      else if (name == mica::BTripleSharp
        || name == mica::CDoubleSharp 
        || name == mica::D
        || name == mica::EDoubleFlat
        || name == mica::FTripleFlat)  noteIndex = 2;

      //All possible names for D Sharp
      else if (name == mica::CTripleSharp
        || name == mica::DSharp
        || name == mica::EFlat
        || name == mica::FDoubleFlat)  noteIndex = 3;

      //All possible names for E
      else if (name == mica::DDoubleSharp 
        || name == mica::E
        || name == mica::FFlat
        || name == mica::GTripleFlat)  noteIndex = 4;

      //All possible names for F
      else if (name == mica::DTripleSharp
        || name == mica::ESharp 
        || name == mica::F
        || name == mica::GDoubleFlat)  noteIndex = 5;

      //All possible names for F Sharp
      else if (name == mica::EDoubleSharp 
        || name == mica::FSharp
        || name == mica::GFlat
        || name == mica::ATripleFlat)  noteIndex = 6;

      //All possible names for G
      else if (name == mica::ETripleSharp
        || name == mica::FDoubleSharp 
        || name == mica::G
        || name == mica::ADoubleFlat)  noteIndex = 7;

      //All possible names for G Sharp
      else if (name == mica::FTripleSharp 
        || name == mica::GSharp
        || name == mica::AFlat
        || name == mica::BTripleFlat)  noteIndex = 8;

      //All possible names for A
      else if (name == mica::GDoubleSharp 
        || name == mica::A
        || name == mica::BDoubleFlat
        || name == mica::CTripleFlat)  noteIndex = 9;

      //All possible names for A Sharp
      else if (name == mica::GTripleSharp
        || name == mica::ASharp
        || name == mica::BFlat
        || name == mica::CDoubleFlat)  noteIndex = 10;

      //All possible names for B
      else if (name == mica::ADoubleSharp 
        || name == mica::B
        || name == mica::CFlat
        || name == mica::DTripleFlat)  noteIndex = 11;
        
      return (octaveIndex * 12) + noteIndex;
    }

    /**Converts a midi note number to a mica::MIDIValue.This function
    likely won't be needed once the new MICA library is incorporated*/
    static inline mica::UUID GetMIDIValueForNoteNumber (int noteNumber)
    {
      switch (noteNumber)
      {
        case 0: return mica::MIDIValue0;
        case 1: return mica::MIDIValue1;
        case 2: return mica::MIDIValue2;
        case 3: return mica::MIDIValue3;
        case 4: return mica::MIDIValue4;
        case 5: return mica::MIDIValue5;
        case 6: return mica::MIDIValue6;
        case 7: return mica::MIDIValue7;
        case 8: return mica::MIDIValue8;
        case 9: return mica::MIDIValue9;

        case 10: return mica::MIDIValue10;
        case 11: return mica::MIDIValue11;
        case 12: return mica::MIDIValue12;
        case 13: return mica::MIDIValue13;
        case 14: return mica::MIDIValue14;
        case 15: return mica::MIDIValue15;
        case 16: return mica::MIDIValue16;
        case 17: return mica::MIDIValue17;
        case 18: return mica::MIDIValue18;
        case 19: return mica::MIDIValue19;

        case 20: return mica::MIDIValue20;
        case 21: return mica::MIDIValue21;
        case 22: return mica::MIDIValue22;
        case 23: return mica::MIDIValue23;
        case 24: return mica::MIDIValue24;
        case 25: return mica::MIDIValue25;
        case 26: return mica::MIDIValue26;
        case 27: return mica::MIDIValue27;
        case 28: return mica::MIDIValue28;
        case 29: return mica::MIDIValue29;

        case 30: return mica::MIDIValue30;
        case 31: return mica::MIDIValue31;
        case 32: return mica::MIDIValue32;
        case 33: return mica::MIDIValue33;
        case 34: return mica::MIDIValue34;
        case 35: return mica::MIDIValue35;
        case 36: return mica::MIDIValue36;
        case 37: return mica::MIDIValue37;
        case 38: return mica::MIDIValue38;
        case 39: return mica::MIDIValue39;

        case 40: return mica::MIDIValue40;
        case 41: return mica::MIDIValue41;
        case 42: return mica::MIDIValue42;
        case 43: return mica::MIDIValue43;
        case 44: return mica::MIDIValue44;
        case 45: return mica::MIDIValue45;
        case 46: return mica::MIDIValue46;
        case 47: return mica::MIDIValue47;
        case 48: return mica::MIDIValue48;
        case 49: return mica::MIDIValue49;

        case 50: return mica::MIDIValue50;
        case 51: return mica::MIDIValue51;
        case 52: return mica::MIDIValue52;
        case 53: return mica::MIDIValue53;
        case 54: return mica::MIDIValue54;
        case 55: return mica::MIDIValue55;
        case 56: return mica::MIDIValue56;
        case 57: return mica::MIDIValue57;
        case 58: return mica::MIDIValue58;
        case 59: return mica::MIDIValue59;

        case 60: return mica::MIDIValue60;
        case 61: return mica::MIDIValue61;
        case 62: return mica::MIDIValue62;
        case 63: return mica::MIDIValue63;
        case 64: return mica::MIDIValue64;
        case 65: return mica::MIDIValue65;
        case 66: return mica::MIDIValue66;
        case 67: return mica::MIDIValue67;
        case 68: return mica::MIDIValue68;
        case 69: return mica::MIDIValue69;

        case 70: return mica::MIDIValue70;
        case 71: return mica::MIDIValue71;
        case 72: return mica::MIDIValue72;
        case 73: return mica::MIDIValue73;
        case 74: return mica::MIDIValue74;
        case 75: return mica::MIDIValue75;
        case 76: return mica::MIDIValue76;
        case 77: return mica::MIDIValue77;
        case 78: return mica::MIDIValue78;
        case 79: return mica::MIDIValue79;

        case 80: return mica::MIDIValue80;
        case 81: return mica::MIDIValue81;
        case 82: return mica::MIDIValue82;
        case 83: return mica::MIDIValue83;
        case 84: return mica::MIDIValue84;
        case 85: return mica::MIDIValue85;
        case 86: return mica::MIDIValue86;
        case 87: return mica::MIDIValue87;
        case 88: return mica::MIDIValue88;
        case 89: return mica::MIDIValue89;

        case 90: return mica::MIDIValue90;
        case 91: return mica::MIDIValue91;
        case 92: return mica::MIDIValue92;
        case 93: return mica::MIDIValue93;
        case 94: return mica::MIDIValue94;
        case 95: return mica::MIDIValue95;
        case 96: return mica::MIDIValue96;
        case 97: return mica::MIDIValue97;
        case 98: return mica::MIDIValue98;
        case 99: return mica::MIDIValue99;

        case 100: return mica::MIDIValue100;
        case 101: return mica::MIDIValue101;
        case 102: return mica::MIDIValue102;
        case 103: return mica::MIDIValue103;
        case 104: return mica::MIDIValue104;
        case 105: return mica::MIDIValue105;
        case 106: return mica::MIDIValue106;
        case 107: return mica::MIDIValue107;
        case 108: return mica::MIDIValue108;
        case 109: return mica::MIDIValue109;

        case 110: return mica::MIDIValue100;
        case 111: return mica::MIDIValue111;
        case 112: return mica::MIDIValue112;
        case 113: return mica::MIDIValue113;
        case 114: return mica::MIDIValue114;
        case 115: return mica::MIDIValue115;
        case 116: return mica::MIDIValue116;
        case 117: return mica::MIDIValue117;
        case 118: return mica::MIDIValue118;
        case 119: return mica::MIDIValue119;

        case 120: return mica::MIDIValue120;
        case 121: return mica::MIDIValue121;
        case 122: return mica::MIDIValue122;
        case 123: return mica::MIDIValue123;
        case 124: return mica::MIDIValue124;
        case 125: return mica::MIDIValue125;
        case 126: return mica::MIDIValue126;
        case 127: return mica::MIDIValue127;

      };
      return mica::Undefined;
    }

    ///Returns the accidental of a LineSpace given the Clef and Key
    static inline mica::UUID GetAccidentalOfLineSpace (mica::UUID Clef, 
      mica::UUID Key, mica::UUID LineSpace)
    {
      //Find the number and type of accidentals (ie: "TwoSharps")
      mica::UUID NumAccsAndType = mica::map (Key, mica::KeySignature);  

      //Find the key type (ie: "Sharp")
      mica::UUID KeyType = mica::map (mica::Accidental, 
        NumAccsAndType);      

      //Find the clef accidentals (ie: "TrebleClefSharps")
      mica::UUID ClefAccs = mica::map (Clef, KeyType);    

      //Find the letter of the LineSpace (ie: "B")
      mica::UUID Letter = mica::map (mica::map (Clef, LineSpace), 
        mica::Letter);  

      //Find the number of accidentals
      int NumOfAccs = mica::index (mica::KeySignatures, NumAccsAndType,
        mica::NoAccidentals) * (KeyType == mica::Flat ? -1 : 1);

      //Determine what note letters have accidentals
      prim::Array<mica::UUID> KeyAccLetters;

      for (int i = 0; i < NumOfAccs; ++i)
      {
        mica::UUID KeyLetter = mica::map (mica::map (Clef, 
          mica::item (ClefAccs, i)), mica::Letter);
        if (KeyLetter != mica::Undefined) 
          KeyAccLetters.Add (KeyLetter);
      }

      /**If the letter for LineSpace is in KeyAccLetters, return
      KeyType, otherwise Natural*/
      for (int i = 0; i < KeyAccLetters.n(); ++i)
        if (KeyAccLetters.Contains (Letter)) return KeyType;

        return mica::Natural;
    }

    ///Returns the notename of a LineSpace
    static inline mica::UUID GetNoteName (mica::UUID Clef, mica::UUID Key, 
      mica::UUID LineSpace, mica::UUID Accidental)
    {
      mica::UUID Note = mica::map (LineSpace, Clef);
      mica::UUID Acc = (Accidental == mica::NoAccidentals ? 
        GetAccidentalOfLineSpace (Clef, Key, LineSpace) : Accidental);
      return mica::map (Note, Acc);
    }
  };
}}
#endif

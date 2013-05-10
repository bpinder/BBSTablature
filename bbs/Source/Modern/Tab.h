#ifndef BELLEBONNESAGE_MODERN_TAB_H
#define BELLEBONNESAGE_MODERN_TAB_H

#include "../Modern/State.h"
    
namespace bellebonnesage { namespace modern
{
  ///Represents a chord of tabbed notes
  struct Tab
  {
    ///Constructor
    Tab() : NumStrings(0), Duration(1, 4), IsRest(false), 
      OriginalNode(0), State(0) {}

    ///Convert ChordToken to Tablature
    void Import(prim::Node* ChordToken, State* state)
    {
      graph::ChordToken* c = dynamic_cast<graph::ChordToken*>(ChordToken);
      if(!c) return;

      Duration = c->Duration;
      State = state;
      OriginalNode = c;
      
      /**If a StringedInstrument exists, convert the notes of the chord to 
      tablature*/
      if (State->ActiveInstrument)
      {
        NumStrings = State->ActiveInstrument->GetStrings().n();

        prim::Array<graph::NoteNode*> a; 
        c->FindAll(a, graph::ID (mica::NoteLink));

        for (prim::count i = 0; i < a.n(); i++)
        {
          //If rest is detected, then stop importing.
          if(a[i]->Modifier == mica::Rest)
          {
            IsRest = true;
            TabNotes.RemoveAll(); //In case notes and rests were mixed.
            WrongNotes.RemoveAll();
            TabNotes.Add().LineSpace = (NumStrings % 2 ? 0 : 1);
            TabNotes.z().OriginalNode = a[i];
            break;
          }
    
          /**Calculate the mica::MIDIValue of the note and get the 
          stringIndex*/
          mica::UUID MidiNote = Utility::GetMIDIValueForNoteNumber(
            Utility::GetNoteNumberForNoteName(
            Utility::GetNoteName(State->ActiveClef, State->ActiveKey, 
            a[i]->Position, a[i]->Modifier)));

          prim::count StringIndex = a[i]->StringIndex;

          bool FoundValidPosition = false;

          /**If the string hasn't be used, check if the note can be played 
          on that string, and if so, add it*/
          if (!UsedStrings.Contains(StringIndex))
          {
            prim::count Fret = 
              State->ActiveInstrument->GetPostionOnStringForNote(
                StringIndex, MidiNote);

            if (Fret >= 0)
            {
              TabNotes.Add().MidiNote = MidiNote;
              TabNotes.z().StringIndex = StringIndex;
              TabNotes.z().Fret = Fret;
              TabNotes.z().LineSpace = Utility::GetLineSpaceForTabbedNote(
                TabNotes.z().StringIndex, NumStrings);
              TabNotes.z().OriginalNode = a[i];
              UsedStrings.Add(StringIndex);
              FoundValidPosition = true;
            }
          }

          if (!FoundValidPosition)
            WrongNotes.Add (a[i]);
        }
    
        //Attempt to map the WrongNotes to tablature
        prim::Array<graph::NoteNode*> FixedNotes;

        for (prim::count i = 0; i < WrongNotes.n(); ++i)
        {
          /**Calculate the mica::MIDIValue of the note and what strings it 
          can be played on*/
          mica::UUID MidiNote = Utility::GetMIDIValueForNoteNumber(
            Utility::GetNoteNumberForNoteName(
            Utility::GetNoteName(State->ActiveClef, State->ActiveKey, 
            WrongNotes[i]->Position, WrongNotes[i]->Modifier)));

          prim::Array<prim::count> AvailableStrings =
            State->ActiveInstrument->GetStringsAvailableForNote(MidiNote);

          // If any of the available strings haven't been used, add the note
          for (prim::count j = 0; j < AvailableStrings.n(); ++j)
          {
            if (!UsedStrings.Contains(AvailableStrings.ith (j)))
            {
              TabNotes.Add().MidiNote = MidiNote;
              TabNotes.z().StringIndex = AvailableStrings.ith (j);
              TabNotes.z().Fret = 
                State->ActiveInstrument->GetPostionOnStringForNote(
                TabNotes.z().StringIndex, MidiNote);
              TabNotes.z().LineSpace = 
                Utility::GetLineSpaceForTabbedNote(TabNotes.z().StringIndex, 
                NumStrings);
              TabNotes.z().OriginalNode = WrongNotes[i];
              UsedStrings.Add(TabNotes.z().StringIndex);
              FixedNotes.Add(WrongNotes[i]);
              break;
            }
          }
        }
    
        //Removed any notes that were fixed from WrongNotes
        for (int i = 0; i < FixedNotes.n(); ++i)
        {
          for (int j = 0; j < WrongNotes.n(); ++j)
          {
            if (WrongNotes[j] == FixedNotes[i])
            {
              WrongNotes.Remove (j);
              break;
            }
          }
        }

        if (WrongNotes.n() > 0) 
          prim::c << "Some notes were unable to be converted to tablature";
      }
    }
    
    ///Engrave the Tab
    void Engrave (Stamp& s, const House& h, const Cache& c, 
      const Typeface& t, const Font& f, bool EngraveRests = false,
      bool EngraveRhythm = false)
    {
      //If the structure is empty, then do not add anything to the stamp
      if(!TabNotes.n() && !IsRest) return;

      Path TabBounds;
      if (!IsRest) EngraveNumbers (s, TabBounds, h, c, f);
      if (EngraveRests) EngraveRest (s, TabBounds, h, t);
      if (EngraveRhythm && !IsRest)
      {
        EngraveStem (s, TabBounds, h);
        EngraveFlags (s, TabBounds, t);
        EngraveDots (s, h, c);
      }
    }
    
    ///Engrave the numbers of the Tab
    void EngraveNumbers(Stamp& s, Path& Bounds, const House& h, 
      const Cache& c, const Font& f)
    {
      prim::planar::Rectangle b;

      //Space around the number
      prim::number margin = h.TabSpaceHeightRatio / 4.0; 

      for (int i = 0; i < TabNotes.n(); ++i)
      {
        if (TabNotes[i].Fret >= 0)
        {
          //Calculate position
          prim::number VerticalPosition = Utility::GetLineSpacePosition(
            TabNotes[i].LineSpace, NumStrings, h) 
            * h.TabSpaceHeightRatio - margin;

          prim::number HorizontalPosition = 
            (TabNotes[i].Fret > 9 ? -(margin / 2.0) : 0) - margin;

          //Draw number text so we can get the bounds
          prim::String t = prim::String (TabNotes[i].Fret);
          Painter::Draw(s.Add().p, t, f, 72.0 * h.TabSpaceHeightRatio, 
            Font::Regular, Text::Justifications::Full);
          s.z().a = Affine::Translate(prim::planar::Vector(
            HorizontalPosition, VerticalPosition));

          prim::planar::Rectangle NumberBounds = s.z().Bounds();
          NumberBounds += prim::planar::Rectangle (
            NumberBounds.Left() - margin, VerticalPosition, 
            NumberBounds.Right() + margin, VerticalPosition);
                    
          //Add a white background 
          Shapes::AddRectangle(s.Add().p, NumberBounds);
          s.z().c = Colors::white;
          b += s.z().Bounds();

          //Redraw the number on top of the white background
          Painter::Draw(s.Add().p, t, f, 72.0 * h.TabSpaceHeightRatio, 
            Font::Regular, Text::Justifications::Full);
          s.z().a = Affine::Translate(prim::planar::Vector(
            HorizontalPosition, VerticalPosition));
          b += s.z().Bounds();

          s.z().n = TabNotes[i].OriginalNode;

          //Add the tab number bounds.
          Path p;
          Shapes::AddRectangle(p, b);
          Bounds.Append(p);
        }
      }
    }

    ///Engraves a rest.
    void EngraveRest(Stamp& s, Path& Bounds, const House& h, const Typeface& t)
    {
      //If the chord is not a rest, then rests do not pertain.
      if(!IsRest || TabNotes.n() != 1) return;
      
      //Determine the symbol to use.
      prim::Ratio Base = Utility::GetUndottedValue(Duration);      
      prim::count SymbolID = 0;
      if(Base == prim::Ratio(1, 1)) SymbolID = 79;
      else if(Base == prim::Ratio(1, 2)) SymbolID = 80;
      else if(Base == prim::Ratio(1, 4)) SymbolID = 81;
      else if(Base == prim::Ratio(1, 8)) SymbolID = 82;
      else if(Base == prim::Ratio(1, 16)) SymbolID = 83;
      else if(Base == prim::Ratio(1, 32)) SymbolID = 84;
      else if(Base == prim::Ratio(1, 64)) SymbolID = 85;
      else if(Base == prim::Ratio(1, 128)) SymbolID = 86;
      
      //If the duration is not supported then abort the rest engraving.
      if(!SymbolID)      
      return;
      
      //Create the graphic.
      s.Add().p2 = t.LookupGlyph(SymbolID);
      s.z().a = Affine::Translate(prim::planar::Vector(0.0,
      Utility::GetLineSpacePosition(
        TabNotes.a().LineSpace, NumStrings, h) * h.TabSpaceHeightRatio)) 
        * Affine::Scale(4.0);
      s.z().n = TabNotes.a().OriginalNode;
      
      //Update the bounding box.
      if(s.z().p2)
      Shapes::AddRectangle(Bounds, s.z().p2->Bounds(s.z().a));
    }
    
    ///Engrave the tab stem
    void EngraveStem (Stamp& s, Path& Bounds, const House& h)
    {
      if(!TabNotes.n() || Duration >= 1) return;

      prim::planar::Vector StemStart(
        Bounds.Bounds().Left() + (Bounds.Bounds().Width() / 2), 
          Bounds.Bounds().Bottom() - h.TabSpaceHeightRatio / 4.0);

      FlagPosition = prim::planar::Vector(StemStart.x, 
        Utility::GetLineSpacePosition(
        Utility::GetBottomLine (NumStrings), NumStrings, h) * 
        h.TabSpaceHeightRatio - h.TabStemHeight);

      if (Duration >= prim::Ratio(1, 2))
        StemStart.y = FlagPosition.y + (h.TabStemHeight / 2);

      Shapes::AddLine(s.Add().p, StemStart, FlagPosition, h.StemWidth);
      FlagPosition.x += (h.StemWidth / 2);
    }

    ///Engrave the tab flags
    void EngraveFlags (Stamp& s, Path& Bounds, const Typeface& t)
    {
      if(!TabNotes.n()) return;

      prim::planar::Rectangle b;

      prim::count Flags = Utility::CountFlags(Duration);
      prim::planar::Vector f = FlagPosition;

      for(prim::count i = 0; i < Flags; i++)
      {
        //Add the flag to the stamp.
        s.Add().p2 = t.LookupGlyph(87);
        s.z().a = Affine::Translate (f) * Affine::Scale(4.0);

        if(TabNotes.n() == 1)
          s.z().n = TabNotes.a().OriginalNode;
        else
          s.z().n = OriginalNode;
        
        //Flip the stem direction for stem down (flag up).
        s.z().a = s.z().a * Affine::Scale (prim::planar::Vector(1.0, -1.0));
        b += s.z().Bounds();
        
        /*Flag positions do not take into account the line-space position
        mapping. Instead, they are spaced equally by one space height.*/
        f.y += 1;

        //Add the flag bounds.
        Path p;
        Shapes::AddRectangle(p, b);
        Bounds.Append(p);
      }
    }
    
    ///Engrave the tab dots
    void EngraveDots (Stamp& s, const House& h, const Cache& c)
    {
      if(!TabNotes.n()) return;
      
      prim::count NumberOfDots = Utility::CountDots(Duration, 
        h.MaxDotsToConsider);
      prim::number StartX = h.RhythmicDotNoteheadDistance;
      prim::number StartY = FlagPosition.y + 1.0;

      for(prim::count j = 0; j < NumberOfDots; j++)
      {
        s.Add().p2 = c[Cache::RhythmicDot];
        s.z().a = Affine::Translate(prim::planar::Vector(
          StartX + (prim::number)j * h.RhythmicDotSpacing, StartY));
        s.z().n = OriginalNode;
      }
    }
    
    /*Describes the vertical position of a tabbed note by its
    StringIndex and Fret*/ 
    struct TabNote
    {
      ///The mica::MIDIValue of the note
      mica::UUID MidiNote;

      ///The string that the note is on
      int StringIndex;

      //The fret number of the note
      prim::count Fret;

      /**The line-space of the tab note. For example, 0 is the middle line 
      (or space if the staff has an even number of staff lines).*/
      prim::count LineSpace;

      ///Pointer back to the original note node
      graph::NoteNode* OriginalNode;
      
      ///Default constructor
      TabNote() : MidiNote(mica::Undefined), StringIndex(0), Fret(0), 
        LineSpace(0), OriginalNode(0)
      {}
      
      //-----------------//
      //Sorting operators//
      //-----------------//
      
      bool operator < (const TabNote& Other) const
      {
        return (LineSpace < Other.LineSpace) ||
          (LineSpace == Other.LineSpace && Fret < Other.Fret);
      }
      
      bool operator > (const TabNote& Other) const
      {
        return (LineSpace > Other.LineSpace) ||
          (LineSpace == Other.LineSpace && Fret > Other.Fret);
      }
  
      bool operator == (const TabNote& Other) const
      {
        return LineSpace == Other.LineSpace &&
          Fret == Other.Fret;
      }
      
      bool operator <= (const TabNote& Other) const
      {
        return *this < Other || *this == Other;
      }
      
      bool operator >= (const TabNote& Other) const
      {
        return *this > Other || *this == Other;
      }
    };

    ///List of Tab notes
    struct TabNoteList : public prim::Sortable::List<TabNote>
    {
      /**Removes duplicate adjacent tab notes. For this to be effective, the
      tab note list should be sorted beforehand.*/
      void RemoveDuplicateNotes()
      {
        for(prim::count i = n() - 1; i >= 1; i--)
          if(ith(i) == ith(i - 1))
            Remove(i);
      }
    };

    ///Number of strings available for this tab (StaffLines equivalent)
    int NumStrings;
    
    ///List of tabbed notes. 
    TabNoteList TabNotes;

    ///Array of used string indexes
    prim::Array<int> UsedStrings;

    /**Array of notes that were unable to be mapped to Tablature.
    This can happen if the string of the note is invalid or already 
    used and the note doesn't exist on any of the available strings*/
    prim::List<graph::NoteNode*> WrongNotes;

    ///The primary rhythmic type (1/8 = eighth for example)
    prim::Ratio Duration;

    ///Whether the chord is a rest.
    bool IsRest;

    ///Position of the flag received from the stem construction.
    prim::planar::Vector FlagPosition;
        
    ///Pointer back to the original chord token.
    graph::MusicNode* OriginalNode;

    /**Pointer to the current state (necessary in order to have 
    access to current StringedInstrument)*/
    State* State;
  };
}
}

#endif //BELLEBONNESAGE_MODERN_TAB_H
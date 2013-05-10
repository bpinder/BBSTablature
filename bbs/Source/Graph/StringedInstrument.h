#ifndef BELLEBONNESAGE_STRINGED_INSTRUMENT_H
#define BELLEBONNESAGE_STRINGED_INSTRUMENT_H

#include "Base.h"

namespace bellebonnesage { namespace graph
{
  /*Represents an instrument that has one or more strings.
  InstrumentType and StringNumber can be used to create
  generic guitar or bass instruments with a specific number
  of strings. Custom stringed instruments can be created by 
  calling RemoveAllStrings() after initialization and then 
  AddString() for every string on the custom instrument*/
  class StringedInstrument : public Token
  {
  public:
        
    ///The stringed instrument type.
    enum InstrumentType
    {
      GUITAR,
      BASS,
      NUM_INSTRUMENT_TYPES
    };

    ///The number of strings on the instrument
    enum StringNumber
    {
      FOUR_STRINGS = 4,
      FIVE_STRINGS = 5,
      SIX_STRINGS = 6,
      SEVEN_STRINGS = 7,
      EIGHT_STRINGS = 8,
    };

    ///The staff display setting
    enum StaffDisplaySetting
    {
      STANDARD,           //Notation
      TAB,                //Tablature
      STANDARD_AND_TAB,   //Notation and tablature
      NUM_DISPLAY_TYPES
    };

    ///Represents a string on an instrument
    struct InstrumentString
    {
      /*The pitch the string is tuned to. Currently a mica::MIDIValue
      to make string/fret calculations easier but when the new MICA 
      library is incorporated, should probably be changed to actual 
      mica note names for readability*/
      mica::UUID MidiNote; 

      /*How many semitones (or frets) are available for this string.
      If 0, only the open string is available*/
      prim::count Semitones;

      InstrumentString() :
        MidiNote (mica::MIDIValue0),
        Semitones (0) {}

      InstrumentString (mica::UUID _note, prim::count _semitones)
      {
        MidiNote = (_note == mica::Undefined ? mica::MIDIValue0 : _note);
        Semitones = (_semitones >= 0 ? _semitones : 0);
      }
    };

    ///StringedInstrument constructor
    StringedInstrument (InstrumentType type, StringNumber numStrings, 
      prim::count numSemitones, 
      StaffDisplaySetting displaySetting = STANDARD) :
        Token (mica::Sol),
        Type (type),
        DefaultNumStrings (numStrings),
        DisplaySetting (displaySetting)
    {
      NumSemitones = (numSemitones >= 0 ? numSemitones : 0);

      if (!IsStringNumberAvailableForInstrument (Type, DefaultNumStrings)) 
        DefaultNumStrings = GetDefaultNumStringsForInstrument (Type);

      InitializeStrings();
    }

    ///StringedInstrument constructor
    StringedInstrument() :
      Token (mica::Sol),
      Type (StringedInstrument::GUITAR),
      DefaultNumStrings (GetDefaultNumStringsForInstrument (Type)),
      NumSemitones (19),
      DisplaySetting (StringedInstrument::STANDARD) 
    {
      InitializeStrings();
    }

    ///StringedInstrument destructor
    ~StringedInstrument() {}

    /*Adds the appropriate strings based on the InstrumentType and
    number of strings*/
    void InitializeStrings()
    {
      RemoveAllStrings();

      switch (Type)
      {
        case StringedInstrument::GUITAR:
        {
          AddString (mica::MIDIValue64, NumSemitones);    //< E4
          AddString (mica::MIDIValue59, NumSemitones);    //< B3
          AddString (mica::MIDIValue55, NumSemitones);    //< G3
          AddString (mica::MIDIValue50, NumSemitones);    //< D3
          AddString (mica::MIDIValue45, NumSemitones);    //< A2
          AddString (mica::MIDIValue40, NumSemitones);    //< E2

          if (DefaultNumStrings > 6) 
            AddString (mica::MIDIValue35, NumSemitones);  //< B1
          if (DefaultNumStrings > 7)
            AddString (mica::MIDIValue30, NumSemitones);  //< GFlat1
          break;
        }
        case StringedInstrument::BASS:
        {
          if (DefaultNumStrings > 5) 
            AddString (mica::MIDIValue48, NumSemitones);  //< C3

          AddString (mica::MIDIValue43, NumSemitones);    //< G2
          AddString (mica::MIDIValue38, NumSemitones);    //< D2
          AddString (mica::MIDIValue33, NumSemitones);    //< A1
          AddString (mica::MIDIValue28, NumSemitones);    //< E1

          if (DefaultNumStrings > 4) 
            AddString (mica::MIDIValue23, NumSemitones);  //< B0
          if (DefaultNumStrings > 6) 
            AddString (mica::MIDIValue18, NumSemitones);  //< GFlat0
          break;
        }
      };
    }

    ///Returns the instrument type
    InstrumentType GetInstrumentType() const 
    { 
      return Type;
    }

    ///Sets the instrument type
    void SetInstrumentType (InstrumentType NewType) 
    { 
      Type = NewType; 
    }

    ///Returns the default number of strings
    StringNumber GetDefaultNumberOfStrings() const 
    { 
      return DefaultNumStrings; 
    }

    ///Sets the default number of strings 
    bool SetDefaultNumberOfStrings (StringNumber NewNumStrings)
    {
      if (IsStringNumberAvailableForInstrument (Type, NewNumStrings))
      {
        DefaultNumStrings = NewNumStrings;
        InitializeStrings();
        return true;
      }

      return false;
    }

    ///Returns the number of semitones
    prim::count GetNumSemitones() const 
    { 
      return NumSemitones; 
    }

    ///Sets the number of semitones 
    void SetNumSemitones (prim::count NewNumSemitones) 
    { 
      NumSemitones = (NewNumSemitones >= 0 ? NewNumSemitones : 0);
    }

    ///Returns the instrument's strings 
    const prim::List<InstrumentString>& GetStrings() const 
    { 
      return Strings;
    }

    ///Sets the tuning of a certain string
    void SetNoteOfString (prim::count StringIndex, mica::UUID Note)
    {
      if (StringIndex >= 0 && StringIndex < Strings.n() 
        && Note != mica::Undefined)
          Strings.ith (StringIndex).MidiNote = Note;
    }

    ///Sets the number of semitones (frets) available for a certain string
    void SetSemitonesOfString (prim::count StringIndex, 
      prim::count numberOfSemitones)
    {
      if (StringIndex >= 0 && StringIndex < Strings.n() 
        && numberOfSemitones >= 0)
          Strings.ith (StringIndex).Semitones = numberOfSemitones;
    }

    ///Adds a string to the instrument
    void AddString (mica::UUID Note, prim::count Semitones)
    {
      Strings.Add (StringedInstrument::InstrumentString (Note, Semitones));
    }

    ///Removes the string at the provided index
    void RemoveString (prim::count StringIndex)
    {
      if (StringIndex >= 0 && StringIndex < Strings.n())
        Strings.Remove (StringIndex);
    }

    ///Removes all strings from the instrument 
    void RemoveAllStrings() 
    { 
      Strings.RemoveAll(); 
    }
        
    ///Returns the staff display setting
    StaffDisplaySetting GetDisplaySetting() const 
    { 
      return DisplaySetting; 
    }

    ///Sets the staff display setting 
    void SetDisplaySetting (StaffDisplaySetting NewDisplaySetting) 
    { 
      DisplaySetting = NewDisplaySetting; 
    }

    /**Returns the position on a string for a given note 
    (in semitones/frets). Returns negative if the note doesn't exist 
    on the string*/
    prim::count GetPostionOnStringForNote (prim::count StringIndex, 
      mica::UUID Note)
    {
      if (StringIndex >= 0 && StringIndex < Strings.n() 
        && Note != mica::Undefined)
      {
        prim::count OpenString = mica::index (mica::MIDIValues, 
          Strings.ith (StringIndex).MidiNote, mica::MIDIValue0);

        prim::count NoteMidi = mica::index (mica::MIDIValues, Note, 
          mica::MIDIValue0);

        if ((NoteMidi >= OpenString) && 
          NoteMidi <= (OpenString + Strings.ith (StringIndex).Semitones))
            return (NoteMidi - OpenString);
      }
      return -1;
    }

    /**Returns all the string indexes that the provided note exists on.
    Returns an empty array if the note doesn't exist on any string.
    If HighestToLowest is true, string indexes will be in order from highest 
    to lowest*/
    prim::Array<prim::count> GetStringsAvailableForNote (mica::UUID Note, 
      bool HighestToLowest = true)
    {
      prim::Array<prim::count> AvailableStrings;

      for (prim::count i = 0; i < (HighestToLowest ? Strings.n() : 
        Strings.n() + 1); ++i)
      {
        prim::count StringIndex = (HighestToLowest ? i : Strings.n() - i);

        if (GetPostionOnStringForNote (StringIndex, Note) >= 0)
          AvailableStrings.Add (StringIndex);
      }

      return AvailableStrings;
    }

    ///Returns true if the number of strings is available for the instrument
    static bool IsStringNumberAvailableForInstrument (
      StringedInstrument::InstrumentType Type, 
      StringedInstrument::StringNumber Number)
    {
      switch (Type)
      {
        case StringedInstrument::GUITAR:
        {
          switch (Number)
          {
            case StringedInstrument::SIX_STRINGS:
            case StringedInstrument::SEVEN_STRINGS:
            case StringedInstrument::EIGHT_STRINGS:
              return true;
            default: return false;
          };
        }
        case StringedInstrument::BASS:
        {
          switch (Number)
          {
            case StringedInstrument::FOUR_STRINGS:
            case StringedInstrument::FIVE_STRINGS:
            case StringedInstrument::SIX_STRINGS:
            case StringedInstrument::SEVEN_STRINGS:
              return true;
            default: return false;
          };
        }
      };

      return false;
    }

    ///Returns the default number of strings for the given instrument type
    static StringedInstrument::StringNumber GetDefaultNumStringsForInstrument (
      StringedInstrument::InstrumentType Type)
    {
      switch (Type)
      {
        case StringedInstrument::GUITAR: 
          return StringedInstrument::SIX_STRINGS;
        case StringedInstrument::BASS: 
          return StringedInstrument::FOUR_STRINGS;
      }

      return StringedInstrument::FOUR_STRINGS;
    }

  private:

    ///The Instrument type
    InstrumentType Type;

    ///The default number of strings on the instrument
    StringNumber DefaultNumStrings;

    ///The number of semitones (or frets) on the instrument
    prim::count NumSemitones;
    
    ///The instrument's strings
    prim::List<InstrumentString> Strings;

    ///The staff display setting
    StaffDisplaySetting DisplaySetting;
  };
}
}

#endif  // BELLEBONNESAGE_STRINGED_INSTRUMENT_H
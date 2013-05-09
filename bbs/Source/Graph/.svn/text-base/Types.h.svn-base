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

#ifndef BELLEBONNESAGE_GRAPH_TYPES_H
#define BELLEBONNESAGE_GRAPH_TYPES_H

#include "Base.h"

namespace bellebonnesage { namespace graph
{
  //Forward declarations
  struct ChordToken;

  //-----//
  //Notes//
  //-----//

  ///Node that stores a note.
  struct NoteNode : public MusicNode
  {
    ///Stores a cleffed note, a note which needs a clef to be identified.
    mica::UUID Position;
    mica::UUID Modifier;
    bool Locked;
    ///Constructor to set the node type.
    NoteNode() : MusicNode(mica::Note), Position(mica::Undefined),
                 Modifier(mica::Undefined), Locked(false) {}
    
    ///Virtual destructor.
    virtual ~NoteNode() {}
    
    ///Returns the nodes parent chord.
    ChordToken* Parent();
    
    ///Returns the parent island.
    Island* ParentIsland();
    
    ///Returns whether the note is the beginning of a tie.
    NoteNode* FollowTieForwards();
    
    ///Returns whether the note is the end of a tie.
    NoteNode* FollowTieBackwards();
    
    /**Determines the tied duration of the note. If the note does not start the
    tie, then it returns the remaining duration.*/
    prim::Ratio RemainingTiedDuration();
    
    /**Transposes the note given the clef context and an interval. If this fails
    then the note is left unchanged and false is returned.*/
    bool Transpose(mica::UUID ClefContext,
      mica::UUID Interval, mica::UUID Direction)
    {
      mica::UUID Pitch = mica::map(ClefContext, Position, Modifier);
      Pitch = mica::map(Pitch, Interval, Direction);
      mica::UUID NewPosition = mica::map(Pitch, mica::TonalNote, ClefContext);
      mica::UUID NewModifier = mica::map(Pitch, mica::Accidental);
      if(NewPosition != mica::Undefined && NewModifier != mica::Undefined)
      {
        Position = NewPosition;
        Modifier = NewModifier;
        return true;
      }
      else
      {
        Position = mica::LS0;
        Modifier = mica::Natural;
        return false;
      }      
    }
    
    ///Returns the position as an integer.
    prim::count GetPosition()
    {
      int p = index(mica::LineSpaces, Position, mica::LS0);
      if(p == mica::Undefined)
        return 0;
      else
        return p;
    }
    
    /**Collects all articulation markings among tied neighbor notes. This will
    only search forwards.*/
    void CollectArticulationsAmongTiedNeighbors(prim::Array<mica::UUID>& Marks);
    
    ///Returns accidental and line space of the note.
    virtual prim::String ToString()
    {
      prim::String s;
      s << mica::M(Position).Name() << ":" << mica::M(Modifier).Name();
      return s;
    }
    
    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::Note);
        return;
      }
      SerializeProperties(s, Mode);
      MusicSerial::DoMICA(s, Position, Mode);
      MusicSerial::DoMICA(s, Modifier, Mode);
      s.Do(Locked,Mode);
      prim::Debug >> ToString();
    }
  };
  
  //----------//
  //Properties//
  //----------//
  
  struct Property : public MusicNode
  {
    ///Stores the property.
    mica::UUID Value;
    
    ///Default constructor
    Property() : MusicNode(mica::Property) {}
    
    ///Adds this property to a node.
    Property(MusicNode* n, mica::Concept Value) : MusicNode(mica::Property),
      Value(Value)
    {
      n->AddLink(this, ID(mica::PropertyLink));
    }
    
    virtual ~Property() {}
    
    virtual prim::String ToString() {return (const prim::ascii*)Value;}
    
    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::Property);
        return;
      }
      SerializeProperties(s, Mode);
      MusicSerial::DoMICA(s, Value, Mode);
      prim::Debug >> ToString();
    }
  };
  
  //------//
  //Tokens//
  //------//

  ///Object in an island.
  struct Token : public MusicNode
  {
    ///Constructor that initializes a node as a token.
    Token(mica::Concept c) : MusicNode(c) {}
    
    ///Virtual destructor.
    virtual ~Token() {}
    
    ///Returns a string indicating this is a token of unknown subtype.
    virtual prim::String ToString() {return "(Token)";}
    
    ///Enumerates the curent possible token types -- should go to MICA.
    enum TokenTypes {Empty, Barline, Chord, Clef, KeySignature, Meter};

    ///Custom callback for out-of-library subclasses.
    virtual void CustomCallback(void* Data1, void* Data2) {Data1 = Data2 = 0;}
    
    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::Token);
        return;
      }
      SerializeProperties(s, Mode);
      prim::Debug >> ToString();
    }
  };

  ///Token that stores a barline.
  struct BarlineToken : public Token
  {
    ///Stores the barline.
    mica::UUID Value;
    
    ///Constructor to initialize class.
    BarlineToken() : Token(mica::BarlineToken), Value(mica::Undefined) {}
    
    ///Virtual destructor.
    virtual ~BarlineToken() {}
    
    ///Returns a string indicating the type of barline.
    prim::String ToString()
    {
      return mica::M(Value).Name();
    }

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::BarlineToken);
        return;
      }
      SerializeProperties(s, Mode);
      MusicSerial::DoMICA(s, Value, Mode);
      prim::Debug >> ToString();
    }
  };

  /**Token that stores a chord. This type of token further links to note nodes
  to identify individual notes.*/
  struct ChordToken : public Token
  {
    /**One-based voice assignment for giving to the analysis. If the ID is zero,
    then no voice assignment has been made.*/
    prim::count CachedVoiceID;
    
    ///Stores the duration of the chord.
    prim::Ratio Duration;
    
    ///Stores the beat of the chord onset within the current measure.
    prim::Ratio Beat;
    
    /**Caches the duration of the instant this chord occupies. The instant
    duration is less than or equal to the chord duration.*/
    prim::Ratio InstantDuration;
    
    ///Default destructor
    ChordToken() : Token(mica::ChordToken), CachedVoiceID(0) {}
    
    ///Get notes of chord.
    prim::Array<bellebonnesage::graph::NoteNode*> GetNotes()
    {
      prim::Array<bellebonnesage::graph::NoteNode*> Nodes;
      FindAll(Nodes, ID(mica::NoteLink));
      return Nodes;
    }
    
    ///Virtual destructor.
    virtual ~ChordToken() {}
    
    ///Returns a string indicating the notes in the chord.
    prim::String ToString()
    {
      prim::String s;
      s << Duration.ToString() << " @ " << Beat.ToString();
      prim::Array<bellebonnesage::graph::NoteNode*> Notes = GetNotes();
      for(prim::count i = 0; i < Notes.n(); i++)
          s << " " << Notes[i]->ToString();
      return s;
    }

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::ChordToken);
        return;
      }
      SerializeProperties(s, Mode);
      s.Do(Duration, Mode);
      s.Do(Beat, Mode);
      s.Do(InstantDuration, Mode);
      prim::Debug >> ToString();
    }
  };

  ///Token storing a clef.
  struct ClefToken : public Token
  {
    ///Stores the clef.
    mica::UUID Value;
    
    ///Constructor to initialize class.
    ClefToken() : Token(mica::ClefToken), Value(mica::Undefined) {}
    
    ///Virtual destructor.
    virtual ~ClefToken() {}
    
    ///Returns a string indicating the name of the clef.
    prim::String ToString() {return mica::M(Value).Name();}

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::ClefToken);
        return;
      }
      SerializeProperties(s, Mode);
      MusicSerial::DoMICA(s, Value, Mode);
      prim::Debug >> ToString();      
    }
  };

  ///Token storing a key or key signature.
  struct KeySignatureToken : public Token
  {
    ///Stores the key (key signature + mode) if that information is available.
    mica::UUID Key;
    
    ///Stores the key signature as a fallback when there is no modal intent.
    mica::UUID KeySignature;
    
    ///Initializes the token to empty values.
    KeySignatureToken() : Token(mica::KeySignatureToken), Key(mica::Undefined),
      KeySignature(mica::Undefined) {}
    
    ///Gets the key signature.
    mica::UUID GetKeySignature()
    {
      /*If there is a tonal key, then return key signature for that key,
      otherwise fallback to an explicit key signature.*/
      if(Key != mica::Undefined)
        return mica::map(Key, mica::KeySignature);
      else
        return KeySignature;
    }
    
    ///Gets the key (given a particular fallback mode if no mode is identified).
    mica::UUID GetKey(mica::UUID FallbackMode = mica::Major)
    {
      /*If there is a tonal key, then return key. Otherwise assume the key
      signature is in the mode given by the parameter.*/
      if(Key != mica::Undefined)
        return Key;
      else
        return mica::map(KeySignature, FallbackMode);
    }
    
    ///Transpose the key signature.
    bool Transpose(mica::UUID Interval, mica::UUID Direction)
    {
      mica::UUID Mode = mica::map(Key, mica::Mode);
      mica::UUID OriginalKeySignature = GetKeySignature();
      if(Mode == mica::Undefined)
      {
        //Use key signature.
        KeySignature = mica::map(OriginalKeySignature, Interval, Direction);
      }
      else
      {
        //Use key (with mode).
        KeySignature = mica::Undefined;
        Key = mica::map(OriginalKeySignature, Interval, Direction, Mode);
      }
      return Key != mica::Undefined || KeySignature != mica::Undefined;
    }
    
    ///Virtual destructor.
    virtual ~KeySignatureToken() {}
    
    ///Returns a string indicating the name of the key signature.
    prim::String ToString()
    {
      return mica::M(GetKey()).Name();
    }

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::KeySignatureToken);
        return;
      }
      SerializeProperties(s, Mode);
      MusicSerial::DoMICA(s, Key, Mode);
      MusicSerial::DoMICA(s, KeySignature, Mode);
      prim::Debug >> ToString();      
    }
  };

  ///Token storing a meter.
  struct MeterToken : public Token
  {
    ///Stores the meter.
    mica::UUID Value;
    
    ///Default constructor.
    MeterToken() : Token(mica::MeterToken) {}
    
    ///Virtual destructor.
    virtual ~MeterToken() {}
    
    ///Returns a string indicating the name of meter.
    prim::String ToString() {return mica::M(Value).Name();}

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::MeterToken);
        return;
      }
      SerializeProperties(s, Mode);
      MusicSerial::DoMICA(s, Value, Mode);
      prim::Debug >> ToString();      
    }
  };

  //------//
  //Floats//
  //------//

  ///Float node base class.
  struct Float : public MusicNode
  {
    ///Default constructor sets the node type to float.
    Float() : MusicNode(mica::Float) {}
    
    ///Virtual destructor.
    virtual ~Float() {}
    
    ///Returns a string indicating this is a float of unknown subtype.
    virtual prim::String ToString() {return "(Float)";}

    protected: 
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::Float);
        return;
      }
      SerializeProperties(s, Mode);
      prim::Debug >> ToString();
    }
  };

  ///Tie span which connects two notes together.
  struct TieSpan : public Float
  {
    ///Virtual destructor.
    virtual ~TieSpan() {}
    
    ///Returns a string indicating this is a tie span.
    virtual prim::String ToString() {return "Tie";}
    
    ///Returns the note before.
    virtual bool Notes(NoteNode*& Left, NoteNode*& Right)
    {
      //Initialize outputs.
      Left = Right = 0;
      
      /*It is not possible to assume in a float which link is first. Therefore
      the parent token continuity will be traced.*/
      prim::Array<Node*> ParentNodes;
      FindAll(ParentNodes, ID(mica::FloatLink),
        prim::Link::Directions::Backwards);

      /*Find the parent tokens. Store both linked chords and notes, so that the
      information does not need to be rederived.*/
      prim::List<ChordToken*> ParentChords;
      prim::List<NoteNode*> ParentNotes;
      for(prim::count i = 0; i < ParentNodes.n(); i++)
      {
        NoteNode* p = dynamic_cast<NoteNode*>(ParentNodes[i]);
        if(!p) continue;
        
        if(ChordToken* a = dynamic_cast<ChordToken*>(p->Find(
          ID(mica::NoteLink), prim::Link::Directions::Backwards)))
        {
          ParentChords.Add() = a;
          ParentNotes.Add() = p;
        }
      }
      
      /*If there are not exactly two chords under consideration, then there is
      a problem with the graph.*/
      if(ParentChords.n() != 2)
        return false;
      
      //Look for a continuity match.
      ChordToken* a = ParentChords.a(), *z = ParentChords.z();
      
      for(prim::count i = 0; i < a->GetLinkCount(); i++)
      {
        prim::Link l = a->GetLink(i);
        
        if(l.Label != ID(mica::ContinuityLink))
          continue;
        
        if(l.x == a && l.y == z)
        {
          Left = ParentNotes.a();
          Right = ParentNotes.z();
        }
        
        if(l.x == z && l.y == a)
        {
          Left = ParentNotes.z();
          Right = ParentNotes.a();
        }
      }
      
      //If the match was found then return the pair.
      if(Left && Right)
        return true;
      else
      {
        Left = Right = 0;
        return false;
      }
    }

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::TieSpan);
        return;
      }
      SerializeProperties(s, Mode);
      prim::Debug >> ToString();
    }
  };

  ///Float node that stores a marking such as a fermata.
  struct MarkFloat : public Float
  {
    ///Stores a marking.
    mica::UUID Value;
    
    ///Virtual destructor.
    virtual ~MarkFloat() {}
    
    ///Returns a string indicating the type of marking.
    virtual prim::String ToString()
    {
      return mica::M(Value).Name();
    }

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::MarkFloat);
        return;
      }
      SerializeProperties(s, Mode);
      MusicSerial::DoMICA(s, Value, Mode);
      prim::Debug >> ToString();
    }
  };

  //--------//
  //Markings//
  //--------//

  ///Node that stores a marking such as an articulation.
  struct Marking : public MusicNode
  {
    ///Construct to set the node type.
    Marking() : MusicNode(mica::Marking) {}
    
    ///Virtual destructor.
    virtual ~Marking() {}
    
    ///Returns a string indicating this is a marking of unknown subtype.
    virtual prim::String ToString() {return "(Marking)";}

    protected: 
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::Marking);
        return;
      }
      SerializeProperties(s, Mode);
      prim::Debug >> ToString();
    }
  };

  ///Marking that stores an articulation.
  struct ArticulationMarking : public Marking
  {
    ///Stores the marking.
    mica::UUID Value;
    
    ///Returns a string indicating the type of articulation.
    prim::String ToString() {return mica::M(Value).Name();}

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::ArticulationMarking);
        return;
      }
      SerializeProperties(s, Mode);
      MusicSerial::DoMICA(s, Value, Mode);
      prim::Debug >> ToString();
    }
  };

  //--------//
  //Metadata//
  //--------//

  /**Node that stores key value information. This base class only stores the
  key. It is up to subclasses to provide specific value types to be stored.*/
  struct MetadataKey : public MusicNode
  {
    ///Stores the metadata key.
    prim::String Key;
    
    ///Default constructor just sets the node type to metadata.
    MetadataKey() : MusicNode(mica::Metadata) {}
    
    ///Constructor to set the key.
    MetadataKey(prim::String Key) : MusicNode(mica::Metadata), Key(Key)
    {
    }
    
    ///Virtual destructor
    virtual ~MetadataKey() {}
    
    ///Returns what kind of metadata this stores.
    virtual prim::String ToString()
    {
      prim::String s;
      s << Key << " metadata";
      return s;
    }
    
    ///Checks for a particular key.
    bool operator == (const prim::String& Other) const
    {
      return Other == Key;
    }
    
    ///Checks for a particular key.
    bool operator == (const prim::ascii*& Other) const
    {
      return Other == Key;
    }

    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::Metadata);
        return;
      }
      SerializeProperties(s, Mode);
      s.Do(Key, Mode);
      prim::Debug >> ToString();
    }
  };

  ///Templated metadata subclass that stores arbitrary keyed-pair metadata.
  template<typename ValueType>
  struct KeyedPairMetadata : public MetadataKey
  {
    ///Stores the value.
    ValueType Value;
    
    ///Default constructor does nothing.
    KeyedPairMetadata() {}
    
    ///Key-value constructor initializes the metadata.
    KeyedPairMetadata(prim::String Key, ValueType Value) : MetadataKey(Key),
      Value(Value) {}
    
    ///Virtual destructor.
    virtual ~KeyedPairMetadata() {}
    
    ///Returns what kind of metadata this stores.
    virtual prim::String ToString()
    {
      prim::String s;
      s << Key << ": " << Value;
      return s;
    }

#if 0 //Deprecated
    ///Dynamically determine the particular metadata type.
    prim::count GetSubtype()
    {
      /*if(dynamic_cast<KeyedPairMetadata<menc::Instrument>*>(this))
        return mica::InstrumentMetadata;
      else*/
      if(dynamic_cast<KeyedPairMetadata<prim::String>*>(this))
        return mica::Metadata;
      prim::c >> "Error: Subtype not defined in Graph/Types.h";
      return 0;
    }
#endif
    
    protected:
    
    ///Serializes this node.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion)
        return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = ID(mica::Metadata);
        return;
      }
      SerializeProperties(s, Mode);
      s.Do(Key, Mode);
      s.Do(Value, Mode);
      prim::Debug >> ToString();
    }
  };
}}

#ifdef BELLEBONNESAGE_COMPILE_INLINE
namespace bellebonnesage { namespace graph
{
  ChordToken* NoteNode::Parent()
  {
    ChordToken* c = 0;
    Find(c, ID(mica::NoteLink), prim::Link::Directions::Backwards);
    return c;
  }
  
  Island* NoteNode::ParentIsland()
  {
    ChordToken* ct = Parent();
    if(!ct)
    {
      prim::c >> "Error: NoteNode has no parent";
      return 0;
    }
    
    Island* i = 0;
    
    if(!ct->Find(i, ID(mica::TokenLink), prim::Link::Directions::Backwards))
    {
      prim::c >> "Error: NoteNode has no parent island";
      return 0;
    }
    
    return i;
  }
  
  NoteNode* NoteNode::FollowTieForwards()
  {
    prim::Array<TieSpan*> Ties;
    FindAll(Ties, ID(mica::FloatLink));
    for(prim::count i = 0; i < Ties.n(); i++)
    {
      NoteNode* x = 0, *y = 0;
      if(Ties[i]->Notes(x, y))
        if(x == this)
          return y;
    }
    return 0;
  }
  
  NoteNode* NoteNode::FollowTieBackwards()
  {
    prim::Array<TieSpan*> Ties;
    FindAll(Ties, ID(mica::FloatLink));
    for(prim::count i = 0; i < Ties.n(); i++)
    {
      NoteNode* x = 0, *y = 0;
      if(Ties[i]->Notes(x, y))
        if(y == this)
          return x;
    }
    return 0;
  }
  
  prim::Ratio NoteNode::RemainingTiedDuration()
  {
    prim::Ratio Total = 0;
    NoteNode* CurrentNote = this;
    while(CurrentNote)
    {
      ChordToken* ParentChord = Parent();
      if(!ParentChord)
        break;
      Total += ParentChord->Duration;
      CurrentNote = CurrentNote->FollowTieForwards();
    }
    return Total;
  }
  
  void NoteNode::CollectArticulationsAmongTiedNeighbors(
    prim::Array<mica::UUID>& Marks)
  {
    Marks.Clear();
    NoteNode* CurrentNote = this;
    while(CurrentNote)
    {
      ChordToken* ParentChord = CurrentNote->Parent();
      if(!ParentChord)
        return;
      
      prim::Array<ArticulationMarking*> Markings;
      ParentChord->FindAll(Markings, ID(mica::MarkingLink));
      for(prim::count i = 0; i < Markings.n(); i++)
        Marks.Add() = Markings[i]->Value;
      
      CurrentNote = CurrentNote->FollowTieForwards();
    }
  }

  prim::Serial::Object* MusicSerial::RestoreObject(prim::UUID x)
  {
    mica::UUID T = ID(x);
    if(T == mica::Island)
      return new Island;
    else if(T == mica::Note)
      return new NoteNode;
    else if(T == mica::BarlineToken)
      return new BarlineToken;
    else if(T == mica::ChordToken)
      return new ChordToken;
    else if(T == mica::ClefToken)
      return new ClefToken;
    else if(T == mica::KeySignatureToken)
      return new KeySignatureToken;
    else if(T == mica::MeterToken)
      return new MeterToken;
    else if(T == mica::TieSpan)
      return new TieSpan;
    else if(T == mica::MarkFloat)
      return new MarkFloat;
    else if(T == mica::ArticulationMarking)
      return new ArticulationMarking;
    else if(T == mica::Metadata)
      return new KeyedPairMetadata<prim::String>;
    else if(T == mica::Property)
      return new Property;
    else if(T == mica::AnalysisToken)
    {
      //Do nothing about the analysis token for now.
    }
    else
      prim::c >> "Error: unknown music object '" << T <<
        "' encountered during serialization";
    return 0;
  }
}}
#endif
#endif

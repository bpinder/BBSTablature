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

#ifndef BELLEBONNESAGE_MODERN_ISLAND_H
#define BELLEBONNESAGE_MODERN_ISLAND_H

#include "Directory.h"

#include "Barline.h"
#include "Beaming.h"
#include "Chord.h"
#include "Tab.h"
#include "Clef.h"
#include "Custom.h"
#include "KeySignature.h"
#include "Meter.h"
#include "State.h"

namespace bellebonnesage { namespace modern
{
  ///Algorithms for typesetting an island.
  struct IslandEngraver : public DirectoryHandler
  {     
    ///Constructor to assign external resources to this engraver.
    IslandEngraver(Directory& d) : DirectoryHandler(d) {}
    
    ///Engraves the island.
    void Engrave(graph::MusicNode* n, Stamp& s, bool isOnExtraStaff = false)
    {
      //Get all the tokens belonging to the island.
      prim::Node::Array<graph::Token> Tokens(n, graph::ID(mica::TokenLink));
      prim::Array<graph::Token*> TokenArray;
      n->FindAll(TokenArray, graph::ID(mica::TokenLink));
      UpdateStemState(TokenArray);

      //Displace rests.
      bellebonnesage::graph::ChordToken* ShiftLeft = 0;
      if(d.s.Current.n() == 2)
      {
        Chord::StateInfo* Bottom, *Top;
        if(d.s.Current[0].d == Chord::StateInfo::Down)
        {
          Bottom = &d.s.Current[0];
          Top = &d.s.Current[1];
        }
        else
        {
          Bottom = &d.s.Current[1];
          Top = &d.s.Current[0];
        }

        prim::Array<bellebonnesage::graph::NoteNode*> BottomNotes =
          Bottom->c->GetNotes();
        prim::Array<bellebonnesage::graph::NoteNode*> TopNotes =
          Top->c->GetNotes();
        
        if(Bottom->IsRest && Top->IsRest)
        {
          BottomNotes.a()->Position = mica::LS_4;
          TopNotes.a()->Position = mica::LS2;
        }
        else if(Bottom->IsRest && !Top->IsRest)
        {
          BottomNotes.a()->Position = mica::item(mica::LineSpaces,
            mica::LS0, Top->Lowest - 5);
          if(mica::index(mica::LineSpaces, BottomNotes.a()->Position, mica::LS0) > 0)
            BottomNotes.a()->Position = mica::LS0;
        }
        else if(!Bottom->IsRest && Top->IsRest)
        {
          TopNotes.a()->Position = mica::item(mica::LineSpaces,
            mica::LS0, Bottom->Highest + 5);
          if(mica::index(mica::LineSpaces, TopNotes.a()->Position, mica::LS0) < 0)
            TopNotes.a()->Position = mica::LS0;
        }
        else
        {
          //Should also look at color...
          if(Top->Lowest - 2 >= Bottom->Highest ||
            Bottom->Lowest - 2 >= Top->Highest)
          {
            ShiftLeft = 0;
          }
          else
          {
            ShiftLeft = Bottom->c;
          }
        }
      }
      
      //Engrave each token.
      for(prim::count i = 0; i < Tokens.n(); i++)
      {
        prim::count Start = s.Graphics.n();
        EngraveToken(Tokens[i], s, isOnExtraStaff);
        
        if(Tokens[i] == ShiftLeft)
          for(prim::count j = Start; j < s.Graphics.n(); j++)
            s.Graphics[j]->a = (Affine::Translate(
              prim::planar::Vector(-1.4, 0.0)) * s.Graphics[j]->a);
      }
    }
    
    ///Updates the current stem state.
    void UpdateStemState(prim::Array<graph::Token*>& Tokens)
    {
      //Get references to the stem states.
      Chord::State& Current = d.s.Current;
      Chord::State& Previous = d.s.Previous;
      
      //Grab all the chord tokens.
      Current.n(0);
      bool ContainsChords = false;
      for(prim::count i = 0; i < Tokens.n(); i++)
      {
        if(graph::ChordToken* ct = dynamic_cast<graph::ChordToken*>(Tokens[i]))
        {
          ContainsChords = true;
          Current.Add().c = ct;
          Current.z().UpdateTessitura();
          Current.z().c->Find<graph::ChordToken>(Current.z().p,
            graph::ID(mica::ContinuityLink), prim::Link::Directions::Backwards);
        }
        else
        {
          Current.Add().c = 0;
          Current.z().Tessitura = 999.0;
        }
      }
      
      //If there are no chords in the island, then there is nothing to process.
      if(!ContainsChords) return;
      
      //Sort from bottom to top.
      Current.Sort();
      
      //Determine the number of new voices.
      prim::count NewVoices = 0;
      for(prim::count i = 0; i < Current.n(); i++)
      {
        Current[i].NewVoice = true;
        NewVoices++;
        if(graph::ChordToken* Needle = Current[i].p)
        {
          for(prim::count j = 0; j < Previous.n(); j++)
          {
            if(Needle == Previous[j].c)
            {
              Current[i].NewVoice = false;
              Current[i].pd = Previous[j].d;
              NewVoices--;
              break;
            }
          }
        }
      }
      
      //By default use same stem arrangements as previous island.
      for(prim::count i = 0; i < Current.n(); i++)
        Current[i].d = Current[i].pd;
          
      //Determine stems.
      if((Previous.n() == 1 && Current.n() > 1) || NewVoices)
      {
        /*There are some new voices and some preexisting voices. Any voices
        which already exist use their existing stem arrangements.*/
        
        //Alternate directions stem directions starting with down on low voice.
        for(prim::count i = 0; i < Current.n(); i++)
        {
          if((Previous.n() == 1 && Current.n() > 1) || Current[i].NewVoice)
          {
            Current[i].d =
              (i % 2 == 0 ? Chord::StateInfo::Down : Chord::StateInfo::Up);
          }
        }
      }
      
      //Recycle stem state.
      if(d.s.Current.n())
        d.s.Previous = d.s.Current;
    }

    ///Engraves the token.
    void EngraveToken(graph::Token* Token, Stamp& s, bool isOnExtraStaff = false)
    {
      if (graph::PartToken* pt = dynamic_cast<graph::PartToken*> (Token))
      {
        graph::StringedInstrument* si = 
          dynamic_cast<graph::StringedInstrument*>(pt->Find (graph::ID (mica::TokenLink)));
        if (si) 
          d.s.ActiveInstrument = si;
        else 
          d.s.ActiveInstrument = 0;
      }
      else if(graph::ChordToken* ct = dynamic_cast<graph::ChordToken*>(Token))
      {
        if (d.s.IsTabStaff() || (d.s.IsStandardAndTabStaff() && isOnExtraStaff))
        {
          Tab TabInfo;
          TabInfo.Import (ct, &d.s);
          TabInfo.Engrave (s, d.h, d.c, d.t, d.f, d.s.IsTabStaff(), d.s.IsTabStaff());
        }
        else
        {
          Chord ChordInfo;
          ChordInfo.Import(ct);

          /*Need to detect any unisons or octaves with different accidentals and
          clear their state if they have differing accidentals. Also octaves
          with same accidentals need to report accidentals if not in active
          state.*/
        
          //Determine the accidental state.
          for(prim::count j = 0; j < ChordInfo.StaffNotes.n(); j++)
          {
            Chord::StaffNote& sn = ChordInfo.StaffNotes[j];
            sn.Accidental =
              d.s.ConsumeAccidental(sn.LineSpace, sn.Accidental);
          }
        
          //Read in the stem state.
          //ChordInfo.DetermineStemDirectionByPosition();

          prim::count k = -1;
          for(prim::count i = 0; i < d.s.Current.n(); i++)
          {
            if(d.s.Current[i].c == ct)
            {
              k = i;
              break;
            }
          }
          if(k >= 0)
          {
            if(d.s.Current[k].d == Chord::StateInfo::PositionBased)  
            {
              ChordInfo.DetermineStemDirectionByPosition();
              prim::c >> "Position based used..";
            }
            else
              ChordInfo.StemUp = d.s.Current[k].d == Chord::StateInfo::Up;
          }
          else
          {
            prim::c >> "ChordState not found";
          }
        
          //Engrave the chord.
          ChordInfo.Engrave(s, d.h, d.c, d.t, d.f);
        }
      }
      else if(graph::ClefToken* ct = dynamic_cast<graph::ClefToken*>(Token))
      {
        prim::number ClefSize = d.s.ActiveClef == mica::Undefined ?
          1.0 : d.h.NonInitialClefSize;
        ClefSize = 1.0; //For now this is hacked after the fact.
        d.s.ActiveClef = ct->Value;

        Clef::Engrave(d, s, d.s.ActiveClef, ClefSize, isOnExtraStaff);
      }
      else if(graph::KeySignatureToken* kt =
        dynamic_cast<graph::KeySignatureToken*>(Token))
      {
        mica::UUID k = kt->GetKeySignature();
        d.s.ActiveKey = kt->GetKey();
        d.s.SetKeySignature(k);
        d.s.ResetActiveAccidentalsToKeySignature();
        KeySignature::Engrave(d, s, kt, isOnExtraStaff);
      }
      else if(graph::MeterToken* mt = dynamic_cast<graph::MeterToken*>(Token))
      {
        Meter::Engrave(d, s, mt, isOnExtraStaff);
      }
      else if(graph::BarlineToken* bt =
        dynamic_cast<graph::BarlineToken*>(Token))
      {
        Barline::Engrave(d, s, bt, isOnExtraStaff);
        d.s.ResetActiveAccidentalsToKeySignature();
      }
      else
      {
        Custom::Engrave(Token, s, d);
      }
    }
  };
}}
#endif

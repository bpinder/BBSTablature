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

#ifndef BELLEBONNESAGE_GRAPH_XML_H
#define BELLEBONNESAGE_GRAPH_XML_H

namespace bellebonnesage { namespace graph
{
  ///Static methods for reading and writing a graph to XML.
  struct XML
  {
    ///Reads a music graph from XML.
    static bool Read(MusicGraph* mg, const prim::String& In)
    {
      return Read(*mg, In);
    }
    
    ///Reads a music graph from XML.
    static bool Read(MusicGraph& mg, const prim::String& In)
    {
      //Clear the graph.
      mg.Clear();
      
      //Read in the document.
      prim::XML::Document d;
      if(!ReadDocument(d, In)) 
          return false;
      
      //Create a node table.
      prim::Table<prim::String, ElementNode> NodeTable;
      
      //Go through each object in the score.
      const prim::List<prim::XML::Object*> Objects = d.Root->GetObjects();
      for(prim::count i = 0; i < Objects.n(); i++)
      {
        //Skip over any objects that are not elements.
        prim::XML::Element* e = Objects[i]->IsElement();
        if(!e) continue;
        
        //Get the ID.
        prim::String NodeID = e->GetAttributeValue("id");
        if(!NodeID)
        {
          prim::c >> "Warning: " << e->GetName() <<
            " score node with no id attribute. Node will be ignored.";
          continue;
        }
        
        if(e->GetName() == "island")
        {
          //Create a new island and read the island's children.
          ElementNode el(e, new Island);
          NodeTable[NodeID] = el;
          ReadIsland(el, NodeTable);
          
          //Set the top to the first island encountered.
          if(!mg.GetTop())
            mg.SetTop(NodeTable[NodeID].n);
        }
      }
      
      //Go through all the nodes in the table and establish the links.
      for(prim::count i = 0; i < NodeTable.n(); i++)
      {
        ElementNode en = NodeTable.ith(i);
        prim::XML::Element* e = en.e;
        MusicNode* n = en.n;
        
        //Connect the islands by part-wise and instant-wise links.
        if(e->GetName() == "island")
        {
          if(MusicNode* Across = NodeTable[e->GetAttributeValue("across")].n)
            n->AddLink(Across, ID(mica::PartWiseLink));
          if(MusicNode* Down = NodeTable[e->GetAttributeValue("down")].n)
            n->AddLink(Down, ID(mica::InstantWiseLink));
        }
        else if(e->GetName() == "chord")
        {
          //Add the continuity and voice links.
          if(prim::String Next = e->GetAttributeValue("next"))
          {
            if(MusicNode* nn = NodeTable[Next].n)
            {
              n->AddLink(nn, ID(mica::ContinuityLink));
              n->AddLink(nn, ID(mica::VoiceLink));
            }
            else
              prim::c >> "Warning: next specified with unknown id: " << Next
                << ". Ignoring continuity and voice link.";
          }
          else if(prim::String Next = e->GetAttributeValue("next-in-voice"))
          {
            if(MusicNode* nn = NodeTable[Next].n)
              n->AddLink(nn, ID(mica::VoiceLink));
            else
              prim::c >> "Warning: next-in-voice specified with unknown id: " <<
                Next << ". Ignoring voice link.";
          }
        }
        else if(e->GetName() == "note")
        {
          if(prim::String TiedID = e->GetAttributeValue("tied-to"))
          {
            if(MusicNode* TiedTo = NodeTable[TiedID].n)
            {
              TieSpan* ts = new TieSpan;
              n->AddLink(ts, ID(mica::FloatLink));
              TiedTo->AddLink(ts, ID(mica::FloatLink));
            }
            else
            {
              prim::c >> "Warning: tie specified with unknown id: " <<
                TiedID << ". Ignoring tie.";
            }
          }
        }
      }

      return true;
    }
    
    ///Writes a music graph to XML.
    static void Write(prim::String& Out, MusicGraph* mg) {Write(Out, *mg);}
    
    ///Writes a music graph to XML.
    static void Write(prim::String& Out, MusicGraph& mg)
    {
      //Parsing the graph first allows us to write out more useful node ids.
      Geometry Geo;
      Geo.Parse(mg);
      mg.SetCustomDataToIDs();
      
      //Clear the output before writing to it
      Out.Clear();
      
      Out >> "<score>";
      for(prim::count i = 0; i < Geo.GetNumberOfParts(); i++)
      {
        for(prim::count j = 0; j < Geo.GetNumberOfInstants(); j++)
        {
          Island* Isle = Geo(i, j);
          if(!Isle) continue;
          
          Out >> "  <island id='" << Isle->UniqueID() << "'";
          
          //Write part-wise and instant-wise links.
          {
            Island* t = 0;
            if(Isle->Find<Island>(t, ID(mica::PartWiseLink)))
              Out << " across='" << t->UniqueID() << "'";
            if(Isle->Find<Island>(t, ID(mica::InstantWiseLink)))
              Out << " down='" << t->UniqueID() << "'";
          }
          
          Out << ">";
          
          //Write tokens.
          prim::Array<Token*> Tokens;
          Isle->FindAll(Tokens, ID(mica::TokenLink));
          for(prim::count k = 0; k < Tokens.n(); k++)
          {
            Token* t = Tokens[k];
            
            //Get short-form type.
            prim::String Type(ID(t->GetType()));
            Type.Replace("Token", "");
            Type.Replace("Signature", "");
            Type.Replace("Do", "part");
            Type = Type.ToLower();
            
            //Write the header to the token.
            if(Type == "part" || Type == "barline" || Type == "chord" || 
              Type == "key" || Type == "meter" || Type == "clef")
                Out >> "    <" << Type << " id='" << t->UniqueID() << "'";
            
            if (PartToken* pt = dynamic_cast<PartToken*>(t))
            {
              //Write Stringed Instrument
              if (StringedInstrument* si = 
                dynamic_cast<StringedInstrument*> (pt->Find (ID (mica::TokenLink))))
              {
                Out << ">";
                Out >> "      <stringInstr id='" << si->UniqueID() << 
                  "' type='" << si->GetInstrumentType() << 
                  "' strings='" << si->GetDefaultNumberOfStrings() << 
                  "' semitones='" << si->GetNumSemitones() << 
                  "' display='" << si->GetDisplaySetting() << "'>";

                const prim::List<StringedInstrument::InstrumentString>& strings = 
                  si->GetStrings();
                for (int s = 0; s < strings.n(); ++s)
                   Out >> "        <string note='" << strings.ith (s).MidiNote << 
                     "' semitones='" << strings.ith (s).Semitones << "'/>";

                Out >> "      </stringInstr>";
                Out >> "    </part>";
              }
              else 
                Out << "/>";
            }
            else if(BarlineToken* bt = dynamic_cast<BarlineToken*>(t))
            {
              Out << " value='" << bt->Value << "'/>";
            }
            else if(ChordToken* ct = dynamic_cast<ChordToken*>(t))
            {
              ChordToken* next_ct = 0;
              if(ct->Find<ChordToken>(next_ct, ID(mica::ContinuityLink)))
                Out << " next='" << next_ct->UniqueID() << "'";
              else if(ct->Find<ChordToken>(next_ct, ID(mica::VoiceLink)))
                Out << " next-in-voice='" << next_ct->UniqueID() << "'";
              Out << " duration='" << ct->Duration << "'";
              Out << " beat='" << ct->Beat << "'";
              Out << " instant='" << ct->InstantDuration << "'>";
              
              //Write notes.
              prim::Array<NoteNode*> Notes;
              ct->FindAll(Notes, ID(mica::NoteLink));
              
              for(prim::count l = 0; l < Notes.n(); l++)
              {
                NoteNode* n = Notes[l];
                prim::String NoteID = n->UniqueID();
                //Out >> "      <note";
                Out >> "      <note id='" << n->UniqueID() << "'";
                Out << " position='" << n->Position << "'";
                Out << " modifier='" << n->Modifier << "'";
                
                if(NoteNode* TiedNote = n->FollowTieForwards())
                  Out << " tied-to='" << TiedNote->UniqueID() << "'";
                
                Out << "/>";
              }
              Out >> "    </chord>";
            }
            else if(ClefToken* clt = dynamic_cast<ClefToken*>(t))
            {
              Out << " value='" << clt->Value << "'/>";
            }
            else if(KeySignatureToken* kt = dynamic_cast<KeySignatureToken*>(t))
            {
              if(kt->GetKey() != mica::Undefined)
                Out << " key='" << kt->GetKey() << "'/>";
              else
                Out << " key-signature='" << kt->GetKeySignature() << "'/>";
            }
            else if(MeterToken* mt = dynamic_cast<MeterToken*>(t))
            {
              Out << " value='" << mt->Value << "'/>";
            }
          }
          
          Out >> "  </island>";
        }
      }
      Out >> "</score>";
      
      mg.ClearCustomData();
    }
    
    private:
    
    ///Binds an XML element to a corresponding music node.
    struct ElementNode
    {
      prim::XML::Element* e;
      
      MusicNode* n;
      
      ElementNode() : e(0), n(0) {}
      
      ElementNode(prim::XML::Element* e, MusicNode* n = 0) : e(e), n(n) {}
      
      bool operator == (const ElementNode& Other) const {return e == Other.e;}

      bool operator != (const ElementNode& Other) const {return e != Other.e;}
      
      operator prim::String ()
      {
        prim::String s;
        if(n)
          s << n->ToString();
        else
          s << "(uninitialized node)";
        return s;
      }
    };
    
    static bool ReadDocument(prim::XML::Document& d, const prim::String& In)
    {
      //Attempt to read in the XML.
      if(prim::XML::Parser::Error e = d.ParseDocument(In))
      {
        prim::c >> e.GetDescription();
        return false;
      }
      
      //Make sure there is a root node.
      if(!d.Root)
      {
        prim::c >> "Error: no root node.";
        return false;
      }
      
      return true;
    }
    
    static void ReadChord(ElementNode& Chord,
      prim::Table<prim::String, ElementNode>& NodeTable)
    {
      //Go through each object in the score.
      const prim::List<prim::XML::Object*> Objects = Chord.e->GetObjects();
      for(prim::count i = 0; i < Objects.n(); i++)
      {
        //Skip over any objects that are not elements.
        prim::XML::Element* e = Objects[i]->IsElement();
        if(!e) continue;
        
        //Get the ID and name.
        prim::String NodeID = e->GetAttributeValue("id");
        prim::String NodeName = e->GetName();
        
        //Create a synthetic ID if one does not exist.
        if(!NodeID)
          NodeID = prim::UUID();
        
        //Create the element node.
        ElementNode en(e);
        
        if(NodeName == "note")
        {
          NoteNode* nn = new NoteNode;
          en.n = nn;
          nn->Position = mica::named(e->GetAttributeValue("position"));
          nn->Modifier = mica::named(e->GetAttributeValue("modifier"));
          Chord.n->AddLink(nn, ID(mica::NoteLink));
        }
        else
        {
          prim::c >> "Warning: unrecognized node type '" << NodeName << "'. "
            "Node will be ignored.";
        }
        
        //Add the element node to the table.
        NodeTable[NodeID] = en;
      }
    }

    static void ReadStringedInstrument (ElementNode& en, PartToken* part)
    {
      //Find the Stringed Instrument element
      const prim::List<prim::XML::Object*> partObjects = en.e->GetObjects();

      for (int p = 0; p < partObjects.n(); ++p)
      {
        prim::XML::Element* siE = partObjects[p]->IsElement();

        if (siE)
        {
          if (siE->GetName() == "stringInstr")
          {
            StringedInstrument* si = new StringedInstrument();
                    
            //Find the instrument type
            prim::count typeIndex = 
              prim::String (siE->GetAttributeValue ("type")).ToNumber();
            if (typeIndex >= 0 && 
              typeIndex < StringedInstrument::NUM_INSTRUMENT_TYPES)
                si->SetInstrumentType(
                  (StringedInstrument::InstrumentType) typeIndex);

            //Find the number of strings
            prim::count numStrings = 
              prim::String (siE->GetAttributeValue ("strings")).ToNumber();
            if (numStrings >= 4 && numStrings <= 8)
              si->SetDefaultNumberOfStrings (
                (StringedInstrument::StringNumber) numStrings);

            //Find the number of semitones (frets)
            prim::count semitones = 
              prim::String (siE->GetAttributeValue ("semitones")).ToNumber();
            if (semitones >= 0) 
              si->SetNumSemitones (semitones);

            //Find the display setting
            prim::count displayIndex = 
              prim::String (siE->GetAttributeValue ("display")).ToNumber();
            if (displayIndex >= 0 && 
              displayIndex < StringedInstrument::NUM_DISPLAY_TYPES)
              si->SetDisplaySetting (
                (StringedInstrument::StaffDisplaySetting) displayIndex);

            //Find all string elements
            const prim::List<prim::XML::Object*> siObjects = siE->GetObjects();

            if (siObjects.n() > 0) si->RemoveAllStrings();

            for (int s = 0; s < siObjects.n(); ++s)
            {
              prim::XML::Element* stringE = siObjects[s]->IsElement();

              //Add the strings to the StringedInstrument
              if (stringE)
              {
                if (stringE->GetName() == "string")
                {
                  mica::UUID note = mica::named (
                    stringE->GetAttributeValue ("note"));
                  prim::count semitones = prim::String (
                    stringE->GetAttributeValue ("semitones")).ToNumber();

                  if (note != mica::Undefined && semitones >= 0)
                    si->AddString (note, semitones);
                }
              }
            }

            // Link the StringedInstrument to the part
            part->AddLink (si, ID (mica::TokenLink));
          }
        }
      }
    }
    
    static void ReadIsland(ElementNode& Isle,
      prim::Table<prim::String, ElementNode>& NodeTable)
    {
      //Go through each object in the score.
      const prim::List<prim::XML::Object*> Objects = Isle.e->GetObjects();
      for(prim::count i = 0; i < Objects.n(); i++)
      {
        //Skip over any objects that are not elements.
        prim::XML::Element* e = Objects[i]->IsElement();
        if(!e) continue;
        
        //Get the ID and name.
        prim::String NodeID = e->GetAttributeValue("id");
        prim::String NodeName = e->GetName();
        
        //Make sure an ID exists.
        if(NodeName == "chord" && !NodeID)
        {
          prim::c >> "Warning: chord node with no id attribute. "
            "Node will be ignored.";
          continue;
        }
        else if(!NodeID)
        {
          //Create a synthetic ID.
          NodeID = prim::UUID();
        }
        
        ElementNode en(e);
        if(NodeName == "part")
        {
          PartToken* pt = new PartToken;
          en.n = pt;
          Isle.n->AddLink(pt, ID (mica::TokenLink));
          ReadStringedInstrument (en, pt);
        }
        else if(NodeName == "clef")
        {
          ClefToken* ct = new ClefToken;
          en.n = ct;
          ct->Value = mica::named(e->GetAttributeValue("value"));
          Isle.n->AddLink(ct, ID(mica::TokenLink));
        }
        else if(NodeName == "barline")
        {
          BarlineToken* bt = new BarlineToken;
          en.n = bt;
          bt->Value = mica::named(e->GetAttributeValue("value"));
          Isle.n->AddLink(bt, ID(mica::TokenLink));
        }
        else if(NodeName == "meter")
        {
          MeterToken* mt = new MeterToken;
          en.n = mt;
          mt->Value = mica::named(e->GetAttributeValue("value"));
          Isle.n->AddLink(mt, ID(mica::TokenLink));
        }
        else if(NodeName == "key")
        {
          KeySignatureToken* kt = new KeySignatureToken;
          en.n = kt;
          if(prim::String Value = e->GetAttributeValue("key"))
            kt->Key = mica::named(Value);
          else if(prim::String Value = e->GetAttributeValue("key-signature"))
            kt->KeySignature = mica::named(Value);
          Isle.n->AddLink(kt, ID(mica::TokenLink));
        }
        else if(NodeName == "chord")
        {
          //Create the chord token and link it to the island.
          ChordToken* ct = new ChordToken;
          en.n = ct;
          Isle.n->AddLink(ct, ID(mica::TokenLink));
          
          //Set rhythmic information.
          if(prim::String Value = e->GetAttributeValue("duration"))
            ct->Duration = Value;
          if(prim::String Value = e->GetAttributeValue("beat"))
            ct->Beat = Value;
          if(prim::String Value = e->GetAttributeValue("instant"))
            ct->InstantDuration = Value;
            
            
          //Read the chord elements.
          ReadChord(en, NodeTable);
        }
        else
        {
          prim::c >> "Warning: unrecognized node type '" << NodeName << "'. "
            "Node will be ignored.";
        }
        
        //Add the element node to the table.
        NodeTable[NodeID] = en;
      }
    }
  };
}}
#endif

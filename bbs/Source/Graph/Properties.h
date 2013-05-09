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

#ifndef BELLEBONNESAGE_GRAPH_PROPERTIES_H
#define BELLEBONNESAGE_GRAPH_PROPERTIES_H

namespace bellebonnesage { namespace graph
{
  ///Static structure to help assign instant properties.
  struct Instant
  {
    struct Properties : public prim::Array<mica::Concept>
    {
      bool IsSystemBreak() const {return Contains(mica::SystemBreak);}
      bool IsOptionalBreak() const {return Contains(mica::OptionalBreak);}
      bool IsRepeatingInstant() const {return Contains(mica::RepeatingInstant);}
      bool IsAbsorbedInstant() const {return Contains(mica::AbsorbedInstant);}
      
      void SetSystemBreak() {if(!IsSystemBreak()) Add(mica::SystemBreak);}
      void SetOptionalBreak() {if(!IsOptionalBreak()) Add(mica::OptionalBreak);}
      void SetRepeatingInstant()
      {
        if(!IsRepeatingInstant()) Add(mica::RepeatingInstant);
      }
      void SetAbsorbedInstant()
      {
        if(!IsAbsorbedInstant()) Add(mica::AbsorbedInstant);
      }
      
      prim::String ToString() const
      {
        prim::String s;
        s >> "System break? " << (IsSystemBreak() ? "yes" : "no");
        s >> "Optional break? " << (IsOptionalBreak() ? "yes" : "no");
        s >> "Repeating instant? " << (IsRepeatingInstant() ? "yes" : "no");
        s >> "Absorbed instant? " << (IsAbsorbedInstant() ? "yes" : "no");
        return s;
      }
    };
    
    /**Sets default properties on score. This method does make a few assumptions
    about the geometry of the score, mostly that it is relatively well-behaved
    as regards the initial material and the appearances of barlines.*/
    static void SetDefaultProperties(MusicGraph& g)
    {
      MusicNode* n = dynamic_cast<MusicNode*>(g.GetTop());
      bool InitialMaterial = true;
      bool BarlineAppeared = false;
      bool ClefAppeared = false;
      bool KeySignatureAppeared = false;
      while(n)
      {
        graph::Token* m = 0;
        if(!n->Find<graph::Token>(m, graph::ID(mica::TokenLink)))
        {
          n->Find<graph::MusicNode>(n, graph::ID(mica::PartWiseLink));
          continue;
        }
        
        graph::Instant::Properties p;

        if(InitialMaterial)
        {
          if(graph::ID(m->GetType()) == mica::BarlineToken && !BarlineAppeared)
          {
            p.SetRepeatingInstant();
            BarlineAppeared = true;
          }
          else if(graph::ID(m->GetType()) == mica::ClefToken && !ClefAppeared)
          {
            p.SetRepeatingInstant();
            ClefAppeared = true;
          }
          else if(graph::ID(m->GetType()) == mica::KeySignatureToken &&
            !KeySignatureAppeared)
          {
            p.SetRepeatingInstant();
            KeySignatureAppeared = true;
          }
          else
            InitialMaterial = false;
        }
        else
        {
          if(graph::ID(m->GetType()) == mica::BarlineToken)
            p.SetOptionalBreak();
        }

        graph::Instant::SetProperties(p, n);
        n->Find<graph::MusicNode>(n, graph::ID(mica::PartWiseLink));
      }
    }
    
    ///Get properties on the instant.
    static void GetProperties(Properties& p, MusicNode* n)
    {
      //Go to the top node of the part before retrieving the properties.
      if(!API::RaiseToTopPart(n))
        return;
      
      //Follow the property links to get the property nodes off this island.
      prim::Array<graph::Property*> PropertyNodes;
      n->FindAll<graph::Property>(PropertyNodes, ID(mica::PropertyLink));
      
      //Copy the properties from the nodes.
      p.n(PropertyNodes.n());
      for(prim::count i = 0; i < PropertyNodes.n(); i++)
        p[i] = PropertyNodes[i]->Value;
    }
    
    ///Clear all properties on the instant.
    static void ClearProperties(MusicNode* n)
    {
      //Go to the top node of the part before retrieving the properties.
      if(!API::RaiseToTopPart(n))
        return;
      
      //Follow the property links to get the property nodes off this island.
      prim::Array<graph::Property*> PropertyNodes;
      n->FindAll<graph::Property>(PropertyNodes, ID(mica::PropertyLink));
      PropertyNodes.ClearAndDeleteAll();
    }
    
    ///Clears all instant properties on the graph.
    static void ClearProperties(MusicGraph& g)
    {
      MusicNode* n = dynamic_cast<MusicNode*>(g.GetTop());
      while(n)
      {
        ClearProperties(n);
        n->Find<graph::MusicNode>(n, graph::ID(mica::PartWiseLink));
      }
    }
    
    ///Set properties on the instant.
    static void SetProperties(const Properties& p, MusicNode* n)
    {
      //Clear properties first.
      ClearProperties(n);
      
      //Go to the top node of the part before retrieving the properties.
      if(!API::RaiseToTopPart(n))
        return;
      
      //Create the properties adding them to the current node.
      for(prim::count i = 0; i < p.n(); i++)
        new graph::Property(n, p[i]);
    }
    
    ///Prints the properties for the instant.
    static void PrintProperties(MusicNode* n)
    {
      Properties p;
      GetProperties(p, n);
      prim::c >> p.ToString();
    }
  };
}}
#endif

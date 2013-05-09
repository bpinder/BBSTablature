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

#ifndef BELLEBONNESAGE_GRAPH_TRANSFORMATION_H
#define BELLEBONNESAGE_GRAPH_TRANSFORMATION_H

namespace bellebonnesage { namespace graph
{
  ///Various graph transformations.
  struct Transformation
  {
    static bool Transpose(MusicGraph& g,
      mica::UUID Interval, mica::UUID Direction)
    {
      //Keep track of whether the transposition works perfectly.
      bool Success = true;
      
      //Gather all the nodes.
      prim::Array<prim::Node*> Nodes;
      prim::Array<prim::Link*> Links;
      g.Gather(Nodes, Links);

      //Rip through the nodes and transpose the transposable items.
      for(prim::count i = 0; i < Nodes.n(); i++)
      {
        if(NoteNode* nn = dynamic_cast<NoteNode*>(Nodes[i]))
        {
          //Get the parent chord.          
          ChordToken* ct = 0;
          nn->Find(ct, ID(mica::NoteLink), prim::Link::Directions::Backwards);
          
          //Get the parent island.
          MusicNode* is = 0;
          ct->Find(is, ID(mica::TokenLink), prim::Link::Directions::Backwards);
          
          /*Look backwards for the clef of the note. Note this is not exactly
          ideal because it could be looking back a long way. However it is
          resilient to atypical graph geometries.*/
          mica::UUID FoundClef = mica::Undefined;
          while(is)
          {
            ClefToken* clt = 0;
            is->Find(clt, ID(mica::TokenLink));
            if(clt)
            {
              FoundClef = clt->Value;
              break;
            }
            is->Find(is, ID(mica::PartWiseLink),
              prim::Link::Directions::Backwards);
          }
          
          //Check to see whether a clef was found.
          if(FoundClef == mica::Undefined)
          {
            Success = false;
            continue;
          }
          
          //Tranpose the note.
          if(!nn->Transpose(FoundClef, Interval, Direction))
            Success = false;
        }
        else if(KeySignatureToken* kt =
          dynamic_cast<KeySignatureToken*>(Nodes[i]))
        {
          //Tranpose the key signature.
          if(!kt->Transpose(Interval, Direction))
          {
            Success = false;
          }
        }
      }
      return Success;
    }
  };
}}
#endif

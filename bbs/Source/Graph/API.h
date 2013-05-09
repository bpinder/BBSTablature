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

#ifndef BELLEBONNESAGE_GRAPH_API_H
#define BELLEBONNESAGE_GRAPH_API_H

namespace bellebonnesage { namespace graph
{
  struct API
  {  
    /**Returns whether the node is an island. In order for this to be the case,
    there must be at least one of the following: a forwards or backwards part-wise
    or instant-wise link, or a forwards token link. An empty island with no tokens
    all alone is an orphan and is not typesettable and so is not considered an
    island.*/
    static bool IsIsland(MusicNode* n)
    {
      return (dynamic_cast<Island*>(n) != 0);
#if 0 //This code was context-based instead of type-based.
      //Some temporary variables for storing the result of the search.
      MusicNode* a = 0;
      Token* t = 0;
      
      //If the node is null, then return immediately.
      if(!n)
        return false;
      
      //Test according to the rules given in the comment above.
      if(n->Find<MusicNode>(a, ID(mica::PartWiseLink),
        prim::Link::Directions::Forwards))
          return true;
      else if(n->Find<MusicNode>(a, ID(mica::PartWiseLink),
        prim::Link::Directions::Backwards))
          return true;
      else if(n->Find<MusicNode>(a, ID(mica::InstantWiseLink),
        prim::Link::Directions::Forwards))
          return true;
      else if(n->Find<MusicNode>(a, ID(mica::InstantWiseLink),
        prim::Link::Directions::Backwards))
          return true;
      else if(n->Find<Token>(t, ID(mica::TokenLink),
        prim::Link::Directions::Forwards))
          return true;
      
      return false;
#endif
    }
    
    /**Moves a node to the top-most part in the instant. If a null pointer is
    sent or if the given node is not an island, but some other type of node,
    then the node is changed to null. If the node returned is not null, then it
    returns true, and otherwise false.*/
    static bool RaiseToTopPart(MusicNode*& n)
    {
      //Make sure the node passed in is an island.
      if(!IsIsland(n))
      {
        n = 0;
        return false;
      }
  
      //Go to the top part of the instant.
      MusicNode* m = 0;
      while(n->Find<MusicNode>(m, ID(mica::InstantWiseLink),
        prim::Link::Directions::Backwards))
          n = m;
  
      return true;
    }
  };
}}
#endif

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

#ifndef BELLEBONNESAGE_GRAPH_GEOMETRY_H
#define BELLEBONNESAGE_GRAPH_GEOMETRY_H

namespace bellebonnesage { namespace graph
{
  ///Static structure to help with part identification.
  class Geometry
  {
    public: //methods
    
    ///Returns the number of parts detected.
    prim::count GetNumberOfParts()
    {
      return PartCount;
    }
    
    ///Returns the number of instants detected.
    prim::count GetNumberOfInstants()
    {
      return InstantCount;
    }
    
    ///Returns the instant range for a given part.
    prim::planar::VectorInt GetPartRange(prim::count i)
    {
      return PartInstantRange[i];
    }
    
    /**Goes through the island subgraph and determines its geometry. When debug
    mode is enabled, a print-out of transitive relationships is shown.*/
    void Parse(MusicGraph& mg, bool DebugMode = false)
    {
      AssignPartIDs(mg, DebugMode);
      AssignInstantIDs(mg);
      AssignAccessors();
    }
    
    ///Gets the part list for a given instant.
    void GetPartListForInstant(prim::count InstantID,
      prim::List<prim::count>& PartList)
    {
      PartList.RemoveAll();
      for(prim::count i = 0; i < PartInstantRange.n(); i++)
        if(PartInstantRange[i].i() <= InstantID &&
          InstantID <= PartInstantRange[i].j())
            PartList.Add() = i;
    }
    
    ///Returns the topmost island in the instant.
    Island* TopMostIslandInInstant(prim::count InstantID)
    {
      graph::Island* Isle = 0;
      for(prim::count i = 0; i < GetNumberOfParts(); i++)
        if((Isle = LookupIsland(i, InstantID)))
          break;
      return Isle;
    }
    
    ///Determines whether the instant is complete.
    bool IsInstantComplete(prim::count InstantID)
    {
      prim::List<prim::count> l;
      GetPartListForInstant(InstantID, l);
      return l.n() == PartsInInstant[InstantID];
    }
    
    ///Returns the number of parts detected for each instant.
    prim::count GetPartsInInstant(prim::count InstantID)
    {
      return PartsInInstant[InstantID];
    }
    
    ///Looks up an island by part and instant IDs.
    Island* LookupIsland(prim::count PartID, prim::count InstantID)
    {
      return IslandMatrix(PartID, InstantID);
    }
    
    ///Looks up an island by part and instant IDs.
    Island* operator () (prim::count PartID, prim::count InstantID)
    {
      return IslandMatrix(PartID, InstantID);
    }
    
    private: //members
    
    ///Contains subgraph of islands.
    prim::Array<Island*> Islands;
    
    ///Number of parts detected.
    prim::count PartCount;
    
    ///Number of instants detected.
    prim::count InstantCount;
      
    ///Contains bounds of parts as indexed into the Islands array.
    prim::Array<prim::Complex<Island*> > PartBounds;
    
    ///Ranges of the parts in terms of their instant IDs.
    prim::Array<prim::planar::VectorInt> PartInstantRange;
    
    ///Number of parts in each instant.
    prim::Array<prim::count> PartsInInstant;
    
    ///Accessor for island using instant by part.
    prim::Matrix<Island*> IslandMatrix;
    
    private: //methods
    
    ///Assigns accessors for reverse lookups.
    void AssignAccessors()
    {
      IslandMatrix.mn(PartCount, InstantCount);
      IslandMatrix.Zero();      
      for(prim::count i = 0; i < Islands.n(); i++)
      {
        prim::count Instant = Islands[i]->Typesetting->InstantID;
        prim::count Part = Islands[i]->Typesetting->PartID;
        IslandMatrix(Part, Instant) = Islands[i];
      }
    }
    
    /**Assigns part IDs to the island subgraph. They are assigned such that the
    minimum number of IDs are used and the part IDs ascend along instant-wise
    links. If the graph was created from an IslandGrid, then the pointer to that
    object can be passed in to help with debugging.*/
    void AssignPartIDs(MusicGraph& mg,  bool DebugMode = false)
    {
      //Gather the islands
      GatherIslands(mg);
      
      //Mark the part strands.
      PartCount = MarkPartStrands();
      
      //Create the rule array.
      TransitiveMapping t(PartCount);
        
      //Observe all part relationships and store them in a rule matrix.
      ObservePartOrders(t);
      
      if(DebugMode) prim::c >> t;
      
      //Solve for the transitive closure and produce the new part mapping.
      t.Solve();
      
      //Check to see whether the graph is in a conflicted state.
      if(t.IsConflicted())
      {
        prim::c >> "Error: the graph has conflicting island relationships such";
        prim::c >> " as crossing staves. Aborting.";
        return;
      }
      
      if(DebugMode) prim::c >> t;
      
      //Create part map from the transitive mapping.
      prim::Array<prim::count> PartMap;
      PartMap.n(PartCount);
      PartMap.Zero();
      for(prim::count i = 0; i < PartCount; i++)
        PartMap[t.Mapping(i)] = i;
      
      //Go through each island and map its part.
      for(prim::count i = 0; i < Islands.n(); i++)
        Islands[i]->Typesetting->PartID =
          PartMap[Islands[i]->Typesetting->PartID];
      
      //Mark the part bounds.
      MarkPartBounds();
    }
    
    ///Helper to convert array to list.
    template <class T> static void ArrayToList(
      prim::Array<T>& a, prim::List<T>& b)
    {
      b.RemoveAll();
      for(prim::count i = 0; i < a.n(); i++)
        b.Add() = a[i];
    }
    
    ///Assigns ordered instant IDs using the leading edge algorithm.
    void AssignInstantIDs(MusicGraph& mg,  bool DebugMode = false)
    {
      //Get the top.
      Island* t = dynamic_cast<Island*>(mg.GetTop());
      prim::Array<Island*> FirstInstant;
      prim::List<Island*> LeadingEdge;
      
      //Gather the islands to the first instant.
      t->FindAll(FirstInstant, ID(mica::InstantWiseLink),
        prim::Link::Directions::Forwards, true);
      ArrayToList(FirstInstant, LeadingEdge);
      
      //Define leading edge for the first instant.
      for(prim::count i = 0; i < LeadingEdge.n(); i++)
        LeadingEdge[i]->Typesetting->InstantID = 0;
      
      //Define part count for first instant.
      PartsInInstant.Clear();
      PartsInInstant.Add() = LeadingEdge.n();

      //Initialize the first non-initial instant ID.
      prim::count InstantID = 1;
      
      //Iterate while there is a leading edge.
      while(LeadingEdge.n())
      {
        //Rotate through the leading edge looking for edges to push further.
        for(prim::count i = 0; i < LeadingEdge.n(); i++)
        {
          //Get the next island.
          Island* NextIsland = 0;
          LeadingEdge[i]->Find(NextIsland, ID(mica::PartWiseLink));
          
          //If there is no next island, then remove this part entry.
          if(!NextIsland)
          {
            LeadingEdge.Remove(i--);
            continue;
          }
          
          //Get the instant group of the next island being tried.
          prim::Array<Island*> InstantGroup;
          NextIsland->FindAll(InstantGroup, ID(mica::InstantWiseLink),
            prim::Link::Directions::Forwards, true);
          
          //Find the penultimate group.
          prim::Array<Island*> PenultimateGroup;
          PenultimateGroup.n(InstantGroup.n());
          PenultimateGroup.Zero();
          for(prim::count j = 0; j < InstantGroup.n(); j++)
          {
            InstantGroup[j]->Find(PenultimateGroup[j], ID(mica::PartWiseLink),
              prim::Link::Directions::Backwards);
          }
          
          //Determine if this group may advance the leading edge.
          bool GroupMayAdvance = true;
          for(prim::count j = 0; j < PenultimateGroup.n(); j++)
          {
            //Skip parts which are being introduced.
            if(!PenultimateGroup[j])
              continue;
            
            //Look for an adjacency with the leading edge.
            bool FoundAdjacency = false;
            for(prim::count k = 0; k < LeadingEdge.n(); k++)
            {
              if(PenultimateGroup[j] == LeadingEdge[k])
              {
                FoundAdjacency = true;
                break;
              }
            }
            
            //If no adjacency is found, the this group may not advance the edge.
            if(!FoundAdjacency)
            {
              GroupMayAdvance = false;
              break;
            }
          }
          
          //The group can not advance, then continue the iteration.
          if(!GroupMayAdvance)
            continue;
          
          /*The group is advancing. Add any new parts to the leading edge. Note
          that part order dependence is not important for solving the leading
          edge, therefore the leading edge is simply appended to, rather than
          determining the proper location in which to insert the new part (which
          is possible but unnecessary). Also assign the instant IDs for this
          instant group.*/
          for(prim::count j = 0; j < InstantGroup.n(); j++)
          {
            if(!PenultimateGroup[j])
              LeadingEdge.Add() = InstantGroup[j];
            else
            {
              //Find the adjacency and update the leading edge.
              for(prim::count k = 0; k < LeadingEdge.n(); k++)
              {
                if(PenultimateGroup[j] == LeadingEdge[k])
                {
                  LeadingEdge[k] = InstantGroup[j];
                  break;
                }
              }
            }
            
            InstantGroup[j]->Typesetting->InstantID = InstantID;
          }
          
          //Record the number of parts detected in this instant.
          PartsInInstant.Add() = InstantGroup.n();
          
          //Increment the instant ID for the next leading edge determination.
          InstantID++;
          
          /*Since the group successfully advanced, try to continue along the
          same part (optional -- only affects internal ordering).*/
          i--;
        }
      }
      
      //The instant ranges can now be marked.
      MarkInstantRanges();
      
      //Save the number of instants detected.
      InstantCount = InstantID;
    }
    
    ///Gathers the island subgraph into an array of islands.
    void GatherIslands(MusicGraph& mg)
    {
      prim::Array<prim::Node*> Nodes;
      prim::Array<prim::Link*> Links;
      Islands.Clear();
      
      //Gather all the islands together.
      mg.Gather(Nodes, Links);
      for(prim::count i = 0; i < Nodes.n(); i++)
        if(Island* mn = dynamic_cast<Island*>(Nodes[i]))
          Islands.Add() = mn;
    }
    
    /**Takes a subgraph of island vertices and marks each part strand. Returns
    the number of parts detected.*/
    prim::count MarkPartStrands()
    {
      //Mark each island with a part-index.
      prim::count PartIndex = 0;
      for(prim::count i = 0; i < Islands.n(); i++)
      {
        //Assumes that nodes exist and are valid islands.
        Island* Current = Islands[i];
        
        //Skip over islands which are not the origin for their part.
        if(Current->Find(ID(mica::PartWiseLink),
          prim::Link::Directions::Backwards))
            continue;
        
        //Tag all islands in a part strand with a part ID.
        while(Current)
        {
          Current->Typesetting->PartID = PartIndex;
          Current->Find(Current, ID(mica::PartWiseLink));
        }
        
        //Increment the part ID.
        PartIndex++;
      }
    
      //Return the number of parts.
      return PartIndex;
    }
    
    ///Marks the bounds of each part.
    void MarkPartBounds()
    {
      //Size the parts bounds for the number of parts detected.
      PartBounds.n(PartCount);
      PartBounds.Zero();
      
      //Look for islands which start or end a part.
      for(prim::count i = 0; i < Islands.n(); i++)
      {
        //Assumes that nodes exist and are valid islands.
        Island* Current = Islands[i];
        
        //Get the current part ID.
        prim::count PartID = Current->Typesetting->PartID;
        
        //Look for a start.
        if(!Current->Find(ID(mica::PartWiseLink),
          prim::Link::Directions::Backwards))
            PartBounds[PartID].i() = Current;
        
        //Look for an end.
        if(!Current->Find(ID(mica::PartWiseLink),
          prim::Link::Directions::Forwards))
            PartBounds[PartID].j() = Current;
      }
    }
    
    ///Marks the instant ranges of each part.
    void MarkInstantRanges()
    {
      PartInstantRange.n(PartBounds.n());
      for(prim::count i = 0; i < PartBounds.n(); i++)
      {
        PartInstantRange[i].i() =
          PartBounds[i].i()->Typesetting->InstantID;
        PartInstantRange[i].j() =
          PartBounds[i].j()->Typesetting->InstantID;
      }
    }
        
    ///Observes all part relationships and stores them in a transitive mapping.
    void ObservePartOrders(TransitiveMapping& t)
    {
      for(prim::count i = 0; i < Islands.n(); i++)
      {
        Island* Current = Islands[i];
        Island* Next = 0;
        if(Current->Find(Next, ID(mica::InstantWiseLink)))
          t.Set(Current->Typesetting->PartID, Next->Typesetting->PartID,
            TransitiveClosure::LessThan);
      }
    }
  };
}}
#endif

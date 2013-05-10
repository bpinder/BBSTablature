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

#ifndef BELLEBONNESAGE_GRAPH_BASE_H
#define BELLEBONNESAGE_GRAPH_BASE_H

namespace bellebonnesage { namespace graph
{
  //Conversion from mica to prim UUID.
  prim::UUID ID(mica::UUID x);
  
  mica::UUID ID(prim::UUID x);

#ifdef BELLEBONNESAGE_COMPILE_INLINE
  prim::UUID ID(mica::UUID x)
  {
    return prim::UUID(x.high, x.low);
  }
  
  mica::UUID ID(prim::UUID x)
  {
    mica::UUID y = {x.High(), x.Low()};
    return y;
  }
#endif

  ///Represents the number of extra staves to display per part
  struct ExtraStaff
  {
    ExtraStaff() : PartID (0), NumExtra (0) {}

    ExtraStaff (prim::count partID, prim::count numExtra) : 
      PartID (0), NumExtra (0)
    {
      SetPartID (partID);
      SetNumExtra (numExtra);
    }

    ~ExtraStaff() {}

    prim::count GetPartID() { return PartID; }

    void SetPartID (prim::count partID)
    {
      if (partID >= 0) PartID = partID;
    }

    prim::count GetNumExtra() { return NumExtra; }

    void SetNumExtra (prim::count numExtra)
    {
      if (numExtra >= 0) NumExtra = numExtra;
    }

    struct ExtraStaffSortingComparator
    {
    public:
      ExtraStaffSortingComparator() {}
      ~ExtraStaffSortingComparator() {}

      const int compareElements (ExtraStaff first, ExtraStaff second) const
      {
        return (first.GetPartID() < second.GetPartID()) ? -1 : 
          ((second.GetPartID() < first.GetPartID()) ? 1 : 0);
      }
    };

    private:
      prim::count PartID;
      prim::count NumExtra; 
  };

  //-------------//
  //Serialization//
  //-------------//
  struct MusicSerial : public prim::Serial
  {
    ///Stores or reads a MICA concept.
    static void DoMICA(prim::Serial& s, mica::UUID& Value,
      prim::Serial::Modes Mode)
    {
      if(Mode == prim::Serial::Reading)
      {
        prim::UUID u;
        s.Read(u);
        Value = ID(u);
      }
      else if(Mode == prim::Serial::Writing)
      {
        s.Write(ID(Value));
      }
    }
    
    ///Given the node type restore the specific node objects.
    virtual prim::Serial::Object* RestoreObject(prim::UUID ID);
  };

  ///Graph vertex which can be subclassed as a container for something.
  struct MusicNode : public prim::Node
  {
    private:
    
    ///Stores musical attributes.
    prim::Table<mica::UUID> Attributes;
    
    ///Stores other information pertinent to the node.
    prim::Table<prim::String> Properties;
    
    public:
    
    ///Gets an associative musical attribute.
    mica::UUID& Set(mica::UUID x) {return Attributes[x];}

    ///Gets a read-only associative musical attribute.
    const mica::UUID& Get(mica::UUID x) const {return Attributes[x];}

    ///Gets an associative general property.
    prim::String& Set(const prim::String& x) {return Properties[x];}
    
    ///Gets an associative general property.
    const prim::String& Get(const prim::String& x) const
    {
      return Properties[x];
    }
    
    public:
    
    ///Base class to help automanage pointers to typesetting info.
    struct TypesettingInfo
    {
      ///Caches the part ID.
      prim::count PartID;
      
      ///Caches the instant ID.
      prim::count InstantID;
      
      ///Constructor to initialize IDs.
      TypesettingInfo() : PartID(-1), InstantID(-1) {}
      
      ///Virtual destructor
      virtual ~TypesettingInfo() {}
    };
    
    ///Optional object to store typesetting features.
    prim::Pointer<TypesettingInfo> Typesetting;

    /**Optional array of extra TypesettingInfo 
    (for displaying more than one StampInstant per node)*/
    prim::Array<prim::Pointer<TypesettingInfo>> ExtraTypesetting;
    
    ///Intended color.
    Color IntendedColor;
 
    public:
  
    ///Constructor which assigns the node type.
    MusicNode(mica::UUID Type) : prim::Node(ID(Type)), Typesetting(0) {}
    
    ///Virtual destructor
    virtual ~MusicNode() {}

    virtual bool IsChild(prim::UUID x, prim::UUID y)
    {
      return mica::map(ID(x), ID(y)) == mica::Child;
    }
    
    /**Returns the unique ID of this node. This method is only applicable after
    MusicGraph::SetCustomDataToIDs() has been called on the parent graph.*/
    virtual prim::String UniqueID()
    {
      prim::String s(ID(GetType()));
      s << ":";
      s << CustomData;
      s.Replace("Token", "");
      s.Replace("Node", "");
      s.Replace("KeySignature", "Key");
      return s.ToLower();
    }
    
    ///Gets a string representing this music node.
    virtual prim::String ToString()
    {
      return mica::M(ID(GetType())).Name();
    }
    
    ///Serializes attribute and property tables.
    void SerializeProperties(prim::Serial &s, prim::Serial::Modes Mode)
    {
      if(Mode == prim::Serial::Writing)
      {
        s.Write(Attributes.n());
        for(prim::count i = 0; i < Attributes.n(); i++)
        {
          mica::UUID k = Attributes.ith(i).Key, v = Attributes.ith(i).Value;
          MusicSerial::DoMICA(s, k, Mode);
          MusicSerial::DoMICA(s, v, Mode);
        }
        s.Write(Properties.n());
        for(prim::count i = 0; i < Properties.n(); i++)
        {
          prim::String k = Properties.ith(i).Key, v = Properties.ith(i).Value;
          s.Write(k);
          s.Write(v);
        }
      }
      else if(Mode == prim::Serial::Reading)
      {
        prim::count Attributes_n = 0;
        s.Read(Attributes_n);
        for(prim::count i = 0; i < Attributes_n; i++)
        {
          mica::UUID k, v;
          MusicSerial::DoMICA(s, k, Mode);
          MusicSerial::DoMICA(s, v, Mode);
          Attributes[k] = v;
        }
        
        prim::count Properties_n = 0;
        s.Read(Properties_n);
        for(prim::count i = 0; i < Properties_n; i++)
        {
          prim::String k, v;
          s.Read(k);
          s.Read(v);
          Properties[k] = v;
        }
      }
    }
    
    ///Pass the serialization method on to its subclasses.
    virtual void Serialize(prim::Serial &s, prim::Serial::Modes Mode,
      prim::UUID &VersionOrID)
    {
      if(Mode == prim::Serial::CheckVersion) return;
      else if(Mode == prim::Serial::CheckID)
      {
        VersionOrID = GetType();
        return;
      }
      
      SerializeProperties(s, Mode);
    }
  };

  //-------//
  //Islands//
  //-------//
  struct Island : public MusicNode
  {
    Island() : MusicNode(mica::Island) {}
    
    ///Returns the unique ID of this object after it has been parsed.
    prim::String UniqueID()
    {
      prim::String s;
      if(Typesetting)
        s << "Island:" << Typesetting->PartID << "," << Typesetting->InstantID;
      else
        s = MusicNode::UniqueID();
      return s.ToLower();
    }
  };

  ///Overloads a graph to provide music specific support.
  struct MusicGraph : public prim::Graph
  {
    void SetCustomDataToIDs()
    {
      prim::Array<prim::Node*> Nodes;
      prim::Array<prim::Link*> Links;
      Gather(Nodes, Links);
      for(prim::count i = 0; i < Nodes.n(); i++)
        Nodes[i]->CustomData = i;
    }
    
    void ClearCustomData()
    {
      prim::Array<prim::Node*> Nodes;
      prim::Array<prim::Link*> Links;
      Gather(Nodes, Links);
      for(prim::count i = 0; i < Nodes.n(); i++)
        Nodes[i]->CustomData.Clear();
    }
    
    ///Clears the graph by traversing islands and recursively deleting children.
    void Clear()
    {
      TraverseRows<Delete>();
      SetTop(0);
    }
    
    ///Creates a certain number of rows and columns of islands.
    void CreateIslands(prim::count Rows, prim::count Columns)
    {
      //Clear the existing graph
      Clear();
      
      //Create the first row.
      {
        prim::Node* x = new Island;
        SetTop(x);
        for(prim::count i = 1; i < Columns; i++)
        {
          prim::Node* y = new Island;
          x->AddLink(y, ID(mica::PartWiseLink));
          x = y;
        }
      }
      
      //Create the rest of the rows.
      {
        prim::Node* xPreviousHead = GetTop();
        for(prim::count i = 1; i < Rows; i++)
        {
          prim::Node* x = new Island;
          xPreviousHead->AddLink(x, ID(mica::InstantWiseLink));
          //g.TraverseIslands<Graph::Print>();
          prim::Node* xPrevious = xPreviousHead;
          for(prim::count j = 1; j < Columns; j++)
          {
            prim::Node* y = new Island;
            x->AddLink(y, ID(mica::PartWiseLink));
            xPrevious = xPrevious->Find(ID(mica::PartWiseLink));
            xPrevious->AddLink(y, ID(mica::InstantWiseLink));
            //g.TraverseIslands<Graph::Print>();
            x = y;
          }
          xPreviousHead = xPreviousHead->Find(ID(mica::InstantWiseLink));
        }
      }
    }
    
    ///Appends an instant of islands given the top node of an instant.
    prim::Node* AppendColumn(prim::Node* TopOfPreviousColumn)
    {
      //Traverse the previous column, and for each island, create adjacent one.
      prim::Node* PreviousColumnNode = TopOfPreviousColumn,
        *CurrentColumnNode = 0;
      
      //Create the first node in the new column and link it.
      CurrentColumnNode = new Island;
      PreviousColumnNode->AddLink(CurrentColumnNode, ID(mica::PartWiseLink));
      TopOfPreviousColumn = CurrentColumnNode; //Save for the return value.
      
      /*For each remaining node in the column, create an adjacent node and link
      to it across and down.*/
      prim::Node* NextInPrevious = 0, *NextInCurrent = 0;
      while((NextInPrevious =
        PreviousColumnNode->Find(ID(mica::InstantWiseLink))))
      {
        NextInCurrent = new Island;
        NextInPrevious->AddLink(NextInCurrent, ID(mica::PartWiseLink));
        CurrentColumnNode->AddLink(NextInCurrent, ID(mica::InstantWiseLink));
        PreviousColumnNode = NextInPrevious;
        CurrentColumnNode = NextInCurrent;
      }
      return TopOfPreviousColumn;
    }

    /**Traverses the island structure by part, then instant. It calls
    Operation::Perform() to perform work on the island.*/
    template <class Operation> void TraverseRows() const
    {
      prim::Node* Current = GetTop();
      while(Current)
      {
        prim::Node* NextPart = Current->Find(ID(mica::InstantWiseLink));
        while(Current)
        {
          prim::Node* NextInstant = Current->Find(ID(mica::PartWiseLink));
          Operation::Perform(this, Current);
          Current = NextInstant;
        }
        Current = NextPart;
      }
    }
    
    /**Traverses the island structure by instant, then part. It calls
    Operation::Perform() to perform work on the island.*/
    template <class Operation> void TraverseColumns() const
    {
      prim::Node* Current = GetTop();
      while(Current)
      {
        prim::Node* NextInstant = Current->Find(ID(mica::PartWiseLink));
        while(Current)
        {
          prim::Node* NextPart = Current->Find(ID(mica::InstantWiseLink));
          Operation::Perform(this, Current);
          Current = NextPart;
        }
        Current = NextInstant;
      }
    }
  };
}}
#endif

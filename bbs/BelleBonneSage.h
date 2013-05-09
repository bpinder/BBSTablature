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

#ifndef BELLEBONNESAGE_H
#define BELLEBONNESAGE_H

//The prim.cc library
#ifndef PRIM_CC_LIBRARY
#define PRIM_WITH_TIMER
#ifdef BELLEBONNESAGE_COMPILE_INLINE
#define PRIM_COMPILE_INLINE
#endif
#pragma warning (push, 0)
#include "Source/prim.h"
#pragma warning (pop)
#endif

//The MICA (Music Information and Concept Archive) library...
#ifndef MICA_H
//Compile mica if including mica here and compiling Belle, Bonne, Sage
#ifdef BELLEBONNESAGE_COMPILE_INLINE
#define MICA_COMPILE_INLINE
#endif
#pragma warning (push, 0)
#include "Source/mica.h"
#pragma warning (pop)
#endif

//The core library...
#pragma warning (push, 0)
#include "Source/Abstracts.h"
#include "Source/Colors.h"
#include "Source/Font.h"
#include "Source/Manual.h"
#include "Source/Optics.h"
#include "Source/Path.h"
#include "Source/Shapes.h"
#include "Source/SVG.h"
#include "Source/Text.h"
#include "Source/Transform.h"
#include "Source/Units.h"

//The graph library...
#include "Source/Graph/Graph.h"

//The modern library...
#include "Source/Modern/Modern.h"

//The painters...
#include "Source/Painters/JUCE.h"
#include "Source/Painters/PDF.h"
#include "Source/Painters/SVG.h"

/*FreeType2 requires a little bit of configuration so it is optional.

To use:
1) Add include path to freetype2 (i.e. -I/usr/local/include/freetype2)
2) Link to freetype (i.e. -lfreetype)
3) Define BELLEBONNESAGE_WITH_FREETYPE

For example:
-I/usr/local/include/freetype2 -lfreetype -DBELLEBONNESAGE_WITH_FREETYPE

Currently Scripts/GetFreeTypeArguments determines this for POSIX systems.*/ 
#include "Modules/FreeType.h"
#pragma warning (pop)

#endif

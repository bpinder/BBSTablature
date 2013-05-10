#ifndef NL_SCORE_H
#define NL_SCORE_H

#include "JuceHeader.h"
#include "../../bbs/BelleBonneSage.h"
#include "../../Fonts/Resources.h"

namespace belle
{
    using namespace bellebonnesage;
}

class Score : public belle::Portfolio
{
public:
    //==============================================================================
    Score (prim::number spaceHeight, prim::number tabSpaceRatio, prim::number staffDistance, prim::number widthInInches);
    ~Score();   

    /**
    * Loads a Belle, Bonne, Sage XML file into the graph
    */
    bool loadXMLFile (const juce::File& file);

    /**
    * Recreates the systems based on the MusicGraph.
    */
    void createSystems();

    /**
    * Returns a pointer to the list of systems
    */
    prim::List<belle::modern::System>* getSystems() noexcept;

    /**
    * Sets the width of each system in inches
    */
    void setSystemWidth (prim::number widthInInches) noexcept;

    /**
    * Returns the height of one space on a notation staff in inches
    */
    prim::number getSpaceHeight() const noexcept;

    /**
    * Returns the height of one space on a tab staff in SpaceHeights
    */
    prim::number getTabSpaceRatio() const noexcept;

    //==============================================================================
    struct Page : public belle::Canvas
    {
        belle::Inches offsetFromOrigin; 

        virtual void Paint (belle::Painter& Painter, Portfolio& Portfolio)
        {
            Score& s = dynamic_cast<Score&>(Portfolio);

            prim::number spaceHeight = s.getSpaceHeight();
            prim::number tabSpaceRatio = s.getTabSpaceRatio();

            prim::planar::Vector BottomLeft = prim::planar::Vector (offsetFromOrigin.x, Dimensions.y + offsetFromOrigin.y);

            for (prim::count i = 0; i < s.systems.n(); i++)
            {
                BottomLeft -= prim::planar::Vector (0.0, s.systems[i].Bounds.Height() * spaceHeight);
                s.systems[i].Paint (Painter, BottomLeft, spaceHeight, tabSpaceRatio);
            }
        }
    };

private:
    //==============================================================================
    prim::number spaceHeight;
    prim::number tabSpaceRatio;
    prim::number staffDistance;
    prim::number systemWidth;
    prim::number systemWidthSpaces;

    belle::Font          scoreFont;
    belle::modern::Cache cache; 
    belle::modern::House houseStyle; 
    belle::modern::Piece piece;

    belle::graph::MusicGraph*         musicGraph; 
    prim::List<belle::modern::System> systems;

    juce::Array<belle::graph::ExtraStaff> extraStaves;
    belle::graph::ExtraStaff::ExtraStaffSortingComparator extraStaffComparator;

    //==============================================================================
    const belle::Typeface& createCache();

    /**
    * Determines the extra staves if any by looking for any StringedInstrument parts 
    * whose display setting is STANDARD_AND_TAB
    */
    void determineExtraStaves();
};

#endif  // NL_SCORE_H
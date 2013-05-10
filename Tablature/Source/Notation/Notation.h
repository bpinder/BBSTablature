#ifndef NOTATION_H
#define NOTATION_H

#include "Score.h"

class Notation : public juce::Component
{
public:
    //==============================================================================
    Notation();
    ~Notation();

    void paint (juce::Graphics& g);
    void resized();

    /**
    * Sets the width of the score in pixels.
    */
    void setScoreWidth (int widthInPixels) noexcept;

    /**
    * Loads a Belle, Bonne, Sage XML file.
    * Returns true if load was successful.
    */
    bool loadXMLFile (const juce::File& file);

private:
    //==============================================================================
    int ppi; //< Pixels per inch
    
    double spaceHeight;     //< The height of a space on a notation staff in inches
    double tabSpaceRatio;   //< The height of a space on a tab staff in SpaceHeights
    double staffDistance;   //< The distance between the middle lines of 2 staves in a system in spaceHeights
    double scoreWidth;      //< The width of the score in inches

    juce::Point<double> dimInches; //< Dimensions of component in inches

    Score score;
       
    //==============================================================================
    void initializeScoreCanvas();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Notation)
};

#endif  // NOTATION_H

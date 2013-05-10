#include "Notation.h"
#include "../Tools/NotationHelpers.h"

Notation::Notation() :
    spaceHeight (0.1),
    tabSpaceRatio (1.5),
    staffDistance (15.0),
    scoreWidth (8),
    ppi (100),
    dimInches (10, 10),
    score (spaceHeight, tabSpaceRatio, staffDistance, scoreWidth)
{
}

Notation::~Notation()
{
}

void Notation::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);

    // paint the score
    if (score.Canvases.n() > 0)
    {
        belle::painters::JUCE painter;
        belle::painters::JUCE::Properties properties;

        properties.GraphicsContext = &g;
        properties.ComponentContext = this;
        properties.IndexOfCanvas = 0;
        properties.PageDimensions = belle::Inches (prim::number (dimInches.x), prim::number (dimInches.y));
        properties.PageArea = prim::planar::RectangleInt (0, 0, prim::integer (getWidth()), prim::integer (getHeight()));
        properties.PageVisibility = properties.PageArea;

        painter.Paint (&score, &properties);
    }    
}

void Notation::resized()
{
    dimInches.x = ((double)getWidth() / ppi);
    dimInches.y = ((double)getHeight() / ppi);
}

//==============================================================================
void Notation::setScoreWidth (int widthInPixels) noexcept
{
    scoreWidth = NotationHelper::pixelsToInches (widthInPixels, ppi);
    score.setSystemWidth (scoreWidth);
    resized();
    repaint();
}

bool Notation::loadXMLFile (const juce::File& file)
{          
    if (score.loadXMLFile (file))
    {
        initializeScoreCanvas();
        return true;
    }
    return false;
}

//==============================================================================
void Notation::initializeScoreCanvas()
{
    score.Canvases.RemoveAndDeleteAll();
    score.Canvases.Add() = new Score::Page;
    score.Canvases.z()->Dimensions = belle::Inches (dimInches.x, dimInches.y);

    Score::Page* page = dynamic_cast<Score::Page*> (score.Canvases.z());
    page->offsetFromOrigin = (belle::Inches (1.0, ((score.getSystems()->ith(0).SystemHeight - score.getSystems()->ith(0).StaffHeights[0]) * spaceHeight) - 1.0));
}

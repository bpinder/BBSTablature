#include "Score.h"
#include "../Tools/NotationHelpers.h"

Score::Score (prim::number _spaceHeight, prim::number _tabSpaceRatio, prim::number _staffDistance, prim::number widthInInches) : 
    musicGraph (new belle::graph::MusicGraph),
    spaceHeight (_spaceHeight),
    tabSpaceRatio (_tabSpaceRatio),
    staffDistance (_staffDistance),
    systemWidth (widthInInches),
    systemWidthSpaces (systemWidth / spaceHeight)
{
    //Import notation font
    scoreFont.Add (belle::Font::Special1)->ImportFromArray ((prim::byte*)Resources::joie_bellefont);

    //Import text font
    prim::String Regular = "../../Fonts/GentiumBasicRegular.bellefont";
    prim::Array<prim::byte> a;
    if(prim::File::Read(Regular, a))
        scoreFont.Add(belle::Font::Regular)->ImportFromArray(&a.a());
    else /// If running on Mac using XCode, path to font file is different
    {
        Regular = "../../../../Fonts/GentiumBasicRegular.bellefont";
        if(prim::File::Read(Regular, a))
            scoreFont.Add(belle::Font::Regular)->ImportFromArray(&a.a());
    }

    houseStyle.SpaceHeight = spaceHeight;
    houseStyle.TabSpaceHeightRatio = tabSpaceRatio;
    houseStyle.StaffDistance = staffDistance;
}

Score::~Score()
{
}

bool Score::loadXMLFile (const juce::File& file)
{
    juce::String xmlString = file.loadFileAsString();
    const char* xmlChar    = xmlString.getCharPointer();
    int xmlLength          = xmlString.length();

    musicGraph->Clear();

    // Attempt to create MusicGraph from XML file
    if (belle::graph::XML::Read (musicGraph, prim::String ((prim::byte*)xmlChar, xmlLength)))
    {
        determineExtraStaves();
        createSystems();
        return true;
    }

    return false;
}

void Score::createSystems()
{
    // Create the Cache and get the Notation Typeface
    const belle::Typeface& typeface = createCache();

    // Initialize the piece and the systems
    prim::Array<belle::graph::ExtraStaff> primExtraStaves;
    for (int i = 0; i < extraStaves.size(); ++i)
        primExtraStaves.Add (extraStaves.getUnchecked (i));

    piece.Initialize (musicGraph, primExtraStaves, houseStyle, cache, typeface, scoreFont);    
    piece.Prepare (systems, systemWidth, systemWidth);
}

prim::List<belle::modern::System>* Score::getSystems() noexcept
{
    return &systems;
}

void Score::setSystemWidth (prim::number widthInInches) noexcept
{
    systemWidth = widthInInches;
    systemWidthSpaces = (systemWidth / spaceHeight);
    
    if (!musicGraph->IsEmpty()) createSystems();
}

prim::number Score::getSpaceHeight() const noexcept
{
    return spaceHeight;
}

prim::number Score::getTabSpaceRatio() const noexcept
{
    return tabSpaceRatio;
}

const belle::Typeface& Score::createCache()
{
    const belle::Typeface& notationTypeface = *scoreFont[0];
    
    cache.ClearAndDeleteAll();
    cache.Create (houseStyle, notationTypeface);

    return notationTypeface;
}

void Score::determineExtraStaves()
{
    extraStaves.clear();

    // Look for any StringedInstruments with display setting STANDARD_AND_TAB
    int partCount = 0;
    belle::graph::MusicNode* m = dynamic_cast<belle::graph::MusicNode*>(musicGraph->GetTop());
    while(m)
    {
        belle::graph::MusicNode* n = m;
        while(n)
        {
            if (belle::graph::Island* isle = dynamic_cast<belle::graph::Island*> (n))
            {
                if (belle::graph::PartToken* pt = dynamic_cast<belle::graph::PartToken*> (isle->Find (belle::graph::ID (mica::TokenLink))))
                {
                    if (belle::graph::StringedInstrument* si = dynamic_cast<belle::graph::StringedInstrument*> (pt->Find (belle::graph::ID (mica::TokenLink))))
                    {
                        if (si->GetDisplaySetting() == belle::graph::StringedInstrument::STANDARD_AND_TAB)
                        {
                            extraStaves.add (belle::graph::ExtraStaff (partCount, 1));
                            extraStaves.sort (extraStaffComparator);
                            break;
                        }
                    }
                }
            }
            n->Find<belle::graph::MusicNode>(n, belle::graph::ID(mica::PartWiseLink));
        }
        m->Find<belle::graph::MusicNode>(m, belle::graph::ID(mica::InstantWiseLink));
        partCount++;
    }
}
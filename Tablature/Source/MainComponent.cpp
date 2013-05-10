#include "MainComponent.h"

//==============================================================================
MainContentComponent::MainContentComponent() :
    notation(),
    loadButton ("Load")
{
    setSize (1000, 800);

    loadButton.addListener (this);
    addAndMakeVisible (&loadButton);

    notation.setScoreWidth (getWidth() - 400);
    addAndMakeVisible (&notation);
}

MainContentComponent::~MainContentComponent()
{
}

void MainContentComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void MainContentComponent::resized()
{
    loadButton.setBounds (getWidth() / 2 - 30, 40, 60, 20);   
    notation.setBounds (100, 100, getWidth() - 200, getHeight() - 200);
}

void MainContentComponent::buttonClicked (juce::Button* button)
{
    if (button == &loadButton)
    {
        if (loadXMLFile())
        {
            notation.resized();
            notation.repaint();
        }
    }
}

//==============================================================================
bool MainContentComponent::loadXMLFile()
{
    juce::FileChooser chooser ("Load an XML file", juce::File::getSpecialLocation (juce::File::userDesktopDirectory), "*.xml", true);

    if (chooser.browseForFileToOpen())
    {
        juce::File file (chooser.getResult());

        if (file.existsAsFile())
        {
            juce::ScopedPointer<juce::XmlDocument> doc (new juce::XmlDocument (file));

            if (doc->getLastParseError() == juce::String::empty)
            {                
                if (notation.loadXMLFile (file))
                    return true;
                else
                    launchErrorLoadingAlert();
            }
            else launchErrorLoadingAlert();
        }
        else launchErrorLoadingAlert();
    } 

    return false;
}


void MainContentComponent::launchErrorLoadingAlert()
{
    juce::AlertWindow::showMessageBox (juce::AlertWindow::AlertIconType::WarningIcon, "Invalid XML file", juce::String::empty, "OK");
}

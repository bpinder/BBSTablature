#ifndef __MAINCOMPONENT_H_208740DB__
#define __MAINCOMPONENT_H_208740DB__

#include "Notation/Notation.h"

class MainContentComponent : public juce::Component,
                             public juce::Button::Listener
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (juce::Graphics&);
    void resized();

    void buttonClicked (juce::Button* button);

private:
    //==============================================================================
    Notation notation;

    juce::TextButton loadButton;

    //==============================================================================
    bool loadXMLFile();
    void launchErrorLoadingAlert();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // __MAINCOMPONENT_H_208740DB__

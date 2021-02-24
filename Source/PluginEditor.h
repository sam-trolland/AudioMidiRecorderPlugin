/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  : public AudioProcessorEditor,
                                        public Button::Listener
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    // Listener interface for buttons
    void buttonClicked (Button* button) override;
    
    bool recording = false;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;

    // GUI Objects
    //juce::Slider midiVolume; // [1]
    TextButton recordButton;
    
    // GUI Control
    //void sliderValueChanged(juce::Slider* slider) override;
    
    // Recording
    File audioRecordingFile;
    File midiRecordingFile;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};

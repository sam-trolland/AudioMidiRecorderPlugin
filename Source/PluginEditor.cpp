/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 200);
    
    // GUI Elements
    /*
    midiVolume.setSliderStyle(juce::Slider::LinearBarVertical);
    midiVolume.setRange(0.0, 127.0, 1.0);
    midiVolume.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    midiVolume.setPopupDisplayEnabled(true, false, this);
    midiVolume.setTextValueSuffix(" Volume");
    midiVolume.setValue(1.0);
    addAndMakeVisible(&midiVolume); // Add control to GUI
    midiVolume.addListener(this);
    */
    
    // Setup Record Button
    addAndMakeVisible(recordButton);
    recordButton.addListener(this);
    recordButton.setButtonText("Record");
    recordButton.setColour(TextButton::buttonColourId,Colours::green);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Midi Volume", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void NewProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    // GUI Components
    //midiVolume.setBounds(40, 30, 20, getHeight() - 60);
    recordButton.setBounds(0,0,getWidth(),getHeight());
}

void NewProjectAudioProcessorEditor::buttonClicked (Button* button)
{
    if (!recording){
        // Start Recording
        auto docsDir = File::getSpecialLocation (File::userDocumentsDirectory);
        auto parentDir = File(docsDir.getFullPathName()+"/DinvernoRecordings" );
        parentDir.createDirectory();
        audioRecordingFile = parentDir.getNonexistentChildFile("dinverno_system_recording", ".wav");    //Wav Audio File Format
        //audioRecordingFile = parentDir.getNonexistentChildFile("dinverno_system_recording", ".ogg");  //OGG Audio File Format
        midiRecordingFile = parentDir.getNonexistentChildFile("dinverno_system_recording", ".mid");
        
        
        
        recordButton.setButtonText("Stop Recording");
        recordButton.setColour(TextButton::buttonColourId,Colours::red);
        recording = true;
        audioProcessor.startRecordingAudio(audioRecordingFile);
        //audioProcessor.startRecordingMidi(midiRecordingFile);
    }else{
        // Stop Recording
        recordButton.setButtonText("Record");
        recordButton.setColour(TextButton::buttonColourId,Colours::green);
        recording = false;
        //audioProcessor.setRecording(recording);
        audioProcessor.stopRecordingAudio();
        //audioProcessor.stopRecordingMidi();
    }
}

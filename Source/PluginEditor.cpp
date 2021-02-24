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
        // Setup Recording Driectory
        auto docsDir = File::getSpecialLocation (File::userDocumentsDirectory);
        auto parentDir = File(docsDir.getFullPathName()+"/DinvernoRecordings" );
        parentDir.createDirectory();
        
        // Audio Recording File (Swap between .wav and .ogg formats here)
        audioRecordingFile = parentDir.getNonexistentChildFile("dinverno_system_recording", ".wav");    //Wav Audio File Format
        //audioRecordingFile = parentDir.getNonexistentChildFile("dinverno_system_recording", ".ogg");  //OGG Audio File Format
        
        // Midi Recording File (same name as audio file - will overwrite if file exists)
        midiRecordingFile = parentDir.getChildFile(audioRecordingFile.getFileNameWithoutExtension()+".mid");
        
        // Tell Audio Processor to start recording
        audioProcessor.startRecordingAudio(audioRecordingFile);
        audioProcessor.startRecordingMidi(midiRecordingFile);
        
        // Update GUI
        recordButton.setButtonText("Stop Recording");
        recordButton.setColour(TextButton::buttonColourId,Colours::red);
        recording = true;
        
    }else{
        // Stop Recording
        
        // Tell Audio Processor to Stop Recording
        audioProcessor.stopRecordingAudio();
        audioProcessor.stopRecordingMidi();
        
        // Update GUI
        recordButton.setButtonText("Record");
        recordButton.setColour(TextButton::buttonColourId,Colours::green);
        recording = false;
    }
}

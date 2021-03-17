/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DinvernoAudioMidiRecorderPluginProcessorEditor::DinvernoAudioMidiRecorderPluginProcessorEditor (DinvernoAudioMidiRecorderPluginProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 100);
    
    // Setup Record Button
    addAndMakeVisible(recordButton);
    recordButton.addListener(this);
    recordButton.setButtonText("Start Recording");
    recordButton.setColour(TextButton::buttonColourId,Colours::green);
    
    // Music Circle Login Manager
    loggin.addEventListener(this);
   
}

DinvernoAudioMidiRecorderPluginProcessorEditor::~DinvernoAudioMidiRecorderPluginProcessorEditor()
{
}

//==============================================================================
void DinvernoAudioMidiRecorderPluginProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void DinvernoAudioMidiRecorderPluginProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    // GUI Components
    recordButton.setBounds(0,0,getWidth(),getHeight());
}

void DinvernoAudioMidiRecorderPluginProcessorEditor::buttonClicked (Button* button)
{
    if (!recording && !uploading){
        // Start Recording
        // Setup Recording Driectory
        auto docsDir = File::getSpecialLocation (File::userMusicDirectory); 
        auto parentDir = File(docsDir.getFullPathName()+"/AudioMidiRecordings" );
        parentDir.createDirectory();
        
        Time dateTime = Time::getCurrentTime();
        String dateTimeFormatted = dateTime.formatted("%Y-%m-%d_%H-%M-%S");
        
        // Audio Recording File (Swap between .wav and .ogg formats here)
        //audioRecordingFile = parentDir.getNonexistentChildFile(dateTimeFormatted, ".wav");    //Wav Audio File Format
        audioRecordingFile = parentDir.getNonexistentChildFile(dateTimeFormatted, ".ogg");  //OGG Audio File Format
        
        // Midi Recording File (same name as audio file - will overwrite if file exists)
        midiRecordingFile = parentDir.getChildFile(audioRecordingFile.getFileNameWithoutExtension()+".mid");
        
        // Tell Audio Processor to start recording
        audioProcessor.startRecordingAudio(audioRecordingFile);
        audioProcessor.startRecordingMidi(midiRecordingFile);
        
        // Update GUI
        recordButton.setButtonText("Stop Recording");
        recordButton.setColour(TextButton::buttonColourId,Colours::red);
        recording = true;
        
    }else if (recording){
        // Stop Recording - Start Uploading
        
        // Tell Audio Processor to Stop Recording
        audioProcessor.stopRecordingAudio();
        audioProcessor.stopRecordingMidi();
        
        
        
        // Update GUI
        recordButton.setButtonText("Uploading...\n"+audioRecordingFile.getFileNameWithoutExtension());
        recordButton.setColour(TextButton::buttonColourId,Colours::blue);
        uploading = true;
        recording = false;
        
        // Music Circle (loggin for now)
        loggin.loginToMC(default_username, default_password);

    }
}

void DinvernoAudioMidiRecorderPluginProcessorEditor::musicCircleEvent(MusicCircleEvent event)
{
    const MessageManagerLock mmLock;

    /*
    // lock the message thread
    // as this func is called from another thread
    // and we get assertion errors otherwise
    const MessageManagerLock mmLock;
    */
    juce::String msg = "";
     
    switch(event){
        case MusicCircleEvent::login_succeeded:
            msg  << "\nLogged in. user id " << loggin.getUserId();
            
            if (uploading){
                loggin.postMedia(audioRecordingFile.getFullPathName().toStdString(), [this](int result){
                    std::cout << "MainComponent::recordingComplete postMedia callback result " << result << std::endl;
                    //loggin.sendQueuedAnnotations();
                });
            }
            break;
        case MusicCircleEvent::login_failed:
            msg << "\nFailed to login with user " << default_username;
            break;
        case MusicCircleEvent::media_upload_succeeded:
            msg << "\nMedia upload succeeded. ";// + (usernameField.getText());
            
            if (uploading){
                // Update GUI
                recordButton.setButtonText("Start Recording");
                recordButton.setColour(TextButton::buttonColourId,Colours::green);
                uploading = false;
            }
            
            break;
        case MusicCircleEvent::logout_succeeded:
            msg << "\nLogged out user " << default_username;
            break;
        case MusicCircleEvent::logout_failed:
            msg << "\nFailed to logout user id " << loggin.getUserId();
            break;
            
    }
    //mcEventMonitor.setText(msg);
    //    mcEventMonitor.repaint();
    std::cout << msg << std::endl;
}

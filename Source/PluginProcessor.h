/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Recording
    void startRecordingAudio (const File& audioFile);
    void stopRecordingAudio ();
    void startRecordingMidi (const File& midiFile);
    void stopRecordingMidi ();
    void setRecording(bool rec);
    MidiFile midiRecordingFile;
    MidiMessageSequence midiSequence;
    
private:
    bool recordingAudio;
    bool recordingMidi;
    bool recording;
    double recordingTime;
    double recordingStartTime;
    double mSampleRate;
    TimeSliceThread writeThread;
    std::unique_ptr<FileOutputStream> stream;
    std::unique_ptr<FileOutputStream> midiStream;
    std::unique_ptr<AudioFormatWriter> writer;
    //std::unique_ptr<AudioFormatWriter::ThreadedWriter> threaded;
    std::unique_ptr<AudioFormatWriter::ThreadedWriter> threadedWriter;
    CriticalSection writerLock;
    std::atomic<AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };

    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};

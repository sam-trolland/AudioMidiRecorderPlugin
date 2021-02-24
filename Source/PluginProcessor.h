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
class AudioMidiRecorderPluginProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioMidiRecorderPluginProcessor();
    ~AudioMidiRecorderPluginProcessor() override;

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
    // Audio and Midi Recording
    void startRecordingAudio (const File& audioFile);
    void stopRecordingAudio ();
    void startRecordingMidi (const File& midiFile);
    void stopRecordingMidi ();
    
private:
    double mSampleRate;
    
    // Midi Recording
    bool recordingMidi;
    double recordingTime;
    std::unique_ptr<FileOutputStream> midiStream;
    MidiFile midiRecordingFile;
    MidiMessageSequence midiSequence;
    
    // Audio Recording
    bool recordingAudio;
    TimeSliceThread writeThread;
    std::unique_ptr<AudioFormatWriter::ThreadedWriter> threadedWriter;
    CriticalSection writerLock;
    std::atomic<AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };

    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioMidiRecorderPluginProcessor)
};

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioMidiRecorderPluginProcessor::AudioMidiRecorderPluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        recordingAudio(false),
        recordingMidi(false),
        writeThread("write thread")
#endif
{
    midiRecordingFile = MidiFile();
    midiSequence = MidiMessageSequence();
    recordingTime = 0;
}

AudioMidiRecorderPluginProcessor::~AudioMidiRecorderPluginProcessor()
{
}

//==============================================================================
const juce::String AudioMidiRecorderPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioMidiRecorderPluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioMidiRecorderPluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioMidiRecorderPluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioMidiRecorderPluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioMidiRecorderPluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioMidiRecorderPluginProcessor::getCurrentProgram()
{
    return 0;
}

void AudioMidiRecorderPluginProcessor::setCurrentProgram (int index)
{
}

const juce::String AudioMidiRecorderPluginProcessor::getProgramName (int index)
{
    return {};
}

void AudioMidiRecorderPluginProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AudioMidiRecorderPluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mSampleRate = sampleRate;
    
    // Start Audio Recording Thread
    writeThread.startThread();
}

void AudioMidiRecorderPluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    stopRecordingAudio();
    writeThread.stopThread (1000);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioMidiRecorderPluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AudioMidiRecorderPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Record Midi Data to Sequence
    if (recordingMidi){
        // Initialise and clear variables
        int numSamples = buffer.getNumSamples();
        double numSecondsInThisBlock = numSamples / mSampleRate;
        double endTime = recordingTime + numSecondsInThisBlock;

        
        // Iterate over midi Messages add to midi sequence recording
        int time;
        juce::MidiMessage m;
        for (juce::MidiBuffer::Iterator i (midiMessages); i.getNextEvent(m, time);)
        {
            // Get time of midi event and convert into ticks per quarter note (1s = 192 ticks)
            double offset = (double)time/(double)numSamples * numSecondsInThisBlock;
            double mTime = 192 * (recordingTime + offset);
            m.setTimeStamp(mTime);
            midiSequence.addEvent(m,0);
        }
        recordingTime = endTime;
    }
    
    // Record Audio Data to file (in seperate thread)
    if (recordingAudio){
        const ScopedLock sl (writerLock);
        if (activeWriter.load() != nullptr) {
            activeWriter.load()->write (buffer.getArrayOfReadPointers (), buffer.getNumSamples());
        }
    }
}

void AudioMidiRecorderPluginProcessor::startRecordingAudio (const File& audioFile) {
    // Exit if already recording
    if (recordingAudio)
        return;
    
    // Clear and reset writers
    stopRecordingAudio();
    
    // Create an OutputStream to write to our destination file...
    audioFile.deleteFile();
    
    if ( auto audioStream = std::unique_ptr<FileOutputStream> (audioFile.createOutputStream()) ) {
        
        if (audioFile.hasFileExtension(".ogg")){
            // OGG file Recorder
            OggVorbisAudioFormat oggFormat;
            
            if (auto audioWriter = oggFormat.createWriterFor (audioStream.get(),mSampleRate,1,16,{},8) ) {
                
                audioStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)
                
                // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                // write the data to disk on our background thread.
                threadedWriter.reset (new AudioFormatWriter::ThreadedWriter (audioWriter, writeThread, 32768));
                //threaded = new AudioFormatWriter::ThreadedWriter (writer, writeThread, 16384);
                
                // And now, swap over our active writer pointer so that the audio callback will start using it..
                const ScopedLock sl (writerLock);
                activeWriter = threadedWriter.get();
                
            }
        }else if (audioFile.hasFileExtension(".wav")){
           // Wav File Recorder
            // Audio File Format
            WavAudioFormat format;
            
            if (auto audioWriter = format.createWriterFor (audioStream.get(), mSampleRate, 1, 16, {}, 0) ) {
                
                audioStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)
                
                // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                // write the data to disk on our background thread.
                threadedWriter.reset (new AudioFormatWriter::ThreadedWriter (audioWriter, writeThread, 32768));
                //threaded = new AudioFormatWriter::ThreadedWriter (writer, writeThread, 16384);
                
                // And now, swap over our active writer pointer so that the audio callback will start using it..
                const ScopedLock sl (writerLock);
                activeWriter = threadedWriter.get();
                
            }
        }
    }
    
    // Update recording flag
    recordingAudio = true;
}

void AudioMidiRecorderPluginProcessor::stopRecordingAudio () {
    
    // First, clear this pointer to stop the audio callback from using our writer object..
    {
        const ScopedLock sl (writerLock);
        activeWriter = nullptr;
    }
    
    // Now we can delete the writer object. It's done in this order because the deletion could
    // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
    // the audio callback while this happens.
    threadedWriter.reset();

    // Update recording flag
    recordingAudio = false;
}

void AudioMidiRecorderPluginProcessor::startRecordingMidi (const File& midiFile) {
    // Exit if already recording
    if (recordingMidi)
        return;
    
    // Clear file before starting recording
    midiFile.deleteFile();
    
    // Initialise variables for recording Midi input to file
    midiStream = midiFile.createOutputStream();
    midiSequence.clear();
    midiRecordingFile.clear();
    recordingTime=0;
    
    // Update recording Flag
    recordingMidi = true;
}

void AudioMidiRecorderPluginProcessor::stopRecordingMidi () {
    // Exit if not recording
    if (!recordingMidi)
        return;
    
    // Add recorded Midi Sequence and write to recording file
    midiRecordingFile.addTrack(midiSequence);
    midiRecordingFile.setTicksPerQuarterNote(96);
    midiRecordingFile.writeTo(*midiStream,0);
    
    // Update recording file
    recordingMidi = false;
}


//==============================================================================
bool AudioMidiRecorderPluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioMidiRecorderPluginProcessor::createEditor()
{
    return new AudioMidiRecorderPluginProcessorEditor (*this);
}

//==============================================================================
void AudioMidiRecorderPluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AudioMidiRecorderPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioMidiRecorderPluginProcessor();
}

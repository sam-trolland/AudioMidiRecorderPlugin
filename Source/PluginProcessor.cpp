/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        recording (false),
        writeThread("write thread")
#endif
{
    midiRecordingFile = MidiFile();
    midiSequence = MidiMessageSequence();
    recordingTime = 0;
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mSampleRate = sampleRate;
    
    // Start Audio Recording Thread
    writeThread.startThread();
    
    
    if (writer == NULL){
        
        // Moved to startAudioRecording
        /*
        writeThread.startThread();
        stream = new FileOutputStream (File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("MLRecordings/rec.wav"));
        WavAudioFormat format;
        writer = format.createWriterFor (stream, sampleRate, 2, 24, StringPairArray(), 0);
        threaded = new AudioFormatWriter::ThreadedWriter (writer, writeThread, 16384);
         */
    }
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    stopRecordingAudio();
    writeThread.stopThread (1000);
    writer = nullptr;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    if (recording){
        int numSamples = buffer.getNumSamples();
        double numSecondsInThisBlock = numSamples / mSampleRate;
        double endTime = recordingTime + numSecondsInThisBlock;
        
        // Initialise and clear variables
        //buffer.clear();
        int time;
        juce::MidiMessage m;
        
        // Iterate over midi Messages and edit 'noteOn' velocities
        for (juce::MidiBuffer::Iterator i (midiMessages); i.getNextEvent(m, time);)
        {
            double eventTime = recordingTime + time*numSecondsInThisBlock;
            
            //Combo1: double mTime = 100 * (Time::getMillisecondCounterHiRes() - recordingTime);
            //Combo1: m.setTimeStamp(mTime);
            //Combo1: midiSequence.addEvent(m,0);
            
            //Combo2: double offset = (double)time/(double)numSamples * numSecondsInThisBlock;
            //Combo2: double mTime = 192 * (Time::getMillisecondCounterHiRes()*0.001 - recordingStartTime + offset);
            
            double offset = (double)time/(double)numSamples * numSecondsInThisBlock;
            double mTime = 192 * (recordingTime + offset);
            m.setTimeStamp(mTime);
            midiSequence.addEvent(m,0);
            
            if (m.isNoteOn())
            {
                //juce::uint8 newVel = (juce::uint8) noteOnVel;
                //m = juce::MidiMessage::noteOn(m.getChannel(), m.getNoteNumber(), newVel);
                //midiSequence.addEvent(m,0);
            }else if (m.isNoteOff())
            {
                //midiSequence.addEvent(m,0);
            }
            else if (m.isAftertouch())
            {
            }
            else if (m.isPitchWheel())
            {
            }
            
        }
        
        
        recordingTime = endTime;
    }
    
    //if (threaded && recording) {
    //threaded->write(buffer.getArrayOfReadPointers (), buffer.getNumSamples());
    //}
    
    if (recordingAudio){
        const ScopedLock sl (writerLock);
        if (activeWriter.load() != nullptr) {
            //threadedWriter->write(buffer.getArrayOfReadPointers (), buffer.getNumSamples() );
            //activeWriter.load()->write(<#const float *const *data#>, <#int numSamples#>);
            activeWriter.load()->write (buffer.getArrayOfReadPointers (), buffer.getNumSamples());
        }
    }
}

void NewProjectAudioProcessor::startRecordingAudio (const File& audioFile) {
    
    stopRecordingAudio();
        
    //stream = new FileOutputStream (File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("MLRecordings/rec.wav"));
    //stream = audioFile.createOutputStream();
    
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
        
        // Writer
        //if (auto OggWriter = oggFormat.createWriterFor (audioStream.get(),sampleRate,1,16,{},8) ) {
        //if (auto audioWriter = format.createWriterFor (stream.get(), mSampleRate, 2, 24, StringPairArray(), 0) ) {


    }
    
    recordingAudio = true;
    
}

void NewProjectAudioProcessor::stopRecordingAudio () {
    
    // First, clear this pointer to stop the audio callback from using our writer object..
    {
        const ScopedLock sl (writerLock);
        activeWriter = nullptr;
    }
    
    // Now we can delete the writer object. It's done in this order because the deletion could
    // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
    // the audio callback while this happens.
    threadedWriter.reset();


    recordingAudio = false;
}

void NewProjectAudioProcessor::startRecordingMidi (const File& midiFile) {
    recordingMidi = true;
}

void NewProjectAudioProcessor::stopRecordingMidi () {
    recordingMidi = false;
}


void NewProjectAudioProcessor::setRecording (bool rec) {
    if (!recording && rec){
        // Start Recording
        midiSequence.clear();
        midiRecordingFile.clear();
        
        // Combo1: recordingTime=Time::getMillisecondCounterHiRes();
        recordingTime=0;        //Time::getMillisecondCounterHiRes();
        recordingStartTime=Time::getMillisecondCounterHiRes()*0.001;
    }else if (recording && !rec){
        // End Recording
        //midiSequence.addEvent(MidiMessage::endOfTrack(),recordingTime);
        //midiSequence.updateMatchedPairs();
        //midiSequence.sort();
        
        MidiMessageSequence midiSequence2 = MidiMessageSequence();
        //Messsage Generation
        int midiChannel = 10;
        double startTime= Time::getMillisecondCounterHiRes()*0.001;
        int c3 = 60;
        MidiMessage message = MidiMessage::noteOn(1, c3, (uint8)100);
        message.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001 - startTime);
        //Extra noteOn
        MidiMessage message1 = MidiMessage::noteOn(1, c3+7, (uint8)100);
        message1.setTimeStamp(message.getTimeStamp());
        MidiMessage messageOff = MidiMessage::noteOff(message.getChannel(), message.getNoteNumber());
        //If 96 ticks per quarter note then 192 are 2 beats
        messageOff.setTimeStamp(message.getTimeStamp() + 192);
        //Extra noteOff
        MidiMessage messageOff1 = MidiMessage::noteOff(message1.getChannel(), message1.getNoteNumber());
        messageOff1.setTimeStamp(message1.getTimeStamp() + 96);
        
        double testTime = message.getTimeStamp() + 192;
        double testTime2 = startTime + 192;
        double testTime3 = startTime*0.001 + 192;
        
        MidiMessage message2 = MidiMessage::noteOn(1, c3+9, (uint8)100);
        message2.setTimeStamp(192);
        MidiMessage messageOff2 = MidiMessage::noteOff(message2.getChannel(), message2.getNoteNumber());
        messageOff2.setTimeStamp(message2.getTimeStamp() + 192);

        
        midiSequence2.addEvent(message, 0);
        midiSequence2.addEvent(message1, 0);
        midiSequence2.addEvent(messageOff, 0);
        midiSequence2.addEvent(messageOff1, 0);
        midiSequence2.addEvent(message2, 0);
        midiSequence2.addEvent(messageOff2, 0);
        
        for (int i=0; i<midiSequence.getNumEvents(); i++){
            auto eventTime = midiSequence.getEventTime(i);
            auto eventOffTime = midiSequence.getTimeOfMatchingKeyUp(i);
            auto event = midiSequence.getEventPointer(i);
            String desc = event->message.getDescription();
            auto eventTimestamp = event->message.getTimeStamp();
            auto upTime = midiSequence.getTimeOfMatchingKeyUp(i);
            int test = 1;
        }
        
        midiRecordingFile.addTrack(midiSequence);
        //Combo1: midiRecordingFile.setSmpteTimeFormat (25, 40);
        //midiRecordingFile.setSmpteTimeFormat (25, 40);
        midiRecordingFile.setTicksPerQuarterNote(96);
        
        File midiFileOut = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("MLRecordings/rec.mid");
        FileOutputStream midiStreamOut (midiFileOut);
        //midiStream = FileOutputStream (File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("test.midi"));
        midiRecordingFile.writeTo(midiStreamOut,0);
        
        //Audio
        stream->flush();
        writer->flush();
        
    }
    recording = rec;
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}

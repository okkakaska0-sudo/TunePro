#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.h"

AutoTuneAudioProcessor::AutoTuneAudioProcessor()
    : AudioProcessor(AudioProcessor::BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", AudioChannelSet::stereo(), true)
#endif
    ),
    pluginParameters(),
    parameters(*this, nullptr, Identifier("AutoTuneParameters"), pluginParameters.createParameterLayout()),
    presetManager(parameters),
    pitchEngine(),
    modeSelector(),
    aiModelLoader()
{
    // Add parameter listeners
    parameters.addParameterListener(Parameters::SPEED_ID, this);
    parameters.addParameterListener(Parameters::AMOUNT_ID, this);
    parameters.addParameterListener(Parameters::MODE_ID, this);
    parameters.addParameterListener(Parameters::KEY_ID, this);
    parameters.addParameterListener(Parameters::SCALE_ID, this);

    // Initialize pitch correction engine
    pitchEngine.prepareToPlay(44100.0, 512);
    
    // Initialize FFT
    fft = std::make_unique<dsp::FFT>(fftOrder);
    window = std::make_unique<dsp::WindowingFunction<float>>(fftSize, dsp::WindowingFunction<float>::hann);
    frequencyData.allocate(fftSize, true);
}

AutoTuneAudioProcessor::~AutoTuneAudioProcessor()
{
    parameters.removeParameterListener(Parameters::SPEED_ID, this);
    parameters.removeParameterListener(Parameters::AMOUNT_ID, this);
    parameters.removeParameterListener(Parameters::MODE_ID, this);
    parameters.removeParameterListener(Parameters::KEY_ID, this);
    parameters.removeParameterListener(Parameters::SCALE_ID, this);
}

const String AutoTuneAudioProcessor::getProgramName(int index)
{
    ignoreUnused(index);
    return {};
}

void AutoTuneAudioProcessor::changeProgramName(int index, const String& newName)
{
    ignoreUnused(index, newName);
}

void AutoTuneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Prepare pitch correction engine
    pitchEngine.prepareToPlay(sampleRate, samplesPerBlock);

    // Initialize buffers
    pitchBuffer.setSize(2, samplesPerBlock);
    correctedBuffer.setSize(2, samplesPerBlock);
    overlapBuffer.setSize(2, overlapSize);
    fftBuffer.setSize(1, fftSize);

    overlapBuffer.clear();
    overlapPosition = 0;

    // Initialize smoothed values
    speedSmoothed.reset(sampleRate, 0.05);
    amountSmoothed.reset(sampleRate, 0.05);
    
    speedSmoothed.setCurrentAndTargetValue(*parameters.getRawParameterValue(Parameters::SPEED_ID));
    amountSmoothed.setCurrentAndTargetValue(*parameters.getRawParameterValue(Parameters::AMOUNT_ID));

#ifdef USE_RUBBERBAND
    // Initialize Rubber Band stretcher
    rubberBand = std::make_unique<RubberBand::RubberBandStretcher>(
        sampleRate, 2,
        RubberBand::RubberBandStretcher::OptionProcessRealTime |
        RubberBand::RubberBandStretcher::OptionPitchHighQuality |
        RubberBand::RubberBandStretcher::OptionEngineFiner  // R3 engine for setPitchScale()
    );
#endif
}

void AutoTuneAudioProcessor::releaseResources()
{
    pitchBuffer.setSize(0, 0);
    correctedBuffer.setSize(0, 0);
    overlapBuffer.setSize(0, 0);
    fftBuffer.setSize(0, 0);

#ifdef USE_RUBBERBAND
    rubberBand.reset();
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AutoTuneAudioProcessor::isBusesLayoutSupported(const AudioProcessor::BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void AutoTuneAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ignoreUnused(midiMessages);

    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (buffer.getNumSamples() == 0)
        return;

    // Update smoothed parameter values
    speedSmoothed.setTargetValue(*parameters.getRawParameterValue(Parameters::SPEED_ID));
    amountSmoothed.setTargetValue(*parameters.getRawParameterValue(Parameters::AMOUNT_ID));

    // Get current mode
    auto currentMode = static_cast<Parameters::Mode>(
        static_cast<int>(*parameters.getRawParameterValue(Parameters::MODE_ID))
    );

    // Process based on selected mode
    switch (currentMode)
    {
        case Parameters::Mode::Classic:
            processClassicMode(buffer);
            break;
        case Parameters::Mode::Hard:
            processHardMode(buffer);
            break;
        case Parameters::Mode::AI:
            processAIMode(buffer);
            break;
    }
}

void AutoTuneAudioProcessor::processClassicMode(AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Get parameter values
    float speed = speedSmoothed.getNextValue();
    float amount = amountSmoothed.getNextValue();
    auto key = static_cast<Parameters::Key>(
        static_cast<int>(*parameters.getRawParameterValue(Parameters::KEY_ID))
    );
    auto scale = static_cast<Parameters::Scale>(
        static_cast<int>(*parameters.getRawParameterValue(Parameters::SCALE_ID))
    );

    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        // Pitch detection and correction
        std::vector<float> pitches(numSamples);
        pitchEngine.detectPitch(channelData, numSamples, pitches.data());
        
        // Apply pitch correction
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentPitch = pitches[sample];
            
            if (currentPitch > 0.0f) // Valid pitch detected
            {
                // Convert to MIDI note number
                float midiNote = Utils::frequencyToMidiNote(currentPitch);
                
                // Quantize to scale
                float targetNote = Utils::quantizeToScale(midiNote, key, scale);
                float targetFrequency = Utils::midiNoteToFrequency(targetNote);
                
                // Calculate correction amount
                float correction = (targetFrequency - currentPitch) * amount * 0.01f;
                
                // Apply smooth correction with speed control
                float smoothedCorrection = correction * speed * 0.01f;
                
                // Apply pitch correction
                if (std::abs(smoothedCorrection) > 0.01f)
                {
                    pitchEngine.correctPitch(&channelData[sample], 1, targetFrequency, speed, amount);
                }
            }
        }
    }
}

void AutoTuneAudioProcessor::processHardMode(AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Get parameter values
    float speed = speedSmoothed.getNextValue();
    float amount = amountSmoothed.getNextValue();
    auto key = static_cast<Parameters::Key>(
        static_cast<int>(*parameters.getRawParameterValue(Parameters::KEY_ID))
    );
    auto scale = static_cast<Parameters::Scale>(
        static_cast<int>(*parameters.getRawParameterValue(Parameters::SCALE_ID))
    );

    // Hard mode applies immediate, aggressive correction
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        // Detect pitch
        std::vector<float> pitches(numSamples);
        pitchEngine.detectPitch(channelData, numSamples, pitches.data());
        
        // Apply hard correction
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float currentPitch = pitches[sample];
            
            if (currentPitch > 0.0f)
            {
                float midiNote = Utils::frequencyToMidiNote(currentPitch);
                float targetNote = Utils::quantizeToScale(midiNote, key, scale);
                float targetFrequency = Utils::midiNoteToFrequency(targetNote);
                
                // Hard correction - immediate snap to target
                float pitchRatio = targetFrequency / currentPitch;
                if (std::abs(pitchRatio - 1.0f) > 0.005f && amount > 0.1f)
                {
                    // Hard snap to target with formant preservation
                    pitchEngine.correctPitchHard(&channelData[sample], 1, targetFrequency, speed, amount);
                }
            }
        }
    }
}

void AutoTuneAudioProcessor::processAIMode(AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Get parameter values
    float speed = speedSmoothed.getNextValue();
    float amount = amountSmoothed.getNextValue();
    auto key = static_cast<Parameters::Key>(
        static_cast<int>(*parameters.getRawParameterValue(Parameters::KEY_ID))
    );
    auto scale = static_cast<Parameters::Scale>(
        static_cast<int>(*parameters.getRawParameterValue(Parameters::SCALE_ID))
    );

    // AI-enhanced processing
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        if (aiModelLoader.areModelsLoaded())
        {
            // Use AI models for pitch detection and synthesis
            auto pitchPrediction = aiModelLoader.predictPitch(channelData, numSamples, currentSampleRate);
            
            if (pitchPrediction.confidence > 0.3f)
            {
                float currentPitch = pitchPrediction.frequency;
                float midiNote = Utils::frequencyToMidiNote(currentPitch);
                float targetNote = Utils::quantizeToScale(midiNote, key, scale);
                float targetFrequency = Utils::midiNoteToFrequency(targetNote);
                
                // Create synthesis parameters
                AIModelLoader::SynthesisParams synthParams;
                synthParams.fundamentalFreq = targetFrequency;
                synthParams.harmonicAmplitudes = pitchPrediction.harmonics;
                synthParams.loudness = amount * 0.01f;
                
                // Process with AI synthesis
                std::vector<float> tempBuffer(numSamples);
                std::copy(channelData, channelData + numSamples, tempBuffer.begin());
                
                if (aiModelLoader.processWithDDSP(tempBuffer.data(), channelData, numSamples, synthParams))
                {
                    // Blend original and AI-processed audio
                    float blendFactor = speed * 0.01f;
                    for (int i = 0; i < numSamples; ++i)
                    {
                        channelData[i] = tempBuffer[i] * (1.0f - blendFactor) + channelData[i] * blendFactor;
                    }
                }
            }
        }
        else
        {
            // Fallback to advanced pitch detection
            std::vector<float> pitches(numSamples);
            pitchEngine.detectPitchAdvanced(channelData, numSamples, pitches.data());
            
            // Apply intelligent correction
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float currentPitch = pitches[sample];
                
                if (currentPitch > 0.0f)
                {
                    float midiNote = Utils::frequencyToMidiNote(currentPitch);
                    float targetNote = Utils::quantizeToScale(midiNote, key, scale);
                    float targetFrequency = Utils::midiNoteToFrequency(targetNote);
                    
                    // AI-style correction with formant preservation
                    float pitchRatio = targetFrequency / currentPitch;
                    if (std::abs(pitchRatio - 1.0f) > 0.01f)
                    {
                        pitchEngine.correctPitchAI(&channelData[sample], 1, targetFrequency, speed, amount);
                    }
                }
            }
        }
    }
}

void AutoTuneAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
    ignoreUnused(parameterID, newValue);
    // Parameter changes are handled through smoothed values in processBlock
}

void AutoTuneAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AutoTuneAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(ValueTree::fromXml(*xmlState));
        }
    }
}

#ifdef MINIMAL_EDITOR
// Use minimal editor for compatibility
#include "PluginEditor_minimal.cpp"
#else
// Create full editor
AudioProcessorEditor* AutoTuneAudioProcessor::createEditor()
{
    return new AutoTuneAudioProcessorEditor(*this);
}
#endif

// This creates new instances of the plugin
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutoTuneAudioProcessor();
}

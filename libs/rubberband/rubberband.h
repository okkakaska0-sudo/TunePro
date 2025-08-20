#ifndef RUBBERBAND_H
#define RUBBERBAND_H

#include <cstddef>
#include <memory>

/**
 * Rubber Band Library
 * 
 * This is a high-quality software library for audio time-stretching and 
 * pitch-shifting. It permits you to change the tempo and pitch of an audio 
 * recording independently of one another.
 */

namespace RubberBand {

/**
 * This is the main interface to the Rubber Band library.
 *
 * Study this class's documentation and the demo application code.
 * The demo application itself is not installed by make install,
 * but its source is included in the source distribution.
 *
 * Note that this class was designed around the real-time constraints
 * of a DAW-style host. For a general-purpose stretching utility, the
 * FFTW implementation may be more suitable than the default.  See the
 * option flags below for more details.
 *
 * Multiple instances of RubberBandStretcher may be created and used
 * in separate threads concurrently. However, for any single instance,
 * you may not call process() more than once concurrently, and you may
 * not change the time or pitch ratio while a process() call is being
 * executed (if the stretcher was created in "real-time mode"; in
 * "offline mode" you can't change the ratios during use regardless).
 */
class RubberBandStretcher
{
public:
    /**
     * Processing options for the timestretcher. The preferred
     * options should normally be set in the constructor, as a bitwise
     * OR of the option flags. The default value (DefaultOptions) is
     * intended to give good results in most situations.
     *
     * 1. Realtime vs Offline mode. Use OptionProcessRealTime for a
     * real-time application, with small incremental blocks of samples.
     * Use OptionProcessOffline for batch processing of complete files.
     * OptionProcessRealTime is the default and usually preferable.
     *
     * 2. Stretch type. Use OptionStretchElastic for most audio.
     * Use OptionStretchPrecise for solo piano and non-percussive
     * music. OptionStretchElastic is the default.
     *
     * 3. Transient type. Use OptionTransientsSmooth for most audio.
     * Use OptionTransientsMixed for non-vocal music.
     * Use OptionTransientsCrisp for drum loops.
     * OptionTransientsSmooth is the default.
     *
     * 4. Phase type. Use OptionPhaseLaminar for most audio.
     * Use OptionPhaseIndependent for complex music.
     * OptionPhaseLaminar is the default.
     *
     * 5. Threading. OptionThreadingAuto selects multi or single
     * threaded code automatically. OptionThreadingNever runs
     * single-threaded always. OptionThreadingAlways runs
     * multi-threaded always. OptionThreadingAuto is the default.
     *
     * 6. Window size. OptionWindowStandard uses the default window
     * size, best for most situations. OptionWindowShort has a
     * shorter window, better for very fast changes. OptionWindowLong
     * has a longer window, better for stable pitch. The default is
     * OptionWindowStandard.
     *
     * 7. Smoothing. OptionSmoothingOff uses no special smoothing.
     * OptionSmoothingOn uses inter-frame smoothing. The default is
     * OptionSmoothingOff.
     *
     * 8. Formants. OptionFormantShifted adjusts the spectral envelope
     * to match the pitch shift. OptionFormantPreserved leaves the
     * spectral envelope unchanged. The default is OptionFormantShifted.
     *
     * 9. Pitch mode. OptionPitchHighSpeed gives rapid results, while
     * OptionPitchHighQuality gives slow but very good results, and
     * OptionPitchHighConsistency gives good results with better
     * consistency for small changes. The default is
     * OptionPitchHighSpeed.
     *
     * 10. Channels. OptionChannelsApart processes multi-channel audio
     * with each channel treated separately. OptionChannelsTogether
     * processes stereo as a single unit. The default is
     * OptionChannelsApart.
     */
    enum Option {

        OptionProcessRealTime   = 0x00000000,
        OptionProcessOffline    = 0x00000001,

        OptionStretchElastic    = 0x00000000,
        OptionStretchPrecise    = 0x00000010,
        
        OptionTransientsSmooth  = 0x00000000,
        OptionTransientsMixed   = 0x00000100,
        OptionTransientsCrisp   = 0x00000200,
        
        OptionDetectorCompound  = 0x00000000,
        OptionDetectorPercussive = 0x00000400,
        OptionDetectorSoft      = 0x00000800,
        
        OptionPhaseLaminar      = 0x00000000,
        OptionPhaseIndependent  = 0x00002000,
        
        OptionThreadingAuto     = 0x00000000,
        OptionThreadingNever    = 0x00010000,
        OptionThreadingAlways   = 0x00020000,

        OptionWindowStandard    = 0x00000000,
        OptionWindowShort       = 0x00100000,
        OptionWindowLong        = 0x00200000,

        OptionSmoothingOff      = 0x00000000,
        OptionSmoothingOn       = 0x00800000,

        OptionFormantShifted    = 0x00000000,
        OptionFormantPreserved  = 0x01000000,

        OptionPitchHighSpeed    = 0x00000000,
        OptionPitchHighQuality  = 0x02000000,
        OptionPitchHighConsistency = 0x04000000,

        OptionChannelsApart     = 0x00000000,
        OptionChannelsTogether  = 0x10000000,

        OptionEngineDefault     = 0x00000000,
        OptionEngineFaster      = 0x20000000,
        OptionEngineFiner       = 0x40000000
    };

    typedef int Options;

    static const Options DefaultOptions = 0x00000000;

    /**
     * Construct a time and pitch stretcher object to run at the given
     * sample rate, with the given number of channels. Processing
     * options and the initial time and pitch scaling ratios may also
     * be provided. The default time and pitch scaling ratios are 1.0
     * (no change); the default processing options are intended to
     * produce reasonable results for most uses.
     *
     * If the sample rate and channel count are not known at the time
     * the stretcher is constructed, you may pass zero values for
     * these and call setMaxProcessSize and setDefaultDebugLevel before
     * calling any of the other methods. However, if you do this, you
     * will not be able to use the stretcher in real-time mode.
     */
    RubberBandStretcher(size_t sampleRate,
                        size_t channels,
                        Options options = DefaultOptions,
                        double initialTimeRatio = 1.0,
                        double initialPitchScale = 1.0);

    /**
     * Destructor.
     */
    ~RubberBandStretcher();

    /**
     * Reset the stretcher's internal buffers. The stretcher should
     * subsequently behave as if it had just been constructed
     * (although retaining the current time and pitch ratio).
     */
    void reset();

    /**
     * Set the time ratio for the stretcher (i.e. the amount the audio
     * should be stretched by). 1.0 means no change, 0.5 means the
     * audio will be half the original length. 2.0 means the audio
     * will be twice the original length.
     *
     * Not all values will sound good (though all should sound
     * reasonable for modest changes).
     *
     * You should set the time ratio before calling study() or the
     * first process() call. You can change it subsequently, but doing
     * so may produce audible artifacts.
     *
     * The stretcher may reset internal buffers as a side-effect of
     * this call. (This happens when running in offline mode or when
     * the ratio has changed more than can be accommodated by adjusting
     * the buffer contents.)
     */
    void setTimeRatio(double ratio);

    /**
     * Set a pitch scaling factor for the stretcher. 1.0 means no
     * change, 0.5 means the audio will sound an octave down,
     * 2.0 means the audio will sound an octave up.
     *
     * You should set the pitch scale before calling study() or the
     * first process() call. You can change it subsequently, but doing
     * so may produce audible artifacts.
     *
     * The stretcher may reset internal buffers as a side-effect of
     * this call. (This happens when running in offline mode or when
     * the ratio has changed more than can be accommodated by adjusting
     * the buffer contents.)
     */
    void setPitchScale(double scale);

    /**
     * Return the last time ratio value that was set (either on
     * construction or with setTimeRatio()).
     */
    double getTimeRatio() const;

    /**
     * Return the last pitch scaling factor value that was set (either
     * on construction or with setPitchScale()).
     */
    double getPitchScale() const;

    /**
     * Return the processing latency of the stretcher. This is the
     * number of audio samples that one would have to discard at the
     * start of the output in order to ensure that the resulting audio
     * was aligned with the input audio (some audio is necessarily
     * introduced at the beginning because of the length of the
     * analysis window).
     */
    size_t getLatency() const;

    /**
     * Change an OptionTransients... option in the current processing
     * options. The stretcher retains all other option settings.
     */
    void setTransientsOption(Options options);

    /**
     * Change an OptionDetector... option in the current processing
     * options. The stretcher retains all other option settings.
     */
    void setDetectorOption(Options options);

    /**
     * Change an OptionPhase... option in the current processing
     * options. The stretcher retains all other option settings.
     */
    void setPhaseOption(Options options);

    /**
     * Change an OptionFormant... option in the current processing
     * options. The stretcher retains all other option settings.
     */
    void setFormantOption(Options options);

    /**
     * Change an OptionPitch... option in the current processing
     * options. The stretcher retains all other option settings.
     */
    void setPitchOption(Options options);

    /**
     * Tell the stretcher exactly how many input samples it will
     * receive. This is only useful in offline mode, when it allows
     * the stretcher to ensure that the number of output samples is
     * exactly correct. In real-time mode it is ignored.
     */
    void setExpectedInputDuration(size_t samples);

    /**
     * Tell the stretcher the maximum number of sample frames that you
     * will ever pass to a single process() call. If you don't call
     * this, the default limit is 16384 frames. If you call process()
     * with more frames than the limit, the excess will be ignored.
     */
    void setMaxProcessSize(size_t samples);

    /**
     * Tell the stretcher how many frames to process before the
     * stretcher produces its first output. The default value is
     * 100; if you need an accurate input-output sample count
     * relationship, you may need to set this to 0 and discard the
     * first getLatency() frames of output.
     */
    void setKeyFrameMap(const std::map<size_t, size_t> &mapping);

    /**
     * Provide a set of mappings from source sample frame to target
     * sample frame, such as may be produced by the study() function.
     * This is used when running in offline mode to produce correct
     * output timing when the input has been analysed in advance to
     * produce time mappings. This function cannot be called while any
     * processing calls are ongoing.
     */
    void setKeyFrameMap(const KeyFrameMap &mapping);

    /**
     * In offline mode, you should call this function before each
     * process() call, providing it with the input you are about to
     * process. This enables the stretcher to provide better results
     * by looking ahead to see what is coming. It is also more
     * efficient to do this than to run process() on the input
     * multiple times.
     *
     * Supply the input frames here, exactly as you will supply them
     * to the subsequent process() call (but note that the final
     * parameter to this call should be true if this is the last call
     * in a series, i.e. no more input remains). The return value is
     * the number of frames that were actually used from the input.
     * 
     * In real-time mode, this function is ignored and will simply
     * return the frame count.
     */
    size_t study(const float *const *input,
                 size_t samples,
                 bool final);

    /**
     * Process the given interleaved audio data, returning the number of
     * output samples available for reading.
     * 
     * Supply pointer to float arrays with the audio data to be
     * stretched, with samples frames of data available across
     * channels channels.
     * 
     * The input must be provided in the form of a set of float arrays
     * (one per channel) as float *input[], not as a single
     * interleaved array.  The output is obtained by calling
     * retrieve().  Note that the number of output samples available to
     * read may not be the same as the number of samples provided as
     * input.
     * 
     * set final to true if this is the final block of data in the
     * input; if so, the stretcher will ensure that all remaining
     * output data will be available from retrieve().  (Note: some
     * code, including the official demo program, fails to set the
     * final flag appropriately.  Usually the result is that a small
     * amount of audio is lost from the end of the file.  Be sure to
     * set this flag properly!)
     * 
     * The return value is an estimate of the number of output frames
     * available for reading via retrieve().
     * 
     * Note that because of the nature of the stretching algorithm,
     * there is likely to be a delay between when you provide input to
     * the stretcher and when you obtain usable output.  Use
     * getLatency() to discover this latency.  Additionally, the number
     * of output frames available after a given input is not
     * necessarily predictable.  You should call available() to
     * discover how many frames can be read using retrieve().
     */
    void process(const float *const *input,
                 size_t samples,
                 bool final);

    /**
     * Ask the stretcher how many frames of output data are available
     * for reading (via retrieve()).
     */
    int available() const;

    /**
     * Obtain output data from the stretcher.  Up to frames frames of
     * output data will be retrieved and stored in the provided
     * float arrays (one per channel, supplied as
     * output[]).
     *
     * The return value is the actual number of frames retrieved.
     */
    size_t retrieve(float *const *output, size_t frames) const;

    /**
     * Return the value of internal frequency cutoff value n.
     */
    float getFrequencyCutoff(int n) const;

    /**
     * Set the value of internal frequency cutoff n to f Hz.
     */
    void setFrequencyCutoff(int n, float f);

    /**
     * Return the hop size used internally for processing. This may be
     * of interest for certain technical applications. It is not
     * otherwise useful to the client.
     */
    size_t getInputIncrement() const;

    /**
     * Return the output increment used internally for processing.
     * See getInputIncrement().
     */
    size_t getOutputIncrement() const;

    /**
     * In offline mode, set the phase coherence level for multi-channel
     * processing. Some values may produce better results for certain
     * types of audio than the default. The default value is 50.
     */
    void setPhaseOption(int level);

    /**
     * Return the version number of the Rubber Band library.
     */
    static std::string getLibraryVersion();

protected:
    class Impl;
    Impl *m_d;
};

}

#endif
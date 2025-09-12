/**
 * @file audio_engine.h
 * @brief SONAR Audio Engine - Shared Library Interface
 * @author Mojibake Development Team
 * @version 1.0.0
 * 
 * This header defines the API for the SONAR audio engine shared library.
 * Provides functions for audio initialization, playback, WAV generation,
 * and file output capabilities for data sonification.
 */

#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#ifdef _WIN32
    #ifdef BUILDING_AUDIO_ENGINE
        #define AUDIO_API __declspec(dllexport) /**< Windows DLL export */
    #else
        #define AUDIO_API __declspec(dllimport) /**< Windows DLL import */
    #endif
#else
    #define AUDIO_API __attribute__((visibility("default"))) /**< Unix shared library export */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Audio sample structure for linked list storage
 * 
 * Represents a single audio sample with frequency, amplitude, duration,
 * and source byte information. Must match the structure in SONAR extension.
 */
typedef struct audio_sample_node {
    double frequency;           /**< Frequency in Hz (e.g., 440.0 for A4) */
    double amplitude;          /**< Amplitude (0.0 - 1.0, where 1.0 is maximum) */
    double duration;           /**< Duration in seconds (e.g., 0.05 for 50ms) */
    unsigned char source_byte; /**< Original byte value that generated this sample */
    struct audio_sample_node *next; /**< Pointer to next node in linked list */
} audio_sample_node_t;

/**
 * @brief Initialize the audio system
 * 
 * Initializes the audio engine with the specified sample rate.
 * Must be called before any other audio functions.
 * 
 * @param sample_rate Audio sample rate in Hz (e.g., 44100)
 * @return 0 on success, negative value on error
 */
AUDIO_API int init_audio(int sample_rate);

/**
 * @brief Play a single frequency tone
 * 
 * Plays a tone at the specified frequency, amplitude, and duration.
 * 
 * @param frequency Frequency in Hz (e.g., 440.0 for A4)
 * @param amplitude Amplitude (0.0 - 1.0, where 1.0 is maximum)
 * @param duration Duration in seconds
 * @return 0 on success, negative value on error
 */
AUDIO_API int play_frequency(double frequency, double amplitude, double duration);

/**
 * @brief Generate WAV file from audio samples
 * 
 * Creates a WAV audio file from the provided linked list of audio samples.
 * Includes harmonics and envelope shaping for enhanced audio quality.
 * 
 * @param filename Output WAV filename
 * @param samples Pointer to first node in audio sample linked list
 * @return 0 on success, negative value on error
 */
AUDIO_API int generate_wav(const char* filename, audio_sample_node_t* samples);

/**
 * @brief Cleanup and shutdown audio system
 * 
 * Releases all audio resources and shuts down the audio engine.
 * Should be called when audio processing is complete.
 */
AUDIO_API void cleanup_audio(void);

/**
 * @brief Set active audio output device
 * 
 * Selects the audio device to use for playback.
 * 
 * @param device_id ID of the audio device to use
 * @return 0 on success, negative value on error
 */
AUDIO_API int set_audio_device(int device_id);

/**
 * @brief Get list of available audio devices
 * 
 * Retrieves a list of available audio output devices.
 * 
 * @param device_list Buffer to store device list (semicolon-separated)
 * @param max_length Maximum length of device_list buffer
 * @return Number of devices found, negative value on error
 */
AUDIO_API int get_audio_devices(char* device_list, int max_length);

/**
 * @brief Play entire audio sample list
 * 
 * Plays all audio samples in the linked list sequentially.
 * 
 * @param head Pointer to first node in audio sample linked list
 * @return 0 on success, negative value on error
 */
AUDIO_API int play_sample_list(audio_sample_node_t* head);

/**
 * @brief Set master volume level
 * 
 * Adjusts the master volume for all audio output.
 * 
 * @param volume Volume level (0.0 - 1.0, where 1.0 is maximum)
 * @return 0 on success, negative value on error
 */
AUDIO_API int set_master_volume(double volume);

/**
 * @brief Apply audio effect to output
 * 
 * Applies the specified audio effect with the given parameter.
 * 
 * @param effect_name Name of the effect to apply
 * @param parameter Effect parameter value
 * @return 0 on success, negative value on error
 */
AUDIO_API int apply_audio_effect(const char* effect_name, double parameter);

/**
 * @brief Generate detailed analysis report
 * 
 * Creates a text file containing detailed analysis of the audio samples
 * including statistics, frequency ranges, and sample data.
 * 
 * @param samples Pointer to first node in audio sample linked list
 * @param filename Base filename for the report (without extension)
 * @return 0 on success, negative value on error
 */
AUDIO_API int generate_analysis_report(audio_sample_node_t* samples, const char* filename);

/**
 * @brief Generate CSV frequency data file
 * 
 * Creates a CSV file containing frequency data for each sample
 * with columns for sample index, byte value, frequency, amplitude, and duration.
 * 
 * @param samples Pointer to first node in audio sample linked list
 * @param filename Base filename for the CSV file (without extension)
 * @return 0 on success, negative value on error
 */
AUDIO_API int generate_frequency_data(audio_sample_node_t* samples, const char* filename);

/**
 * @brief Generate JSON metadata file
 * 
 * Creates a JSON file containing structured metadata about the audio samples
 * including engine information, configuration, and sample data.
 * 
 * @param samples Pointer to first node in audio sample linked list
 * @param filename Base filename for the JSON file (without extension)
 * @return 0 on success, negative value on error
 */
AUDIO_API int generate_metadata_json(audio_sample_node_t* samples, const char* filename);

/**
 * @brief Get audio engine library version
 * 
 * Returns the version string of the audio engine library.
 * 
 * @return Pointer to version string (e.g., "AudioEngine 1.0.0")
 */
AUDIO_API const char* get_library_version(void);

/**
 * @brief Get audio engine library name
 * 
 * Returns the name of the audio engine library.
 * 
 * @return Pointer to library name string (e.g., "SONAR Audio Engine")
 */
AUDIO_API const char* get_library_name(void);

/**
 * @brief Get supported sample rates
 * 
 * Retrieves an array of supported audio sample rates.
 * 
 * @param rates Array to store supported sample rates
 * @param max_count Maximum number of rates to store
 * @return Number of supported sample rates, negative value on error
 */
AUDIO_API int get_supported_sample_rates(int* rates, int max_count);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_ENGINE_H
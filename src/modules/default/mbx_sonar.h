/**
 * @file mbx_sonar.h
 * @brief SONAR Extension - Convert file data to audio using dynamic libraries
 * @author Mojibake Development Team
 * @version 1.0.0
 * 
 * The SONAR extension transforms binary file data into audio frequencies,
 * enabling data sonification and audio-based file analysis. Uses linked lists
 * for efficient audio sample management and dynamic libraries for audio processing.
 */

#ifndef MBX_SONAR_H
#define MBX_SONAR_H
#include <stdbool.h>
#include "mojibake/mojibake.h"

/**
 * @brief Audio sample node for linked list storage
 * 
 * Represents a single audio sample generated from a byte value,
 * containing frequency, amplitude, duration, and source information.
 */
typedef struct audio_sample_node {
    double frequency;           /**< Frequency in Hz (e.g., 440.0 for A4) */
    double amplitude;          /**< Amplitude (0.0 - 1.0, where 1.0 is maximum) */
    double duration;           /**< Duration in seconds (e.g., 0.05 for 50ms) */
    unsigned char source_byte; /**< Original byte value that generated this sample */
    struct audio_sample_node *next; /**< Pointer to next node in linked list */
} audio_sample_node_t;

/**
 * @brief Audio configuration structure
 * 
 * Contains all parameters needed for audio generation including
 * sample rate, frequency mapping, and library usage settings.
 */
typedef struct {
    int sample_rate;           /**< Audio sample rate in Hz (e.g., 44100) */
    double base_frequency;     /**< Base frequency for mapping in Hz (e.g., 220 Hz - A3) */
    double frequency_range;    /**< Frequency range in Hz (e.g., 2000 Hz) */
    double sample_duration;    /**< Duration per byte sample in seconds (e.g., 0.05) */
    bool use_dynamic_lib;      /**< Whether to use dynamic audio library for playback */
} sonar_config_t;

/**
 * @brief Function pointers for dynamic library loading
 * 
 * Structure containing function pointers to dynamically loaded
 * audio library functions for enhanced audio processing capabilities.
 */
typedef struct {
    void* lib_handle;          /**< Handle to loaded shared library */
    int (*init_audio)(int sample_rate); /**< Initialize audio system */
    int (*play_frequency)(double freq, double amp, double duration); /**< Play single frequency */
    int (*generate_wav)(const char* filename, audio_sample_node_t* samples); /**< Generate WAV file */
    void (*cleanup_audio)(void); /**< Cleanup audio system */
    int (*generate_analysis_report)(audio_sample_node_t* samples, const char* filename); /**< Generate analysis report */
    int (*generate_frequency_data)(audio_sample_node_t* samples, const char* filename); /**< Generate CSV frequency data */
    int (*generate_metadata_json)(audio_sample_node_t* samples, const char* filename); /**< Generate JSON metadata */
} audio_lib_t;

/**
 * @brief Main SONAR module function
 * 
 * Processes a file partition and converts it to audio frequencies.
 * Generates WAV files, analysis reports, and metadata.
 * 
 * @param target Pointer to mojibake target structure containing file data
 * @param index Partition index to process
 * @param arg Pointer to sonar_config_t configuration structure
 * @return true if processing successful, false otherwise
 */
bool mbx_sonar(mojibake_target_t *target, unsigned int index, void *arg);

/**
 * @brief Create audio sample from byte value
 * 
 * Converts a single byte to an audio sample with frequency, amplitude,
 * and duration based on the provided configuration.
 * 
 * @param byte Input byte value (0-255)
 * @param config Pointer to SONAR configuration structure
 * @return Pointer to newly created audio sample node, NULL on failure
 */
audio_sample_node_t* create_audio_sample(unsigned char byte, sonar_config_t *config);

/**
 * @brief Add sample to linked list
 * 
 * Appends a new audio sample to the end of the linked list.
 * 
 * @param head Pointer to pointer of list head (may be modified)
 * @param new_sample Pointer to audio sample to add
 */
void add_sample_to_list(audio_sample_node_t **head, audio_sample_node_t *new_sample);

/**
 * @brief Play audio sample list
 * 
 * Plays all audio samples in the linked list using the provided audio library.
 * 
 * @param head Pointer to first node in audio sample list
 * @param audio_lib Pointer to loaded audio library structure
 */
void play_audio_list(audio_sample_node_t *head, audio_lib_t *audio_lib);

/**
 * @brief Free audio sample linked list
 * 
 * Deallocates memory for all nodes in the audio sample linked list.
 * 
 * @param head Pointer to first node in list to free
 */
void free_audio_list(audio_sample_node_t *head);

/**
 * @brief Load dynamic audio library
 * 
 * Dynamically loads the audio engine library and initializes function pointers.
 * 
 * @param audio_lib Pointer to audio library structure to populate
 * @param lib_path Path to the audio library file (e.g., "lib/audio_engine.dll")
 * @return true if library loaded successfully, false otherwise
 */
bool load_audio_library(audio_lib_t *audio_lib, const char *lib_path);

/**
 * @brief Unload dynamic audio library
 * 
 * Unloads the previously loaded audio library and cleans up resources.
 * 
 * @param audio_lib Pointer to audio library structure to cleanup
 */
void unload_audio_library(audio_lib_t *audio_lib);

/**
 * @brief Generate WAV file from audio samples
 * 
 * Creates a WAV audio file from the provided audio sample linked list.
 * 
 * @param head Pointer to first node in audio sample list
 * @param filename Output WAV filename
 * @param config Pointer to SONAR configuration structure
 */
void generate_wav_file(audio_sample_node_t *head, const char *filename, sonar_config_t *config);

/**
 * @brief Map byte value to audio frequency
 * 
 * Converts a byte value (0-255) to an audio frequency based on
 * the base frequency and frequency range in the configuration.
 * 
 * @param byte Input byte value (0-255)
 * @param config Pointer to SONAR configuration structure
 * @return Calculated frequency in Hz
 */
double map_byte_to_frequency(unsigned char byte, sonar_config_t *config);

/**
 * @brief Map byte value to audio amplitude
 * 
 * Converts a byte value (0-255) to an audio amplitude (0.0-1.0).
 * 
 * @param byte Input byte value (0-255)
 * @return Calculated amplitude (0.0-1.0)
 */
double map_byte_to_amplitude(unsigned char byte);

#endif
/**
 * @file mbx_dsonar.h
 * @brief dSONAR Extension - Reverse Audio to Data Conversion
 * @author Mojibake Development Team
 * @version 1.0.0
 * 
 * The dSONAR extension reconstructs original binary data from SONAR-generated
 * audio files, CSV frequency data, JSON metadata, or analysis reports.
 * Supports multiple input formats and provides confidence scoring for reconstruction quality.
 */

#ifndef MBX_DSONAR_H
#define MBX_DSONAR_H
#include <stdbool.h>
#include "mojibake/mojibake.h"

/**
 * @brief Reverse audio sample node for reconstruction
 * 
 * Represents a single reconstructed sample with confidence scoring
 * and position tracking for accurate data reconstruction.
 */
typedef struct reverse_sample_node {
    unsigned char reconstructed_byte;  /**< Reconstructed byte value (0-255) */
    double source_frequency;          /**< Original frequency from audio in Hz */
    double confidence_score;          /**< Reconstruction confidence (0.0-1.0) */
    int sample_index;                 /**< Position in original sequence */
    struct reverse_sample_node *next; /**< Pointer to next node in linked list */
} reverse_sample_node_t;

/**
 * @brief dSONAR configuration structure
 * 
 * Contains parameters for reverse audio-to-data conversion including
 * frequency mapping, tolerance settings, and input format preferences.
 */
typedef struct {
    double base_frequency;            /**< Base frequency for reverse mapping in Hz */
    double frequency_range;           /**< Frequency range for reverse mapping in Hz */
    double tolerance;                 /**< Frequency matching tolerance in Hz */
    bool strict_mode;                 /**< Enable strict frequency matching */
    char* input_format;               /**< Input format: "wav", "csv", "json", "auto" */
} dsonar_config_t;

/**
 * @brief Input source types enumeration
 * 
 * Defines the supported input formats for dSONAR reconstruction.
 */
typedef enum {
    DSONAR_INPUT_WAV,      /**< WAV audio file input */
    DSONAR_INPUT_CSV,      /**< CSV frequency data input */
    DSONAR_INPUT_JSON,     /**< JSON metadata input */
    DSONAR_INPUT_ANALYSIS, /**< Analysis report input */
    DSONAR_INPUT_AUTO      /**< Automatic format detection */
} dsonar_input_type_t;

/**
 * @brief Reconstruction result structure
 * 
 * Contains the reconstructed binary data along with quality metrics
 * and statistics about the reconstruction process.
 */
typedef struct {
    unsigned char* reconstructed_data; /**< Reconstructed binary data buffer */
    int data_length;                  /**< Length of reconstructed data in bytes */
    double average_confidence;        /**< Average reconstruction confidence (0.0-1.0) */
    int successful_samples;           /**< Number of successfully reconstructed samples */
    int total_samples;                /**< Total number of samples processed */
} dsonar_result_t;

/**
 * @brief Main dSONAR module function
 * 
 * Processes a file partition and attempts to reconstruct original binary data
 * from SONAR-generated audio files, CSV data, JSON metadata, or analysis reports.
 * 
 * @param target Pointer to mojibake target structure containing file data
 * @param index Partition index to process
 * @param arg Pointer to dsonar_config_t configuration structure
 * @return true if reconstruction successful, false otherwise
 */
bool mbx_dsonar(mojibake_target_t *target, unsigned int index, void *arg);

/**
 * @brief Reconstruct data from WAV audio file
 * 
 * Analyzes WAV audio file and reconstructs original binary data using
 * frequency analysis and zero-crossing detection algorithms.
 * 
 * @param wav_filename Path to input WAV file
 * @param config Pointer to dSONAR configuration structure
 * @return Pointer to reconstruction result, NULL on failure
 */
dsonar_result_t* reconstruct_from_wav(const char* wav_filename, dsonar_config_t* config);

/**
 * @brief Reconstruct data from CSV frequency file
 * 
 * Reads CSV frequency data and reconstructs original binary data using
 * direct frequency-to-byte mapping for highest accuracy.
 * 
 * @param csv_filename Path to input CSV frequency file
 * @param config Pointer to dSONAR configuration structure
 * @return Pointer to reconstruction result, NULL on failure
 */
dsonar_result_t* reconstruct_from_csv(const char* csv_filename, dsonar_config_t* config);

/**
 * @brief Reconstruct data from JSON metadata file
 * 
 * Parses JSON metadata and reconstructs original binary data using
 * stored frequency and amplitude information.
 * 
 * @param json_filename Path to input JSON metadata file
 * @param config Pointer to dSONAR configuration structure
 * @return Pointer to reconstruction result, NULL on failure
 */
dsonar_result_t* reconstruct_from_json(const char* json_filename, dsonar_config_t* config);

/**
 * @brief Reconstruct data from analysis report file
 * 
 * Parses text analysis report and reconstructs original binary data using
 * frequency information extracted from the report.
 * 
 * @param analysis_filename Path to input analysis report file
 * @param config Pointer to dSONAR configuration structure
 * @return Pointer to reconstruction result, NULL on failure
 */
dsonar_result_t* reconstruct_from_analysis(const char* analysis_filename, dsonar_config_t* config);

/**
 * @brief Extract frequency from WAV file at specific sample index
 * 
 * @param wav_filename Path to WAV file
 * @param sample_index Index of sample to analyze
 * @return Extracted frequency in Hz, 0.0 on failure
 */
double extract_frequency_from_wav(const char* wav_filename, int sample_index);

/**
 * @brief Convert frequency back to byte value
 * 
 * @param frequency Input frequency in Hz
 * @param config Pointer to dSONAR configuration structure
 * @return Reconstructed byte value (0-255)
 */
unsigned char frequency_to_byte(double frequency, dsonar_config_t* config);

/**
 * @brief Calculate reconstruction confidence score
 * 
 * @param target_freq Expected frequency in Hz
 * @param actual_freq Detected frequency in Hz
 * @param tolerance Frequency tolerance in Hz
 * @return Confidence score (0.0-1.0)
 */
double calculate_confidence(double target_freq, double actual_freq, double tolerance);

/**
 * @brief Create reverse sample node
 * 
 * @param frequency Source frequency in Hz
 * @param index Sample index in sequence
 * @param config Pointer to dSONAR configuration structure
 * @return Pointer to new reverse sample node, NULL on failure
 */
reverse_sample_node_t* create_reverse_sample(double frequency, int index, dsonar_config_t* config);

/**
 * @brief Add reverse sample to linked list
 * 
 * @param head Pointer to pointer of list head (may be modified)
 * @param sample Pointer to reverse sample to add
 */
void add_reverse_sample(reverse_sample_node_t** head, reverse_sample_node_t* sample);

/**
 * @brief Convert reverse sample list to byte array
 * 
 * @param head Pointer to first node in reverse sample list
 * @param length Pointer to store output array length
 * @return Pointer to allocated byte array, NULL on failure
 */
unsigned char* samples_to_bytes(reverse_sample_node_t* head, int* length);

/**
 * @brief Free reverse sample linked list
 * 
 * @param head Pointer to first node in list to free
 */
void free_reverse_samples(reverse_sample_node_t* head);

/**
 * @brief Free dSONAR result structure
 * 
 * @param result Pointer to result structure to free
 */
void free_dsonar_result(dsonar_result_t* result);

/**
 * @brief Detect input file format automatically
 * 
 * @param filename Path to input file
 * @return Detected input format type
 */
dsonar_input_type_t detect_input_format(const char* filename);

/**
 * @brief Parse CSV frequency data file
 * 
 * @param filename Path to CSV file
 * @param samples Pointer to store parsed samples
 * @param config Pointer to dSONAR configuration structure
 * @return true if parsing successful, false otherwise
 */
bool parse_csv_frequency_data(const char* filename, reverse_sample_node_t** samples, dsonar_config_t* config);

/**
 * @brief Parse JSON metadata file
 * 
 * @param filename Path to JSON file
 * @param samples Pointer to store parsed samples
 * @param config Pointer to dSONAR configuration structure
 * @return true if parsing successful, false otherwise
 */
bool parse_json_metadata(const char* filename, reverse_sample_node_t** samples, dsonar_config_t* config);

/**
 * @brief Parse analysis report file
 * 
 * @param filename Path to analysis report file
 * @param samples Pointer to store parsed samples
 * @param config Pointer to dSONAR configuration structure
 * @return true if parsing successful, false otherwise
 */
bool parse_analysis_report(const char* filename, reverse_sample_node_t** samples, dsonar_config_t* config);

/**
 * @brief Analyze WAV file frequencies
 * 
 * @param filename Path to WAV file
 * @param samples Pointer to store analyzed samples
 * @param config Pointer to dSONAR configuration structure
 * @return true if analysis successful, false otherwise
 */
bool analyze_wav_frequencies(const char* filename, reverse_sample_node_t** samples, dsonar_config_t* config);

/**
 * @brief Read WAV file header information
 * 
 * @param wav_file Pointer to open WAV file
 * @param sample_rate Pointer to store sample rate
 * @param channels Pointer to store channel count
 * @param bits_per_sample Pointer to store bits per sample
 * @return 1 if successful, 0 on failure
 */
int read_wav_header(FILE* wav_file, int* sample_rate, int* channels, int* bits_per_sample);

/**
 * @brief Extract frequency spectrum from WAV file
 * 
 * @param wav_filename Path to WAV file
 * @param spectrum_length Pointer to store spectrum length
 * @return Pointer to frequency spectrum array, NULL on failure
 */
double* extract_frequency_spectrum(const char* wav_filename, int* spectrum_length);

/**
 * @brief Save reconstructed data to file
 * 
 * @param output_filename Path to output file
 * @param result Pointer to reconstruction result
 * @return true if save successful, false otherwise
 */
bool save_reconstructed_data(const char* output_filename, dsonar_result_t* result);

/**
 * @brief Validate reconstruction against original file
 * 
 * @param result Pointer to reconstruction result
 * @param original_filename Path to original file for comparison
 * @return true if validation successful, false otherwise
 */
bool validate_reconstruction(dsonar_result_t* result, const char* original_filename);

/**
 * @brief Print detailed reconstruction report
 * 
 * @param result Pointer to reconstruction result
 * @param source_filename Path to source file used for reconstruction
 */
void print_reconstruction_report(dsonar_result_t* result, const char* source_filename);

/**
 * @brief Auto-detect input format and reconstruct data
 * 
 * @param input_pattern Input file pattern or filename
 * @param output_filename Output filename for reconstructed data
 * @param config Pointer to dSONAR configuration structure
 * @return true if reconstruction successful, false otherwise
 */
bool auto_detect_and_reconstruct(const char* input_pattern, const char* output_filename, dsonar_config_t* config);

/**
 * @brief Batch reconstruct multiple partition files
 * 
 * @param partition_pattern Pattern for partition filenames
 * @param output_prefix Prefix for output filenames
 * @param config Pointer to dSONAR configuration structure
 * @return true if batch reconstruction successful, false otherwise
 */
bool batch_reconstruct_partitions(const char* partition_pattern, const char* output_prefix, dsonar_config_t* config);

/**
 * @brief Calculate reconstruction accuracy percentage
 * 
 * @param original Pointer to original data
 * @param reconstructed Pointer to reconstructed data
 * @param length Length of data to compare
 * @return Accuracy percentage (0.0-100.0)
 */
double calculate_reconstruction_accuracy(const unsigned char* original, const unsigned char* reconstructed, int length);

/**
 * @brief Combine multiple partition files into single file
 * 
 * @param partition_count Number of partition files to combine
 * @param original_filename Base filename for output
 * @return true if combination successful, false otherwise
 */
bool combine_partition_files(int partition_count, const char* original_filename);

/**
 * @brief Batch process multiple partitions with dSONAR
 * 
 * @param target Pointer to mojibake target structure
 * @param arg Pointer to dSONAR configuration structure
 * @return true if batch processing successful, false otherwise
 */
bool mbx_dsonar_batch_process(mojibake_target_t *target, void *arg);

#endif
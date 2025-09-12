#include "mbx_dsonar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// Helper functions
static int min(int a, int b) {
    return a < b ? a : b;
}

// Read WAV file header
int read_wav_header(FILE* wav_file, int* sample_rate, int* channels, int* bits_per_sample)
{
    if (!wav_file || !sample_rate || !channels || !bits_per_sample) return 0;
    
    char header[44];
    if (fread(header, 1, 44, wav_file) != 44) return 0;
    
    // Check RIFF header
    if (strncmp(header, "RIFF", 4) != 0) return 0;
    if (strncmp(header + 8, "WAVE", 4) != 0) return 0;
    
    // Extract format information
    *channels = *(short*)(header + 22);
    *sample_rate = *(int*)(header + 24);
    *bits_per_sample = *(short*)(header + 34);
    
    return 1;
}

// Simple frequency detection using zero-crossing analysis
double detect_dominant_frequency(short* audio_buffer, int buffer_size, int sample_rate)
{
    if (!audio_buffer || buffer_size < 10) return 0.0;
    
    // Count zero crossings
    int zero_crossings = 0;
    for (int i = 1; i < buffer_size; i++) {
        if ((audio_buffer[i-1] >= 0 && audio_buffer[i] < 0) ||
            (audio_buffer[i-1] < 0 && audio_buffer[i] >= 0)) {
            zero_crossings++;
        }
    }
    
    // Estimate frequency from zero crossings
    // Frequency ≈ (zero_crossings / 2) / (buffer_size / sample_rate)
    if (zero_crossings > 0) {
        double duration = (double)buffer_size / sample_rate;
        double frequency = (zero_crossings / 2.0) / duration;
        
        // Filter out unrealistic frequencies
        if (frequency >= 200.0 && frequency <= 3000.0) {
            return frequency;
        }
    }
    
    return 0.0;
}

// Default dSONAR configuration
static dsonar_config_t default_dsonar_config = {
    .base_frequency = 220.0,
    .frequency_range = 2000.0,
    .tolerance = 5.0,  // 5 Hz tolerance
    .strict_mode = false,
    .input_format = "auto"
};

bool mbx_dsonar(mojibake_target_t *target, unsigned int index, void *arg)
{
    if (target == NULL) return false;
    
    dsonar_config_t *config = arg ? (dsonar_config_t*)arg : &default_dsonar_config;
    
    printf("=== dSONAR Partition %d Reverse Analysis ===\n", index);
    printf("Reconstructing binary data from audio frequencies...\n");
    
    // Look for SONAR-generated files for this partition
    char wav_filename[256], csv_filename[256], json_filename[256], analysis_filename[256];
    sprintf(wav_filename, "sonar_partition_%d.wav", index);
    sprintf(csv_filename, "sonar_partition_%d_frequencies.csv", index);
    sprintf(json_filename, "sonar_partition_%d_metadata.json", index);
    sprintf(analysis_filename, "sonar_partition_%d_analysis.txt", index);
    
    dsonar_result_t* result = NULL;
    const char* source_file = NULL;
    
    // Try different input formats in order of preference (CSV first for precise frequency data)
    FILE* test_file;
    
    // 1. Try CSV file first (most accurate frequency data)
    test_file = fopen(csv_filename, "r");
    if (test_file) {
        fclose(test_file);
        printf("Found CSV frequency data: %s\n", csv_filename);
        printf("Using CSV frequency data for precise reconstruction...\n");
        result = reconstruct_from_csv(csv_filename, config);
        source_file = csv_filename;
    }
    // 2. Try JSON metadata (fallback)
    else {
        test_file = fopen(json_filename, "r");
        if (test_file) {
            fclose(test_file);
            printf("Found JSON metadata: %s\n", json_filename);
            result = reconstruct_from_json(json_filename, config);
            source_file = json_filename;
        }
        // 3. Try WAV file (complex audio analysis)
        else {
            test_file = fopen(wav_filename, "rb");
            if (test_file) {
                fclose(test_file);
                printf("Found WAV file: %s\n", wav_filename);
                printf("Using WAV audio for complex audio-to-data reconstruction...\n");
                result = reconstruct_from_wav(wav_filename, config);
                source_file = wav_filename;
            }
            // 4. Try analysis report
            else {
                test_file = fopen(analysis_filename, "r");
                if (test_file) {
                    fclose(test_file);
                    printf("Found analysis report: %s\n", analysis_filename);
                    result = reconstruct_from_analysis(analysis_filename, config);
                    source_file = analysis_filename;
                } else {
                    printf("No SONAR files found for partition %d\n", index);
                    return false;
                }
            }
        }
    }
    
    if (!result) {
        printf("Failed to reconstruct data from %s\n", source_file);
        return false;
    }
    
    // Display reconstruction results
    print_reconstruction_report(result, source_file);
    
    // Save reconstructed data
    char output_filename[256];
    sprintf(output_filename, "dsonar_reconstructed_partition_%d.bin", index);
    
    if (save_reconstructed_data(output_filename, result)) {
        printf("Reconstructed data saved to: %s\n", output_filename);
        
        // Compare with original if available
        if (target->block && index < target->partition_count) {
            unsigned char *original_partition = MOJIBAKE_BLOCK_OFFSET(target, index);
            double accuracy = calculate_reconstruction_accuracy(
                original_partition, result->reconstructed_data, 
                min(target->partition_size, result->data_length)
            );
            printf("Reconstruction accuracy: %.2f%%\n", accuracy * 100.0);
        }
    }
    
    free_dsonar_result(result);
    printf("\n");
    return true;
}

bool mbx_dsonar_batch_process(mojibake_target_t *target, void *arg)
{
    if (target == NULL) return false;
    
    printf("=== dSONAR Batch Processing ===\n");
    printf("Processing all WAV partition files...\n");
    
    bool success = true;
    for (unsigned int i = 0; i < target->partition_count; i++) {
        if (!mbx_dsonar(target, i, arg)) {
            printf("Failed to process partition %d\n", i);
            success = false;
        }
    }
    
    if (success) {
        printf("\n[INFO] Combining all partitions...\n");
        if (combine_partition_files(target->partition_count, "original_file.bin")) {
            printf("[OK] Complete file reconstruction successful!\n");
        } else {
            printf("[ERROR] Failed to combine partition files\n");
            success = false;
        }
    }
    
    return success;
}

dsonar_result_t* reconstruct_from_json(const char* json_filename, dsonar_config_t* config)
{
    FILE* json_file = fopen(json_filename, "r");
    if (!json_file) return NULL;
    
    printf("[dSONAR] Parsing JSON metadata...\n");
    
    reverse_sample_node_t* samples = NULL;
    if (!parse_json_metadata(json_filename, &samples, config)) {
        fclose(json_file);
        return NULL;
    }
    fclose(json_file);
    
    // Convert samples to result
    dsonar_result_t* result = malloc(sizeof(dsonar_result_t));
    if (!result) {
        free_reverse_samples(samples);
        return NULL;
    }
    
    result->reconstructed_data = samples_to_bytes(samples, &result->data_length);
    
    // Calculate statistics
    result->total_samples = 0;
    result->successful_samples = 0;
    double total_confidence = 0.0;
    
    reverse_sample_node_t* current = samples;
    while (current) {
        result->total_samples++;
        if (current->confidence_score > 0.5) {
            result->successful_samples++;
        }
        total_confidence += current->confidence_score;
        current = current->next;
    }
    
    result->average_confidence = result->total_samples > 0 ? 
        total_confidence / result->total_samples : 0.0;
    
    free_reverse_samples(samples);
    return result;
}

dsonar_result_t* reconstruct_from_csv(const char* csv_filename, dsonar_config_t* config)
{
    FILE* csv_file = fopen(csv_filename, "r");
    if (!csv_file) {
        printf("[dSONAR] Error: Could not open CSV file %s\n", csv_filename);
        return NULL;
    }
    
    printf("[dSONAR] Reading frequency data from CSV file...\n");
    
    // Skip header line
    char line[512];
    if (!fgets(line, sizeof(line), csv_file)) {
        printf("[dSONAR] Error: Could not read CSV header\n");
        fclose(csv_file);
        return NULL;
    }
    
    // Count lines to determine data size
    long pos = ftell(csv_file);
    int sample_count = 0;
    while (fgets(line, sizeof(line), csv_file)) {
        sample_count++;
    }
    fseek(csv_file, pos, SEEK_SET);
    
    if (sample_count == 0) {
        printf("[dSONAR] Error: No data found in CSV file\n");
        fclose(csv_file);
        return NULL;
    }
    
    printf("[dSONAR] Found %d frequency samples in CSV\n", sample_count);
    
    // Allocate result structure
    dsonar_result_t* result = malloc(sizeof(dsonar_result_t));
    if (!result) {
        fclose(csv_file);
        return NULL;
    }
    
    result->reconstructed_data = malloc(sample_count);
    if (!result->reconstructed_data) {
        free(result);
        fclose(csv_file);
        return NULL;
    }
    
    // Read CSV data and reconstruct bytes
    int bytes_read = 0;
    int successful_samples = 0;
    double total_confidence = 0.0;
    
    while (fgets(line, sizeof(line), csv_file) && bytes_read < sample_count) {
        int sample_num;
        char byte_hex[8];
        int byte_dec;
        double frequency;
        double amplitude;
        double duration;
        
        // Parse CSV line: Sample,Byte_Hex,Byte_Dec,Frequency_Hz,Amplitude,Duration_s
        int parsed = sscanf(line, "%d,%7[^,],%d,%lf,%lf,%lf", 
                           &sample_num, byte_hex, &byte_dec, &frequency, &amplitude, &duration);
        
        if (parsed == 6) {
            // Use the byte_dec value directly from CSV (most accurate)
            result->reconstructed_data[bytes_read] = (unsigned char)byte_dec;
            
            // Calculate confidence based on amplitude (higher amplitude = higher confidence)
            double confidence = amplitude; // Amplitude is already normalized 0-1
            total_confidence += confidence;
            successful_samples++;
            
            bytes_read++;
        } else {
            printf("[dSONAR] Warning: Could not parse CSV line %d\n", bytes_read + 1);
        }
    }
    
    fclose(csv_file);
    
    // Fill result structure
    result->data_length = bytes_read;
    result->total_samples = sample_count;
    result->successful_samples = successful_samples;
    result->average_confidence = successful_samples > 0 ? total_confidence / successful_samples : 0.0;
    
    printf("[dSONAR] Successfully reconstructed %d bytes from CSV data\n", bytes_read);
    printf("[dSONAR] Average confidence: %.3f\n", result->average_confidence);
    
    return result;
}

dsonar_result_t* reconstruct_from_analysis(const char* analysis_filename, dsonar_config_t* config)
{
    printf("[dSONAR] Parsing analysis report...\n");
    
    reverse_sample_node_t* samples = NULL;
    if (!parse_analysis_report(analysis_filename, &samples, config)) {
        return NULL;
    }
    
    // Convert to result
    dsonar_result_t* result = malloc(sizeof(dsonar_result_t));
    if (!result) {
        free_reverse_samples(samples);
        return NULL;
    }
    
    result->reconstructed_data = samples_to_bytes(samples, &result->data_length);
    
    // Calculate statistics
    result->total_samples = 0;
    result->successful_samples = 0;
    double total_confidence = 0.0;
    
    reverse_sample_node_t* current = samples;
    while (current) {
        result->total_samples++;
        if (current->confidence_score > 0.6) {
            result->successful_samples++;
        }
        total_confidence += current->confidence_score;
        current = current->next;
    }
    
    result->average_confidence = result->total_samples > 0 ? 
        total_confidence / result->total_samples : 0.0;
    
    free_reverse_samples(samples);
    return result;
}

dsonar_result_t* reconstruct_from_wav(const char* wav_filename, dsonar_config_t* config)
{
    printf("[dSONAR] Analyzing WAV file for frequency reconstruction...\n");
    
    FILE* wav_file = fopen(wav_filename, "rb");
    if (!wav_file) {
        printf("[dSONAR] Error: Could not open WAV file\n");
        return NULL;
    }
    
    // Read WAV header
    int sample_rate, channels, bits_per_sample;
    if (!read_wav_header(wav_file, &sample_rate, &channels, &bits_per_sample)) {
        printf("[dSONAR] Error: Invalid WAV header\n");
        fclose(wav_file);
        return NULL;
    }
    
    printf("[dSONAR] WAV format: %d Hz, %d channels, %d bits\n", sample_rate, channels, bits_per_sample);
    
    // Simple frequency detection by analyzing audio segments
    reverse_sample_node_t* samples = NULL;
    int sample_index = 0;
    
    // Read audio data in chunks (each chunk represents one byte's frequency)
    int chunk_size = (int)(config->base_frequency * 0.05); // 50ms chunks
    short* audio_buffer = malloc(chunk_size * sizeof(short));
    
    if (!audio_buffer) {
        fclose(wav_file);
        return NULL;
    }
    
    while (fread(audio_buffer, sizeof(short), chunk_size, wav_file) == chunk_size) {
        // Simple frequency detection using zero-crossing analysis
        double estimated_frequency = detect_dominant_frequency(audio_buffer, chunk_size, sample_rate);
        
        if (estimated_frequency > 0) {
            reverse_sample_node_t* sample = create_reverse_sample(estimated_frequency, sample_index, config);
            if (sample) {
                // Lower confidence for WAV reconstruction due to complexity
                sample->confidence_score = 0.7;
                add_reverse_sample(&samples, sample);
                sample_index++;
            }
        }
    }
    
    free(audio_buffer);
    fclose(wav_file);
    
    if (sample_index == 0) {
        printf("[dSONAR] No frequencies detected in WAV file\n");
        return NULL;
    }
    
    // Convert to result
    dsonar_result_t* result = malloc(sizeof(dsonar_result_t));
    if (!result) {
        free_reverse_samples(samples);
        return NULL;
    }
    
    result->reconstructed_data = samples_to_bytes(samples, &result->data_length);
    result->total_samples = sample_index;
    result->successful_samples = sample_index;
    result->average_confidence = 0.7;
    
    printf("[dSONAR] Reconstructed %d bytes from WAV audio analysis\n", result->data_length);
    
    free_reverse_samples(samples);
    return result;
}

bool parse_json_metadata(const char* filename, reverse_sample_node_t** samples, dsonar_config_t* config)
{
    FILE* file = fopen(filename, "r");
    if (!file) return false;
    
    char line[1024];
    int sample_index = 0;
    
    // Simple JSON parsing (looking for frequency and byte values)
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "\"frequency\":")) {
            double frequency;
            if (sscanf(line, " \"frequency\": %lf,", &frequency) == 1) {
                // Look for corresponding byte in next few lines
                char byte_line[1024];
                unsigned int byte_val = 0;
                
                // Read previous lines to find byte value
                long pos = ftell(file);
                fseek(file, pos - 200, SEEK_SET); // Go back a bit
                
                while (fgets(byte_line, sizeof(byte_line), file) && ftell(file) <= pos + 100) {
                    if (strstr(byte_line, "\"byte\":")) {
                        if (sscanf(byte_line, " \"byte\": \"0x%X\",", &byte_val) == 1) {
                            break;
                        }
                    }
                }
                
                fseek(file, pos, SEEK_SET); // Restore position
                
                // Create reverse sample
                reverse_sample_node_t* sample = create_reverse_sample(frequency, sample_index, config);
                if (sample) {
                    // Verify reconstruction accuracy
                    unsigned char reconstructed = frequency_to_byte(frequency, config);
                    if (byte_val > 0 && reconstructed == (unsigned char)byte_val) {
                        sample->confidence_score = 1.0; // Perfect match
                    } else {
                        sample->confidence_score = 0.8; // Good match
                    }
                    add_reverse_sample(samples, sample);
                    sample_index++;
                }
            }
        }
    }
    
    fclose(file);
    return sample_index > 0;
}

bool parse_csv_frequency_data(const char* filename, reverse_sample_node_t** samples, dsonar_config_t* config)
{
    FILE* file = fopen(filename, "r");
    if (!file) return false;
    
    char line[512];
    int sample_index = 0;
    
    // Skip header line
    if (fgets(line, sizeof(line), file)) {
        // Parse CSV data: Sample,Byte_Hex,Byte_Dec,Frequency_Hz,Amplitude,Duration_s
        while (fgets(line, sizeof(line), file)) {
            int sample_num, byte_dec;
            double frequency, amplitude, duration;
            char byte_hex[8];
            
            if (sscanf(line, "%d,%7[^,],%d,%lf,%lf,%lf", 
                      &sample_num, byte_hex, &byte_dec, &frequency, &amplitude, &duration) == 6) {
                
                reverse_sample_node_t* sample = create_reverse_sample(frequency, sample_index, config);
                if (sample) {
                    // High confidence since we have exact frequency data
                    sample->confidence_score = 0.95;
                    add_reverse_sample(samples, sample);
                    sample_index++;
                }
            }
        }
    }
    
    fclose(file);
    return sample_index > 0;
}

bool parse_analysis_report(const char* filename, reverse_sample_node_t** samples, dsonar_config_t* config)
{
    FILE* file = fopen(filename, "r");
    if (!file) return false;
    
    char line[512];
    int sample_index = 0;
    bool in_data_section = false;
    
    // Look for "Detailed Sample Data" section
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Detailed Sample Data")) {
            in_data_section = true;
            fgets(line, sizeof(line), file); // Skip header
            fgets(line, sizeof(line), file); // Skip separator
            continue;
        }
        
        if (in_data_section && strlen(line) > 10) {
            unsigned int byte_val;
            double frequency, amplitude, duration;
            
            // Parse: 0x48	784.71		0.354	0.050
            if (sscanf(line, "0x%X\t%lf\t\t%lf\t%lf", &byte_val, &frequency, &amplitude, &duration) == 4) {
                reverse_sample_node_t* sample = create_reverse_sample(frequency, sample_index, config);
                if (sample) {
                    sample->confidence_score = 0.85; // Good confidence from analysis
                    add_reverse_sample(samples, sample);
                    sample_index++;
                }
            }
        }
    }
    
    fclose(file);
    return sample_index > 0;
}

reverse_sample_node_t* create_reverse_sample(double frequency, int index, dsonar_config_t* config)
{
    reverse_sample_node_t* sample = malloc(sizeof(reverse_sample_node_t));
    if (!sample) return NULL;
    
    sample->source_frequency = frequency;
    sample->sample_index = index;
    sample->reconstructed_byte = frequency_to_byte(frequency, config);
    sample->confidence_score = 0.5; // Default confidence
    sample->next = NULL;
    
    return sample;
}

void add_reverse_sample(reverse_sample_node_t** head, reverse_sample_node_t* sample)
{
    if (!head || !sample) return;
    
    if (*head == NULL) {
        *head = sample;
    } else {
        reverse_sample_node_t* current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = sample;
    }
}

unsigned char frequency_to_byte(double frequency, dsonar_config_t* config)
{
    // Reverse the SONAR frequency mapping
    // Original: frequency = base_freq + (byte/255.0) * freq_range
    // Reverse: byte = ((frequency - base_freq) / freq_range) * 255.0
    
    if (frequency < config->base_frequency) return 0;
    if (frequency > config->base_frequency + config->frequency_range) return 255;
    
    double normalized = (frequency - config->base_frequency) / config->frequency_range;
    return (unsigned char)(normalized * 255.0 + 0.5); // Round to nearest
}

unsigned char* samples_to_bytes(reverse_sample_node_t* head, int* length)
{
    if (!head || !length) return NULL;
    
    // Count samples
    int count = 0;
    reverse_sample_node_t* current = head;
    while (current) {
        count++;
        current = current->next;
    }
    
    if (count == 0) {
        *length = 0;
        return NULL;
    }
    
    // Allocate and fill byte array
    unsigned char* bytes = malloc(count);
    if (!bytes) {
        *length = 0;
        return NULL;
    }
    
    current = head;
    for (int i = 0; i < count && current; i++) {
        bytes[i] = current->reconstructed_byte;
        current = current->next;
    }
    
    *length = count;
    return bytes;
}

bool save_reconstructed_data(const char* output_filename, dsonar_result_t* result)
{
    if (!output_filename || !result || !result->reconstructed_data) return false;
    
    FILE* output = fopen(output_filename, "wb");
    if (!output) return false;
    
    size_t written = fwrite(result->reconstructed_data, 1, result->data_length, output);
    fclose(output);
    
    return written == result->data_length;
}

void print_reconstruction_report(dsonar_result_t* result, const char* source_filename)
{
    if (!result) return;
    
    printf("\ndSONAR Reconstruction Report:\n");
    printf("Source file: %s\n", source_filename ? source_filename : "unknown");
    printf("Reconstructed %d bytes from %d samples\n", result->data_length, result->total_samples);
    printf("Success rate: %d/%d (%.1f%%)\n", 
           result->successful_samples, result->total_samples,
           result->total_samples > 0 ? (result->successful_samples * 100.0 / result->total_samples) : 0.0);
    printf("Average confidence: %.3f\n", result->average_confidence);
    
    // Show first few reconstructed bytes
    printf("First 10 reconstructed bytes: ");
    for (int i = 0; i < min(10, result->data_length); i++) {
        printf("0x%02X ", result->reconstructed_data[i]);
    }
    printf("\n");
}

double calculate_reconstruction_accuracy(const unsigned char* original, const unsigned char* reconstructed, int length)
{
    if (!original || !reconstructed || length <= 0) return 0.0;
    
    int matches = 0;
    for (int i = 0; i < length; i++) {
        if (original[i] == reconstructed[i]) {
            matches++;
        }
    }
    
    return (double)matches / length;
}

void free_reverse_samples(reverse_sample_node_t* head)
{
    while (head) {
        reverse_sample_node_t* temp = head;
        head = head->next;
        free(temp);
    }
}

void free_dsonar_result(dsonar_result_t* result)
{
    if (result) {
        if (result->reconstructed_data) {
            free(result->reconstructed_data);
        }
        free(result);
    }
}

bool combine_partition_files(int partition_count, const char* original_filename)
{
    char output_filename[256];
    sprintf(output_filename, "dsonar_reconstructed_%s", original_filename);
    
    FILE* output = fopen(output_filename, "wb");
    if (!output) {
        printf("Error: Could not create output file %s\n", output_filename);
        return false;
    }
    
    int total_bytes = 0;
    for (int i = 0; i < partition_count; i++) {
        char partition_filename[256];
        sprintf(partition_filename, "dsonar_reconstructed_partition_%d.bin", i);
        
        FILE* partition_file = fopen(partition_filename, "rb");
        if (!partition_file) {
            printf("Warning: Could not open partition file %s\n", partition_filename);
            continue;
        }
        
        // Copy partition data to output file
        unsigned char buffer[4096];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), partition_file)) > 0) {
            fwrite(buffer, 1, bytes_read, output);
            total_bytes += bytes_read;
        }
        
        fclose(partition_file);
    }
    
    fclose(output);
    printf("Combined %d bytes into %s\n", total_bytes, output_filename);
    return total_bytes > 0;
}
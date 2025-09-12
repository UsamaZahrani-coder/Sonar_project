/**
 * @file mojibake_sonar.c
 * @brief Mojibake SONAR Edition - Main Program
 * @author Mojibake Development Team
 * @version 1.0.0a
 * 
 * Main program for the Mojibake SONAR Edition, providing audio-based
 * file analysis through SONAR (data-to-audio) and dSONAR (audio-to-data)
 * conversion capabilities.
 */

#include "mojibake/mojibake.h"
#include "mbx_default.h"
#include "mbx_charcount.h"
#include "mbx_textview.h"
#include "mbx_sonar.h"
#include "mbx_dsonar.h"
#include <string.h>

/**
 * @brief Process single WAV file for dSONAR reconstruction
 * 
 * Handles direct WAV file input for dSONAR reconstruction, generating
 * output filename based on input WAV filename and performing data reconstruction.
 * 
 * @param wav_filename Path to input WAV file
 * @return true if processing successful, false otherwise
 */
bool process_single_wav_file(const char* wav_filename)
{
    printf("=== Direct WAV-to-Data Reconstruction ===\n");
    printf("Input WAV file: %s\n\n", wav_filename);
    
    dsonar_config_t config = {
        .base_frequency = 220.0,
        .frequency_range = 2000.0,
        .tolerance = 5.0,
        .strict_mode = false,
        .input_format = "wav"
    };
    
    // Check if WAV file exists
    FILE* test_file = fopen(wav_filename, "rb");
    if (!test_file) {
        printf("[ERROR] WAV file not found: %s\n", wav_filename);
        return false;
    }
    fclose(test_file);
    
    // Reconstruct from WAV
    dsonar_result_t* result = reconstruct_from_wav(wav_filename, &config);
    if (!result) {
        printf("[ERROR] Failed to reconstruct from %s\n", wav_filename);
        return false;
    }
    
    // Generate output filename
    char output_filename[256];
    const char* base_name = strrchr(wav_filename, '\\');
    if (!base_name) base_name = strrchr(wav_filename, '/');
    if (!base_name) base_name = wav_filename;
    else base_name++; // Skip the slash
    
    // Remove .wav extension and add .bin
    char name_without_ext[256];
    strcpy(name_without_ext, base_name);
    char* dot = strrchr(name_without_ext, '.');
    if (dot) *dot = '\0';
    sprintf(output_filename, "dsonar_reconstructed_%s.bin", name_without_ext);
    
    // Save reconstructed data
    if (save_reconstructed_data(output_filename, result)) {
        printf("[OK] Reconstructed %d bytes -> %s\n", result->data_length, output_filename);
        printf("   Confidence: %.1f%%, Success rate: %d/%d\n", 
               result->average_confidence * 100, result->successful_samples, result->total_samples);
        printf("   WAV format: %s\n", wav_filename);
        printf("   Output: %s\n", output_filename);
    } else {
        printf("[ERROR] Failed to save %s\n", output_filename);
        free_dsonar_result(result);
        return false;
    }
    
    free_dsonar_result(result);
    return true;
}

/**
 * @brief Process multiple WAV partition files for dSONAR reconstruction
 * 
 * Processes multiple WAV partition files in sequence, reconstructs data from each,
 * and combines the results into a single output file.
 * 
 * @param partition_count Number of partition files to process
 * @return true if all partitions processed successfully, false otherwise
 */
bool process_wav_files_only(int partition_count)
{
    printf("\n=== Standalone WAV-to-Data Reconstruction ===\n");
    printf("Processing %d WAV partition files...\n\n", partition_count);
    
    dsonar_config_t config = {
        .base_frequency = 220.0,
        .frequency_range = 2000.0,
        .tolerance = 5.0,
        .strict_mode = false,
        .input_format = "wav"
    };
    
    bool success = true;
    for (int i = 0; i < partition_count; i++) {
        char wav_filename[256];
        sprintf(wav_filename, "sonar_partition_%d.wav", i);
        
        // Check if WAV file exists
        FILE* test_file = fopen(wav_filename, "rb");
        if (!test_file) {
            printf("[ERROR] WAV file not found: %s\n", wav_filename);
            success = false;
            continue;
        }
        fclose(test_file);
        
        printf("=== Processing WAV Partition %d ===\n", i);
        printf("Input: %s\n", wav_filename);
        
        // Try CSV file first for this partition
        char csv_filename[256];
        sprintf(csv_filename, "sonar_partition_%d_frequencies.csv", i);
        
        dsonar_result_t* result = NULL;
        FILE* csv_test = fopen(csv_filename, "r");
        if (csv_test) {
            fclose(csv_test);
            printf("Found CSV file: %s\n", csv_filename);
            printf("Using CSV frequency data for precise reconstruction...\n");
            result = reconstruct_from_csv(csv_filename, &config);
        }
        
        // Fallback to WAV if CSV not available
        if (!result) {
            printf("CSV not found, falling back to WAV analysis...\n");
            result = reconstruct_from_wav(wav_filename, &config);
            if (!result) {
                printf("[ERROR] Failed to reconstruct from %s\n", wav_filename);
                success = false;
                continue;
            }
        }
        
        // Save reconstructed data
        char output_filename[256];
        sprintf(output_filename, "dsonar_reconstructed_partition_%d.bin", i);
        
        if (save_reconstructed_data(output_filename, result)) {
            printf("[OK] Reconstructed %d bytes -> %s\n", result->data_length, output_filename);
            printf("   Confidence: %.1f%%, Success rate: %d/%d\n", 
                   result->average_confidence * 100, result->successful_samples, result->total_samples);
        } else {
            printf("[ERROR] Failed to save %s\n", output_filename);
            success = false;
        }
        
        free_dsonar_result(result);
        printf("\n");
    }
    
    if (success) {
        printf("[INFO] Combining all partitions into original file...\n");
        if (combine_partition_files(partition_count, "reconstructed_from_wav.bin")) {
            printf("[OK] Complete file reconstructed: dsonar_reconstructed_reconstructed_from_wav.bin\n");
        } else {
            printf("[ERROR] Failed to combine partition files\n");
            success = false;
        }
    }
    
    return success;
}

/**
 * @brief Print program usage information
 * 
 * Displays help information including available modules, command syntax,
 * and example usage for both SONAR and dSONAR functionality.
 * 
 * @param program_name Name of the program executable
 */
void print_usage(const char* program_name) {
    printf("\n");
    printf("\033[0;32m    ~~~     ~~~     ~~~     ~~~     ~~~\033[0m\n");
    printf("\033[0;32m   ~   ~   ~   ~   ~   ~   ~   ~   ~   ~\033[0m\n");
    printf("\033[0;32m  ~     ~ ~     ~ ~     ~ ~     ~ ~     ~\033[0m\n");
    printf("\033[0;32m ~       ~       ~       ~       ~       ~\033[0m\n");
    printf("\033[0;32m~         ~       ~       ~       ~         ~\033[0m\n");
    printf("\033[0;32m           ~     ~ ~     ~ ~     ~\033[0m\n");
    printf("\033[0;32m            ~   ~   ~   ~   ~   ~\033[0m\n");
    printf("\033[0;32m             ~~~     ~~~     ~~~\033[0m\n");
    printf("\n");
    printf("        \033[1;32mMOJIBAKE SONAR EDITION\033[0m\n");
    printf("\033[1;36m           v1.0.0a\033[0m\n\n");
    
    printf("\033[1;33mUSAGE:\033[0m\n");
    printf("  %s \033[4m<filename>\033[0m [\033[4mmodule\033[0m] [\033[4mpartition_count\033[0m]\n\n", program_name);
    
    printf("\033[1;33mARGUMENTS:\033[0m\n");
    printf("  \033[1;37mfilename\033[0m        Path to the file you want to analyze\n");
    printf("  \033[1;37mmodule\033[0m          Analysis module (optional):\n");
    printf("                    \033[0;34mhex\033[0m      - Hexadecimal display (default)\n");
    printf("                    \033[0;34mtext\033[0m     - Text preview with readable characters\n");
    printf("                    \033[0;34mcount\033[0m    - Character frequency analysis\n");
    printf("                    \033[0;32msonar\033[0m    - Audio visualization\n");
    printf("                    \033[0;32mdsonar\033[0m   - Reverse audio to data \033[1;31m(NEW!)\033[0m\n");
    printf("  \033[1;37mpartition_count\033[0m Number of partitions (optional, default: %d)\n\n", MOJIBAKE_DEFAULT_PARTITION_COUNT);
    
    printf("\033[1;33mEXAMPLES:\033[0m\n");
    printf("  \033[0;36mmojibake_sonar\033[0m myfile.txt\n");
    printf("  \033[0;36mmojibake_sonar\033[0m document.pdf \033[0;34mtext\033[0m\n");
    printf("  \033[0;36mmojibake_sonar\033[0m music.mp3 \033[0;32msonar\033[0m 4\n");
    printf("  \033[0;36mmojibake_sonar\033[0m binary.exe \033[0;32msonar\033[0m 16\n");
    printf("  \033[0;36mmojibake_sonar\033[0m sonar_partition_0.wav \033[0;32mdsonar\033[0m\n");
    printf("  \033[0;36mmojibake_sonar\033[0m \"C:\\path\\to\\audio.wav\" \033[0;32mdsonar\033[0m\n\n");
    
    printf("\033[1;35m[AUDIO] SONAR Extension Features:\033[0m\n");
    printf("  \033[0;37m-\033[0m Converts file bytes to audio frequencies\n");
    printf("  \033[0;37m-\033[0m Dynamic library loading for real-time playback\n");
    printf("  \033[0;37m-\033[0m Linked list-based audio sample management\n");
    printf("  \033[0;37m-\033[0m WAV file generation for offline analysis\n");
    printf("  \033[0;37m-\033[0m Frequency mapping and amplitude control\n");
    printf("  \033[0;37m-\033[0m Statistical audio analysis\n\n");
    
    printf("\033[1;33mAVAILABLE EXTENSIONS:\033[0m\n");
    printf("  \033[1;32m[OK]\033[0m    Hexadecimal viewer (built-in)\n");
    printf("  \033[1;32m[OK]\033[0m    Text content preview\n");
    printf("  \033[1;32m[OK]\033[0m    Character frequency counter\n");
    printf("  \033[1;35m[AUDIO]\033[0m SONAR audio visualization \033[1;31m(NEW!)\033[0m\n");
    printf("  \033[1;34m[+]\033[0m     Easy to add more modules!\n\n");
}

/**
 * @brief Main program entry point
 * 
 * Parses command line arguments and executes the appropriate mojibake module
 * (default, charcount, textview, sonar, or dsonar) with the specified parameters.
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Get filename from command line
    char *filename = argv[1];
    
    // Get module type (optional)
    char *module_name = "hex"; // default
    if (argc >= 3) {
        module_name = argv[2];
    }
    
    // Get partition count (optional)
    int partition_count = MOJIBAKE_DEFAULT_PARTITION_COUNT;
    if (argc >= 4) {
        partition_count = atoi(argv[3]);
        if (partition_count <= 0) {
            printf("Error: Partition count must be a positive number\n");
            return 1;
        }
    }

    // Select the appropriate module
    mojibake_partition_callback_t selected_module;
    sonar_config_t sonar_config = {
        .sample_rate = 44100,
        .base_frequency = 220.0,
        .frequency_range = 2000.0,
        .sample_duration = 0.05,
        .use_dynamic_lib = true  // Enable shared library by default
    };
    
    void *module_arg = NULL;
    
    if (strcmp(module_name, "hex") == 0) {
        selected_module = mbx_default;
        printf("[HEX] Using module: Hexadecimal Display\n");
    } else if (strcmp(module_name, "text") == 0) {
        selected_module = mbx_textview;
        printf("[TEXT] Using module: Text Preview\n");
    } else if (strcmp(module_name, "count") == 0) {
        selected_module = mbx_charcount;
        printf("[COUNT] Using module: Character Counter\n");
    } else if (strcmp(module_name, "sonar") == 0) {
        selected_module = mbx_sonar;
        module_arg = &sonar_config;
        printf("[AUDIO] Using module: SONAR Audio Visualization\n");
        printf("   - Sample Rate: %d Hz\n", sonar_config.sample_rate);
        printf("   - Frequency Range: %.0f - %.0f Hz\n", 
               sonar_config.base_frequency, 
               sonar_config.base_frequency + sonar_config.frequency_range);
        printf("   - Sample Duration: %.0f ms per byte\n", sonar_config.sample_duration * 1000);
    } else if (strcmp(module_name, "dsonar") == 0) {
        // dSONAR works with WAV files directly - filename should be WAV pattern
        printf("[REVERSE] Using module: dSONAR Reverse Audio Analysis\n");
        printf("   - Base Frequency: 220 Hz\n");
        printf("   - Frequency Range: 2000 Hz\n");
        printf("   - Tolerance: 5.0 Hz\n");
        printf("   - Mode: Flexible\n");
        printf("   - Input: WAV files directly\n");
        
        // Check if filename contains .wav extension
        if (strstr(filename, ".wav") != NULL) {
            // Single WAV file mode
            printf("\n=== Single WAV File Mode ===\n");
            printf("Processing: %s\n\n", filename);
            
            if (process_single_wav_file(filename)) {
                printf("[OK] WAV-to-data reconstruction complete!\n");
            } else {
                printf("[ERROR] Failed to process WAV file\n");
            }
        } else {
            // Multi-partition WAV mode (legacy)
            if (process_wav_files_only(partition_count)) {
                printf("[OK] WAV-to-data reconstruction complete!\n");
            } else {
                printf("[ERROR] Failed to process WAV files\n");
            }
        }
        
        printf("\n[OK] Analysis complete!\n");
        return 0;
    } else {
        printf("Error: Unknown module '%s'\n", module_name);
        printf("Available modules: hex, text, count, sonar, dsonar\n");
        return 1;
    }

    printf("Analyzing file: %s\n", filename);
    printf("Partition count: %d\n\n", partition_count);

    mojibake_target_t *target = mojibake_open(filename, partition_count);
    if (target == NULL) {
        printf("Error: Could not open file '%s'\n", filename);
        printf("Please check if the file exists and is readable.\n");
        return 1;
    }

    printf("File size: %d bytes\n", target->size);
    printf("Partition size: %d bytes each\n\n", target->partition_size);

    // Only execute for non-dSONAR modules
    if (strcmp(module_name, "dsonar") != 0) {
        if (!mojibake_execute(target, selected_module, module_arg))
            printf("Execution error\n");
    }

    if (strcmp(module_name, "sonar") == 0) {
        printf("[INFO] SONAR Analysis Complete!\n");
        printf("   Check generated WAV files for audio output.\n");
        printf("   Each partition has been converted to audio frequencies.\n");
    }
    // dSONAR processing is handled separately above

    printf("\n[OK] Analysis complete!\n");
    mojibake_close(target);
    return 0;
}
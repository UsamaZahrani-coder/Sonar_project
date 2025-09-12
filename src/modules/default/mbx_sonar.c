#include "mbx_sonar.h"
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _WIN32
#include <windows.h>
#define LOAD_LIBRARY(path) LoadLibrary(path)
#define GET_PROC_ADDRESS(handle, name) GetProcAddress(handle, name)
#define FREE_LIBRARY(handle) FreeLibrary(handle)
#else
#include <dlfcn.h>
#define LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
#define GET_PROC_ADDRESS(handle, name) dlsym(handle, name)
#define FREE_LIBRARY(handle) dlclose(handle)
#endif

// Default SONAR configuration
static sonar_config_t default_config = {
    .sample_rate = 44100,
    .base_frequency = 220.0,    // A3 note
    .frequency_range = 2000.0,  // 220Hz to 2220Hz range
    .sample_duration = 0.05,    // 50ms per byte
    .use_dynamic_lib = true    
};

bool mbx_sonar(mojibake_target_t *target, unsigned int index, void *arg)
{
    if (target == NULL || target->block == NULL || index >= target->partition_count)
        return false;

    unsigned char *partition = MOJIBAKE_BLOCK_OFFSET(target, index);
    sonar_config_t *config = arg ? (sonar_config_t*)arg : &default_config;
    
    printf("=== SONAR Partition %d Audio Analysis ===\n", index);
    printf("Converting %d bytes to audio frequencies...\n", target->partition_size);
    
    // Initialize linked list for audio samples
    audio_sample_node_t *audio_head = NULL;
    
    // Convert each byte to audio sample
    for (int i = 0; i < target->partition_size; i++) {
        audio_sample_node_t *sample = create_audio_sample(partition[i], config);
        if (sample) {
            add_sample_to_list(&audio_head, sample);
        }
    }
    
    // Try to load dynamic audio library
    audio_lib_t audio_lib = {0};
    bool lib_loaded = false;
    
    if (config->use_dynamic_lib) {
        lib_loaded = load_audio_library(&audio_lib, "c:/Users/Usama/Downloads/Sonar2/mojibake/lib/audio_engine.dll"); 
        if (lib_loaded) {
            printf("Dynamic audio library loaded successfully!\n");
        } else {
            printf("Dynamic library not found, using built-in audio generation.\n");
        }
    }
    
    // Generate audio output
    if (lib_loaded) {
        // Use dynamic library for audio playback
        play_audio_list(audio_head, &audio_lib);
        
        // Generate additional files using shared library
        char base_filename[256];
        sprintf(base_filename, "sonar_partition_%d", index);
        
        // Generate WAV file using shared library
        if (audio_lib.generate_wav) {
            char wav_filename[256];
            sprintf(wav_filename, "%s.wav", base_filename);
            audio_lib.generate_wav(wav_filename, audio_head);
        }
        
        // Generate analysis report
        if (audio_lib.generate_analysis_report) {
            audio_lib.generate_analysis_report(audio_head, base_filename);
        }
        
        // Generate frequency data CSV
        if (audio_lib.generate_frequency_data) {
            audio_lib.generate_frequency_data(audio_head, base_filename);
        }
        
        // Generate metadata JSON
        if (audio_lib.generate_metadata_json) {
            audio_lib.generate_metadata_json(audio_head, base_filename);
        }
    } else {
        // Use built-in WAV file generation
        char filename[256];
        sprintf(filename, "sonar_partition_%d.wav", index);
        generate_wav_file(audio_head, filename, config);
        printf("Audio saved to: %s\n", filename);
    }
    
    // Display frequency analysis
    printf("\nFrequency Analysis:\n");
    audio_sample_node_t *current = audio_head;
    int sample_count = 0;
    double total_freq = 0.0;
    double min_freq = 999999.0, max_freq = 0.0;
    
    while (current && sample_count < 10) { // Show first 10 samples
        printf("Byte 0x%02X -> %.2f Hz (Amp: %.2f)\n", 
               current->source_byte, current->frequency, current->amplitude);
        
        total_freq += current->frequency;
        if (current->frequency < min_freq) min_freq = current->frequency;
        if (current->frequency > max_freq) max_freq = current->frequency;
        
        current = current->next;
        sample_count++;
    }
    
    if (sample_count > 0) {
        printf("\nStatistics:\n");
        printf("Average frequency: %.2f Hz\n", total_freq / sample_count);
        printf("Frequency range: %.2f - %.2f Hz\n", min_freq, max_freq);
        printf("Total audio duration: %.2f seconds\n", 
               target->partition_size * config->sample_duration);
    }
    
    // Cleanup
    free_audio_list(audio_head);
    if (lib_loaded) {
        unload_audio_library(&audio_lib);
    }
    
    printf("\n");
    return true;
}

audio_sample_node_t* create_audio_sample(unsigned char byte, sonar_config_t *config)
{
    audio_sample_node_t *sample = malloc(sizeof(audio_sample_node_t));
    if (!sample) return NULL;
    
    sample->source_byte = byte;
    sample->frequency = map_byte_to_frequency(byte, config);
    sample->amplitude = map_byte_to_amplitude(byte);
    sample->duration = config->sample_duration;
    sample->next = NULL;
    
    return sample;
}

void add_sample_to_list(audio_sample_node_t **head, audio_sample_node_t *new_sample)
{
    if (!head || !new_sample) return;
    
    if (*head == NULL) {
        *head = new_sample;
    } else {
        // Add to end of list
        audio_sample_node_t *current = *head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_sample;
    }
}

void play_audio_list(audio_sample_node_t *head, audio_lib_t *audio_lib)
{
    if (!head || !audio_lib || !audio_lib->play_frequency) return;
    
    printf("Playing audio through dynamic library...\n");
    
    audio_sample_node_t *current = head;
    while (current) {
        audio_lib->play_frequency(current->frequency, current->amplitude, current->duration);
        current = current->next;
    }
}

void free_audio_list(audio_sample_node_t *head)
{
    while (head) {
        audio_sample_node_t *temp = head;
        head = head->next;
        free(temp);
    }
}

bool load_audio_library(audio_lib_t *audio_lib, const char *lib_path)
{
    if (!audio_lib || !lib_path) return false;
    
    printf("[DEBUG] Attempting to load library: %s\n", lib_path);
    audio_lib->lib_handle = LOAD_LIBRARY(lib_path);
    if (!audio_lib->lib_handle) {
#ifdef _WIN32
        DWORD error = GetLastError();
        printf("[DEBUG] LoadLibrary failed with error code: %lu\n", error);
#endif
        return false;
    }
    printf("[DEBUG] Library loaded successfully\n");
    
    // Load function pointers
    printf("[DEBUG] Loading function pointers...\n");
    audio_lib->init_audio = (int(*)(int))GET_PROC_ADDRESS(audio_lib->lib_handle, "init_audio");
    audio_lib->play_frequency = (int(*)(double,double,double))GET_PROC_ADDRESS(audio_lib->lib_handle, "play_frequency");
    audio_lib->generate_wav = (int(*)(const char*,audio_sample_node_t*))GET_PROC_ADDRESS(audio_lib->lib_handle, "generate_wav");
    audio_lib->cleanup_audio = (void(*)(void))GET_PROC_ADDRESS(audio_lib->lib_handle, "cleanup_audio");
    
    // Load file generation functions
    audio_lib->generate_analysis_report = (int(*)(audio_sample_node_t*,const char*))GET_PROC_ADDRESS(audio_lib->lib_handle, "generate_analysis_report");
    audio_lib->generate_frequency_data = (int(*)(audio_sample_node_t*,const char*))GET_PROC_ADDRESS(audio_lib->lib_handle, "generate_frequency_data");
    audio_lib->generate_metadata_json = (int(*)(audio_sample_node_t*,const char*))GET_PROC_ADDRESS(audio_lib->lib_handle, "generate_metadata_json");
    
    // Check if essential functions are loaded
    printf("[DEBUG] init_audio: %p, play_frequency: %p\n", (void*)audio_lib->init_audio, (void*)audio_lib->play_frequency);
    if (!audio_lib->init_audio || !audio_lib->play_frequency) {
        printf("[DEBUG] Essential functions not found in library\n");
        FREE_LIBRARY(audio_lib->lib_handle);
        return false;
    }
    
    // Initialize audio system
    if (audio_lib->init_audio(44100) != 0) {
        FREE_LIBRARY(audio_lib->lib_handle);
        return false;
    }
    
    return true;
}

void unload_audio_library(audio_lib_t *audio_lib)
{
    if (!audio_lib || !audio_lib->lib_handle) return;
    
    if (audio_lib->cleanup_audio) {
        audio_lib->cleanup_audio();
    }
    
    FREE_LIBRARY(audio_lib->lib_handle);
    memset(audio_lib, 0, sizeof(audio_lib_t));
}

void generate_wav_file(audio_sample_node_t *head, const char *filename, sonar_config_t *config)
{
    // Simple WAV file generation (header + PCM data)
    FILE *wav_file = fopen(filename, "wb");
    if (!wav_file) {
        printf("Error: Could not create WAV file %s\n", filename);
        return;
    }
    
    // Calculate total samples
    int total_samples = 0;
    audio_sample_node_t *current = head;
    while (current) {
        total_samples += (int)(current->duration * config->sample_rate);
        current = current->next;
    }
    
    // WAV header (44 bytes)
    int data_size = total_samples * 2; // 16-bit samples
    int file_size = data_size + 36;
    
    fwrite("RIFF", 1, 4, wav_file);
    fwrite(&file_size, 4, 1, wav_file);
    fwrite("WAVE", 1, 4, wav_file);
    fwrite("fmt ", 1, 4, wav_file);
    
    int fmt_size = 16;
    short audio_format = 1; // PCM
    short channels = 1;
    int sample_rate = config->sample_rate;
    int byte_rate = sample_rate * channels * 2;
    short block_align = channels * 2;
    short bits_per_sample = 16;
    
    fwrite(&fmt_size, 4, 1, wav_file);
    fwrite(&audio_format, 2, 1, wav_file);
    fwrite(&channels, 2, 1, wav_file);
    fwrite(&sample_rate, 4, 1, wav_file);
    fwrite(&byte_rate, 4, 1, wav_file);
    fwrite(&block_align, 2, 1, wav_file);
    fwrite(&bits_per_sample, 2, 1, wav_file);
    
    fwrite("data", 1, 4, wav_file);
    fwrite(&data_size, 4, 1, wav_file);
    
    // Generate PCM data
    current = head;
    while (current) {
        int samples_for_this_note = (int)(current->duration * config->sample_rate);
        
        for (int i = 0; i < samples_for_this_note; i++) {
            double t = (double)i / config->sample_rate;
            double sample = current->amplitude * sin(2.0 * M_PI * current->frequency * t);
            short pcm_sample = (short)(sample * 32767.0);
            fwrite(&pcm_sample, 2, 1, wav_file);
        }
        
        current = current->next;
    }
    
    fclose(wav_file);
}

double map_byte_to_frequency(unsigned char byte, sonar_config_t *config)
{
    // Map byte value (0-255) to frequency range
    double normalized = (double)byte / 255.0;
    return config->base_frequency + (normalized * config->frequency_range);
}

double map_byte_to_amplitude(unsigned char byte)
{
    // Map byte value to amplitude (0.1 to 1.0)
    return 0.1 + ((double)byte / 255.0) * 0.9;
}
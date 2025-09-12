#define BUILDING_AUDIO_ENGINE
#include "audio_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#else
#include <unistd.h>
#include <sys/time.h>
#endif

// Library state
static int g_sample_rate = 44100;
static double g_master_volume = 1.0;
static int g_initialized = 0;
static char g_library_version[] = "AudioEngine 1.0.0";
static char g_library_name[] = "SONAR Audio Engine";

// Audio buffer for real-time playback
#define BUFFER_SIZE 4096
static short g_audio_buffer[BUFFER_SIZE];
static int g_buffer_pos = 0;

// Initialize audio system
AUDIO_API int init_audio(int sample_rate)
{
    if (g_initialized) {
        printf("[AudioEngine] Already initialized\n");
        return 0;
    }
    
    g_sample_rate = sample_rate;
    g_master_volume = 1.0;
    g_buffer_pos = 0;
    
    // Clear audio buffer
    memset(g_audio_buffer, 0, sizeof(g_audio_buffer));
    
    printf("[AudioEngine] Initialized with sample rate: %d Hz\n", sample_rate);
    g_initialized = 1;
    
    return 0;
}

// Play a single frequency (simulated with console output and optional beep)
AUDIO_API int play_frequency(double frequency, double amplitude, double duration)
{
    if (!g_initialized) {
        printf("[AudioEngine] Error: Not initialized\n");
        return -1;
    }
    
    printf("[AudioEngine] Playing %.2f Hz at %.2f amplitude for %.3f seconds\n", 
           frequency, amplitude * g_master_volume, duration);
    
    // Generate samples for this frequency
    int samples = (int)(duration * g_sample_rate);
    
    for (int i = 0; i < samples && i < BUFFER_SIZE; i++) {
        double t = (double)i / g_sample_rate;
        double sample = amplitude * g_master_volume * sin(2.0 * M_PI * frequency * t);
        g_audio_buffer[i] = (short)(sample * 32767.0);
    }
    
#ifdef _WIN32
    // On Windows, we can use Beep() for actual sound
    if (frequency >= 37 && frequency <= 32767) {
        Beep((DWORD)frequency, (DWORD)(duration * 1000));
    }
#else
    // On Linux, simulate with sleep
    usleep((useconds_t)(duration * 1000000));
#endif
    
    return 0;
}

// Generate WAV file from sample list
AUDIO_API int generate_wav(const char* filename, audio_sample_node_t* samples)
{
    if (!g_initialized) {
        printf("[AudioEngine] Error: Not initialized\n");
        return -1;
    }
    
    if (!filename || !samples) {
        printf("[AudioEngine] Error: Invalid parameters\n");
        return -1;
    }
    
    printf("[AudioEngine] Generating enhanced WAV file: %s\n", filename);
    
    FILE *wav_file = fopen(filename, "wb");
    if (!wav_file) {
        printf("[AudioEngine] Error: Could not create file %s\n", filename);
        return -1;
    }
    
    // Calculate total samples
    int total_samples = 0;
    audio_sample_node_t *current = samples;
    while (current) {
        total_samples += (int)(current->duration * g_sample_rate);
        current = current->next;
    }
    
    // Enhanced WAV header with better quality
    int data_size = total_samples * 2; // 16-bit samples
    int file_size = data_size + 36;
    
    // RIFF header
    fwrite("RIFF", 1, 4, wav_file);
    fwrite(&file_size, 4, 1, wav_file);
    fwrite("WAVE", 1, 4, wav_file);
    
    // Format chunk
    fwrite("fmt ", 1, 4, wav_file);
    int fmt_size = 16;
    short audio_format = 1; // PCM
    short channels = 1;
    int sample_rate = g_sample_rate;
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
    
    // Data chunk
    fwrite("data", 1, 4, wav_file);
    fwrite(&data_size, 4, 1, wav_file);
    
    // Generate enhanced PCM data with smooth transitions
    current = samples;
    double phase = 0.0;
    
    while (current) {
        int samples_for_this_note = (int)(current->duration * g_sample_rate);
        double freq = current->frequency;
        double amp = current->amplitude * g_master_volume;
        
        for (int i = 0; i < samples_for_this_note; i++) {
            double t = (double)i / g_sample_rate;
            
            // Enhanced waveform with harmonics for richer sound
            double fundamental = sin(2.0 * M_PI * freq * t + phase);
            double harmonic2 = 0.3 * sin(2.0 * M_PI * freq * 2.0 * t + phase);
            double harmonic3 = 0.1 * sin(2.0 * M_PI * freq * 3.0 * t + phase);
            
            // Apply envelope for smooth attack/decay
            double envelope = 1.0;
            if (i < samples_for_this_note * 0.1) {
                envelope = (double)i / (samples_for_this_note * 0.1); // Attack
            } else if (i > samples_for_this_note * 0.9) {
                envelope = 1.0 - ((double)(i - samples_for_this_note * 0.9) / (samples_for_this_note * 0.1)); // Decay
            }
            
            double sample = amp * envelope * (fundamental + harmonic2 + harmonic3);
            short pcm_sample = (short)(sample * 32767.0 * 0.8); // Prevent clipping
            fwrite(&pcm_sample, 2, 1, wav_file);
        }
        
        // Maintain phase continuity
        phase += 2.0 * M_PI * freq * current->duration;
        current = current->next;
    }
    
    fclose(wav_file);
    printf("[AudioEngine] WAV file generated successfully\n");
    return 0;
}

// Play entire sample list
AUDIO_API int play_sample_list(audio_sample_node_t* head)
{
    if (!g_initialized) {
        return -1;
    }
    
    printf("[AudioEngine] Playing sample list...\n");
    
    audio_sample_node_t *current = head;
    int sample_count = 0;
    
    while (current) {
        play_frequency(current->frequency, current->amplitude, current->duration);
        current = current->next;
        sample_count++;
    }
    
    printf("[AudioEngine] Played %d samples\n", sample_count);
    return 0;
}

// Set master volume
AUDIO_API int set_master_volume(double volume)
{
    if (volume < 0.0) volume = 0.0;
    if (volume > 1.0) volume = 1.0;
    
    g_master_volume = volume;
    printf("[AudioEngine] Master volume set to %.2f\n", volume);
    return 0;
}

// Apply audio effect (basic implementation)
AUDIO_API int apply_audio_effect(const char* effect_name, double parameter)
{
    if (!effect_name) return -1;
    
    printf("[AudioEngine] Applying effect '%s' with parameter %.2f\n", effect_name, parameter);
    
    if (strcmp(effect_name, "volume") == 0) {
        return set_master_volume(parameter);
    } else if (strcmp(effect_name, "pitch") == 0) {
        // Pitch shift effect (placeholder)
        printf("[AudioEngine] Pitch shift by %.2f semitones\n", parameter);
        return 0;
    } else if (strcmp(effect_name, "reverb") == 0) {
        // Reverb effect (placeholder)
        printf("[AudioEngine] Reverb with %.2f decay\n", parameter);
        return 0;
    }
    
    printf("[AudioEngine] Unknown effect: %s\n", effect_name);
    return -1;
}

// Get audio devices (placeholder)
AUDIO_API int get_audio_devices(char* device_list, int max_length)
{
    if (!device_list || max_length < 50) return -1;
    
    strncpy(device_list, "Default Audio Device;SONAR Virtual Device", max_length - 1);
    device_list[max_length - 1] = '\0';
    return 2; // Number of devices
}

// Set audio device (placeholder)
AUDIO_API int set_audio_device(int device_id)
{
    printf("[AudioEngine] Audio device set to ID: %d\n", device_id);
    return 0;
}

// Get supported sample rates
AUDIO_API int get_supported_sample_rates(int* rates, int max_count)
{
    if (!rates || max_count < 4) return -1;
    
    rates[0] = 22050;
    rates[1] = 44100;
    rates[2] = 48000;
    rates[3] = 96000;
    
    return 4; // Number of supported rates
}

// Get library version
AUDIO_API const char* get_library_version(void)
{
    return g_library_version;
}

// Get library name
AUDIO_API const char* get_library_name(void)
{
    return g_library_name;
}

// Generate analysis report file
AUDIO_API int generate_analysis_report(audio_sample_node_t* samples, const char* filename)
{
    if (!samples || !filename) return -1;
    
    char report_filename[512];
    snprintf(report_filename, sizeof(report_filename), "%s_analysis.txt", filename);
    
    FILE* report = fopen(report_filename, "w");
    if (!report) {
        printf("[AudioEngine] Error: Could not create analysis report\n");
        return -1;
    }
    
    fprintf(report, "SONAR Audio Analysis Report\n");
    fprintf(report, "===========================\n\n");
    fprintf(report, "Generated by: %s\n", get_library_name());
    fprintf(report, "Version: %s\n\n", get_library_version());
    
    // Analyze samples
    int sample_count = 0;
    double total_freq = 0.0, min_freq = 999999.0, max_freq = 0.0;
    double total_amp = 0.0, min_amp = 1.0, max_amp = 0.0;
    
    audio_sample_node_t* current = samples;
    while (current) {
        total_freq += current->frequency;
        total_amp += current->amplitude;
        
        if (current->frequency < min_freq) min_freq = current->frequency;
        if (current->frequency > max_freq) max_freq = current->frequency;
        if (current->amplitude < min_amp) min_amp = current->amplitude;
        if (current->amplitude > max_amp) max_amp = current->amplitude;
        
        sample_count++;
        current = current->next;
    }
    
    if (sample_count > 0) {
        fprintf(report, "Sample Statistics:\n");
        fprintf(report, "- Total samples: %d\n", sample_count);
        fprintf(report, "- Average frequency: %.2f Hz\n", total_freq / sample_count);
        fprintf(report, "- Frequency range: %.2f - %.2f Hz\n", min_freq, max_freq);
        fprintf(report, "- Average amplitude: %.3f\n", total_amp / sample_count);
        fprintf(report, "- Amplitude range: %.3f - %.3f\n\n", min_amp, max_amp);
        
        fprintf(report, "Detailed Sample Data:\n");
        fprintf(report, "Byte\tFreq(Hz)\tAmp\tDuration(s)\n");
        fprintf(report, "----\t--------\t---\t-----------\n");
        
        current = samples;
        while (current) {
            fprintf(report, "0x%02X\t%.2f\t\t%.3f\t%.3f\n", 
                   current->source_byte, current->frequency, 
                   current->amplitude, current->duration);
            current = current->next;
        }
    }
    
    fclose(report);
    printf("[AudioEngine] Analysis report saved: %s\n", report_filename);
    return 0;
}

// Generate frequency data CSV file
AUDIO_API int generate_frequency_data(audio_sample_node_t* samples, const char* filename)
{
    if (!samples || !filename) return -1;
    
    char csv_filename[512];
    snprintf(csv_filename, sizeof(csv_filename), "%s_frequencies.csv", filename);
    
    FILE* csv = fopen(csv_filename, "w");
    if (!csv) {
        printf("[AudioEngine] Error: Could not create CSV file\n");
        return -1;
    }
    
    fprintf(csv, "Sample,Byte_Hex,Byte_Dec,Frequency_Hz,Amplitude,Duration_s\n");
    
    audio_sample_node_t* current = samples;
    int index = 0;
    
    while (current) {
        fprintf(csv, "%d,0x%02X,%d,%.2f,%.3f,%.3f\n", 
               index, current->source_byte, current->source_byte,
               current->frequency, current->amplitude, current->duration);
        current = current->next;
        index++;
    }
    
    fclose(csv);
    printf("[AudioEngine] Frequency data saved: %s\n", csv_filename);
    return 0;
}

// Generate audio metadata JSON file
AUDIO_API int generate_metadata_json(audio_sample_node_t* samples, const char* filename)
{
    if (!samples || !filename) return -1;
    
    char json_filename[512];
    snprintf(json_filename, sizeof(json_filename), "%s_metadata.json", filename);
    
    FILE* json = fopen(json_filename, "w");
    if (!json) {
        printf("[AudioEngine] Error: Could not create JSON file\n");
        return -1;
    }
    
    fprintf(json, "{\n");
    fprintf(json, "  \"audio_engine\": {\n");
    fprintf(json, "    \"name\": \"%s\",\n", get_library_name());
    fprintf(json, "    \"version\": \"%s\"\n", get_library_version());
    fprintf(json, "  },\n");
    fprintf(json, "  \"audio_config\": {\n");
    fprintf(json, "    \"sample_rate\": %d,\n", g_sample_rate);
    fprintf(json, "    \"master_volume\": %.2f\n", g_master_volume);
    fprintf(json, "  },\n");
    fprintf(json, "  \"samples\": [\n");
    
    audio_sample_node_t* current = samples;
    int first = 1;
    
    while (current) {
        if (!first) fprintf(json, ",\n");
        fprintf(json, "    {\n");
        fprintf(json, "      \"byte\": \"0x%02X\",\n", current->source_byte);
        fprintf(json, "      \"frequency\": %.2f,\n", current->frequency);
        fprintf(json, "      \"amplitude\": %.3f,\n", current->amplitude);
        fprintf(json, "      \"duration\": %.3f\n", current->duration);
        fprintf(json, "    }");
        current = current->next;
        first = 0;
    }
    
    fprintf(json, "\n  ]\n");
    fprintf(json, "}\n");
    
    fclose(json);
    printf("[AudioEngine] Metadata saved: %s\n", json_filename);
    return 0;
}

// Cleanup audio system
AUDIO_API void cleanup_audio(void)
{
    if (!g_initialized) {
        return;
    }
    
    printf("[AudioEngine] Cleaning up audio system\n");
    
    // Clear buffers
    memset(g_audio_buffer, 0, sizeof(g_audio_buffer));
    g_buffer_pos = 0;
    g_master_volume = 1.0;
    g_initialized = 0;
    
    printf("[AudioEngine] Cleanup complete\n");
}

// DLL entry point for Windows
#ifdef _WIN32
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        printf("[AudioEngine] DLL loaded\n");
        break;
    case DLL_PROCESS_DETACH:
        cleanup_audio();
        printf("[AudioEngine] DLL unloaded\n");
        break;
    }
    return TRUE;
}
#endif
# SONAR

![SONAR](docs/Documentation/sonar_icon.svg)

A powerful data analysis and visualization toolkit that transforms binary files into audio representations for unique auditory analysis. SONAR reads files, partitions them, and applies various transformations including audio sonification, enabling innovative approaches to data inspection and pattern recognition.

## 🎵 Features

- **Audio Sonification (SONAR)**: Convert binary data to audio frequencies for auditory analysis
- **Data Reconstruction (dSONAR)**: Reverse-engineer audio back to original binary data
- **Multiple Analysis Modules**: Hexadecimal display, character counting, text viewing
- **Dynamic Audio Engine**: Extensible audio processing with DLL support
- **Visualization Tools**: 3D audio visualization and spectral analysis
- **Cross-Platform**: Windows and Linux support
- **High Performance**: Optimized C implementation with modular architecture

## 🚀 Quick Start

### Prerequisites

- GCC compiler
- Make build system
- Python 3.x (for visualization tools)
- Windows: MinGW or Visual Studio Build Tools
- Linux: Standard development tools

### Building

```bash
# Clone and navigate to project
cd mojibake

# Build the project
make

# Build debug version (optional)
make debug

# Build shared library (optional)
make shared
```

### Basic Usage

```bash
# Convert file to audio (SONAR mode)
./build/bin/mojibake_sonar input.txt sonar

# Hexadecimal analysis
./build/bin/mojibake_sonar input.bin hex

# Character count analysis
./build/bin/mojibake_sonar document.txt charcount

# Text view analysis
./build/bin/mojibake_sonar data.txt textview

# Reverse audio to data (dSONAR mode)
./build/bin/mojibake_sonar audio.wav dsonar
```

## 📁 Project Structure

```
mojibake/
├── src/                    # Source code
│   ├── core/              # Core application logic
│   ├── main/              # Main executable
│   └── modules/default/   # Analysis modules
├── lib/                   # Audio engine library
│   ├── audio_engine.dll  # Dynamic audio library
│   ├── audio_engine.c    # Audio engine source
│   └── audio_engine.h    # Audio engine header
├── include/               # Public headers
├── build/                 # Build output
│   ├── bin/              # Executables
│   └── obj/              # Object files
├── samples/               # Example files
│   ├── audio/            # Audio samples
│   ├── data/             # Data samples
│   └── images/           # Generated visualizations
├── tools/                 # Python visualization tools
├── docs/                  # Generated documentation
├── web/                   # Web documentation
└── exports/               # Visualization exports
```

## 🔧 Analysis Modules

### SONAR Module
Converts binary data to audio frequencies:
- Maps bytes to specific frequencies (220Hz - 2220Hz range)
- Generates WAV files for each data partition
- Provides frequency analysis and statistics
- Supports dynamic audio library loading

### dSONAR Module
Reverse-engineers audio back to binary data:
- Analyzes WAV files to extract frequency data
- Reconstructs original byte sequences
- Supports CSV and binary output formats
- Validates reconstruction accuracy

### Other Modules
- **Hexadecimal Display**: Traditional hex dump with ASCII representation
- **Character Count**: Statistical analysis of character frequencies
- **Text View**: Human-readable text extraction and display

## 🎨 Visualization Tools

Python-based visualization suite in the `tools/` directory:

```bash
# 3D audio visualization
python tools/sound_ball.py

# Enhanced visualization with additional features
python tools/enhanced_sound_ball.py

# Advanced audio analysis
python tools/advanced_audio_viz.py
```

### Dependencies
```bash
pip install numpy matplotlib plotly scipy librosa
```

## 📖 Documentation

- **API Documentation**: Generated with Doxygen in `docs/Documentation/`
- **Project Overview**: `web/project_overview.html`
- **Sample Usage**: `samples/README.md`
- **Visualization Guide**: `tools/visualization_README.md`

## 🔊 Audio Engine

The project includes a dynamic audio engine (`lib/audio_engine.dll`) that provides:
- Real-time audio playback
- WAV file generation
- Audio analysis and reporting
- Cross-platform audio support

### Audio Engine API
```c
// Initialize audio system
int init_audio(int sample_rate);

// Play frequency with specified amplitude and duration
int play_frequency(double frequency, double amplitude, double duration);

// Generate WAV file from audio samples
int generate_wav(const char* filename, audio_sample_node_t* samples);

// Cleanup audio resources
void cleanup_audio(void);
```

## 🛠️ Build Targets

```bash
make                # Build main executable
make debug          # Build with debug symbols
make shared         # Build shared library
make clean          # Clean build artifacts
make docs           # Generate documentation (requires Doxygen)
```

## 📊 Example Workflows

### 1. Binary File Analysis
```bash
# Analyze a binary file with SONAR
./build/bin/mojibake_sonar firmware.bin sonar

# Generated files:
# - sonar_partition_0.wav, sonar_partition_1.wav, ...
# - Frequency analysis output
```

### 2. Audio Data Recovery
```bash
# Recover data from audio file
./build/bin/mojibake_sonar recovered_audio.wav dsonar

# Generated files:
# - reconstructed_data.bin
# - frequency_analysis.csv
```

### 3. Visualization Pipeline
```bash
# Generate audio from data
./build/bin/mojibake_sonar data.txt sonar

# Create 3D visualization
python tools/enhanced_sound_ball.py

# View results in exports/ directory
```

## 🐛 Troubleshooting

### Common Issues

**Dynamic library not found**:
- Ensure `lib/audio_engine.dll` exists
- Check file permissions
- Verify path in configuration

**Build errors**:
- Install required development tools
- Check GCC version compatibility
- Ensure all dependencies are available

**Audio playback issues**:
- Verify audio drivers are installed
- Check system audio configuration
- Test with built-in audio generation

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is part of the SONAR Development Team's research into data sonification and analysis techniques.

## 🔗 Related Projects

- Audio analysis and visualization tools
- Binary data inspection utilities
- Data sonification research

## 📞 Support

For questions, issues, or contributions:
- Check the documentation in `docs/Documentation/`
- Review sample files in `samples/`
- Examine the project overview at `web/project_overview.html`

---

**SONAR** - Transforming data into sound, sound into insight.
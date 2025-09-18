# SONAR - Interactive Data Sonification & Analysis Toolkit


[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)]()
[![Language](https://img.shields.io/badge/language-C%20%7C%20Python-green.svg)]()

A powerful data analysis and visualization toolkit that transforms binary files into audio representations for unique auditory analysis. SONAR reads files, partitions them, and applies various transformations including audio sonification, enabling innovative approaches to data inspection and pattern recognition.

## Key Features

### Audio Sonification (SONAR)
- Convert binary data to audio frequencies for auditory analysis
- Map bytes to specific frequencies (220Hz - 2220Hz range)
- Generate WAV files for each data partition
- Real-time audio playback with interactive controls

### Data Reconstruction (dSONAR)
- Reverse-engineer audio back to original binary data
- Analyze WAV files to extract frequency data
- Reconstruct original byte sequences with validation
- Support for CSV and binary output formats


![SONAR](docs/Documentation/sonar_icon.svg)

### Interactive Visualization
- **Interactive Byte Viewer**: Modern UI with grid, sequential, and spectrogram views
- **Zoom Functionality**: Detailed spectrum analysis with 0.5x to 8x zoom levels
- **Real-time Navigation**: Arrow key navigation and mouse interaction
- **3D Audio Visualization**: Advanced plotly-based 3D representations
- **Spectral Analysis**: Waterfall displays and frequency domain analysis

### Analysis Modules
- Hexadecimal display with ASCII representation
- Character frequency analysis and statistics
- Text extraction and viewing capabilities
- Dynamic audio engine with DLL support

## Quick Start

### Prerequisites

**System Requirements:**
- GCC compiler or Visual Studio Build Tools
- Make build system
- Python 3.7+ (for visualization tools)

**Python Dependencies:**
```bash
pip install pygame numpy matplotlib plotly scipy librosa tkinter
```

### Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/SONAR_Project.git
cd SONAR_Project

# Build the project
make

# Build debug version (optional)
make debug
```

### Basic Usage

#### Command Line Interface
```bash
# Convert file to audio (SONAR mode)
./build/bin/mojibake_sonar input.txt sonar

# Reverse audio to data (dSONAR mode)
./build/bin/mojibake_sonar audio.wav dsonar

# Hexadecimal analysis
./build/bin/mojibake_sonar input.bin hex

# Character count analysis
./build/bin/mojibake_sonar document.txt charcount
```

#### Interactive Byte Viewer
```bash
# Launch interactive visualization tool
python tools/interactive_byte_viewer.py
```

**Controls:**
- **Arrow Keys**: Navigate through data
- **Mouse Click**: Select bytes and play sounds
- **Spacebar**: Play current byte sound
- **S**: Toggle between grid/sequential/spectrogram views
- **+/-**: Zoom in/out (spectrum view)
- **0**: Reset zoom
- **L**: Load audio files
- **ESC**: Exit

## Project Structure

```
SONAR_Project/
├── src/                     # Core C implementation
│   ├── core/                   # Core application logic
│   ├── main/                   # Main executable
│   └── modules/default/        # Analysis modules
├── lib/                     # Audio engine library
│   ├── audio_engine.dll       # Dynamic audio library
│   ├── audio_engine.c         # Audio engine source
│   └── audio_engine.h         # Audio engine headers
├── tools/                   # Python visualization suite
│   ├── interactive_byte_viewer.py  # Main interactive tool
│   ├── advanced_audio_viz.py   # Advanced visualizations
│   ├── enhanced_sound_ball.py  # 3D audio visualization
│   └── sound_ball.py          # Basic 3D visualization
├── samples/                 # Example files and data
│   ├── audio/                 # Audio samples
│   ├── data/                  # Test data files
│   └── images/                # Generated visualizations
├── build/                   # Build output
│   ├── bin/                   # Executables
│   └── obj/                   # Object files
├── docs/                    # Generated documentation
├── web/                     # Web documentation
└── exports/                 # Visualization exports
```

## Interactive Byte Viewer Features

### View Modes

#### Grid View
- Traditional hex grid layout
- Color-coded frequency mapping
- Interactive cell selection
- Real-time audio feedback

#### Sequential View
- Linear byte progression
- Enhanced arrow key navigation
- Smooth scrolling and paging
- Continuous data flow visualization

#### Spectrogram View
- Frequency domain analysis
- **Zoom Functionality**: 0.5x to 8x magnification
- Time and frequency axis labels
- Interactive position highlighting
- Smart zoom centering

### Modern UI Features
- **Responsive Design**: Adapts to different screen sizes
- **Rounded Corners**: Modern visual aesthetics
- **Hover Effects**: Interactive button feedback
- **Tooltips**: Contextual help and information
- **Status Bar**: Real-time information display
- **Progress Indicators**: Visual feedback for operations
- **Help Overlay**: Built-in keyboard shortcuts guide

## Audio Engine API

The dynamic audio engine provides comprehensive audio processing:

```c
// Initialize audio system
int init_audio(int sample_rate);

// Play frequency with specified parameters
int play_frequency(double frequency, double amplitude, double duration);

// Generate WAV file from audio samples
int generate_wav(const char* filename, audio_sample_node_t* samples);

// Cleanup audio resources
void cleanup_audio(void);
```

## Visualization Tools

### 3D Audio Visualization
```bash
# Basic 3D visualization
python tools/sound_ball.py

# Enhanced visualization with additional features
python tools/enhanced_sound_ball.py

# Advanced audio analysis with multiple views
python tools/advanced_audio_viz.py
```

### Export Formats
- **HTML**: Interactive 3D plots
- **PNG**: High-resolution static images
- **CSV**: Raw data for further analysis
- **WAV**: Audio file generation

## Build System

```bash
make                # Build main executable
make debug          # Build with debug symbols
make shared         # Build shared library
make clean          # Clean build artifacts
make docs           # Generate documentation (requires Doxygen)
```

## Example Workflows

### 1. Interactive Data Exploration
```bash
# Launch interactive viewer
python tools/interactive_byte_viewer.py

# Load your data file using 'L' key
# Switch between views using 'S' key
# Use zoom controls (+/-/0) in spectrogram view
# Navigate with arrow keys or mouse clicks
```

### 2. Binary File Analysis Pipeline
```bash
# Step 1: Convert binary to audio
./build/bin/mojibake_sonar firmware.bin sonar

# Step 2: Visualize in 3D
python tools/enhanced_sound_ball.py

# Step 3: Interactive analysis
python tools/interactive_byte_viewer.py

# Generated files:
# - sonar_partition_*.wav (audio files)
# - 3d_landscape.html (3D visualization)
# - spectral_waterfall.png (frequency analysis)
```

### 3. Audio Data Recovery
```bash
# Recover data from audio file
./build/bin/mojibake_sonar recovered_audio.wav dsonar

# Verify reconstruction
python tools/interactive_byte_viewer.py

# Generated files:
# - reconstructed_data.bin
# - frequency_analysis.csv
```

## Advanced Features

### Zoom Functionality (Spectrogram View)
- **Zoom Levels**: 0.5x to 8.0x magnification
- **Smart Centering**: Automatically centers on current selection
- **Bounds Checking**: Prevents zoom beyond data limits
- **Visual Feedback**: Real-time zoom level display
- **Keyboard Controls**: Intuitive +/- and 0 key bindings

### Navigation Enhancements
- **Arrow Key Navigation**: Smooth movement in all views
- **Page Navigation**: Quick data traversal
- **Mouse Interaction**: Click-to-select functionality
- **Position Highlighting**: Visual current position indicators

## Troubleshooting

### Common Issues

**Python Dependencies Missing:**
```bash
pip install -r samples/data/requirements_viz.txt
```

**Audio Engine Not Found:**
- Ensure `lib/audio_engine.dll` exists
- Check file permissions and path configuration
- Verify audio drivers are installed

**Build Errors:**
- Install required development tools
- Check GCC version compatibility
- Ensure all dependencies are available

**Interactive Viewer Issues:**
- Verify pygame installation: `pip install pygame`
- Check Python version (3.7+ required)
- Ensure tkinter is available for file dialogs

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests if applicable
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Development Guidelines
- Follow existing code style and conventions
- Add documentation for new features
- Include sample files for testing
- Update README for significant changes

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- SONAR Development Team for innovative data sonification research
- Contributors to the visualization and audio processing modules
- Open source community for tools and libraries used

## Support & Documentation

- **API Documentation**: `docs/Documentation/index.html`
- **Project Overview**: `web/project_overview.html`
- **Sample Usage**: `samples/README.md`
- **Visualization Guide**: `tools/visualization_README.md`
- **Byte Viewer Guide**: `tools/byte_viewer_README.md`

## Related Projects

- [Audio Analysis Tools](samples/audio/)
- [Binary Data Inspection Utilities](tools/)
- [Data Sonification Research](docs/)

---

**SONAR** - *Transforming data into sound, sound into insight.*

**Experience your data in a whole new dimension**

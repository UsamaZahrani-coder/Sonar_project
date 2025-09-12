# Visualization Tools

This directory contains Python scripts for visualizing and analyzing audio data from the SONAR project.

## Scripts

### Core Visualization Tools
- **`sound_ball.py`** - Main 3D visualization tool for audio data
- **`enhanced_sound_ball.py`** - Enhanced version with additional features and improvements

### Testing and Development
- **`test_visualizations.py`** - Test suite for visualization functions
- **`test_3d_fix.py`** - Testing script for 3D rendering fixes
- **`test_ms_conversion.py`** - Testing millisecond conversion utilities

## Usage

Run the visualization tools with audio data from the samples directory:

```bash
python visualization/sound_ball.py
python visualization/enhanced_sound_ball.py
```

## Dependencies

Make sure you have the required Python packages installed:
- numpy
- matplotlib
- plotly (for 3D visualizations)
- scipy (for audio processing)

## Output

Visualization outputs are typically saved to the `exports/` directory in the project root.
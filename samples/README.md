# Samples Directory

This directory contains sample files organized by type for the SONAR project.

## Directory Structure

### `audio/`
Contains audio files (.wav) used for testing SONAR and dSONAR functionality:
- `encrypt1.wav` - Encrypted audio sample
- `notencrypted_sample.wav` - Unencrypted audio sample
- `Same_1.wav`, `Same_2.wav` - Comparison audio files
- `sonar_partition_0.wav` - SONAR partition sample
- `test_audio.wav` - General test audio file

### `data/`
Contains data files for testing various modules:
- `sample.txt` - Text sample file
- `sample.securestuff` - Encrypted/secure data sample
- `requirements_viz.txt` - Visualization requirements

### `images/`
Contains generated image files from SONAR analysis:
- `enc1.png` - Encrypted data visualization
- `Same_1.png`, `Same2.png` - Comparison visualizations

### `output/`
Directory for storing generated output files from analysis.

### `test/`
Directory for temporary test files and experimental data.

## Usage

These sample files can be used with the mojibake_sonar executable:

```bash
# Test with audio file
./build/bin/mojibake_sonar samples/audio/test_audio.wav sonar

# Test with data file
./build/bin/mojibake_sonar samples/data/sample.txt hex

# Test dSONAR functionality
./build/bin/mojibake_sonar samples/audio/sonar_partition_0.wav dsonar
```
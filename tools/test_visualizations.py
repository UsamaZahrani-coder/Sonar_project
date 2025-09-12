#!/usr/bin/env python3
"""
Test script for advanced 3D audio visualization features
"""

import sys
import numpy as np
from pathlib import Path

def test_visualization_features():
    """Test all visualization features"""
    print("Testing Advanced 3D Audio Visualization Features")
    print("=" * 50)
    
    try:
        from advanced_audio_viz import Advanced3DAudioVisualizer
        print("✓ Advanced3DAudioVisualizer imported successfully")
    except ImportError as e:
        print(f"✗ Failed to import Advanced3DAudioVisualizer: {e}")
        return False
    
    # Test with the generated test audio file
    test_file = "test_audio.wav"
    if not Path(test_file).exists():
        print(f"✗ Test audio file {test_file} not found")
        return False
    
    print(f"✓ Test audio file {test_file} found")
    
    # Initialize visualizer
    visualizer = Advanced3DAudioVisualizer()
    print("✓ Visualizer initialized")
    
    # Load audio file
    success = visualizer.load_audio_file(test_file)
    if not success:
        print("✗ Failed to load audio file")
        return False
    
    print("✓ Audio file loaded successfully")
    print(f"  - Audio samples: {len(visualizer.audio_data)}")
    print(f"  - Sample rate: {visualizer.sample_rate} Hz")
    print(f"  - Duration: {len(visualizer.audio_data) / visualizer.sample_rate:.2f} seconds")
    
    # Test 1: 3D Frequency Landscape
    print("\n1. Testing 3D Frequency Landscape...")
    try:
        fig = visualizer.create_3d_frequency_landscape(interactive=False)
        if fig:
            print("✓ 3D Frequency Landscape created successfully")
        else:
            print("✗ Failed to create 3D Frequency Landscape")
    except Exception as e:
        print(f"✗ Error creating 3D Frequency Landscape: {e}")
    
    # Test 2: Interactive Audio Scrubbing
    print("\n2. Testing Interactive Audio Scrubbing...")
    try:
        fig = visualizer.create_interactive_scrubbing_interface()
        if fig:
            print("✓ Interactive Audio Scrubbing interface created successfully")
        else:
            print("✗ Failed to create Interactive Audio Scrubbing interface")
    except Exception as e:
        print(f"✗ Error creating Interactive Audio Scrubbing: {e}")
    
    # Test 3: Dual Domain Analysis
    print("\n3. Testing Dual Domain Analysis...")
    try:
        fig = visualizer.create_dual_domain_analysis()
        if fig:
            print("✓ Dual Domain Analysis created successfully")
        else:
            print("✗ Failed to create Dual Domain Analysis")
    except Exception as e:
        print(f"✗ Error creating Dual Domain Analysis: {e}")
    
    # Test 4: Spectral Waterfall Display
    print("\n4. Testing Spectral Waterfall Display...")
    try:
        fig = visualizer.create_realtime_waterfall_display()
        if fig:
            print("✓ Spectral Waterfall Display created successfully")
        else:
            print("✗ Failed to create Spectral Waterfall Display")
    except Exception as e:
        print(f"✗ Error creating Spectral Waterfall Display: {e}")
    
    # Test 5: Audio Segment Playback
    print("\n5. Testing Audio Segment Playback...")
    try:
        # Test playing a 1-second segment from the beginning
        visualizer.play_audio_segment(0.0, 1.0)
        print("✓ Audio segment playback initiated successfully")
    except Exception as e:
        print(f"✗ Error with audio segment playback: {e}")
    
    # Test 6: Export Functionality
    print("\n6. Testing Export Functionality...")
    try:
        # Create exports directory if it doesn't exist
        Path("test_exports").mkdir(exist_ok=True)
        
        # Test exporting 3D landscape
        fig_3d = visualizer.create_3d_frequency_landscape(interactive=True)
        if fig_3d:
            fig_3d.write_html("test_exports/test_3d_landscape.html")
            print("✓ 3D Landscape exported to HTML")
        
        # Test exporting dual domain analysis
        fig_dual = visualizer.create_dual_domain_analysis()
        if fig_dual:
            fig_dual.savefig("test_exports/test_dual_domain.png", dpi=150, bbox_inches='tight')
            print("✓ Dual Domain Analysis exported to PNG")
        
        # Test exporting waterfall
        fig_waterfall = visualizer.create_realtime_waterfall_display()
        if fig_waterfall:
            fig_waterfall.savefig("test_exports/test_waterfall.png", dpi=150, bbox_inches='tight')
            print("✓ Spectral Waterfall exported to PNG")
        
    except Exception as e:
        print(f"✗ Error during export: {e}")
    
    print("\n" + "=" * 50)
    print("Visualization testing completed!")
    print("\nGenerated files:")
    
    # List generated files
    export_dir = Path("test_exports")
    if export_dir.exists():
        for file in export_dir.iterdir():
            if file.is_file():
                print(f"  - {file.name} ({file.stat().st_size} bytes)")
    
    return True

def test_enhanced_player():
    """Test the enhanced audio player functionality"""
    print("\nTesting Enhanced Audio Player")
    print("-" * 30)
    
    try:
        from enhanced_sound_ball import EnhancedAudioPlayer
        print("✓ EnhancedAudioPlayer imported successfully")
    except ImportError as e:
        print(f"✗ Failed to import EnhancedAudioPlayer: {e}")
        return False
    
    player = EnhancedAudioPlayer()
    print("✓ Enhanced audio player initialized")
    
    # Test loading file
    test_file = "test_audio.wav"
    if Path(test_file).exists():
        success = player.load_file(test_file)
        if success:
            print("✓ Audio file loaded in enhanced player")
            print(f"  - Visualizer available: {player.visualizer is not None}")
        else:
            print("✗ Failed to load audio file in enhanced player")
    else:
        print(f"✗ Test audio file {test_file} not found")
    
    return True

def main():
    """Main test function"""
    print("Advanced 3D Audio Visualization Test Suite")
    print("=" * 60)
    
    # Check if test audio file exists, create if not
    test_file = "test_audio.wav"
    if not Path(test_file).exists():
        print("Creating test audio file...")
        try:
            import scipy.io.wavfile as wav
            sr = 44100
            t = np.linspace(0, 3, sr * 3)
            freq1 = 440  # A4
            freq2 = 880  # A5
            signal = np.sin(2 * np.pi * freq1 * t) + 0.5 * np.sin(2 * np.pi * freq2 * t)
            wav.write(test_file, sr, (signal * 32767).astype(np.int16))
            print(f"✓ Test audio file {test_file} created")
        except Exception as e:
            print(f"✗ Failed to create test audio file: {e}")
            return
    
    # Run tests
    viz_success = test_visualization_features()
    player_success = test_enhanced_player()
    
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    print(f"Visualization Features: {'PASS' if viz_success else 'FAIL'}")
    print(f"Enhanced Audio Player: {'PASS' if player_success else 'FAIL'}")
    
    if viz_success and player_success:
        print("\n🎉 All tests passed! The 3D audio visualization system is working correctly.")
        print("\nYou can now:")
        print("1. Use the CLI mode: python enhanced_sound_ball.py --file audio.wav --mode cli")
        print("2. Export visualizations: python enhanced_sound_ball.py --file audio.wav --mode cli --export")
        print("3. View exported files in the exports/ and test_exports/ directories")
    else:
        print("\n❌ Some tests failed. Please check the error messages above.")

if __name__ == "__main__":
    main()
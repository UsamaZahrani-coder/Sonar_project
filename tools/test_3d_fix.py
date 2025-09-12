#!/usr/bin/env python3
"""
Test script to verify the 3D visualization fix
"""

import sys
from pathlib import Path

# Import the enhanced application components
try:
    from enhanced_sound_ball import EnhancedAudioPlayer
    from advanced_audio_viz import Advanced3DAudioVisualizer
except ImportError as e:
    print(f"Import error: {e}")
    sys.exit(1)

def test_3d_visualization():
    """Test the 3D visualization functionality"""
    print("Testing 3D Visualization Fix...")
    
    # Create enhanced audio player
    player = EnhancedAudioPlayer()
    
    # Test file path
    test_file = "test_audio.wav"
    if not Path(test_file).exists():
        print(f"Error: Test file {test_file} not found")
        return False
    
    print(f"Loading audio file: {test_file}")
    
    # Load the audio file
    success = player.load_file(test_file)
    if not success:
        print("Failed to load audio file")
        return False
    
    print("Audio file loaded successfully")
    
    # Check if visualizer is available
    if not player.visualizer:
        print("Error: Visualizer not available")
        return False
    
    print("Visualizer available")
    
    # Check if audio data is loaded
    if player.visualizer.audio_data is None:
        print("Error: Audio data not loaded in visualizer")
        return False
    
    print(f"Audio data loaded: {len(player.visualizer.audio_data)} samples")
    print(f"Sample rate: {player.visualizer.sample_rate} Hz")
    
    # Test 3D landscape creation
    print("Testing 3D landscape creation...")
    try:
        fig = player.visualizer.create_3d_frequency_landscape(interactive=True)
        if fig:
            print("✅ 3D landscape created successfully!")
            
            # Save to file for verification
            fig.write_html("test_3d_landscape.html")
            print("3D landscape saved to test_3d_landscape.html")
            return True
        else:
            print("❌ Failed to create 3D landscape - returned None")
            return False
    except Exception as e:
        print(f"❌ Error creating 3D landscape: {e}")
        return False

def test_other_visualizations():
    """Test other visualization methods"""
    print("\nTesting other visualizations...")
    
    player = EnhancedAudioPlayer()
    player.load_file("test_audio.wav")
    
    tests = [
        ("Interactive Scrubbing", lambda: player.visualizer.create_interactive_scrubbing_interface()),
        ("Dual Domain Analysis", lambda: player.visualizer.create_dual_domain_analysis()),
        ("Spectral Waterfall", lambda: player.visualizer.create_realtime_waterfall_display())
    ]
    
    results = []
    for name, test_func in tests:
        try:
            result = test_func()
            if result:
                print(f"✅ {name}: Success")
                results.append(True)
            else:
                print(f"❌ {name}: Failed (returned None)")
                results.append(False)
        except Exception as e:
            print(f"❌ {name}: Error - {e}")
            results.append(False)
    
    return all(results)

if __name__ == "__main__":
    print("=" * 50)
    print("3D Visualization Fix Test")
    print("=" * 50)
    
    # Test main 3D visualization
    main_test = test_3d_visualization()
    
    # Test other visualizations
    other_tests = test_other_visualizations()
    
    print("\n" + "=" * 50)
    print("Test Results:")
    print(f"3D Landscape: {'✅ PASS' if main_test else '❌ FAIL'}")
    print(f"Other Visualizations: {'✅ PASS' if other_tests else '❌ FAIL'}")
    
    if main_test and other_tests:
        print("\n🎉 All tests passed! The 3D visualization fix is working.")
    else:
        print("\n⚠️ Some tests failed. Please check the error messages above.")
    
    print("=" * 50)
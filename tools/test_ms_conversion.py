#!/usr/bin/env python3
"""
Test script to verify time conversion to milliseconds
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from advanced_audio_viz import Advanced3DAudioVisualizer

def test_millisecond_conversion():
    print("Testing millisecond conversion...")
    
    # Create visualizer and load audio
    viz = Advanced3DAudioVisualizer()
    
    if not viz.load_audio_file('test_audio.wav'):
        print("❌ Failed to load audio file")
        return False
    
    print(f"Audio loaded: {len(viz.audio_data)} samples at {viz.sample_rate} Hz")
    
    # Test 3D landscape creation
    print("Creating 3D landscape...")
    fig = viz.create_3d_frequency_landscape(interactive=True)
    
    if fig is None:
        print("❌ Failed to create 3D landscape")
        return False
    
    # Check if the figure has the correct axis title
    layout = fig.layout
    scene = layout.scene
    xaxis_title = scene.xaxis.title.text
    
    print(f"X-axis title: {xaxis_title}")
    
    if 'ms' in xaxis_title.lower():
        print("✅ Time axis correctly shows milliseconds")
        return True
    else:
        print("❌ Time axis does not show milliseconds")
        return False

if __name__ == "__main__":
    print("=" * 50)
    print("Millisecond Conversion Test")
    print("=" * 50)
    
    success = test_millisecond_conversion()
    
    print("=" * 50)
    if success:
        print("🎉 Test PASSED: Time conversion to milliseconds works!")
    else:
        print("❌ Test FAILED: Time conversion issue detected")
    print("=" * 50)
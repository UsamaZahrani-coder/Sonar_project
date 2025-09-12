#!/usr/bin/env python3
"""
Enhanced Sound Ball with Advanced 3D Visualization Integration
Combines the original sound_ball.py functionality with new 3D visualization features
"""

import sys
import math
import argparse
import colorsys
import time
from pathlib import Path
import numpy as np
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import threading
import pygame

# Import original sound_ball functionality
try:
    from sound_ball import (
        load_audio_mono, dominant_freq_series, hz_to_hex_color,
        Ball, FreqLabel, FreqBallApp, FileNameLabel
    )
except ImportError:
    print("Warning: Could not import from sound_ball.py")
    # Fallback implementations would go here

# Import advanced visualization
try:
    from advanced_audio_viz import Advanced3DAudioVisualizer
except ImportError:
    print("Warning: Could not import advanced_audio_viz.py")
    Advanced3DAudioVisualizer = None

from textual.app import App, ComposeResult
from textual.containers import Container, Horizontal, Vertical
from textual.reactive import reactive
from textual.screen import ModalScreen
from textual.widgets import Static, Button, Footer, Input, Label, Select, Checkbox
from textual import events

class Enhanced3DVisualizationScreen(ModalScreen):
    """Modal screen for 3D visualization options"""
    
    def __init__(self, visualizer, **kwargs):
        super().__init__(**kwargs)
        self.visualizer = visualizer
    
    def compose(self) -> ComposeResult:
        with Container(id="viz-modal"):
            yield Label("Advanced 3D Visualization Options", id="viz-title")
            
            with Vertical():
                yield Button("🏔️ 3D Frequency Landscape", id="btn-3d-landscape")
                yield Button("🎵 Interactive Audio Scrubbing", id="btn-scrubbing")
                yield Button("📊 Dual Domain Analysis", id="btn-dual-domain")
                yield Button("🌊 Spectral Waterfall", id="btn-waterfall")
                yield Button("📁 Export All Visualizations", id="btn-export-all")
                
            with Horizontal():
                yield Button("Close", id="btn-close", variant="primary")
    
    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "btn-close":
            self.dismiss()
        elif event.button.id == "btn-3d-landscape":
            self._create_3d_landscape()
        elif event.button.id == "btn-scrubbing":
            self._create_scrubbing_interface()
        elif event.button.id == "btn-dual-domain":
            self._create_dual_domain()
        elif event.button.id == "btn-waterfall":
            self._create_waterfall()
        elif event.button.id == "btn-export-all":
            self._export_all_visualizations()
    
    def _create_3d_landscape(self):
        """Create and display 3D frequency landscape"""
        if self.visualizer:
            try:
                fig = self.visualizer.create_3d_frequency_landscape(interactive=True)
                if fig:
                    import plotly.offline as pyo
                    pyo.plot(fig, filename='3d_landscape.html', auto_open=True)
                    self.notify("3D Landscape visualization opened in browser")
                else:
                    self.notify("Failed to create 3D landscape", severity="error")
            except Exception as e:
                self.notify(f"Error creating 3D landscape: {e}", severity="error")
    
    def _create_scrubbing_interface(self):
        """Create interactive scrubbing interface"""
        if self.visualizer:
            try:
                fig = self.visualizer.create_interactive_scrubbing_interface()
                if fig:
                    import plotly.offline as pyo
                    pyo.plot(fig, filename='interactive_scrubbing.html', auto_open=True)
                    self.notify("Interactive scrubbing interface opened in browser")
                else:
                    self.notify("Failed to create scrubbing interface", severity="error")
            except Exception as e:
                self.notify(f"Error creating scrubbing interface: {e}", severity="error")
    
    def _create_dual_domain(self):
        """Create dual domain analysis"""
        if self.visualizer:
            try:
                fig = self.visualizer.create_dual_domain_analysis()
                if fig:
                    import matplotlib.pyplot as plt
                    plt.show()
                    self.notify("Dual domain analysis displayed")
                else:
                    self.notify("Failed to create dual domain analysis", severity="error")
            except Exception as e:
                self.notify(f"Error creating dual domain analysis: {e}", severity="error")
    
    def _create_waterfall(self):
        """Create spectral waterfall display"""
        if self.visualizer:
            try:
                fig = self.visualizer.create_realtime_waterfall_display()
                if fig:
                    import matplotlib.pyplot as plt
                    plt.show()
                    self.notify("Spectral waterfall display shown")
                else:
                    self.notify("Failed to create waterfall display", severity="error")
            except Exception as e:
                self.notify(f"Error creating waterfall display: {e}", severity="error")
    
    def _export_all_visualizations(self):
        """Export all visualizations to files"""
        if self.visualizer:
            try:
                # Create all visualizations and export them
                export_count = 0
                
                # 3D Landscape
                fig_3d = self.visualizer.create_3d_frequency_landscape(interactive=True)
                if fig_3d:
                    fig_3d.write_html("exports/3d_landscape.html")
                    export_count += 1
                
                # Interactive Scrubbing
                fig_scrub = self.visualizer.create_interactive_scrubbing_interface()
                if fig_scrub:
                    fig_scrub.write_html("exports/interactive_scrubbing.html")
                    export_count += 1
                
                # Dual Domain
                fig_dual = self.visualizer.create_dual_domain_analysis()
                if fig_dual:
                    fig_dual.savefig("exports/dual_domain_analysis.png", dpi=300, bbox_inches='tight')
                    export_count += 1
                
                # Waterfall
                fig_waterfall = self.visualizer.create_realtime_waterfall_display()
                if fig_waterfall:
                    fig_waterfall.savefig("exports/spectral_waterfall.png", dpi=300, bbox_inches='tight')
                    export_count += 1
                
                self.notify(f"Exported {export_count} visualizations to exports/ folder")
                
            except Exception as e:
                self.notify(f"Error exporting visualizations: {e}", severity="error")

class EnhancedAudioPlayer:
    """Enhanced audio player with 3D visualization capabilities"""
    
    def __init__(self):
        pygame.mixer.init()
        self.sound = None
        self.is_playing = False
        self.is_paused = False
        self.file_path = None
        self.visualizer = None
        
        if Advanced3DAudioVisualizer:
            self.visualizer = Advanced3DAudioVisualizer()
    
    def load_file(self, file_path):
        """Load audio file for both playback and visualization"""
        try:
            # Load for pygame playback
            self.sound = pygame.mixer.Sound(file_path)
            self.file_path = file_path
            
            # Load for visualization
            if self.visualizer:
                success = self.visualizer.load_audio_file(file_path)
                if not success:
                    print("Warning: Failed to load file for visualization")
            
            return True
        except Exception as e:
            print(f"Error loading file: {e}")
            return False
    
    def play(self):
        if self.sound and not self.is_playing:
            if self.is_paused:
                pygame.mixer.unpause()
                self.is_paused = False
            else:
                self.sound.play()
            self.is_playing = True
    
    def pause(self):
        if self.is_playing:
            pygame.mixer.pause()
            self.is_playing = False
            self.is_paused = True
    
    def stop(self):
        pygame.mixer.stop()
        self.is_playing = False
        self.is_paused = False
    
    def play_segment(self, start_time, duration=1.0):
        """Play a specific segment using the visualizer"""
        if self.visualizer:
            self.visualizer.play_audio_segment(start_time, duration)

class EnhancedFreqBallApp(FreqBallApp):
    """Enhanced version of the original FreqBallApp with 3D visualization"""
    
    CSS = """
    Screen { align: center middle; }
    
    #main-container {
        width: 100%;
        height: 100%;
        align: center middle;
    }
    
    #controls {
        dock: bottom;
        height: auto;
        background: $surface;
        padding: 1;
    }
    
    #viz-controls {
        dock: right;
        width: 30;
        background: $surface;
        padding: 1;
    }
    
    #ball-container {
        align: center middle;
        width: 100%;
        height: 100%;
    }
    
    Ball {
        margin: 1;
    }
    
    Button {
        margin: 1;
        min-width: 16;
    }
    
    #viz-title {
        text-align: center;
        text-style: bold;
        color: $accent;
        margin-bottom: 1;
    }
    
    #file-info {
        text-align: center;
        color: $text-muted;
        margin: 1;
    }
    
    #freq-display {
        text-align: center;
        text-style: bold;
        color: $primary;
        margin: 1;
    }
    """
    
    def __init__(self, freqs=None, sr=22050, hop=2048, win=4096, fmin=30.0, fmax=6000.0, audio_file=None, **kwargs):
        # Use default values if not provided
        if freqs is None:
            freqs = np.array([440.0])  # Default A4 frequency
        
        super().__init__(freqs, sr, hop, win, fmin, fmax, audio_file, **kwargs)
        self.enhanced_player = EnhancedAudioPlayer()
        self.current_file = audio_file
    
    def compose(self) -> ComposeResult:
        # Initialize components first
        self.ball = Ball()
        self.label = FreqLabel(id="freq")
        self.filename_label = FileNameLabel(id="filename")
        
        # Set initial filename if provided
        if self.current_file:
            self.filename_label.update_filename(self.current_file)
        
        with Container(id="main-container"):
            # Visualization controls panel
            with Container(id="viz-controls"):
                yield Label("3D Visualization", id="viz-title")
                yield Button("🏔️ 3D Landscape", id="btn-3d-viz")
                yield Button("🎵 Audio Scrubbing", id="btn-scrubbing")
                yield Button("📊 Dual Analysis", id="btn-dual")
                yield Button("🌊 Waterfall", id="btn-waterfall")
                yield Button("📁 Load File", id="btn-load-file")
                yield Label("", id="file-info")
            
            # Main ball visualization area
            with Container(id="ball-container"):
                yield self.ball
                yield self.label
                yield self.filename_label
                yield Label("", id="freq-display")
            
            # Audio controls
            with Container(id="controls"):
                yield Button("▶️ Play", id="btn-play")
                yield Button("⏸️ Pause", id="btn-pause")
                yield Button("⏹️ Stop", id="btn-stop")
                yield Button("🎛️ Advanced Viz", id="btn-advanced-viz")
        
        yield Footer()
    
    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "btn-load-file":
            self._load_audio_file()
        elif event.button.id == "btn-play":
            self.enhanced_player.play()
        elif event.button.id == "btn-pause":
            self.enhanced_player.pause()
        elif event.button.id == "btn-stop":
            self.enhanced_player.stop()
        elif event.button.id == "btn-advanced-viz":
            self._show_advanced_viz()
        elif event.button.id == "btn-3d-viz":
            self._create_3d_landscape()
        elif event.button.id == "btn-scrubbing":
            self._create_scrubbing()
        elif event.button.id == "btn-dual":
            self._create_dual_analysis()
        elif event.button.id == "btn-waterfall":
            self._create_waterfall()
    
    def _load_audio_file(self):
        """Load audio file using file dialog"""
        def load_in_thread():
            try:
                import tkinter as tk
                from tkinter import filedialog
                
                root = tk.Tk()
                root.withdraw()  # Hide the main window
                
                file_path = filedialog.askopenfilename(
                    title="Select Audio File",
                    filetypes=[
                        ("Audio Files", "*.wav *.mp3 *.flac *.ogg"),
                        ("WAV Files", "*.wav"),
                        ("All Files", "*.*")
                    ]
                )
                
                root.destroy()
                
                if file_path:
                    success = self.enhanced_player.load_file(file_path)
                    if success:
                        self.current_file = file_path
                        file_name = Path(file_path).name
                        self.call_from_thread(self._update_file_info, file_name)
                        self.notify(f"Loaded: {file_name}")
                    else:
                        self.call_from_thread(self.notify, "Failed to load audio file", severity="error")
                        
            except Exception as e:
                self.call_from_thread(self.notify, f"Error loading file: {e}", severity="error")
        
        threading.Thread(target=load_in_thread, daemon=True).start()
    
    def _update_file_info(self, filename):
        """Update file info display"""
        file_info = self.query_one("#file-info")
        file_info.update(f"File: {filename}")
    
    def _show_advanced_viz(self):
        """Show advanced visualization modal"""
        if self.enhanced_player.visualizer and self.current_file:
            modal = Enhanced3DVisualizationScreen(self.enhanced_player.visualizer)
            self.push_screen(modal)
        else:
            self.notify("Please load an audio file first", severity="warning")
    
    def _create_3d_landscape(self):
        """Create 3D landscape visualization"""
        if not self.enhanced_player.visualizer:
            self.notify("3D Visualizer not available", severity="error")
            return
            
        if not self.current_file:
            self.notify("Please load an audio file first", severity="warning")
            return
            
        # Check if audio data is actually loaded in the visualizer
        if self.enhanced_player.visualizer.audio_data is None:
            self.notify("Audio data not loaded. Please reload the file.", severity="warning")
            # Try to reload the file
            success = self.enhanced_player.visualizer.load_audio_file(self.current_file)
            if not success:
                self.notify("Failed to load audio data for visualization", severity="error")
                return
        
        try:
            fig = self.enhanced_player.visualizer.create_3d_frequency_landscape(interactive=True)
            if fig:
                import plotly.offline as pyo
                pyo.plot(fig, filename='3d_landscape.html', auto_open=True)
                self.notify("3D Landscape opened in browser")
            else:
                self.notify("Failed to create 3D landscape - no data available", severity="error")
        except Exception as e:
            self.notify(f"Failed to create 3D image: {e}", severity="error")
    
    def _create_scrubbing(self):
        """Create interactive scrubbing interface"""
        if not self.enhanced_player.visualizer:
            self.notify("3D Visualizer not available", severity="error")
            return
            
        if not self.current_file:
            self.notify("Please load an audio file first", severity="warning")
            return
            
        # Check if audio data is actually loaded in the visualizer
        if self.enhanced_player.visualizer.audio_data is None:
            success = self.enhanced_player.visualizer.load_audio_file(self.current_file)
            if not success:
                self.notify("Failed to load audio data for visualization", severity="error")
                return
        
        try:
            fig = self.enhanced_player.visualizer.create_interactive_scrubbing_interface()
            if fig:
                import plotly.offline as pyo
                pyo.plot(fig, filename='interactive_scrubbing.html', auto_open=True)
                self.notify("Interactive scrubbing opened in browser")
            else:
                self.notify("Failed to create scrubbing interface", severity="error")
        except Exception as e:
            self.notify(f"Error creating scrubbing interface: {e}", severity="error")
    
    def _create_dual_analysis(self):
        """Create dual domain analysis"""
        if not self.enhanced_player.visualizer:
            self.notify("3D Visualizer not available", severity="error")
            return
            
        if not self.current_file:
            self.notify("Please load an audio file first", severity="warning")
            return
            
        # Check if audio data is actually loaded in the visualizer
        if self.enhanced_player.visualizer.audio_data is None:
            success = self.enhanced_player.visualizer.load_audio_file(self.current_file)
            if not success:
                self.notify("Failed to load audio data for visualization", severity="error")
                return
        
        try:
            fig = self.enhanced_player.visualizer.create_dual_domain_analysis()
            if fig:
                import matplotlib.pyplot as plt
                plt.show()
                self.notify("Dual domain analysis displayed")
            else:
                self.notify("Failed to create dual domain analysis", severity="error")
        except Exception as e:
            self.notify(f"Error creating dual domain analysis: {e}", severity="error")
    
    def _create_waterfall(self):
        """Create spectral waterfall display"""
        if not self.enhanced_player.visualizer:
            self.notify("3D Visualizer not available", severity="error")
            return
            
        if not self.current_file:
            self.notify("Please load an audio file first", severity="warning")
            return
            
        # Check if audio data is actually loaded in the visualizer
        if self.enhanced_player.visualizer.audio_data is None:
            success = self.enhanced_player.visualizer.load_audio_file(self.current_file)
            if not success:
                self.notify("Failed to load audio data for visualization", severity="error")
                return
        
        try:
            fig = self.enhanced_player.visualizer.create_realtime_waterfall_display()
            if fig:
                import matplotlib.pyplot as plt
                plt.show()
                self.notify("Spectral waterfall displayed")
            else:
                self.notify("Failed to create waterfall display", severity="error")
        except Exception as e:
            self.notify(f"Error creating waterfall display: {e}", severity="error")

def main():
    """Main entry point for enhanced sound ball application"""
    parser = argparse.ArgumentParser(description="Enhanced Sound Ball with 3D Visualization")
    parser.add_argument("--file", "-f", help="Audio file to load on startup")
    parser.add_argument("--mode", "-m", choices=["gui", "cli", "viz"], default="gui",
                       help="Application mode: gui (default), cli, or viz")
    parser.add_argument("--export", "-e", action="store_true",
                       help="Export all visualizations (CLI mode only)")
    
    args = parser.parse_args()
    
    if args.mode == "cli" or args.mode == "viz":
        # Command line mode
        if not args.file:
            print("Error: Audio file required for CLI mode")
            sys.exit(1)
        
        if not Advanced3DAudioVisualizer:
            print("Error: Advanced visualization not available")
            sys.exit(1)
        
        visualizer = Advanced3DAudioVisualizer()
        
        if visualizer.load_audio_file(args.file):
            print(f"Loaded audio file: {args.file}")
            
            if args.export:
                print("Creating and exporting all visualizations...")
                
                # Create exports directory
                exports_dir = Path(__file__).parent / "exports"
                exports_dir.mkdir(exist_ok=True)
                
                # Export all visualizations
                try:
                    fig_3d = visualizer.create_3d_frequency_landscape(interactive=True)
                    if fig_3d:
                        fig_3d.write_html(str(exports_dir / "3d_landscape.html"))
                        print("✓ 3D landscape exported")
                    
                    fig_scrub = visualizer.create_interactive_scrubbing_interface()
                    if fig_scrub:
                        fig_scrub.write_html(str(exports_dir / "interactive_scrubbing.html"))
                        print("✓ Interactive scrubbing exported")
                    
                    fig_dual = visualizer.create_dual_domain_analysis()
                    if fig_dual:
                        fig_dual.savefig(str(exports_dir / "dual_domain_analysis.png"), dpi=300, bbox_inches='tight')
                        print("✓ Dual domain analysis exported")
                    
                    fig_waterfall = visualizer.create_realtime_waterfall_display()
                    if fig_waterfall:
                        fig_waterfall.savefig(str(exports_dir / "spectral_waterfall.png"), dpi=300, bbox_inches='tight')
                        print("✓ Spectral waterfall exported")
                    
                    print("\nAll visualizations exported to exports/ folder")
                    
                except Exception as e:
                    print(f"Error during export: {e}")
            else:
                # Interactive CLI mode
                print("\nAvailable visualizations:")
                print("1. 3D Frequency Landscape")
                print("2. Interactive Audio Scrubbing")
                print("3. Dual Domain Analysis")
                print("4. Spectral Waterfall")
                print("5. All visualizations")
                
                choice = input("\nSelect visualization (1-5): ")
                
                if choice == "1":
                    fig = visualizer.create_3d_frequency_landscape(interactive=True)
                    if fig:
                        import plotly.offline as pyo
                        pyo.plot(fig, auto_open=True)
                elif choice == "2":
                    fig = visualizer.create_interactive_scrubbing_interface()
                    if fig:
                        import plotly.offline as pyo
                        pyo.plot(fig, auto_open=True)
                elif choice == "3":
                    fig = visualizer.create_dual_domain_analysis()
                    if fig:
                        import matplotlib.pyplot as plt
                        plt.show()
                elif choice == "4":
                    fig = visualizer.create_realtime_waterfall_display()
                    if fig:
                        import matplotlib.pyplot as plt
                        plt.show()
                elif choice == "5":
                    # Show all visualizations
                    print("Creating all visualizations...")
                    
                    fig_3d = visualizer.create_3d_frequency_landscape(interactive=True)
                    if fig_3d:
                        import plotly.offline as pyo
                        pyo.plot(fig_3d, filename='3d_landscape.html', auto_open=True)
                    
                    fig_scrub = visualizer.create_interactive_scrubbing_interface()
                    if fig_scrub:
                        pyo.plot(fig_scrub, filename='interactive_scrubbing.html', auto_open=True)
                    
                    fig_dual = visualizer.create_dual_domain_analysis()
                    if fig_dual:
                        import matplotlib.pyplot as plt
                        plt.show()
                    
                    fig_waterfall = visualizer.create_realtime_waterfall_display()
                    if fig_waterfall:
                        plt.show()
        else:
            print(f"Failed to load audio file: {args.file}")
            sys.exit(1)
    
    else:
        # GUI mode
        try:
            # Load audio file if specified to get frequency data
            freqs = None
            sr = 22050
            if args.file:
                try:
                    y, sr = load_audio_mono(args.file, target_sr=sr)
                    freqs = dominant_freq_series(y, sr)
                    print(f"Loaded {len(freqs)} frequency samples from {args.file}")
                except Exception as e:
                    print(f"Warning: Could not analyze audio file: {e}")
                    freqs = np.array([440.0])  # Default frequency
            
            app = EnhancedFreqBallApp(
                freqs=freqs,
                sr=sr,
                hop=2048,
                win=4096,
                fmin=30.0,
                fmax=6000.0,
                audio_file=args.file
            )
            
            app.run()
            
        except Exception as e:
            print(f"Error starting GUI: {e}")
            print("Make sure all required packages are installed:")
            print("pip install -r requirements_viz.txt")
            sys.exit(1)

if __name__ == "__main__":
    main()
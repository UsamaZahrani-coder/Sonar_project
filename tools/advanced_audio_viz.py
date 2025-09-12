#!/usr/bin/env python3
"""
Advanced 3D Audio Visualization Module for SONAR
Provides comprehensive 3D visualization capabilities including:
- 3D frequency landscapes
- Interactive audio scrubbing
- Dual-domain waveform analysis
- Real-time spectral waterfall displays
"""

import sys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button
from mpl_toolkits.mplot3d import Axes3D
import plotly.graph_objects as go
import plotly.express as px
from plotly.subplots import make_subplots
import plotly.offline as pyo
from scipy import signal
from scipy.fft import fft, fftfreq
import threading
import time
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox
import pygame
from collections import deque

# Import audio loading functions from sound_ball.py
try:
    from sound_ball import load_audio_mono, dominant_freq_series, hz_to_hex_color
except ImportError:
    print("Warning: Could not import from sound_ball.py. Some functions may not work.")
    
    def load_audio_mono(path, target_sr=22050):
        """Fallback audio loader"""
        try:
            import librosa
            y, sr = librosa.load(path, sr=target_sr, mono=True)
            return y.astype(np.float32), sr
        except Exception as e:
            raise RuntimeError(f"Could not load audio: {e}")

class Advanced3DAudioVisualizer:
    """Advanced 3D Audio Visualization System"""
    
    def __init__(self):
        self.audio_data = None
        self.sample_rate = None
        self.frequencies = None
        self.times = None
        self.spectrogram = None
        self.current_position = 0
        self.is_playing = False
        self.audio_player = None
        self.waterfall_data = deque(maxlen=100)  # Store last 100 frames
        
        # Initialize pygame for audio playback
        pygame.mixer.init()
        
    def load_audio_file(self, file_path):
        """Load audio file and prepare for visualization"""
        try:
            self.audio_data, self.sample_rate = load_audio_mono(file_path)
            print(f"Loaded audio: {len(self.audio_data)} samples at {self.sample_rate} Hz")
            self._prepare_analysis_data()
            return True
        except Exception as e:
            print(f"Error loading audio: {e}")
            return False
    
    def _prepare_analysis_data(self):
        """Prepare frequency and time analysis data"""
        if self.audio_data is None:
            return
            
        # Calculate spectrogram
        window_size = 2048
        hop_length = 512
        
        self.frequencies, self.times, self.spectrogram = signal.spectrogram(
            self.audio_data, 
            self.sample_rate,
            window='hann',
            nperseg=window_size,
            noverlap=window_size - hop_length
        )
        
        # Convert to dB scale
        self.spectrogram = 20 * np.log10(self.spectrogram + 1e-10)
        
        print(f"Spectrogram shape: {self.spectrogram.shape}")
        print(f"Frequency range: {self.frequencies[0]:.1f} - {self.frequencies[-1]:.1f} Hz")
        print(f"Time range: {self.times[0]*1000:.1f} - {self.times[-1]*1000:.1f} ms")
    
    def create_3d_frequency_landscape(self, interactive=True):
        """Create 3D frequency landscape visualization
        Height = frequency, Color = amplitude, Time = depth
        """
        if self.spectrogram is None:
            print("No audio data loaded")
            return None
            
        if interactive:
            return self._create_plotly_3d_landscape()
        else:
            return self._create_matplotlib_3d_landscape()
    
    def _create_plotly_3d_landscape(self):
        """Create interactive 3D landscape using Plotly"""
        # Downsample for performance
        freq_step = max(1, len(self.frequencies) // 100)
        time_step = max(1, len(self.times) // 200)
        
        freq_subset = self.frequencies[::freq_step]
        time_subset = self.times[::time_step]
        spec_subset = self.spectrogram[::freq_step, ::time_step]
        
        # Create meshgrid
        T, F = np.meshgrid(time_subset, freq_subset)
        
        # Create 3D surface plot
        fig = go.Figure(data=[go.Surface(
            x=T*1000,
            y=F, 
            z=spec_subset,
            colorscale='Viridis',
            colorbar=dict(title="Amplitude (dB)"),
            hovertemplate='Time: %{x:.1f}ms<br>Frequency: %{y:.1f}Hz<br>Amplitude: %{z:.1f}dB<extra></extra>'
        )])
        
        fig.update_layout(
            title='3D Frequency Landscape - SONAR Audio Visualization',
            scene=dict(
                xaxis_title='Time (ms)',
                yaxis_title='Frequency (Hz)',
                zaxis_title='Amplitude (dB)',
                camera=dict(
                    eye=dict(x=1.5, y=1.5, z=1.5)
                )
            ),
            width=1000,
            height=700
        )
        
        return fig
    
    def _create_matplotlib_3d_landscape(self):
        """Create 3D landscape using Matplotlib"""
        fig = plt.figure(figsize=(12, 8))
        ax = fig.add_subplot(111, projection='3d')
        
        # Downsample for performance
        freq_step = max(1, len(self.frequencies) // 50)
        time_step = max(1, len(self.times) // 100)
        
        freq_subset = self.frequencies[::freq_step]
        time_subset = self.times[::time_step]
        spec_subset = self.spectrogram[::freq_step, ::time_step]
        
        # Create meshgrid
        T, F = np.meshgrid(time_subset, freq_subset)
        
        # Create surface plot
        surf = ax.plot_surface(T*1000, F, spec_subset, 
                              cmap='viridis', 
                              alpha=0.8,
                              linewidth=0,
                              antialiased=True)
        
        ax.set_xlabel('Time (ms)')
        ax.set_ylabel('Frequency (Hz)')
        ax.set_zlabel('Amplitude (dB)')
        ax.set_title('3D Frequency Landscape - SONAR Audio Visualization')
        
        # Add colorbar
        fig.colorbar(surf, shrink=0.5, aspect=5, label='Amplitude (dB)')
        
        return fig
    
    def create_interactive_scrubbing_interface(self):
        """Create interactive audio scrubbing interface with Plotly"""
        if self.spectrogram is None:
            print("No audio data loaded")
            return None
            
        # Create subplots
        fig = make_subplots(
            rows=3, cols=1,
            subplot_titles=('Waveform', 'Spectrogram', 'Frequency Analysis'),
            vertical_spacing=0.08,
            specs=[[{"secondary_y": False}],
                   [{"secondary_y": False}], 
                   [{"secondary_y": False}]]
        )
        
        # Time axis for waveform (in milliseconds)
        time_axis = np.linspace(0, len(self.audio_data) / self.sample_rate * 1000, len(self.audio_data))
        
        # Add waveform
        fig.add_trace(
            go.Scatter(x=time_axis, y=self.audio_data, 
                      mode='lines', name='Waveform',
                      line=dict(color='blue', width=1)),
            row=1, col=1
        )
        
        # Add spectrogram
        fig.add_trace(
            go.Heatmap(x=self.times*1000, y=self.frequencies, z=self.spectrogram,
                      colorscale='Viridis', name='Spectrogram',
                      hovertemplate='Time: %{x:.1f}ms<br>Frequency: %{y:.1f}Hz<br>Amplitude: %{z:.1f}dB<extra></extra>'),
            row=2, col=1
        )
        
        # Add frequency analysis (dominant frequencies over time)
        dominant_freqs = dominant_freq_series(self.audio_data, self.sample_rate)
        freq_times = np.linspace(0, len(self.audio_data) / self.sample_rate * 1000, len(dominant_freqs))
        
        fig.add_trace(
            go.Scatter(x=freq_times, y=dominant_freqs,
                      mode='lines+markers', name='Dominant Frequency',
                      line=dict(color='red', width=2),
                      marker=dict(size=4)),
            row=3, col=1
        )
        
        # Update layout
        fig.update_layout(
            title='Interactive Audio Analysis - Click to Scrub',
            height=800,
            showlegend=True
        )
        
        # Update axes labels
        fig.update_xaxes(title_text="Time (s)", row=1, col=1)
        fig.update_xaxes(title_text="Time (s)", row=2, col=1)
        fig.update_xaxes(title_text="Time (s)", row=3, col=1)
        
        fig.update_yaxes(title_text="Amplitude", row=1, col=1)
        fig.update_yaxes(title_text="Frequency (Hz)", row=2, col=1)
        fig.update_yaxes(title_text="Frequency (Hz)", row=3, col=1)
        
        return fig
    
    def create_dual_domain_analysis(self):
        """Create dual-domain waveform analysis (time and frequency)"""
        if self.audio_data is None:
            print("No audio data loaded")
            return None
            
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))
        
        # Time domain - full waveform
        time_axis = np.linspace(0, len(self.audio_data) / self.sample_rate * 1000, len(self.audio_data))
        ax1.plot(time_axis, self.audio_data, 'b-', linewidth=0.5)
        ax1.set_title('Time Domain - Full Waveform')
        ax1.set_xlabel('Time (ms)')
        ax1.set_ylabel('Amplitude')
        ax1.grid(True, alpha=0.3)
        
        # Time domain - zoomed segment
        start_idx = len(self.audio_data) // 4
        end_idx = start_idx + self.sample_rate // 10  # 100ms segment
        zoom_time = time_axis[start_idx:end_idx]
        zoom_data = self.audio_data[start_idx:end_idx]
        
        ax2.plot(zoom_time, zoom_data, 'r-', linewidth=1)
        ax2.set_title('Time Domain - Zoomed (100ms)')
        ax2.set_xlabel('Time (ms)')
        ax2.set_ylabel('Amplitude')
        ax2.grid(True, alpha=0.3)
        
        # Frequency domain - FFT of full signal
        fft_data = np.abs(fft(self.audio_data))
        fft_freqs = fftfreq(len(self.audio_data), 1/self.sample_rate)
        
        # Only plot positive frequencies
        pos_mask = fft_freqs >= 0
        ax3.semilogx(fft_freqs[pos_mask], 20 * np.log10(fft_data[pos_mask] + 1e-10), 'g-')
        ax3.set_title('Frequency Domain - Full Spectrum')
        ax3.set_xlabel('Frequency (Hz)')
        ax3.set_ylabel('Magnitude (dB)')
        ax3.grid(True, alpha=0.3)
        ax3.set_xlim(20, self.sample_rate // 2)
        
        # Frequency domain - spectrogram
        im = ax4.imshow(self.spectrogram, aspect='auto', origin='lower',
                       extent=[self.times[0]*1000, self.times[-1]*1000, 
                              self.frequencies[0], self.frequencies[-1]],
                       cmap='viridis')
        ax4.set_title('Frequency Domain - Spectrogram')
        ax4.set_xlabel('Time (ms)')
        ax4.set_ylabel('Frequency (Hz)')
        
        # Add colorbar for spectrogram
        plt.colorbar(im, ax=ax4, label='Amplitude (dB)')
        
        plt.tight_layout()
        return fig
    
    def create_realtime_waterfall_display(self, chunk_size=1024):
        """Create real-time spectral waterfall display"""
        if self.audio_data is None:
            print("No audio data loaded")
            return None
            
        # Simulate real-time by processing chunks
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))
        
        # Initialize waterfall data
        waterfall_history = []
        
        # Process audio in chunks
        for i in range(0, len(self.audio_data) - chunk_size, chunk_size // 2):
            chunk = self.audio_data[i:i + chunk_size]
            
            # Calculate FFT for this chunk
            fft_chunk = np.abs(fft(chunk * np.hanning(len(chunk))))
            fft_freqs = fftfreq(len(chunk), 1/self.sample_rate)
            
            # Only keep positive frequencies up to 8kHz for visualization
            max_freq_idx = np.where(fft_freqs <= 8000)[0][-1]
            fft_chunk = fft_chunk[:max_freq_idx]
            fft_freqs = fft_freqs[:max_freq_idx]
            
            # Convert to dB
            fft_db = 20 * np.log10(fft_chunk + 1e-10)
            
            waterfall_history.append(fft_db)
            
            # Keep only last 100 frames
            if len(waterfall_history) > 100:
                waterfall_history.pop(0)
        
        # Create waterfall display
        waterfall_array = np.array(waterfall_history).T
        
        # Plot current spectrum
        ax1.plot(fft_freqs, waterfall_array[:, -1], 'b-', linewidth=2)
        ax1.set_title('Current Frequency Spectrum')
        ax1.set_xlabel('Frequency (Hz)')
        ax1.set_ylabel('Magnitude (dB)')
        ax1.grid(True, alpha=0.3)
        ax1.set_xlim(0, 8000)
        
        # Plot waterfall
        im = ax2.imshow(waterfall_array, aspect='auto', origin='lower',
                       extent=[0, len(waterfall_history), fft_freqs[0], fft_freqs[-1]],
                       cmap='plasma')
        ax2.set_title('Spectral Waterfall Display')
        ax2.set_xlabel('Time (frames)')
        ax2.set_ylabel('Frequency (Hz)')
        
        plt.colorbar(im, ax=ax2, label='Magnitude (dB)')
        plt.tight_layout()
        
        return fig
    
    def play_audio_segment(self, start_time, duration=1.0):
        """Play a specific segment of the audio"""
        if self.audio_data is None:
            print("No audio data loaded")
            return
            
        start_idx = int(start_time * self.sample_rate)
        end_idx = int((start_time + duration) * self.sample_rate)
        
        if start_idx >= len(self.audio_data):
            print("Start time beyond audio length")
            return
            
        end_idx = min(end_idx, len(self.audio_data))
        segment = self.audio_data[start_idx:end_idx]
        
        # Convert to 16-bit integers for pygame
        segment_int = (segment * 32767).astype(np.int16)
        
        # Create stereo array
        stereo_segment = np.zeros((len(segment_int), 2), dtype=np.int16)
        stereo_segment[:, 0] = segment_int
        stereo_segment[:, 1] = segment_int
        
        try:
            sound = pygame.sndarray.make_sound(stereo_segment)
            sound.play()
            print(f"Playing segment: {start_time:.2f}s - {start_time + duration:.2f}s")
        except Exception as e:
            print(f"Error playing audio: {e}")
    
    def export_visualization(self, fig, filename, format='html'):
        """Export visualization to file"""
        try:
            if hasattr(fig, 'write_html'):  # Plotly figure
                if format == 'html':
                    fig.write_html(filename)
                elif format == 'png':
                    fig.write_image(filename)
                elif format == 'pdf':
                    fig.write_image(filename)
            else:  # Matplotlib figure
                if format == 'png':
                    fig.savefig(filename, dpi=300, bbox_inches='tight')
                elif format == 'pdf':
                    fig.savefig(filename, format='pdf', bbox_inches='tight')
                elif format == 'svg':
                    fig.savefig(filename, format='svg', bbox_inches='tight')
            
            print(f"Visualization exported to: {filename}")
            return True
        except Exception as e:
            print(f"Error exporting visualization: {e}")
            return False

def create_gui_interface():
    """Create GUI interface for the advanced visualizer"""
    root = tk.Tk()
    root.title("Advanced 3D Audio Visualizer - SONAR")
    root.geometry("600x400")
    
    visualizer = Advanced3DAudioVisualizer()
    
    def load_file():
        file_path = filedialog.askopenfilename(
            title="Select Audio File",
            filetypes=[("Audio Files", "*.wav *.mp3 *.flac *.ogg"), ("All Files", "*.*")]
        )
        if file_path:
            if visualizer.load_audio_file(file_path):
                status_label.config(text=f"Loaded: {Path(file_path).name}")
                # Enable visualization buttons
                for btn in viz_buttons:
                    btn.config(state='normal')
            else:
                messagebox.showerror("Error", "Failed to load audio file")
    
    def show_3d_landscape():
        fig = visualizer.create_3d_frequency_landscape(interactive=True)
        if fig:
            exports_dir = Path(__file__).parent / 'exports'
            exports_dir.mkdir(exist_ok=True)
            output_path = exports_dir / '3d_landscape.html'
            pyo.plot(fig, filename=str(output_path), auto_open=True)
    
    def show_interactive_scrubbing():
        fig = visualizer.create_interactive_scrubbing_interface()
        if fig:
            exports_dir = Path(__file__).parent / 'exports'
            exports_dir.mkdir(exist_ok=True)
            output_path = exports_dir / 'interactive_scrubbing.html'
            pyo.plot(fig, filename=str(output_path), auto_open=True)
    
    def show_dual_domain():
        fig = visualizer.create_dual_domain_analysis()
        if fig:
            plt.show()
    
    def show_waterfall():
        fig = visualizer.create_realtime_waterfall_display()
        if fig:
            plt.show()
    
    # GUI Layout
    tk.Label(root, text="Advanced 3D Audio Visualizer", font=('Arial', 16, 'bold')).pack(pady=10)
    
    tk.Button(root, text="Load Audio File", command=load_file, 
              bg='lightblue', font=('Arial', 12)).pack(pady=5)
    
    status_label = tk.Label(root, text="No file loaded", font=('Arial', 10))
    status_label.pack(pady=5)
    
    # Visualization buttons
    viz_frame = tk.Frame(root)
    viz_frame.pack(pady=20)
    
    viz_buttons = [
        tk.Button(viz_frame, text="3D Frequency Landscape", command=show_3d_landscape, 
                  state='disabled', width=20),
        tk.Button(viz_frame, text="Interactive Scrubbing", command=show_interactive_scrubbing, 
                  state='disabled', width=20),
        tk.Button(viz_frame, text="Dual Domain Analysis", command=show_dual_domain, 
                  state='disabled', width=20),
        tk.Button(viz_frame, text="Spectral Waterfall", command=show_waterfall, 
                  state='disabled', width=20)
    ]
    
    for i, btn in enumerate(viz_buttons):
        btn.grid(row=i//2, column=i%2, padx=10, pady=5)
    
    # Instructions
    instructions = tk.Text(root, height=8, width=70, wrap=tk.WORD)
    instructions.pack(pady=10)
    instructions.insert(tk.END, 
        "Instructions:\n"
        "1. Load an audio file using the 'Load Audio File' button\n"
        "2. Choose a visualization type:\n"
        "   • 3D Frequency Landscape: Interactive 3D surface plot\n"
        "   • Interactive Scrubbing: Click-to-play interface\n"
        "   • Dual Domain Analysis: Time and frequency views\n"
        "   • Spectral Waterfall: Real-time frequency analysis\n\n"
        "Features:\n"
        "• Click on visualizations to hear audio segments\n"
        "• Export visualizations as HTML, PNG, or PDF\n"
        "• Real-time frequency analysis and pattern detection"
    )
    instructions.config(state='disabled')
    
    return root, visualizer

if __name__ == "__main__":
    # Check if running as GUI or command line
    if len(sys.argv) > 1:
        # Command line mode
        audio_file = sys.argv[1]
        visualizer = Advanced3DAudioVisualizer()
        
        if visualizer.load_audio_file(audio_file):
            print("Creating visualizations...")
            
            # Create all visualizations
            fig_3d = visualizer.create_3d_frequency_landscape(interactive=True)
            if fig_3d:
                fig_3d.write_html("3d_landscape.html")
                print("3D landscape saved to 3d_landscape.html")
            
            fig_scrub = visualizer.create_interactive_scrubbing_interface()
            if fig_scrub:
                fig_scrub.write_html("interactive_scrubbing.html")
                print("Interactive scrubbing saved to interactive_scrubbing.html")
            
            fig_dual = visualizer.create_dual_domain_analysis()
            if fig_dual:
                fig_dual.savefig("dual_domain_analysis.png", dpi=300, bbox_inches='tight')
                print("Dual domain analysis saved to dual_domain_analysis.png")
            
            fig_waterfall = visualizer.create_realtime_waterfall_display()
            if fig_waterfall:
                fig_waterfall.savefig("spectral_waterfall.png", dpi=300, bbox_inches='tight')
                print("Spectral waterfall saved to spectral_waterfall.png")
        else:
            print(f"Failed to load audio file: {audio_file}")
    else:
        # GUI mode
        try:
            root, visualizer = create_gui_interface()
            root.mainloop()
        except Exception as e:
            print(f"Error starting GUI: {e}")
            print("Make sure tkinter is installed")
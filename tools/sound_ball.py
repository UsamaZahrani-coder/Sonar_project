#!/usr/bin/env python3
import sys, math, argparse, colorsys, time
from pathlib import Path
import numpy as np
import tkinter as tk
from tkinter import filedialog
import threading
import pygame

from textual.app import App, ComposeResult
from textual.containers import Container
from textual.reactive import reactive
from textual.screen import ModalScreen
from textual.widgets import Static, Button, Footer, Input, Label
from textual import events

# --- Try flexible audio loaders ---
def load_audio_mono(path: str, target_sr: int = 22050):
    """Return (y, sr). Prefers librosa (broad format support); falls back to soundfile."""
    try:
        import librosa  # type: ignore
        y, sr = librosa.load(path, sr=target_sr, mono=True)
        return y.astype(np.float32), sr
    except Exception:
        try:
            import soundfile as sf  # type: ignore
        except Exception as e:
            raise RuntimeError(
                "No audio loader available. Install `librosa` or `soundfile`."
            ) from e
        y, sr = sf.read(path)
        if y.ndim == 2:
            y = y.mean(axis=1)
        if sr != target_sr:
            # tiny resampler to avoid extra deps if librosa missing
            y = resample_poly(y, sr, target_sr)
            sr = target_sr
        return y.astype(np.float32), sr

def resample_poly(y, sr_in, sr_out):
    """Very small polyphase resampler; good enough for visualization."""
    from math import gcd
    g = gcd(sr_in, sr_out)
    up, down = sr_out // g, sr_in // g
    # Upsample
    y_up = np.zeros(len(y) * up, dtype=np.float32)
    y_up[::up] = y
    # Simple low-pass FIR (Hamming) for decimation
    taps = 101
    cutoff = 0.5 / down  # normalized (Nyquist=0.5)
    n = np.arange(taps) - (taps - 1) / 2
    h = np.sinc(2 * cutoff * n) * (0.54 - 0.46 * np.cos(2 * np.pi * np.arange(taps)/(taps-1)))
    h /= h.sum()
    y_f = np.convolve(y_up, h, mode="same")
    return y_f[::down]

def dominant_freq_series(y: np.ndarray, sr: int, win=4096, hop=2048, fmin=30, fmax=6000):
    """Extract dominant frequency over time using FFT."""
    if len(y) < win:
        return np.array([0.0])
    
    freqs = []
    for i in range(0, len(y) - win + 1, hop):
        chunk = y[i:i+win]
        # Apply window
        windowed = chunk * np.hanning(len(chunk))
        # FFT
        fft = np.fft.rfft(windowed)
        magnitude = np.abs(fft)
        # Frequency bins
        freq_bins = np.fft.rfftfreq(len(windowed), 1/sr)
        # Filter by frequency range
        valid_idx = (freq_bins >= fmin) & (freq_bins <= fmax)
        if not np.any(valid_idx):
            freqs.append(0.0)
            continue
        # Find peak
        peak_idx = np.argmax(magnitude[valid_idx])
        peak_freq = freq_bins[valid_idx][peak_idx]
        freqs.append(float(peak_freq))
    
    return np.array(freqs)

# --- Shape definitions ---
class Shape:
    """Class to generate different geometric shapes using Unicode characters"""
    
    @staticmethod
    def get_shape_for_frequency(freq_hz, diameter=15):
        """Return shape type based on frequency range"""
        if freq_hz < 600:  # Very low frequencies - hexagon
            return "hexagon"
        elif freq_hz < 750:  # Low frequencies - square
            return "square"
        elif freq_hz < 900:  # Mid-low frequencies - circle
            return "circle"
        elif freq_hz < 1050:  # Mid frequencies - triangle
            return "triangle"
        elif freq_hz < 1150:  # High frequencies - diamond
            return "diamond"
        else:  # Ultra high frequencies - star
            return "star"
    
    @staticmethod
    def render_hexagon(diameter):
        """Render a hexagon shape"""
        d = diameter
        lines = []
        center = d // 2
        
        for y in range(d):
            line = ""
            # Calculate hexagon width for this row
            if y <= center:
                width = center + y // 2
            else:
                width = center + (d - y - 1) // 2
            
            start = center - width // 2
            end = center + width // 2
            
            for x in range(d):
                if start <= x <= end:
                    dist_from_edge = min(x - start, end - x, y, d - y - 1)
                    if dist_from_edge == 0:
                        line += "█"
                    elif dist_from_edge <= 2:
                        line += "▓"
                    else:
                        line += "▒"
                else:
                    line += " "
            lines.append(line)
        return "\n".join(lines)
    
    @staticmethod
    def render_square(diameter):
        """Render a square shape with gradient effect"""
        d = diameter
        lines = []
        center = d // 2
        
        for y in range(d):
            line = ""
            for x in range(d):
                dist_from_center = max(abs(x - center), abs(y - center))
                dist_from_edge = min(x, y, d - x - 1, d - y - 1)
                
                if dist_from_edge == 0:
                    line += "█"
                elif dist_from_edge <= 1:
                    line += "▓"
                elif dist_from_center <= center // 2:
                    line += "▒"
                else:
                    line += "░"
            lines.append(line)
        return "\n".join(lines)
    
    @staticmethod
    def render_circle(diameter):
        """Render a circle shape with smooth gradients"""
        d = diameter
        center = (d - 1) / 2
        lines = []
        
        for y in range(d):
            line = ""
            for x in range(d):
                dist = ((x - center) ** 2 + (y - center) ** 2) ** 0.5
                radius = center
                
                if dist > radius:
                    line += " "
                elif dist > radius * 0.8:
                    line += "░"
                elif dist > radius * 0.6:
                    line += "▒"
                elif dist > radius * 0.3:
                    line += "▓"
                else:
                    line += "█"
            lines.append(line)
        return "\n".join(lines)
    
    @staticmethod
    def render_triangle(diameter):
        """Render an upward pointing triangle"""
        d = diameter
        lines = []
        center = d // 2
        
        for y in range(d):
            line = ""
            # Triangle gets wider as we go down
            if y < d // 6:  # Skip top portion for better triangle shape
                lines.append(" " * d)
                continue
                
            triangle_y = y - d // 6
            max_width = (triangle_y * 2 * center) // (d - d // 6)
            start = center - max_width // 2
            end = center + max_width // 2
            
            for x in range(d):
                if start <= x <= end:
                    dist_from_edge = min(x - start, end - x)
                    if dist_from_edge == 0 or y == d - 1:
                        line += "█"
                    elif dist_from_edge <= 1:
                        line += "▓"
                    elif dist_from_edge <= 3:
                        line += "▒"
                    else:
                        line += "░"
                else:
                    line += " "
            lines.append(line)
        return "\n".join(lines)
    
    @staticmethod
    def render_diamond(diameter):
        """Render a diamond shape"""
        d = diameter
        lines = []
        center = d // 2
        
        for y in range(d):
            line = ""
            # Diamond calculation
            if y <= center:
                width = y * 2
            else:
                width = (d - y - 1) * 2
            
            start = center - width // 2
            end = center + width // 2
            
            for x in range(d):
                if start <= x <= end:
                    dist_from_center = abs(x - center)
                    if x == start or x == end:
                        line += "█"
                    elif dist_from_center <= 1:
                        line += "▓"
                    elif dist_from_center <= width // 4:
                        line += "▒"
                    else:
                        line += "░"
                else:
                    line += " "
            lines.append(line)
        return "\n".join(lines)
    
    @staticmethod
    def render_star(diameter):
        """Render a star shape"""
        d = diameter
        lines = []
        center = d // 2
        
        for y in range(d):
            line = ""
            for x in range(d):
                dx = x - center
                dy = y - center
                dist = (dx * dx + dy * dy) ** 0.5
                
                # Create star pattern using angle calculations
                if dist == 0:
                    line += "█"
                else:
                    angle = np.arctan2(dy, dx)
                    # Create 5-pointed star
                    star_radius = center * (0.7 + 0.3 * np.cos(5 * angle))
                    
                    if dist <= star_radius * 0.3:
                        line += "█"
                    elif dist <= star_radius * 0.6:
                        line += "▓"
                    elif dist <= star_radius * 0.9:
                        line += "▒"
                    elif dist <= star_radius:
                        line += "░"
                    else:
                        line += " "
            lines.append(line)
        return "\n".join(lines)

# --- Color mapping ---
def hz_to_hex_color(hz, hz_min=30, hz_max=6000, s=0.9, v=0.95):
    """Map frequency (Hz) to a hex color string with enhanced color variety based on shape."""
    if hz <= 0: return "#404040"
    
    # Get shape type for this frequency
    shape_type = Shape.get_shape_for_frequency(hz)
    
    log_hz = np.log2(max(1e-6, hz))
    log_min = np.log2(max(1e-6, hz_min))
    log_max = np.log2(max(1e-6, hz_max))
    
    # Shape-specific color schemes
    if shape_type == "hexagon":
        # Very low frequencies: deep earth tones (brown, dark orange)
        hue = np.interp(log_hz, [log_min, log_max], [0.08, 0.12])  # Brown to dark orange
        saturation = 0.8 + 0.2 * np.sin(hz / 30)  # Subtle variation
        value = 0.7 + 0.2 * np.cos(hz / 40)  # Darker tones
    elif shape_type == "square":
        # Low frequencies: warm colors (reds, oranges, yellows)
        hue = np.interp(log_hz, [log_min, log_max], [0.0, 0.15])  # Red to orange
        saturation = 0.8 + 0.2 * np.sin(hz / 50)  # Subtle variation
        value = 0.9
    elif shape_type == "circle":
        # Mid frequencies: cool colors (greens, cyans)
        hue = np.interp(log_hz, [log_min, log_max], [0.25, 0.55])  # Green to cyan
        saturation = 0.7 + 0.3 * np.cos(hz / 200)  # Oscillating saturation
        value = 0.85
    elif shape_type == "triangle":
        # High frequencies: blue to purple spectrum
        hue = np.interp(log_hz, [log_min, log_max], [0.6, 0.8])  # Blue to purple
        saturation = 0.9
        value = 0.95 - 0.1 * np.sin(hz / 500)  # Varying brightness
    elif shape_type == "diamond":
        # Very high frequencies: purple to magenta spectrum
        hue = np.interp(log_hz, [log_min, log_max], [0.8, 0.95])  # Purple to magenta
        saturation = 1.0
        value = 0.9 + 0.1 * np.cos(hz / 1000)  # Bright with variation
    else:  # star
        # Ultra high frequencies: bright rainbow spectrum
        hue = (hz % 2000) / 2000.0  # Full rainbow cycle
        saturation = 0.95 + 0.05 * np.sin(hz / 1500)  # Very saturated
        value = 0.95 + 0.05 * np.cos(hz / 2000)  # Very bright
    
    # Clamp values
    saturation = max(0.3, min(1.0, saturation))
    value = max(0.5, min(1.0, value))
    
    # Convert HSV to RGB and return hex color
    r, g, b = colorsys.hsv_to_rgb(hue, saturation, value)
    return f"#{int(r*255):02x}{int(g*255):02x}{int(b*255):02x}"

# --- Audio Player ---
class AudioPlayer:
    def __init__(self):
        pygame.mixer.init()
        self.current_file = None
        self.is_playing = False
        
    def load_file(self, file_path):
        try:
            pygame.mixer.music.load(file_path)
            self.current_file = file_path
            return True
        except Exception as e:
            print(f"Error loading audio file: {e}")
            return False
    
    def play(self):
        if self.current_file:
            pygame.mixer.music.play()
            self.is_playing = True
    
    def pause(self):
        pygame.mixer.music.pause()
        self.is_playing = False
    
    def stop(self):
        pygame.mixer.music.stop()
        self.is_playing = False

# --- File name display widget ---
class FileNameLabel(Static):
    def __init__(self, **kwargs):
        super().__init__("No file loaded", **kwargs)
        

    def update_filename(self, file_path):
        if file_path:
            filename = Path(file_path).name
            self.update(f"File: {filename}")
        else:
            self.update("No file loaded")

# --- Native file dialog function ---
def show_file_dialog():
    """Show native file dialog and return selected file path."""
    # Create a root window but hide it
    root = tk.Tk()
    root.withdraw()
    
    # Show file dialog
    file_path = filedialog.askopenfilename(
        title="Select Audio File",
        filetypes=[
            ("Audio Files", "*.wav *.mp3 *.m4a *.ogg *.flac"),
            ("WAV Files", "*.wav"),
            ("MP3 Files", "*.mp3"),
            ("All Files", "*.*")
        ]
    )
    
    # Clean up
    root.destroy()
    
    return file_path if file_path else None

# --- Textual UI ---

class Ball(Static):
    freq_hz = reactive(0.0)
    color = reactive("#808080")
    diameter = reactive(15)
    shape_type = reactive("circle")

    def render(self):
        # Get shape type based on current frequency
        self.shape_type = Shape.get_shape_for_frequency(self.freq_hz, self.diameter)
        
        # Create frequency info display
        freq_info = f"\n{self.freq_hz:.1f} Hz - {self.shape_type.upper()}\n"
        
        # Render the appropriate shape
        if self.shape_type == "hexagon":
            shape_render = Shape.render_hexagon(self.diameter)
        elif self.shape_type == "square":
            shape_render = Shape.render_square(self.diameter)
        elif self.shape_type == "triangle":
            shape_render = Shape.render_triangle(self.diameter)
        elif self.shape_type == "diamond":
            shape_render = Shape.render_diamond(self.diameter)
        elif self.shape_type == "star":
            shape_render = Shape.render_star(self.diameter)
        else:  # default to circle
            shape_render = Shape.render_circle(self.diameter)
            
        return freq_info + shape_render + freq_info

    def watch_color(self, c): self.styles.color = c
    def watch_freq_hz(self, f): 
        # Update shape type based on frequency
        self.shape_type = Shape.get_shape_for_frequency(f)
        # Update color based on frequency
        self.color = hz_to_hex_color(f)
        # Update border title
        self.border_title = f"{f:7.1f} Hz ({self.shape_type})" if f > 0 else ""
        # Refresh to update both shape and color
        self.refresh()
    def watch_diameter(self, d): self.refresh()
    def watch_shape_type(self, s): self.refresh()

class FreqLabel(Static):
    freq_hz = reactive(0.0)
    def watch_freq_hz(self, f):
        self.update(f"{f:7.1f} Hz")

class FreqBallApp(App):
    CSS = """
    Screen { align: center middle; }
    #wrap { layout: vertical; align: center middle; }
    #title { color: cyan; }
    #freq { color: white; text-style: bold; }
    #filename { color: yellow; text-style: italic; margin: 1; }
    #controls { layout: horizontal; align: center middle; margin: 1; }
    """

    BINDINGS = [
        ("q", "quit", "Quit"),
        ("space", "toggle_pause", "Pause/Play"),
        ("[", "smaller", "Smaller"),
        ("]", "larger", "Larger"),
        ("u", "upload", "Upload File"),
        ("p", "play_audio", "Play Audio"),
    ]

    def __init__(self, freqs: np.ndarray, sr: int, hop: int, win: int, fmin: float, fmax: float, audio_file: str = None, **kwargs):
        super().__init__(**kwargs)
        self.freqs = freqs
        self.sr = sr
        self.hop = hop
        self.win = win
        self.fmin = fmin
        self.fmax = fmax
        self.audio_file = audio_file
        self.update_freq_stats()
        self._i = 0
        self._paused = False
        self._frame_dt = self.hop / sr if sr > 0 and len(freqs) > 0 else 0.08
        
        # Initialize audio player
        self.audio_player = AudioPlayer()
        if audio_file:
            self.audio_player.load_file(audio_file)

    def update_freq_stats(self):
        if len(self.freqs) > 0 and np.any(self.freqs > 0):
            self.hz_min = max(30.0, np.percentile(self.freqs[self.freqs>0], 5))
            self.hz_max = max(self.hz_min+1, np.percentile(self.freqs, 95))
        else:
            self.hz_min = 30.0
            self.hz_max = 6000.0

    def compose(self) -> ComposeResult:
        self.ball = Ball()
        self.label = FreqLabel(id="freq")
        self.filename_label = FileNameLabel(id="filename")
        
        # Set initial filename
        if self.audio_file:
            self.filename_label.update_filename(self.audio_file)
        
        yield Container(
            Static("Frequency Ball", id="title"),
            self.ball,
            self.label,
            self.filename_label,
            Container(
                Button("Upload", variant="primary", id="upload"),
                Button("Play", variant="success", id="play"),
                Button("Pause", variant="warning", id="pause"),
                id="controls"
            ),
            id="wrap"
        )
        yield Footer()

    def on_mount(self):
        if len(self.freqs):
            self.timer = self.set_interval(max(0.03, min(0.15, self._frame_dt)), self._tick)

    def action_toggle_pause(self):
        self._paused = not self._paused

    def action_smaller(self):
        self.ball.diameter = max(9, self.ball.diameter - 2)

    def action_larger(self):
        self.ball.diameter = min(39, self.ball.diameter + 2)

    def action_play_audio(self):
        self.audio_player.play()

    async def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "upload":
            await self.action_upload()
        elif event.button.id == "play":
            self.audio_player.play()
        elif event.button.id == "pause":
            self.audio_player.pause()

    async def action_upload(self) -> None:
        """Show a native file dialog to upload a new audio file."""
        def get_file_path():
            return show_file_dialog()
        
        # Run file dialog in a thread to avoid blocking the UI
        import asyncio
        loop = asyncio.get_event_loop()
        path = await loop.run_in_executor(None, get_file_path)
        
        if path:
            self.load_audio_and_restart(path)

    def load_audio_and_restart(self, path: str):
        try:
            y, sr = load_audio_mono(str(path), target_sr=self.sr)
            self.freqs = dominant_freq_series(y, sr, win=self.win, hop=self.hop, fmin=self.fmin, fmax=self.fmax)
            self.update_freq_stats()
            self._i = 0
            self._paused = False
            self._frame_dt = self.hop / self.sr if self.sr > 0 and len(self.freqs) > 0 else 0.08
            
            # Load audio file for playback
            self.audio_file = path
            self.audio_player.load_file(path)
            self.filename_label.update_filename(path)
            
            if hasattr(self, 'timer'):
                self.timer.stop()
            self.timer = self.set_interval(max(0.03, min(0.15, self._frame_dt)), self._tick)
        except Exception as e:
            self.log(f"Error loading audio file: {e}")

    def _tick(self):
        if self._paused or not len(self.freqs):
            return
        f = float(self.freqs[self._i])
        self.ball.freq_hz = f  # This will automatically update color and shape via watch_freq_hz
        self.label.freq_hz = f

        # Create small multi-directional movement based on frequency
        if self.hz_max > self.hz_min:
            # Normalize frequency to [0, 1] range
            freq_normalized = (f - self.hz_min) / (self.hz_max - self.hz_min)
            # Use frequency to create circular motion with small radius (1-2 characters)
            import math
            angle = freq_normalized * 2 * math.pi  # Full circle based on frequency
            radius = 2  # Small movement radius (about 1mm equivalent in terminal)
            x_offset = int(radius * math.cos(angle))
            y_offset = int(radius * math.sin(angle))
            self.ball.styles.offset = (x_offset, y_offset)
        else:
            self.ball.styles.offset = (0, 0)

        self._i = (self._i + 1) % len(self.freqs)

    async def on_key(self, event: events.Key):
        # quick seek with left/right arrows
        if event.key == "left":
            self._i = (self._i - 10) % max(1, len(self.freqs))
        elif event.key == "right":
            self._i = (self._i + 10) % max(1, len(self.freqs))

def main():
    ap = argparse.ArgumentParser(description="Textual Frequency Ball")
    ap.add_argument("audio", type=str, nargs='?', default=None, help="Path to audio file (mp3, wav, m4a, ogg, ...)")
    ap.add_argument("--sr", type=int, default=22050, help="Target sample rate (default 22050)")
    ap.add_argument("--win", type=int, default=4096, help="FFT window size")
    ap.add_argument("--hop", type=int, default=2048, help="Hop length")
    ap.add_argument("--fmin", type=float, default=30.0, help="Min Hz to consider")
    ap.add_argument("--fmax", type=float, default=6000.0, help="Max Hz to consider")
    args = ap.parse_args()

    freqs = np.array([])
    audio_file = None
    if args.audio:
        path = Path(args.audio)
        if not path.exists():
            print(f"File not found: {path}", file=sys.stderr)
        else:
            try:
                y, sr = load_audio_mono(str(path), target_sr=args.sr)
                freqs = dominant_freq_series(y, sr, win=args.win, hop=args.hop, fmin=args.fmin, fmax=args.fmax)
                audio_file = str(path)
            except Exception as e:
                print(f"Error loading audio: {e}", file=sys.stderr)
    
    FreqBallApp(freqs=freqs, sr=args.sr, hop=args.hop, win=args.win, fmin=args.fmin, fmax=args.fmax, audio_file=audio_file).run()

if __name__ == "__main__":
    main()
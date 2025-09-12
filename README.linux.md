# Linux Build Instructions

This document provides instructions for building the SONAR project on Linux systems.

## Prerequisites

### Required Packages

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential gcc make git
```

**CentOS/RHEL/Fedora:**
```bash
# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install gcc make git

# Fedora
sudo dnf groupinstall "Development Tools"
sudo dnf install gcc make git
```

**Arch Linux:**
```bash
sudo pacman -S base-devel gcc make git
```

### Optional Packages (for advanced features)

```bash
# For memory debugging
sudo apt-get install valgrind

# For code coverage
sudo apt-get install gcov lcov

# Alternative compilers
sudo apt-get install clang
```

## Building

### Quick Start

```bash
# Clone the repository (if not already done)
git clone <repository-url>
cd mojibake

# Build using Linux Makefile
make -f Makefile.linux

# Or copy it as the default Makefile
cp Makefile.linux Makefile
make
```

### Build Targets

| Target | Description |
|--------|-------------|
| `all` | Build release version (default) |
| `debug` | Build debug version with sanitizers |
| `static` | Build static version (no dynamic linking) |
| `clean` | Remove object files and executables |
| `clean-all` | Remove all build artifacts |
| `install` | Install to system (requires sudo) |
| `uninstall` | Remove from system (requires sudo) |
| `package` | Create distribution tar.gz |
| `test` | Run all tests |
| `valgrind-check` | Memory check with valgrind |
| `coverage` | Generate code coverage report |
| `sysinfo` | Show system information |

### Build Examples

```bash
# Standard release build
make -f Makefile.linux

# Debug build with sanitizers
make -f Makefile.linux debug

# Static build (no dependencies)
make -f Makefile.linux static

# Use clang instead of gcc
make -f Makefile.linux CC=clang

# Custom optimization
make -f Makefile.linux CFLAGS="-O3 -march=native"

# Install to custom location
make -f Makefile.linux PREFIX=/opt/mojibake install
```

## Installation

### System-wide Installation

```bash
# Build and install (default: /usr/local/bin)
sudo make -f Makefile.linux install

# Install to custom location
sudo make -f Makefile.linux PREFIX=/opt/local install
```

### User Installation

```bash
# Install to user's home directory
make -f Makefile.linux PREFIX=$HOME/.local install

# Make sure ~/.local/bin is in your PATH
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## Testing

### Basic Tests

```bash
# Run all tests
make -f Makefile.linux test

# Individual tests
make -f Makefile.linux test-hex
make -f Makefile.linux test-sonar
make -f Makefile.linux test-dsonar
```

### Memory Testing

```bash
# Requires valgrind
make -f Makefile.linux valgrind-check
```

### Code Coverage

```bash
# Generate coverage report
make -f Makefile.linux coverage

# View coverage files
ls *.gcov
```

## Advanced Features

### Cross-Platform Support

The Linux Makefile automatically detects:
- Linux (adds `-D_GNU_SOURCE`, `-lpthread`, `-ldl`)
- macOS (adds `-D_DARWIN_C_SOURCE`)

### Compiler Options

```bash
# Use different compilers
make CC=gcc        # GNU Compiler Collection
make CC=clang      # LLVM Clang
make CC=icc        # Intel C Compiler
```

### Debug Features

The debug build includes:
- Debug symbols (`-g`)
- Address Sanitizer (`-fsanitize=address`)
- Undefined Behavior Sanitizer (`-fsanitize=undefined`)
- Debug preprocessor flag (`-DDEBUG`)

### Version Information

The build system automatically includes:
- Version number
- Git commit hash
- Build date/time

## Troubleshooting

### Common Issues

**Build fails with "command not found":**
```bash
# Install build tools
sudo apt-get install build-essential
```

**Permission denied during install:**
```bash
# Use sudo for system installation
sudo make -f Makefile.linux install
```

**Missing dependencies:**
```bash
# Check system info
make -f Makefile.linux sysinfo
```

**Valgrind not found:**
```bash
# Install valgrind
sudo apt-get install valgrind
```

### Getting Help

```bash
# Show all available targets
make -f Makefile.linux help

# Show system information
make -f Makefile.linux sysinfo
```

## Performance Optimization

### Compiler Optimizations

```bash
# Maximum optimization
make CFLAGS="-O3 -march=native -mtune=native"

# Size optimization
make CFLAGS="-Os -DNDEBUG"

# Link-time optimization (if supported)
make CFLAGS="-O3 -flto" LDFLAGS="-flto"
```

### Parallel Building

```bash
# Use multiple CPU cores
make -j$(nproc) -f Makefile.linux

# Specific number of jobs
make -j4 -f Makefile.linux
```

## Distribution

### Creating Packages

```bash
# Create source distribution
make -f Makefile.linux package

# This creates: mojibake-sonar-1.0.0.tar.gz
```

### Binary Distribution

```bash
# Static build for distribution
make -f Makefile.linux static

# The static binary has no external dependencies
file build/bin/mojibake_sonar_static
```
# MSDFGEN Binary Builds

This repository contains automated builds of [MSDFGEN](https://github.com/Chlumsky/msdfgen) with Skia geometry preprocessing enabled as dynamic libraries for multiple platforms.

## What is MSDFGEN?

MSDFGEN is a utility for generating signed distance fields from vector shapes and font glyphs. It produces multi-channel signed distance fields that can reproduce sharp corners almost perfectly by utilizing all three color channels.

## Features

- **Core Library**: Multi-channel signed distance field generation
- **Extensions**: Font loading (FreeType), SVG support (TinyXML2), PNG export (libpng)
- **Skia Integration**: Advanced geometry preprocessing for better edge detection
- **Cross-Platform**: Windows, macOS, and Linux support
- **Dynamic Libraries**: Shared libraries (.dll, .so, .dylib) for easy integration

## Build Status

| Platform | Architecture | Status |
|----------|-------------|--------|
| Windows  | x64         | ![Windows](https://github.com/your-username/msdfgen-binaries/workflows/Windows/badge.svg) |
| macOS    | x64         | ![macOS](https://github.com/your-username/msdfgen-binaries/workflows/macOS/badge.svg) |
| Linux    | x64         | ![Linux](https://github.com/your-username/msdfgen-binaries/workflows/Linux/badge.svg) |

## Downloads

Pre-built binaries are available in the [Releases](https://github.com/your-username/msdfgen-binaries/releases) section.

### File Naming Convention

- `msdfgen-core-{version}-{platform}.{ext}` - Core library only
- `msdfgen-ext-{version}-{platform}.{ext}` - Extensions library with dependencies
- `msdfgen-full-{version}-{platform}.{ext}` - Combined core + extensions

Where:
- `{version}`: MSDFGEN version (e.g., 1.13.0)
- `{platform}`: Platform identifier (windows-x64, macos-x64, linux-x64)
- `{ext}`: Library extension (.dll, .so, .dylib)

## Dependencies

This build includes the following dependencies:

- **FreeType**: Font loading and rendering
- **TinyXML2**: SVG file parsing
- **libpng**: PNG image output
- **Skia**: Advanced geometry preprocessing

## Usage

### CMake Integration

```cmake
find_package(msdfgen REQUIRED)

# Link against the full library (core + extensions)
target_link_libraries(your_target PRIVATE msdfgen::msdfgen)

# Or link individually
target_link_libraries(your_target PRIVATE 
    msdfgen::msdfgen-core 
    msdfgen::msdfgen-ext
)
```

### Basic Example

```cpp
#include <msdfgen.h>
#include <msdfgen-ext.h>

using namespace msdfgen;

int main() {
    if (FreetypeHandle *ft = initializeFreetype()) {
        if (FontHandle *font = loadFont(ft, "path/to/font.ttf")) {
            Shape shape;
            if (loadGlyph(shape, font, 'A')) {
                shape.normalize();
                edgeColoringSimple(shape, 3.0);
                
                Bitmap<float, 3> msdf(32, 32);
                SDFTransformation t(Projection(32.0, Vector2(0.125, 0.125)), Range(0.125));
                generateMSDF(msdf, shape, t);
                savePng(msdf, "output.png");
            }
            destroyFont(font);
        }
        deinitializeFreetype(ft);
    }
    return 0;
}
```

## Building Locally

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- vcpkg package manager

### Build Steps

```bash
# Clone the repository
git clone https://github.com/your-username/msdfgen-binaries.git
cd msdfgen-binaries

# Set up vcpkg (if not already installed)
export VCPKG_ROOT=/path/to/vcpkg

# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## License

This project follows the same license as MSDFGEN. See the [original repository](https://github.com/Chlumsky/msdfgen) for license information.

## Contributing

This repository is primarily for automated builds. For contributing to MSDFGEN itself, please visit the [upstream repository](https://github.com/Chlumsky/msdfgen).

## Issues

If you encounter issues with the pre-built binaries, please:
1. Check if the issue is reproduced with the upstream MSDFGEN build
2. If it's specific to these binaries, open an issue in this repository
3. Include your platform, architecture, and specific error details
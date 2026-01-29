# MSDFGEN Binary Builds

Pre-built dynamic libraries for [msdfgen](https://github.com/Chlumsky/msdfgen) with Skia geometry preprocessing support.

## Features

- **Multi-platform**: macOS (arm64, x64), Windows (x64), Linux (x64)
- **Skia integration**: Geometry preprocessing for handling overlapping contours and self-intersections
- **C API wrapper**: Simple C interface for FFI/P/Invoke from other languages
- **Shared libraries**: Ready-to-use dynamic libraries (.dylib, .dll, .so)

## Libraries

| Library | Description |
|---------|-------------|
| `msdfgen-core` | Core MSDF generation functionality |
| `msdfgen-ext` | Extensions (font loading, SVG, PNG support, Skia preprocessing) |
| `msdfgen_c` | C API wrapper for FFI/P/Invoke |

## Downloads

Pre-built binaries are available in the [Releases](../../releases) section.

## C API

The `msdfgen_c` library provides a simple C interface for use from other languages:

```c
#include "msdfgen_c_api.h"

// Create a shape
MsdfgenShape shape = msdfgen_shape_create();

// Add contours and edges
int contour = msdfgen_shape_add_contour(shape);
msdfgen_contour_add_quadratic(shape, contour, x0, y0, x1, y1, x2, y2);

// Preprocess with Skia (handles overlapping contours)
msdfgen_resolve_shape_geometry(shape);

// Normalize and color edges
msdfgen_shape_normalize(shape);
msdfgen_edge_coloring_simple(shape, 3.0, 0);

// Generate MTSDF
MsdfgenResult result = msdfgen_generate_mtsdf(shape, width, height,
    scaleX, scaleY, translateX, translateY, rangePixels);

// Use result.pixels (RGBA float data)
// ...

// Cleanup
msdfgen_free_pixels(result.pixels);
msdfgen_shape_destroy(shape);
```

## C++ Usage

```cpp
#include <msdfgen.h>
#include <msdfgen-ext.h>

using namespace msdfgen;

int main() {
    if (FreetypeHandle *ft = initializeFreetype()) {
        if (FontHandle *font = loadFont(ft, "path/to/font.ttf")) {
            Shape shape;
            if (loadGlyph(shape, font, 'A')) {
                // Preprocess with Skia (if available)
                resolveShapeGeometry(shape);

                shape.normalize();
                edgeColoringSimple(shape, 3.0);

                Bitmap<float, 4> mtsdf(32, 32);
                SDFTransformation t(Projection(32.0, Vector2(0.125, 0.125)), Range(4.0));
                generateMTSDF(mtsdf, shape, t);
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

- CMake 3.15+
- vcpkg
- C++17 compiler

### Build Steps

```bash
# Clone with submodules
git clone --recursive https://github.com/user/msdfgen-binaries.git
cd msdfgen-binaries

# Configure with vcpkg
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DMSDFGEN_USE_SKIA=ON \
    -DBUILD_SHARED_LIBS=ON

# Build
cmake --build build --config Release
```

Libraries are output to `build/lib/` (or `build/bin/` for Windows DLLs).

## Dependencies

Built with vcpkg:
- freetype - Font loading
- tinyxml2 - SVG parsing
- libpng - PNG output
- skia - Geometry preprocessing

## License

msdfgen is licensed under the MIT license. See the [msdfgen repository](https://github.com/Chlumsky/msdfgen) for details.

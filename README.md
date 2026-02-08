# msdfgen-binaries

Pre-built dynamic libraries for [msdfgen](https://github.com/Chlumsky/msdfgen) v1.13+ with optional FreeType, Skia, and other extensions.

## Downloads

| Release | Description |
|---------|-------------|
| [v1.13](../../releases/tag/v1.13) | Latest stable release |
| [dev-latest](../../releases/tag/dev-latest) | Development build (updated daily) |

## Platforms

- macOS arm64 (Apple Silicon)
- macOS x64 (Intel)
- Windows x64
- Linux x64

## Libraries

| Library | Dependencies | Description |
|---------|--------------|-------------|
| `msdfgen-core` | None | Core MSDF generation (always built) |
| `msdfgen-ext` | FreeType, TinyXML2, libpng | Extensions for font loading, SVG import, PNG export |
| `msdfgen-c` | msdfgen-core (+ msdfgen-ext if enabled) | C API wrapper for FFI/P/Invoke |

### Optional Features

| Feature | CMake Option | vcpkg Feature | Description |
|---------|--------------|---------------|-------------|
| FreeType/Extensions | `MSDFGEN_USE_FREETYPE` | `extensions` | Font loading, SVG import, PNG export |
| Skia | `MSDFGEN_USE_SKIA` | `geometry-preprocessing` | Geometry preprocessing for overlapping contours |

## Build Configurations

### Core Only (No Dependencies)

Build msdfgen-core only - no external dependencies:

```bash
cmake -B build \
    -DMSDFGEN_USE_FREETYPE=OFF \
    -DMSDFGEN_USE_SKIA=OFF

cmake --build build --config Release
```

### With Extensions (FreeType, PNG, SVG)

Build with FreeType for font loading, libpng for PNG export, TinyXML2 for SVG:

```bash
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DMSDFGEN_USE_FREETYPE=ON \
    -DMSDFGEN_USE_SKIA=OFF

cmake --build build --config Release
```

### Full Build (Extensions + Skia)

Full build with all features including Skia geometry preprocessing:

```bash
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DMSDFGEN_USE_FREETYPE=ON \
    -DMSDFGEN_USE_SKIA=ON

cmake --build build --config Release
```

## C API

The `msdfgen-c` library provides a simple C interface for use from other languages (C#, Rust, etc.). It works with both core-only and full builds.

### Core API (Always Available)

```c
#include "msdfgen_c_api.h"

// Create shape manually
MsdfgenShape shape = msdfgen_shape_create();
int contour = msdfgen_shape_add_contour(shape);
msdfgen_contour_add_quadratic(shape, contour, x0, y0, x1, y1, x2, y2);

// Generate MTSDF
msdfgen_shape_normalize(shape);
msdfgen_edge_coloring_simple(shape, 3.0, 0);
MsdfgenBitmap result = msdfgen_generate_mtsdf(shape, width, height,
    scaleX, scaleY, translateX, translateY, rangePixels);

// Cleanup
msdfgen_bitmap_free(&result);
msdfgen_shape_destroy(shape);
```

### Extension API (When MSDFGEN_USE_FREETYPE=ON)

```c
// Load font and generate MSDF for a glyph
MsdfgenFreetype freetype = msdfgen_freetype_init();
MsdfgenFont font = msdfgen_font_load(freetype, "font.ttf");
MsdfgenShape shape = msdfgen_shape_create();

msdfgen_font_load_glyph(shape, font, 'A', MSDFGEN_FONT_SCALING_EM_NORMALIZED, NULL);
msdfgen_shape_normalize(shape);
msdfgen_edge_coloring_simple(shape, 3.0, 0);

MsdfgenBitmap result = msdfgen_generate_msdf(shape, width, height,
    scaleX, scaleY, translateX, translateY, rangePixels);

// Cleanup
msdfgen_bitmap_free(&result);
msdfgen_shape_destroy(shape);
msdfgen_font_destroy(font);
msdfgen_freetype_deinit(freetype);
```

### Runtime Feature Detection

```c
// Check if extensions are available
if (msdfgen_has_extension_support()) {
    // Can use font loading, SVG import, PNG export
}

// Check if Skia is available
if (msdfgen_has_skia_support()) {
    // Can use msdfgen_resolve_shape_geometry()
}
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `MSDFGEN_USE_FREETYPE` | `ON` | Build with FreeType, libpng, TinyXML2 (required for msdfgen-ext) |
| `MSDFGEN_USE_SKIA` | `OFF` | Build with Skia geometry preprocessing |
| `BUILD_SHARED_LIBS` | `ON` | Build shared libraries (forced ON) |
| `MSDFGEN_BUILD_STANDALONE` | `OFF` | Build standalone msdfgen executable |

## vcpkg Features

The `vcpkg.json` supports these feature flags:

```bash
# Install only core dependencies (none)
vcpkg install msdfgen-binaries

# Install with extensions (FreeType, TinyXML2, libpng)
vcpkg install msdfgen-binaries[extensions]

# Install with Skia geometry preprocessing
vcpkg install msdfgen-binaries[extensions,geometry-preprocessing]
```

## Creating a Release

To create a new release matching an msdfgen version:

1. Push a tag: `git tag v1.13 && git push origin v1.13`
2. Or use workflow dispatch with `create_release: true`

## License

[MIT License](https://github.com/Chlumsky/msdfgen/blob/master/LICENSE.txt) - same as msdfgen.

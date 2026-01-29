# msdfgen-binaries

Pre-built dynamic libraries for [msdfgen](https://github.com/Chlumsky/msdfgen) v1.13+ with Skia geometry preprocessing.

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

| Library | Description |
|---------|-------------|
| `msdfgen-core` | Core MSDF generation |
| `msdfgen-ext` | Extensions: FreeType, TinyXML2, libpng, **Skia** |
| `msdfgen_c` | C API wrapper for FFI/P/Invoke |

## C API

The `msdfgen_c` library provides a simple C interface for use from other languages (C#, Rust, etc.):

```c
#include "msdfgen_c_api.h"

// Create shape
MsdfgenShape shape = msdfgen_shape_create();
int contour = msdfgen_shape_add_contour(shape);
msdfgen_contour_add_quadratic(shape, contour, x0, y0, x1, y1, x2, y2);

// Preprocess with Skia (handles overlapping contours)
msdfgen_resolve_shape_geometry(shape);

// Generate MTSDF
msdfgen_shape_normalize(shape);
msdfgen_edge_coloring_simple(shape, 3.0, 0);
MsdfgenResult result = msdfgen_generate_mtsdf(shape, width, height,
    scaleX, scaleY, translateX, translateY, rangePixels);

// result.pixels contains RGBA float data
msdfgen_free_pixels(result.pixels);
msdfgen_shape_destroy(shape);
```

## Building

```bash
git clone --recursive https://github.com/user/msdfgen-binaries.git
cd msdfgen-binaries

cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DMSDFGEN_USE_SKIA=ON

cmake --build build --config Release
```

## Creating a Release

To create a new release matching an msdfgen version:

1. Push a tag: `git tag v1.13 && git push origin v1.13`
2. Or use workflow dispatch with `create_release: true`

## License

[MIT License](https://github.com/Chlumsky/msdfgen/blob/master/LICENSE.txt) - same as msdfgen.

/*
 * C API wrapper for msdfgen library
 * Provides a simple C interface for use with P/Invoke from C#
 */

#ifndef MSDFGEN_C_API_H
#define MSDFGEN_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #ifdef MSDFGEN_C_API_EXPORTS
        #define MSDFGEN_C_API __declspec(dllexport)
    #else
        #define MSDFGEN_C_API __declspec(dllimport)
    #endif
#else
    #define MSDFGEN_C_API __attribute__((visibility("default")))
#endif

/* Opaque handle to a shape */
typedef void* MsdfgenShape;

/* Result structure for MTSDF generation */
typedef struct {
    float* pixels;      /* RGBA float pixels (4 floats per pixel) */
    int width;
    int height;
    int success;
} MsdfgenResult;

/* Bounds structure */
typedef struct {
    double left;
    double bottom;
    double right;
    double top;
} MsdfgenBounds;

/*
 * Create a new empty shape.
 * Returns: Opaque handle to the shape, or NULL on failure.
 */
MSDFGEN_C_API MsdfgenShape msdfgen_shape_create(void);

/*
 * Destroy a shape and free its resources.
 */
MSDFGEN_C_API void msdfgen_shape_destroy(MsdfgenShape shape);

/*
 * Add a new contour to the shape and return its index.
 */
MSDFGEN_C_API int msdfgen_shape_add_contour(MsdfgenShape shape);

/*
 * Add a quadratic bezier curve to a contour.
 * contourIndex: Index of the contour (from msdfgen_shape_add_contour)
 * x0, y0: Start point
 * x1, y1: Control point
 * x2, y2: End point
 */
MSDFGEN_C_API void msdfgen_contour_add_quadratic(
    MsdfgenShape shape,
    int contourIndex,
    double x0, double y0,
    double x1, double y1,
    double x2, double y2
);

/*
 * Add a linear segment to a contour.
 * contourIndex: Index of the contour
 * x0, y0: Start point
 * x1, y1: End point
 */
MSDFGEN_C_API void msdfgen_contour_add_linear(
    MsdfgenShape shape,
    int contourIndex,
    double x0, double y0,
    double x1, double y1
);

/*
 * Add a cubic bezier curve to a contour.
 * contourIndex: Index of the contour
 * x0, y0: Start point
 * x1, y1: Control point 1
 * x2, y2: Control point 2
 * x3, y3: End point
 */
MSDFGEN_C_API void msdfgen_contour_add_cubic(
    MsdfgenShape shape,
    int contourIndex,
    double x0, double y0,
    double x1, double y1,
    double x2, double y2,
    double x3, double y3
);

/*
 * Normalize the shape (required before generation).
 */
MSDFGEN_C_API void msdfgen_shape_normalize(MsdfgenShape shape);

/*
 * Validate the shape.
 * Returns: 1 if valid, 0 if invalid.
 */
MSDFGEN_C_API int msdfgen_shape_validate(MsdfgenShape shape);

/*
 * Get the bounds of the shape.
 */
MSDFGEN_C_API MsdfgenBounds msdfgen_shape_get_bounds(MsdfgenShape shape);

/*
 * Orient contours to conform to non-zero winding rule.
 */
MSDFGEN_C_API void msdfgen_shape_orient_contours(MsdfgenShape shape);

/*
 * Apply simple edge coloring for MSDF generation.
 * angleThreshold: Maximum angle in radians to be considered a corner (e.g., 3.0 for ~172 degrees)
 * seed: Random seed for coloring (0 for default)
 */
MSDFGEN_C_API void msdfgen_edge_coloring_simple(MsdfgenShape shape, double angleThreshold, unsigned long long seed);

/*
 * Resolve shape geometry using Skia (handle overlapping contours).
 * Only available if built with MSDFGEN_USE_SKIA.
 * Returns: 1 if successful, 0 if failed or Skia not available.
 */
MSDFGEN_C_API int msdfgen_resolve_shape_geometry(MsdfgenShape shape);

/*
 * Generate MTSDF (Multi-channel SDF with True distance in alpha).
 *
 * shape: The shape to render
 * width, height: Output bitmap dimensions in pixels
 * scaleX, scaleY: Scale factors (shape units to pixels)
 * translateX, translateY: Translation offset (in shape units)
 * rangePixels: Distance field range in pixels
 *
 * Returns: MsdfgenResult with pixel data. Caller must free pixels with msdfgen_free_pixels().
 */
MSDFGEN_C_API MsdfgenResult msdfgen_generate_mtsdf(
    MsdfgenShape shape,
    int width,
    int height,
    double scaleX,
    double scaleY,
    double translateX,
    double translateY,
    double rangePixels
);

/*
 * Generate MSDF (Multi-channel SDF without alpha).
 */
MSDFGEN_C_API MsdfgenResult msdfgen_generate_msdf(
    MsdfgenShape shape,
    int width,
    int height,
    double scaleX,
    double scaleY,
    double translateX,
    double translateY,
    double rangePixels
);

/*
 * Free pixel data returned by msdfgen_generate_mtsdf/msdfgen_generate_msdf.
 */
MSDFGEN_C_API void msdfgen_free_pixels(float* pixels);

/*
 * Convert float pixels to RGBA bytes.
 * floatPixels: Input float pixels (4 floats per pixel for MTSDF, 3 for MSDF)
 * bytePixels: Output byte buffer (must be pre-allocated, 4 bytes per pixel)
 * width, height: Dimensions
 * channels: Number of input channels (3 for MSDF, 4 for MTSDF)
 * edgeValue: Byte value at shape edge (typically 128)
 * rangePixels: The range used during generation (for proper scaling)
 */
MSDFGEN_C_API void msdfgen_float_to_bytes(
    const float* floatPixels,
    unsigned char* bytePixels,
    int width,
    int height,
    int channels,
    unsigned char edgeValue,
    double rangePixels
);

#ifdef __cplusplus
}
#endif

#endif /* MSDFGEN_C_API_H */

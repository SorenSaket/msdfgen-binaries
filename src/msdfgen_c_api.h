/*
 * C API wrapper for msdfgen library
 * Provides a complete C interface for use with FFI/P/Invoke
 */

#ifndef MSDFGEN_C_API_H
#define MSDFGEN_C_API_H

#include <stddef.h>

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

/* ============================================================================
 * Opaque handles
 * ============================================================================ */

typedef void* MsdfgenShape;
typedef void* MsdfgenFreetype;
typedef void* MsdfgenFont;

/* ============================================================================
 * Enumerations
 * ============================================================================ */

typedef enum {
    MSDFGEN_FILL_NONZERO = 0,
    MSDFGEN_FILL_ODD = 1,
    MSDFGEN_FILL_POSITIVE = 2,
    MSDFGEN_FILL_NEGATIVE = 3
} MsdfgenFillRule;

typedef enum {
    MSDFGEN_Y_BOTTOM_UP = 0,
    MSDFGEN_Y_TOP_DOWN = 1
} MsdfgenYAxisOrientation;

typedef enum {
    MSDFGEN_ERROR_CORRECTION_DISABLED = 0,
    MSDFGEN_ERROR_CORRECTION_INDISCRIMINATE = 1,
    MSDFGEN_ERROR_CORRECTION_EDGE_PRIORITY = 2,
    MSDFGEN_ERROR_CORRECTION_EDGE_ONLY = 3
} MsdfgenErrorCorrectionMode;

typedef enum {
    MSDFGEN_DISTANCE_CHECK_NONE = 0,
    MSDFGEN_DISTANCE_CHECK_AT_EDGE = 1,
    MSDFGEN_DISTANCE_CHECK_ALWAYS = 2
} MsdfgenDistanceCheckMode;

typedef enum {
    MSDFGEN_FONT_SCALING_NONE = 0,
    MSDFGEN_FONT_SCALING_EM_NORMALIZED = 1,
    MSDFGEN_FONT_SCALING_LEGACY = 2
} MsdfgenFontCoordinateScaling;

/* ============================================================================
 * Structures
 * ============================================================================ */

typedef struct {
    double left;
    double bottom;
    double right;
    double top;
} MsdfgenBounds;

typedef struct {
    float* pixels;      /* Float pixels (channels per pixel depends on type) */
    int width;
    int height;
    int channels;       /* 1=SDF, 3=MSDF, 4=MTSDF */
    int success;
} MsdfgenBitmap;

typedef struct {
    MsdfgenErrorCorrectionMode mode;
    MsdfgenDistanceCheckMode distanceCheckMode;
    double minDeviationRatio;
    double minImproveRatio;
} MsdfgenErrorCorrectionConfig;

typedef struct {
    int overlapSupport;
    MsdfgenErrorCorrectionConfig errorCorrection;
} MsdfgenGeneratorConfig;

typedef struct {
    double emSize;
    double ascenderY;
    double descenderY;
    double lineHeight;
    double underlineY;
    double underlineThickness;
} MsdfgenFontMetrics;

typedef struct {
    const char* name;
    double minValue;
    double maxValue;
    double defaultValue;
} MsdfgenFontVariationAxis;

/* ============================================================================
 * Shape creation and destruction
 * ============================================================================ */

MSDFGEN_C_API MsdfgenShape msdfgen_shape_create(void);
MSDFGEN_C_API void msdfgen_shape_destroy(MsdfgenShape shape);

/* ============================================================================
 * Contour and edge operations
 * ============================================================================ */

MSDFGEN_C_API int msdfgen_shape_add_contour(MsdfgenShape shape);

MSDFGEN_C_API void msdfgen_contour_add_linear(
    MsdfgenShape shape,
    int contourIndex,
    double x0, double y0,
    double x1, double y1
);

MSDFGEN_C_API void msdfgen_contour_add_quadratic(
    MsdfgenShape shape,
    int contourIndex,
    double x0, double y0,
    double x1, double y1,
    double x2, double y2
);

MSDFGEN_C_API void msdfgen_contour_add_cubic(
    MsdfgenShape shape,
    int contourIndex,
    double x0, double y0,
    double x1, double y1,
    double x2, double y2,
    double x3, double y3
);

/* ============================================================================
 * Shape operations
 * ============================================================================ */

MSDFGEN_C_API void msdfgen_shape_normalize(MsdfgenShape shape);
MSDFGEN_C_API int msdfgen_shape_validate(MsdfgenShape shape);
MSDFGEN_C_API MsdfgenBounds msdfgen_shape_get_bounds(MsdfgenShape shape);
MSDFGEN_C_API MsdfgenBounds msdfgen_shape_get_bounds_ex(MsdfgenShape shape, double border, double miterLimit, int polarity);
MSDFGEN_C_API void msdfgen_shape_orient_contours(MsdfgenShape shape);
MSDFGEN_C_API int msdfgen_shape_edge_count(MsdfgenShape shape);
MSDFGEN_C_API MsdfgenYAxisOrientation msdfgen_shape_get_y_axis_orientation(MsdfgenShape shape);
MSDFGEN_C_API void msdfgen_shape_set_y_axis_orientation(MsdfgenShape shape, MsdfgenYAxisOrientation orientation);
MSDFGEN_C_API int msdfgen_shape_contour_count(MsdfgenShape shape);

/* ============================================================================
 * Edge coloring
 * ============================================================================ */

MSDFGEN_C_API void msdfgen_edge_coloring_simple(MsdfgenShape shape, double angleThreshold, unsigned long long seed);
MSDFGEN_C_API void msdfgen_edge_coloring_ink_trap(MsdfgenShape shape, double angleThreshold, unsigned long long seed);
MSDFGEN_C_API void msdfgen_edge_coloring_by_distance(MsdfgenShape shape, double angleThreshold, unsigned long long seed);

/* ============================================================================
 * Configuration helpers
 * ============================================================================ */

MSDFGEN_C_API MsdfgenErrorCorrectionConfig msdfgen_error_correction_config_default(void);
MSDFGEN_C_API MsdfgenGeneratorConfig msdfgen_generator_config_default(void);

/* ============================================================================
 * SDF Generation
 * ============================================================================ */

MSDFGEN_C_API MsdfgenBitmap msdfgen_generate_sdf(
    MsdfgenShape shape,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels
);

MSDFGEN_C_API MsdfgenBitmap msdfgen_generate_psdf(
    MsdfgenShape shape,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels
);

MSDFGEN_C_API MsdfgenBitmap msdfgen_generate_msdf(
    MsdfgenShape shape,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels
);

MSDFGEN_C_API MsdfgenBitmap msdfgen_generate_mtsdf(
    MsdfgenShape shape,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels
);

/* Extended versions with full configuration */
MSDFGEN_C_API MsdfgenBitmap msdfgen_generate_sdf_ex(
    MsdfgenShape shape,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels,
    int overlapSupport
);

MSDFGEN_C_API MsdfgenBitmap msdfgen_generate_psdf_ex(
    MsdfgenShape shape,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels,
    int overlapSupport
);

MSDFGEN_C_API MsdfgenBitmap msdfgen_generate_msdf_ex(
    MsdfgenShape shape,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels,
    const MsdfgenGeneratorConfig* config
);

MSDFGEN_C_API MsdfgenBitmap msdfgen_generate_mtsdf_ex(
    MsdfgenShape shape,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels,
    const MsdfgenGeneratorConfig* config
);

/* ============================================================================
 * Bitmap operations
 * ============================================================================ */

MSDFGEN_C_API void msdfgen_bitmap_free(MsdfgenBitmap* bitmap);

MSDFGEN_C_API void msdfgen_bitmap_to_bytes(
    const MsdfgenBitmap* bitmap,
    unsigned char* output,
    unsigned char edgeValue
);

/* ============================================================================
 * Geometry resolution (requires Skia)
 * ============================================================================ */

MSDFGEN_C_API int msdfgen_resolve_shape_geometry(MsdfgenShape shape);

/* ============================================================================
 * SVG import
 * ============================================================================ */

MSDFGEN_C_API int msdfgen_shape_load_from_svg_path(MsdfgenShape shape, const char* pathDef, double endpointSnapRange);
MSDFGEN_C_API int msdfgen_shape_load_from_svg_file(MsdfgenShape shape, const char* filename, int pathIndex);
MSDFGEN_C_API int msdfgen_shape_load_from_svg_file_ex(MsdfgenShape shape, MsdfgenBounds* viewBox, const char* filename);

/* ============================================================================
 * FreeType font operations
 * ============================================================================ */

MSDFGEN_C_API MsdfgenFreetype msdfgen_freetype_init(void);
MSDFGEN_C_API void msdfgen_freetype_deinit(MsdfgenFreetype freetype);

MSDFGEN_C_API MsdfgenFont msdfgen_font_load(MsdfgenFreetype freetype, const char* filename);
MSDFGEN_C_API MsdfgenFont msdfgen_font_load_data(MsdfgenFreetype freetype, const unsigned char* data, int length);
MSDFGEN_C_API void msdfgen_font_destroy(MsdfgenFont font);

MSDFGEN_C_API int msdfgen_font_get_metrics(MsdfgenFont font, MsdfgenFontMetrics* metrics, MsdfgenFontCoordinateScaling scaling);
MSDFGEN_C_API int msdfgen_font_get_whitespace_width(MsdfgenFont font, double* spaceAdvance, double* tabAdvance, MsdfgenFontCoordinateScaling scaling);
MSDFGEN_C_API int msdfgen_font_get_glyph_count(MsdfgenFont font, unsigned int* count);
MSDFGEN_C_API int msdfgen_font_get_glyph_index(MsdfgenFont font, unsigned int unicode, unsigned int* glyphIndex);

MSDFGEN_C_API int msdfgen_font_load_glyph(
    MsdfgenShape shape,
    MsdfgenFont font,
    unsigned int unicode,
    MsdfgenFontCoordinateScaling scaling,
    double* outAdvance
);

MSDFGEN_C_API int msdfgen_font_load_glyph_by_index(
    MsdfgenShape shape,
    MsdfgenFont font,
    unsigned int glyphIndex,
    MsdfgenFontCoordinateScaling scaling,
    double* outAdvance
);

MSDFGEN_C_API int msdfgen_font_get_kerning(
    MsdfgenFont font,
    unsigned int unicode0,
    unsigned int unicode1,
    MsdfgenFontCoordinateScaling scaling,
    double* kerning
);

MSDFGEN_C_API int msdfgen_font_get_kerning_by_index(
    MsdfgenFont font,
    unsigned int glyphIndex0,
    unsigned int glyphIndex1,
    MsdfgenFontCoordinateScaling scaling,
    double* kerning
);

/* Variable font support */
MSDFGEN_C_API int msdfgen_font_set_variation_axis(
    MsdfgenFreetype freetype,
    MsdfgenFont font,
    const char* name,
    double coordinate
);

MSDFGEN_C_API int msdfgen_font_get_variation_axis_count(
    MsdfgenFreetype freetype,
    MsdfgenFont font,
    int* count
);

MSDFGEN_C_API int msdfgen_font_get_variation_axis(
    MsdfgenFreetype freetype,
    MsdfgenFont font,
    int index,
    MsdfgenFontVariationAxis* axis
);

/* ============================================================================
 * Utility functions
 * ============================================================================ */

/* Calculate auto-frame parameters for a shape */
MSDFGEN_C_API void msdfgen_auto_frame(
    MsdfgenShape shape,
    int width, int height,
    double rangePixels,
    double* outScaleX, double* outScaleY,
    double* outTranslateX, double* outTranslateY
);

/* Get library version */
MSDFGEN_C_API const char* msdfgen_get_version(void);

/* Check if Skia support is available */
MSDFGEN_C_API int msdfgen_has_skia_support(void);

#ifdef __cplusplus
}
#endif

#endif /* MSDFGEN_C_API_H */

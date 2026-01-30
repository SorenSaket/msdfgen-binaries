/*
 * C API wrapper implementation for msdfgen library
 */

#include "msdfgen_c_api.h"
#include "msdfgen.h"
#include "msdfgen-ext.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

using namespace msdfgen;

/* ============================================================================
 * Internal helpers
 * ============================================================================ */

static FontCoordinateScaling toFontScaling(MsdfgenFontCoordinateScaling scaling) {
    switch (scaling) {
        case MSDFGEN_FONT_SCALING_NONE: return FONT_SCALING_NONE;
        case MSDFGEN_FONT_SCALING_EM_NORMALIZED: return FONT_SCALING_EM_NORMALIZED;
        case MSDFGEN_FONT_SCALING_LEGACY:
        default: return FONT_SCALING_LEGACY;
    }
}

static ErrorCorrectionConfig toErrorCorrectionConfig(const MsdfgenErrorCorrectionConfig& config) {
    ErrorCorrectionConfig::Mode mode;
    switch (config.mode) {
        case MSDFGEN_ERROR_CORRECTION_DISABLED: mode = ErrorCorrectionConfig::DISABLED; break;
        case MSDFGEN_ERROR_CORRECTION_INDISCRIMINATE: mode = ErrorCorrectionConfig::INDISCRIMINATE; break;
        case MSDFGEN_ERROR_CORRECTION_EDGE_ONLY: mode = ErrorCorrectionConfig::EDGE_ONLY; break;
        case MSDFGEN_ERROR_CORRECTION_EDGE_PRIORITY:
        default: mode = ErrorCorrectionConfig::EDGE_PRIORITY; break;
    }

    ErrorCorrectionConfig::DistanceCheckMode distCheck;
    switch (config.distanceCheckMode) {
        case MSDFGEN_DISTANCE_CHECK_NONE: distCheck = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE; break;
        case MSDFGEN_DISTANCE_CHECK_ALWAYS: distCheck = ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE; break;
        case MSDFGEN_DISTANCE_CHECK_AT_EDGE:
        default: distCheck = ErrorCorrectionConfig::CHECK_DISTANCE_AT_EDGE; break;
    }

    return ErrorCorrectionConfig(mode, distCheck, config.minDeviationRatio, config.minImproveRatio);
}

static MSDFGeneratorConfig toMSDFGeneratorConfig(const MsdfgenGeneratorConfig* config) {
    if (!config) {
        return MSDFGeneratorConfig();
    }
    return MSDFGeneratorConfig(config->overlapSupport != 0, toErrorCorrectionConfig(config->errorCorrection));
}

template<int N>
static MsdfgenBitmap createBitmapResult(const Bitmap<float, N>& bitmap) {
    MsdfgenBitmap result = {nullptr, 0, 0, 0, 0};
    int width = bitmap.width();
    int height = bitmap.height();

    size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height) * N;
    result.pixels = static_cast<float*>(malloc(pixelCount * sizeof(float)));
    if (result.pixels) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const float* src = bitmap(x, y);
                float* dst = result.pixels + (y * width + x) * N;
                for (int c = 0; c < N; c++) {
                    dst[c] = src[c];
                }
            }
        }
        result.width = width;
        result.height = height;
        result.channels = N;
        result.success = 1;
    }
    return result;
}

/* ============================================================================
 * Shape creation and destruction
 * ============================================================================ */

extern "C" {

MsdfgenShape msdfgen_shape_create(void) {
    return new Shape();
}

void msdfgen_shape_destroy(MsdfgenShape handle) {
    if (handle) {
        delete static_cast<Shape*>(handle);
    }
}

/* ============================================================================
 * Contour and edge operations
 * ============================================================================ */

int msdfgen_shape_add_contour(MsdfgenShape handle) {
    if (!handle) return -1;
    Shape* shape = static_cast<Shape*>(handle);
    shape->addContour();
    return static_cast<int>(shape->contours.size() - 1);
}

void msdfgen_contour_add_linear(
    MsdfgenShape handle,
    int contourIndex,
    double x0, double y0,
    double x1, double y1
) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    if (contourIndex < 0 || contourIndex >= static_cast<int>(shape->contours.size())) return;

    shape->contours[contourIndex].addEdge(EdgeHolder(
        new LinearSegment(Point2(x0, y0), Point2(x1, y1))
    ));
}

void msdfgen_contour_add_quadratic(
    MsdfgenShape handle,
    int contourIndex,
    double x0, double y0,
    double x1, double y1,
    double x2, double y2
) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    if (contourIndex < 0 || contourIndex >= static_cast<int>(shape->contours.size())) return;

    shape->contours[contourIndex].addEdge(EdgeHolder(
        new QuadraticSegment(Point2(x0, y0), Point2(x1, y1), Point2(x2, y2))
    ));
}

void msdfgen_contour_add_cubic(
    MsdfgenShape handle,
    int contourIndex,
    double x0, double y0,
    double x1, double y1,
    double x2, double y2,
    double x3, double y3
) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    if (contourIndex < 0 || contourIndex >= static_cast<int>(shape->contours.size())) return;

    shape->contours[contourIndex].addEdge(EdgeHolder(
        new CubicSegment(Point2(x0, y0), Point2(x1, y1), Point2(x2, y2), Point2(x3, y3))
    ));
}

/* ============================================================================
 * Shape operations
 * ============================================================================ */

void msdfgen_shape_normalize(MsdfgenShape handle) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    shape->normalize();
}

int msdfgen_shape_validate(MsdfgenShape handle) {
    if (!handle) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    return shape->validate() ? 1 : 0;
}

MsdfgenBounds msdfgen_shape_get_bounds(MsdfgenShape handle) {
    MsdfgenBounds bounds = {0, 0, 0, 0};
    if (!handle) return bounds;

    Shape* shape = static_cast<Shape*>(handle);
    Shape::Bounds b = shape->getBounds();
    bounds.left = b.l;
    bounds.bottom = b.b;
    bounds.right = b.r;
    bounds.top = b.t;
    return bounds;
}

MsdfgenBounds msdfgen_shape_get_bounds_ex(MsdfgenShape handle, double border, double miterLimit, int polarity) {
    MsdfgenBounds bounds = {0, 0, 0, 0};
    if (!handle) return bounds;

    Shape* shape = static_cast<Shape*>(handle);
    Shape::Bounds b = shape->getBounds(border, miterLimit, polarity);
    bounds.left = b.l;
    bounds.bottom = b.b;
    bounds.right = b.r;
    bounds.top = b.t;
    return bounds;
}

void msdfgen_shape_orient_contours(MsdfgenShape handle) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    shape->orientContours();
}

int msdfgen_shape_edge_count(MsdfgenShape handle) {
    if (!handle) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    return shape->edgeCount();
}

MsdfgenYAxisOrientation msdfgen_shape_get_y_axis_orientation(MsdfgenShape handle) {
    if (!handle) return MSDFGEN_Y_BOTTOM_UP;
    Shape* shape = static_cast<Shape*>(handle);
    return shape->getYAxisOrientation() == Y_DOWNWARD ? MSDFGEN_Y_TOP_DOWN : MSDFGEN_Y_BOTTOM_UP;
}

void msdfgen_shape_set_y_axis_orientation(MsdfgenShape handle, MsdfgenYAxisOrientation orientation) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    shape->setYAxisOrientation(orientation == MSDFGEN_Y_TOP_DOWN ? Y_DOWNWARD : Y_UPWARD);
}

int msdfgen_shape_contour_count(MsdfgenShape handle) {
    if (!handle) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    return static_cast<int>(shape->contours.size());
}

/* ============================================================================
 * Edge coloring
 * ============================================================================ */

void msdfgen_edge_coloring_simple(MsdfgenShape handle, double angleThreshold, unsigned long long seed) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    edgeColoringSimple(*shape, angleThreshold, seed);
}

void msdfgen_edge_coloring_ink_trap(MsdfgenShape handle, double angleThreshold, unsigned long long seed) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    edgeColoringInkTrap(*shape, angleThreshold, seed);
}

void msdfgen_edge_coloring_by_distance(MsdfgenShape handle, double angleThreshold, unsigned long long seed) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    edgeColoringByDistance(*shape, angleThreshold, seed);
}

/* ============================================================================
 * Configuration helpers
 * ============================================================================ */

MsdfgenErrorCorrectionConfig msdfgen_error_correction_config_default(void) {
    MsdfgenErrorCorrectionConfig config;
    config.mode = MSDFGEN_ERROR_CORRECTION_EDGE_PRIORITY;
    config.distanceCheckMode = MSDFGEN_DISTANCE_CHECK_AT_EDGE;
    // These match msdfgen's default values (10/9 ≈ 1.111...)
    config.minDeviationRatio = 1.11111111111111111;
    config.minImproveRatio = 1.11111111111111111;
    return config;
}

MsdfgenGeneratorConfig msdfgen_generator_config_default(void) {
    MsdfgenGeneratorConfig config;
    config.overlapSupport = 1;
    config.errorCorrection = msdfgen_error_correction_config_default();
    return config;
}

/* ============================================================================
 * SDF Generation
 * ============================================================================ */

MsdfgenBitmap msdfgen_generate_sdf(
    MsdfgenShape handle,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels
) {
    return msdfgen_generate_sdf_ex(handle, width, height, scaleX, scaleY, translateX, translateY, rangePixels, 1);
}

MsdfgenBitmap msdfgen_generate_sdf_ex(
    MsdfgenShape handle,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels,
    int overlapSupport
) {
    MsdfgenBitmap result = {nullptr, 0, 0, 0, 0};
    if (!handle || width <= 0 || height <= 0) return result;

    Shape* shape = static_cast<Shape*>(handle);
    Bitmap<float, 1> bitmap(width, height);

    Vector2 scale(scaleX, scaleY);
    Vector2 translate(translateX, translateY);
    Projection projection(scale, translate);
    Range range(rangePixels / std::min(scaleX, scaleY));
    SDFTransformation transformation(projection, range);

    GeneratorConfig config(overlapSupport != 0);
    generateSDF(bitmap, *shape, transformation, config);

    return createBitmapResult(bitmap);
}

MsdfgenBitmap msdfgen_generate_psdf(
    MsdfgenShape handle,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels
) {
    return msdfgen_generate_psdf_ex(handle, width, height, scaleX, scaleY, translateX, translateY, rangePixels, 1);
}

MsdfgenBitmap msdfgen_generate_psdf_ex(
    MsdfgenShape handle,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels,
    int overlapSupport
) {
    MsdfgenBitmap result = {nullptr, 0, 0, 0, 0};
    if (!handle || width <= 0 || height <= 0) return result;

    Shape* shape = static_cast<Shape*>(handle);
    Bitmap<float, 1> bitmap(width, height);

    Vector2 scale(scaleX, scaleY);
    Vector2 translate(translateX, translateY);
    Projection projection(scale, translate);
    Range range(rangePixels / std::min(scaleX, scaleY));
    SDFTransformation transformation(projection, range);

    GeneratorConfig config(overlapSupport != 0);
    generatePSDF(bitmap, *shape, transformation, config);

    return createBitmapResult(bitmap);
}

MsdfgenBitmap msdfgen_generate_msdf(
    MsdfgenShape handle,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels
) {
    return msdfgen_generate_msdf_ex(handle, width, height, scaleX, scaleY, translateX, translateY, rangePixels, nullptr);
}

MsdfgenBitmap msdfgen_generate_msdf_ex(
    MsdfgenShape handle,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels,
    const MsdfgenGeneratorConfig* config
) {
    MsdfgenBitmap result = {nullptr, 0, 0, 0, 0};
    if (!handle || width <= 0 || height <= 0) return result;

    Shape* shape = static_cast<Shape*>(handle);
    Bitmap<float, 3> bitmap(width, height);

    Vector2 scale(scaleX, scaleY);
    Vector2 translate(translateX, translateY);
    Projection projection(scale, translate);
    Range range(rangePixels / std::min(scaleX, scaleY));
    SDFTransformation transformation(projection, range);

    MSDFGeneratorConfig genConfig = toMSDFGeneratorConfig(config);
    generateMSDF(bitmap, *shape, transformation, genConfig);

    return createBitmapResult(bitmap);
}

MsdfgenBitmap msdfgen_generate_mtsdf(
    MsdfgenShape handle,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels
) {
    return msdfgen_generate_mtsdf_ex(handle, width, height, scaleX, scaleY, translateX, translateY, rangePixels, nullptr);
}

MsdfgenBitmap msdfgen_generate_mtsdf_ex(
    MsdfgenShape handle,
    int width, int height,
    double scaleX, double scaleY,
    double translateX, double translateY,
    double rangePixels,
    const MsdfgenGeneratorConfig* config
) {
    MsdfgenBitmap result = {nullptr, 0, 0, 0, 0};
    if (!handle || width <= 0 || height <= 0) return result;

    Shape* shape = static_cast<Shape*>(handle);
    Bitmap<float, 4> bitmap(width, height);

    Vector2 scale(scaleX, scaleY);
    Vector2 translate(translateX, translateY);
    Projection projection(scale, translate);
    Range range(rangePixels / std::min(scaleX, scaleY));
    SDFTransformation transformation(projection, range);

    MSDFGeneratorConfig genConfig = toMSDFGeneratorConfig(config);
    generateMTSDF(bitmap, *shape, transformation, genConfig);

    return createBitmapResult(bitmap);
}

/* ============================================================================
 * Bitmap operations
 * ============================================================================ */

void msdfgen_bitmap_free(MsdfgenBitmap* bitmap) {
    if (bitmap && bitmap->pixels) {
        free(bitmap->pixels);
        bitmap->pixels = nullptr;
        bitmap->width = 0;
        bitmap->height = 0;
        bitmap->channels = 0;
        bitmap->success = 0;
    }
}

void msdfgen_bitmap_to_bytes(
    const MsdfgenBitmap* bitmap,
    unsigned char* output,
    unsigned char edgeValue
) {
    if (!bitmap || !bitmap->pixels || !output || bitmap->width <= 0 || bitmap->height <= 0) return;

    int width = bitmap->width;
    int height = bitmap->height;
    int channels = bitmap->channels;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcIdx = (y * width + x) * channels;
            int dstIdx = (y * width + x) * 4;

            for (int c = 0; c < std::min(channels, 4); c++) {
                float val = bitmap->pixels[srcIdx + c];
                val = std::max(0.0f, std::min(1.0f, val));

                float scaled;
                if (val <= 0.5f) {
                    scaled = val * 2.0f * edgeValue;
                } else {
                    scaled = edgeValue + (val - 0.5f) * 2.0f * (255 - edgeValue);
                }
                output[dstIdx + c] = static_cast<unsigned char>(scaled + 0.5f);
            }

            for (int c = channels; c < 4; c++) {
                output[dstIdx + c] = 255;
            }
        }
    }
}

/* ============================================================================
 * Geometry resolution (requires Skia)
 * ============================================================================ */

int msdfgen_resolve_shape_geometry(MsdfgenShape handle) {
#ifdef MSDFGEN_USE_SKIA
    if (!handle) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    return resolveShapeGeometry(*shape) ? 1 : 0;
#else
    (void)handle;
    return 0;
#endif
}

/* ============================================================================
 * SVG import
 * ============================================================================ */

#ifndef MSDFGEN_DISABLE_SVG

int msdfgen_shape_load_from_svg_path(MsdfgenShape handle, const char* pathDef, double endpointSnapRange) {
    if (!handle || !pathDef) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    return buildShapeFromSvgPath(*shape, pathDef, endpointSnapRange) ? 1 : 0;
}

int msdfgen_shape_load_from_svg_file(MsdfgenShape handle, const char* filename, int pathIndex) {
    if (!handle || !filename) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    return loadSvgShape(*shape, filename, pathIndex, nullptr) ? 1 : 0;
}

int msdfgen_shape_load_from_svg_file_ex(MsdfgenShape handle, MsdfgenBounds* viewBox, const char* filename) {
    if (!handle || !filename) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    Shape::Bounds bounds;
    int result = loadSvgShape(*shape, bounds, filename);
    if (viewBox) {
        viewBox->left = bounds.l;
        viewBox->bottom = bounds.b;
        viewBox->right = bounds.r;
        viewBox->top = bounds.t;
    }
    return result;
}

#else

int msdfgen_shape_load_from_svg_path(MsdfgenShape, const char*, double) { return 0; }
int msdfgen_shape_load_from_svg_file(MsdfgenShape, const char*, int) { return 0; }
int msdfgen_shape_load_from_svg_file_ex(MsdfgenShape, MsdfgenBounds*, const char*) { return 0; }

#endif

/* ============================================================================
 * FreeType font operations
 * ============================================================================ */

MsdfgenFreetype msdfgen_freetype_init(void) {
    return initializeFreetype();
}

void msdfgen_freetype_deinit(MsdfgenFreetype freetype) {
    if (freetype) {
        deinitializeFreetype(static_cast<FreetypeHandle*>(freetype));
    }
}

MsdfgenFont msdfgen_font_load(MsdfgenFreetype freetype, const char* filename) {
    if (!freetype || !filename) return nullptr;
    return loadFont(static_cast<FreetypeHandle*>(freetype), filename);
}

MsdfgenFont msdfgen_font_load_data(MsdfgenFreetype freetype, const unsigned char* data, int length) {
    if (!freetype || !data || length <= 0) return nullptr;
    return loadFontData(static_cast<FreetypeHandle*>(freetype), data, length);
}

void msdfgen_font_destroy(MsdfgenFont font) {
    if (font) {
        destroyFont(static_cast<FontHandle*>(font));
    }
}

int msdfgen_font_get_metrics(MsdfgenFont font, MsdfgenFontMetrics* metrics, MsdfgenFontCoordinateScaling scaling) {
    if (!font || !metrics) return 0;
    FontMetrics m;
    if (!getFontMetrics(m, static_cast<FontHandle*>(font), toFontScaling(scaling))) {
        return 0;
    }
    metrics->emSize = m.emSize;
    metrics->ascenderY = m.ascenderY;
    metrics->descenderY = m.descenderY;
    metrics->lineHeight = m.lineHeight;
    metrics->underlineY = m.underlineY;
    metrics->underlineThickness = m.underlineThickness;
    return 1;
}

int msdfgen_font_get_whitespace_width(MsdfgenFont font, double* spaceAdvance, double* tabAdvance, MsdfgenFontCoordinateScaling scaling) {
    if (!font) return 0;
    double space = 0, tab = 0;
    if (!getFontWhitespaceWidth(space, tab, static_cast<FontHandle*>(font), toFontScaling(scaling))) {
        return 0;
    }
    if (spaceAdvance) *spaceAdvance = space;
    if (tabAdvance) *tabAdvance = tab;
    return 1;
}

int msdfgen_font_get_glyph_count(MsdfgenFont font, unsigned int* count) {
    if (!font || !count) return 0;
    unsigned c = 0;
    if (!getGlyphCount(c, static_cast<FontHandle*>(font))) {
        return 0;
    }
    *count = c;
    return 1;
}

int msdfgen_font_get_glyph_index(MsdfgenFont font, unsigned int unicode, unsigned int* glyphIndex) {
    if (!font || !glyphIndex) return 0;
    GlyphIndex idx;
    if (!getGlyphIndex(idx, static_cast<FontHandle*>(font), unicode)) {
        return 0;
    }
    *glyphIndex = idx.getIndex();
    return 1;
}

int msdfgen_font_load_glyph(
    MsdfgenShape handle,
    MsdfgenFont font,
    unsigned int unicode,
    MsdfgenFontCoordinateScaling scaling,
    double* outAdvance
) {
    if (!handle || !font) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    double advance = 0;
    if (!loadGlyph(*shape, static_cast<FontHandle*>(font), unicode, toFontScaling(scaling), &advance)) {
        return 0;
    }
    if (outAdvance) *outAdvance = advance;
    return 1;
}

int msdfgen_font_load_glyph_by_index(
    MsdfgenShape handle,
    MsdfgenFont font,
    unsigned int glyphIndex,
    MsdfgenFontCoordinateScaling scaling,
    double* outAdvance
) {
    if (!handle || !font) return 0;
    Shape* shape = static_cast<Shape*>(handle);
    double advance = 0;
    if (!loadGlyph(*shape, static_cast<FontHandle*>(font), GlyphIndex(glyphIndex), toFontScaling(scaling), &advance)) {
        return 0;
    }
    if (outAdvance) *outAdvance = advance;
    return 1;
}

int msdfgen_font_get_kerning(
    MsdfgenFont font,
    unsigned int unicode0,
    unsigned int unicode1,
    MsdfgenFontCoordinateScaling scaling,
    double* kerning
) {
    if (!font || !kerning) return 0;
    double k = 0;
    if (!getKerning(k, static_cast<FontHandle*>(font), unicode0, unicode1, toFontScaling(scaling))) {
        return 0;
    }
    *kerning = k;
    return 1;
}

int msdfgen_font_get_kerning_by_index(
    MsdfgenFont font,
    unsigned int glyphIndex0,
    unsigned int glyphIndex1,
    MsdfgenFontCoordinateScaling scaling,
    double* kerning
) {
    if (!font || !kerning) return 0;
    double k = 0;
    if (!getKerning(k, static_cast<FontHandle*>(font), GlyphIndex(glyphIndex0), GlyphIndex(glyphIndex1), toFontScaling(scaling))) {
        return 0;
    }
    *kerning = k;
    return 1;
}

#ifndef MSDFGEN_DISABLE_VARIABLE_FONTS

int msdfgen_font_set_variation_axis(
    MsdfgenFreetype freetype,
    MsdfgenFont font,
    const char* name,
    double coordinate
) {
    if (!freetype || !font || !name) return 0;
    return setFontVariationAxis(static_cast<FreetypeHandle*>(freetype), static_cast<FontHandle*>(font), name, coordinate) ? 1 : 0;
}

int msdfgen_font_get_variation_axis_count(
    MsdfgenFreetype freetype,
    MsdfgenFont font,
    int* count
) {
    if (!freetype || !font || !count) return 0;
    std::vector<FontVariationAxis> axes;
    if (!listFontVariationAxes(axes, static_cast<FreetypeHandle*>(freetype), static_cast<FontHandle*>(font))) {
        return 0;
    }
    *count = static_cast<int>(axes.size());
    return 1;
}

int msdfgen_font_get_variation_axis(
    MsdfgenFreetype freetype,
    MsdfgenFont font,
    int index,
    MsdfgenFontVariationAxis* axis
) {
    if (!freetype || !font || !axis || index < 0) return 0;
    std::vector<FontVariationAxis> axes;
    if (!listFontVariationAxes(axes, static_cast<FreetypeHandle*>(freetype), static_cast<FontHandle*>(font))) {
        return 0;
    }
    if (index >= static_cast<int>(axes.size())) return 0;
    axis->name = axes[index].name;
    axis->minValue = axes[index].minValue;
    axis->maxValue = axes[index].maxValue;
    axis->defaultValue = axes[index].defaultValue;
    return 1;
}

#else

int msdfgen_font_set_variation_axis(MsdfgenFreetype, MsdfgenFont, const char*, double) { return 0; }
int msdfgen_font_get_variation_axis_count(MsdfgenFreetype, MsdfgenFont, int*) { return 0; }
int msdfgen_font_get_variation_axis(MsdfgenFreetype, MsdfgenFont, int, MsdfgenFontVariationAxis*) { return 0; }

#endif

/* ============================================================================
 * Utility functions
 * ============================================================================ */

void msdfgen_auto_frame(
    MsdfgenShape handle,
    int width, int height,
    double rangePixels,
    double* outScaleX, double* outScaleY,
    double* outTranslateX, double* outTranslateY
) {
    if (!handle || width <= 0 || height <= 0) return;

    Shape* shape = static_cast<Shape*>(handle);
    Shape::Bounds bounds = shape->getBounds();

    double shapeWidth = bounds.r - bounds.l;
    double shapeHeight = bounds.t - bounds.b;

    if (shapeWidth <= 0 || shapeHeight <= 0) return;

    double availableWidth = width - 2.0 * rangePixels;
    double availableHeight = height - 2.0 * rangePixels;

    double scaleX = availableWidth / shapeWidth;
    double scaleY = availableHeight / shapeHeight;
    double scale = std::min(scaleX, scaleY);

    double translateX = rangePixels / scale - bounds.l + (availableWidth / scale - shapeWidth) * 0.5;
    double translateY = rangePixels / scale - bounds.b + (availableHeight / scale - shapeHeight) * 0.5;

    if (outScaleX) *outScaleX = scale;
    if (outScaleY) *outScaleY = scale;
    if (outTranslateX) *outTranslateX = translateX;
    if (outTranslateY) *outTranslateY = translateY;
}

const char* msdfgen_get_version(void) {
    return "1.13";
}

int msdfgen_has_skia_support(void) {
#ifdef MSDFGEN_USE_SKIA
    return 1;
#else
    return 0;
#endif
}

} // extern "C"

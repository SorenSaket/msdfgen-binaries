/*
 * C API wrapper implementation for msdfgen library
 */

#define MSDFGEN_C_API_EXPORTS
#include "msdfgen_c_api.h"
#include "msdfgen.h"
#include "core/edge-coloring.h"

#ifdef MSDFGEN_USE_SKIA
#include "ext/resolve-shape-geometry.h"
#endif

#include <cstdlib>
#include <cstring>
#include <algorithm>

using namespace msdfgen;

extern "C" {

MsdfgenShape msdfgen_shape_create(void) {
    return new Shape();
}

void msdfgen_shape_destroy(MsdfgenShape handle) {
    if (handle) {
        delete static_cast<Shape*>(handle);
    }
}

int msdfgen_shape_add_contour(MsdfgenShape handle) {
    if (!handle) return -1;
    Shape* shape = static_cast<Shape*>(handle);
    shape->addContour();
    return static_cast<int>(shape->contours.size() - 1);
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

void msdfgen_shape_orient_contours(MsdfgenShape handle) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    shape->orientContours();
}

void msdfgen_edge_coloring_simple(MsdfgenShape handle, double angleThreshold, unsigned long long seed) {
    if (!handle) return;
    Shape* shape = static_cast<Shape*>(handle);
    edgeColoringSimple(*shape, angleThreshold, seed);
}

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

MsdfgenResult msdfgen_generate_mtsdf(
    MsdfgenShape handle,
    int width,
    int height,
    double scaleX,
    double scaleY,
    double translateX,
    double translateY,
    double rangePixels
) {
    MsdfgenResult result = {nullptr, 0, 0, 0};

    if (!handle || width <= 0 || height <= 0) return result;

    Shape* shape = static_cast<Shape*>(handle);

    // Create bitmap
    Bitmap<float, 4> bitmap(width, height);

    // Create projection and range
    Vector2 scale(scaleX, scaleY);
    Vector2 translate(translateX, translateY);
    Projection projection(scale, translate);
    Range range(rangePixels / std::min(scaleX, scaleY));
    SDFTransformation transformation(projection, range);

    // Generate MTSDF with overlap support
    MSDFGeneratorConfig config;
    config.overlapSupport = true;
    generateMTSDF(bitmap, *shape, transformation, config);

    // Allocate output buffer and copy
    size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    result.pixels = static_cast<float*>(malloc(pixelCount * sizeof(float)));
    if (result.pixels) {
        // Copy pixel data (msdfgen stores row-major, bottom-to-top)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float* src = bitmap(x, y);
                float* dst = result.pixels + (y * width + x) * 4;
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
            }
        }
        result.width = width;
        result.height = height;
        result.success = 1;
    }

    return result;
}

MsdfgenResult msdfgen_generate_msdf(
    MsdfgenShape handle,
    int width,
    int height,
    double scaleX,
    double scaleY,
    double translateX,
    double translateY,
    double rangePixels
) {
    MsdfgenResult result = {nullptr, 0, 0, 0};

    if (!handle || width <= 0 || height <= 0) return result;

    Shape* shape = static_cast<Shape*>(handle);

    // Create bitmap (3 channels for MSDF)
    Bitmap<float, 3> bitmap(width, height);

    // Create projection and range
    Vector2 scale(scaleX, scaleY);
    Vector2 translate(translateX, translateY);
    Projection projection(scale, translate);
    Range range(rangePixels / std::min(scaleX, scaleY));
    SDFTransformation transformation(projection, range);

    // Generate MSDF with overlap support
    MSDFGeneratorConfig config;
    config.overlapSupport = true;
    generateMSDF(bitmap, *shape, transformation, config);

    // Allocate output buffer (still 4 channels for RGBA, alpha = 1)
    size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    result.pixels = static_cast<float*>(malloc(pixelCount * sizeof(float)));
    if (result.pixels) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float* src = bitmap(x, y);
                float* dst = result.pixels + (y * width + x) * 4;
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = 1.0f; // Full alpha
            }
        }
        result.width = width;
        result.height = height;
        result.success = 1;
    }

    return result;
}

void msdfgen_free_pixels(float* pixels) {
    free(pixels);
}

void msdfgen_float_to_bytes(
    const float* floatPixels,
    unsigned char* bytePixels,
    int width,
    int height,
    int channels,
    unsigned char edgeValue,
    double rangePixels
) {
    if (!floatPixels || !bytePixels || width <= 0 || height <= 0) return;

    // Convert float distances to bytes
    // In msdfgen, the distance field values are normalized so that:
    // - 0.5 is on the edge
    // - Values < 0.5 are inside
    // - Values > 0.5 are outside
    // We map this to bytes where edgeValue is on the edge

    double edgeNorm = edgeValue / 255.0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcIdx = (y * width + x) * channels;
            int dstIdx = (y * width + x) * 4;

            for (int c = 0; c < std::min(channels, 4); c++) {
                // msdfgen outputs values where 0.5 = edge
                // Map to byte range: inside (>0.5) -> high values, outside (<0.5) -> low values
                float val = floatPixels[srcIdx + c];
                // Clamp to [0, 1] range and convert to byte
                val = std::max(0.0f, std::min(1.0f, val));
                bytePixels[dstIdx + c] = static_cast<unsigned char>(val * 255.0f + 0.5f);
            }

            // Fill remaining channels with 255 if needed
            for (int c = channels; c < 4; c++) {
                bytePixels[dstIdx + c] = 255;
            }
        }
    }
}

} // extern "C"

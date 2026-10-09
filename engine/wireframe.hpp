#pragma once
#include <stddef.h>
#include <stdint.h>

class Draw;

// Borrowed indexed geometry. Callers retain buffers for the synchronous render.
struct WireframeGeometry
{
    size_t vertexCount, edgeCount, faceCount;
    const uint16_t *corners, *edges, *faceEdges;
    const uint8_t *hiddenEdges;
    void *context;
    void (*vertex)(void *, size_t, double[3]);
    bool (*visible)(void *, size_t);
};

struct WireframeView
{
    double basis[3][3], center[3];
    double distance, centerX, centerY, focal, near;
    double left, top, width, height;
    bool orthographic, culling, assetOnly;
    uint16_t color = 0xBDF7, selectionColor = 0xFFE0, activeColor = 0x07FF;
};

struct WireframePoint
{
    double x, y, z, screenX, screenY;
    uint8_t clip;
};

// Caller-owned scratch permits bounded blocks on fragmented device heaps.
struct WireframeScratch
{
    void *context;
    WireframePoint &(*point)(void *, size_t);
    uint8_t *edges;
    size_t pointCapacity; // Remaining vertices are projected on demand, without allocation.
};

class Wireframe
{
  public:
    using Line = void (*)(void *, int, int, int, int, uint16_t);
    static bool render(const WireframeGeometry &, const WireframeView &, WireframeScratch &,
                       const uint8_t *facePriorities, Line, void *);
    static bool render(const WireframeGeometry &, const WireframeView &, WireframeScratch &,
                       const uint8_t *facePriorities, Draw &);
};

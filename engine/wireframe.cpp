#include "wireframe.hpp"
#include "draw.hpp"
#include "projection.hpp"
#include <math.h>
#include <string.h>

bool Wireframe::render(const WireframeGeometry &g, const WireframeView &v, WireframeScratch &scratch,
                       const uint8_t *priorities, Line line, void *context)
{
    if (!g.vertex || !scratch.point || !line || !scratch.edges || !g.corners || !g.edges || !g.faceEdges ||
        g.vertexCount > 65536 || g.edgeCount > 32768 || v.width <= 0 || v.height <= 0 || v.distance <= 0 ||
        v.near <= 0 || v.focal <= 0)
        return false;
    if (!isfinite(v.left) || !isfinite(v.top) || !isfinite(v.width) || !isfinite(v.height) ||
        !isfinite(v.distance) || !isfinite(v.near) || !isfinite(v.focal) || !isfinite(v.centerX) ||
        !isfinite(v.centerY) || v.left < 0 || v.top < 0 || v.left + v.width > 65536 ||
        v.top + v.height > 65536)
        return false;
    for (size_t i = 0; i < g.edgeCount * 2; ++i)
        if (g.edges[i] >= g.vertexCount)
            return false;
    for (size_t i = 0; i < g.faceCount * 3; ++i)
        if (g.corners[i] >= g.vertexCount || (g.faceEdges[i] & 0x7fff) >= g.edgeCount)
            return false;
    const double right = v.left + v.width - 1, bottom = v.top + v.height - 1;
    auto project = [&](size_t i) -> WireframePoint {
        double input[3];
        g.vertex(g.context, i, input);
        for (unsigned j = 0; j < 3; ++j)
            input[j] -= v.center[j];
        WireframePoint p{};
        p.x = input[0] * v.basis[0][0] + input[1] * v.basis[0][1] + input[2] * v.basis[0][2];
        p.y = input[0] * v.basis[1][0] + input[1] * v.basis[1][1] + input[2] * v.basis[1][2];
        p.z = input[0] * v.basis[2][0] + input[1] * v.basis[2][1] + input[2] * v.basis[2][2] + v.distance;
        if (!isfinite(p.x) || !isfinite(p.y) || !isfinite(p.z))
        {
            p.clip = 32;
            return p;
        }
        if (!v.orthographic && p.z < v.near)
        {
            p.clip = 16;
            p.screenX = p.screenY = 0;
            return p;
        }
        projectPoint(p.x, p.y, v.orthographic ? v.distance : p.z, v.centerX, v.centerY, v.focal, p.screenX,
                     p.screenY);
        if (!isfinite(p.screenX) || !isfinite(p.screenY))
        {
            p.clip = 32;
            return p;
        }
        p.clip = (p.screenX < v.left  ? 1
                  : p.screenX > right ? 2
                                      : 0) |
                 (p.screenY < v.top    ? 4
                  : p.screenY > bottom ? 8
                                       : 0);
        return p;
    };
    for (size_t i = 0; i < g.vertexCount; ++i)
    {
        WireframePoint p = project(i);
        if (p.clip == 32)
            return false;
        if (i < scratch.pointCapacity)
            scratch.point(scratch.context, i) = p;
    }
    auto point = [&](size_t i) -> WireframePoint {
        return i < scratch.pointCapacity ? scratch.point(scratch.context, i) : project(i);
    };
    const bool filtered = priorities || v.culling || v.assetOnly;
    memset(scratch.edges, 0, g.edgeCount);
    if (filtered)
        for (size_t face = 0; face < g.faceCount; ++face)
        {
            const uint8_t priority = priorities ? priorities[face] : 0;
            if (priorities && !priority)
                continue;
            if (!priorities)
            {
                if (v.assetOnly && g.visible && !g.visible(g.context, face))
                    continue;
                const WireframePoint a = point(g.corners[face * 3]);
                const WireframePoint b = point(g.corners[face * 3 + 1]);
                const WireframePoint c = point(g.corners[face * 3 + 2]);
                const double ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
                const double vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
                const double nx = uy * vz - uz * vy, ny = uz * vx - ux * vz, nz = ux * vy - uy * vx;
                const double facing = v.orthographic ? nz : nx * a.x + ny * a.y + nz * a.z;
                if (v.culling && facing <= 0)
                    continue;
            }
            for (unsigned j = 0; j < 3; ++j)
            {
                const uint16_t entry = g.faceEdges[face * 3 + j];
                uint8_t &flag = scratch.edges[entry & 0x7fff];
                if (priorities)
                {
                    if (priority >= (flag & 3))
                        flag = priority | ((entry & 0x8000) ? 0x80 : 0);
                }
                else if (!flag)
                    flag = (entry & 0x8000) ? 2 : 1;
            }
        }
    for (unsigned pass = priorities ? 1 : 0; pass <= (priorities ? 2u : 0u); ++pass)
        for (size_t i = 0; i < g.edgeCount; ++i)
        {
            const uint8_t flag = scratch.edges[i];
            if ((g.hiddenEdges && g.hiddenEdges[i]) || (filtered && !flag) ||
                (priorities && (flag & 3) != pass))
                continue;
            size_t a = g.edges[i * 2], b = g.edges[i * 2 + 1];
            if (filtered && (priorities ? (flag & 0x80) != 0 : flag == 2))
            {
                size_t tmp = a;
                a = b;
                b = tmp;
            }
            WireframePoint pa = point(a), pb = point(b);
            if (pa.clip & pb.clip)
                continue;
            if ((pa.clip | pb.clip) & 16)
            {
                if (pa.clip & 16)
                {
                    const double t = (v.near - pa.z) / (pb.z - pa.z);
                    pa.x += t * (pb.x - pa.x);
                    pa.y += t * (pb.y - pa.y);
                    pa.z += t * (pb.z - pa.z);
                    projectPoint(pa.x, pa.y, pa.z, v.centerX, v.centerY, v.focal, pa.screenX, pa.screenY);
                }
                if (pb.clip & 16)
                {
                    const double t = (v.near - pb.z) / (pa.z - pb.z);
                    pb.x += t * (pa.x - pb.x);
                    pb.y += t * (pa.y - pb.y);
                    pb.z += t * (pa.z - pb.z);
                    projectPoint(pb.x, pb.y, pb.z, v.centerX, v.centerY, v.focal, pb.screenX, pb.screenY);
                }
            }
            if (!isfinite(pa.screenX) || !isfinite(pa.screenY) || !isfinite(pb.screenX) ||
                !isfinite(pb.screenY))
                continue;
            if ((pa.clip | pb.clip) && !clipProjectedLine(pa.screenX, pa.screenY, pb.screenX, pb.screenY,
                                                          v.left, v.top, right, bottom))
                continue;
            const uint16_t color = priorities ? (pass == 2 ? v.activeColor : v.selectionColor) : v.color;
            line(context, int(pa.screenX), int(pa.screenY), int(pb.screenX), int(pb.screenY), color);
        }
    return true;
}

static void wireframeLine(void *context, int x1, int y1, int x2, int y2, uint16_t color)
{
    static_cast<Draw *>(context)->line(x1, y1, x2, y2, color);
}

bool Wireframe::render(const WireframeGeometry &g, const WireframeView &v, WireframeScratch &scratch,
                       const uint8_t *priorities, Draw &draw)
{
    return render(g, v, scratch, priorities, wireframeLine, &draw);
}

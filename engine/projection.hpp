#pragma once

// Shared perspective projection, retaining the caller's numeric precision.
template <typename T>
inline void projectPoint(T x, T y, T depth, T centerX, T centerY, T focal, T &screenX, T &screenY)
{
    screenX = centerX + x * focal / depth;
    screenY = centerY - y * focal / depth;
}

// Preserve the established float raster rounding used by triangle projection.
inline void projectPoint(float x, float y, float depth, float centerX, float centerY, float focal,
                         float &screenX, float &screenY)
{
    const float scale = focal / depth;
    screenX = x * scale + centerX;
    screenY = -y * scale + centerY;
}

template <typename T>
inline bool clipProjectedLine(T &x, T &y, T &endX, T &endY, T left, T top, T right, T bottom)
{
    const T dx = endX - x, dy = endY - y;
    const T p[] = {-dx, dx, -dy, dy};
    const T q[] = {x - left, right - x, y - top, bottom - y};
    T low = 0, high = 1;
    for (unsigned i = 0; i < 4; ++i)
    {
        if (p[i] == 0)
        {
            if (q[i] < 0)
                return false;
        }
        else if (p[i] < 0)
        {
            const T t = q[i] / p[i];
            if (t > low)
                low = t;
        }
        else
        {
            const T t = q[i] / p[i];
            if (t < high)
                high = t;
        }
        if (low > high)
            return false;
    }
    endX = x + high * dx;
    endY = y + high * dy;
    x += low * dx;
    y += low * dy;
    return true;
}

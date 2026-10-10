#include "image.hpp"
#include <math.h>

#include ENGINE_MEM_INCLUDE

#ifdef ENGINE_STORAGE_INCLUDE
#include ENGINE_STORAGE_INCLUDE
#endif

#ifdef ENGINE_LOG_INCLUDE
#include ENGINE_LOG_INCLUDE
#endif

bool Image::getBillboardFrame(uint32_t frame, uint8_t view, const uint8_t *&pixels,
                             const uint8_t *&mask) const
{
    pixels = mask = nullptr;
    if (!isBillboard() || frame >= billboardFrames || view >= billboardViewCount)
        return false;
    const size_t count = size_t(size.x) * size_t(size.y);
    const size_t index = size_t(frame) * billboardViewCount + view;
    pixels = billboardPixels ? billboardPixels[index] : static_cast<const uint8_t *>(data) + index * count * 2;
    mask = billboardMasks ? billboardMasks[index] : billboardMask + index * ((count + 7) / 8);
    if (!pixels || !mask)
    {
        pixels = mask = nullptr;
        return false;
    }
    return true;
}

uint8_t Image::getBillboardView(float facingX, float facingY, float toCameraX, float toCameraY)
{
    if (!isfinite(facingX) || !isfinite(facingY) || !isfinite(toCameraX) || !isfinite(toCameraY))
        return 0;
    const float facing = (facingX == 0 && facingY == 0) ? 0 : atan2f(facingY, facingX);
    const float angle = atan2f(toCameraY, toCameraX) - facing;
    const int bucket = int(floorf(angle * (4.0f / 3.14159265358979323846f) + 0.5f));
    return uint8_t((bucket % 8 + 8) % 8);
}

bool Image::getData(void *buffer, size_t buffer_size)
{
    if (this->data != nullptr)
    {
        // If data is already loaded in memory, copy it to the buffer
        size_t data_size = size.x * size.y * (is_8bit ? 1 : 2); // Calculate data size based on color depth
        if (data_size > buffer_size)
        {
            ENGINE_LOG_INFO("Image::getData buffer too small for image data\n");
            return false; // Buffer too small
        }
        memcpy(buffer, this->data, data_size);
        return true;
    }

    if (path == nullptr || path[0] == '\0')
    {
        ENGINE_LOG_INFO("Image::getData called but image path is null or empty\n");
        return false;
    }
#ifdef ENGINE_STORAGE_READ
    size_t bytes_read = ENGINE_STORAGE_READ(this->path, buffer, buffer_size);
    return bytes_read > 0;
#else
    return false;
#endif
}

bool Image::isBillboard() const
{
    if ((!billboardPixels && !data) || is_8bit || (!billboardMasks && !billboardMask) || !billboardFrames ||
        !isfinite(size.x) || !isfinite(size.y) || size.x < 1 || size.y < 1 ||
        size.x > 1024 || size.y > 1024 || floorf(size.x) != size.x || floorf(size.y) != size.y)
        return false;
    const size_t frameBytes = size_t(size.x) * size_t(size.y) * 2 * billboardViewCount;
    return billboardFrames <= SIZE_MAX / frameBytes;
}

void Image::render(Draw *draw, const Vector &position)
{
    if (draw != nullptr)
    {
        if (is_8bit)
        {
            uint8_t *data_buffer = (uint8_t *)ENGINE_MEM_MALLOC(size.x * size.y);
            if (!data_buffer)
            {
                ENGINE_LOG_INFO("Image::render failed to allocate memory for 8-bit image data\n");
                return;
            }
            if (!getData(data_buffer, size.x * size.y))
            {
                ENGINE_MEM_FREE(data_buffer);
                ENGINE_LOG_INFO("Image::render failed to get 8-bit image data\n");
                return;
            }
            draw->image(position, data_buffer, size);
            ENGINE_MEM_FREE(data_buffer);
        }
        else
        {
            uint16_t *data_buffer = (uint16_t *)ENGINE_MEM_MALLOC(size.x * size.y * 2);
            if (!data_buffer)
            {
                ENGINE_LOG_INFO("Image::render failed to allocate memory for 16-bit image data\n");
                return;
            }
            if (!getData(data_buffer, size.x * size.y * 2))
            {
                ENGINE_MEM_FREE(data_buffer);
                ENGINE_LOG_INFO("Image::render failed to get 16-bit image data\n");
                return;
            }
            draw->image(position, data_buffer, size);
            ENGINE_MEM_FREE(data_buffer);
        }
    }
}

void Image::render(Draw *draw, int16_t x, int16_t y)
{
    if (draw != nullptr)
    {
        if (is_8bit)
        {
            uint8_t *data_buffer = (uint8_t *)ENGINE_MEM_MALLOC(size.x * size.y);
            if (!data_buffer)
            {
                ENGINE_LOG_INFO("Image::render failed to allocate memory for 8-bit image data\n");
                return;
            }
            if (!getData(data_buffer, size.x * size.y))
            {
                ENGINE_MEM_FREE(data_buffer);
                ENGINE_LOG_INFO("Image::render failed to get 8-bit image data\n");
                return;
            }
            draw->image(x, y, data_buffer, size.x, size.y);
            ENGINE_MEM_FREE(data_buffer);
        }
        else
        {
            uint16_t *data_buffer = (uint16_t *)ENGINE_MEM_MALLOC(size.x * size.y * 2);
            if (!data_buffer)
            {
                ENGINE_LOG_INFO("Image::render failed to allocate memory for 16-bit image data\n");
                return;
            }
            if (!getData(data_buffer, size.x * size.y * 2))
            {
                ENGINE_MEM_FREE(data_buffer);
                ENGINE_LOG_INFO("Image::render failed to get 16-bit image data\n");
                return;
            }
            draw->image(x, y, data_buffer, size.x, size.y);
            ENGINE_MEM_FREE(data_buffer);
        }
    }
}

bool Image::setBillboard(const uint8_t *mask, uint32_t frames)
{
    Image candidate = *this;
    candidate.billboardPixels = candidate.billboardMasks = nullptr;
    candidate.billboardMask = mask;
    candidate.billboardFrames = frames;
    if (!candidate.isBillboard())
        return false;
    *this = candidate;
    return true;
}

bool Image::setBillboardFrames(const uint8_t *const *pixels, const uint8_t *const *masks,
                               uint32_t frames)
{
    if (!pixels || !masks || !frames || frames > SIZE_MAX / billboardViewCount / sizeof(*pixels))
        return false;
    Image candidate = *this;
    candidate.billboardPixels = pixels;
    candidate.billboardMasks = masks;
    candidate.billboardFrames = frames;
    if (!candidate.isBillboard())
        return false;
    const size_t count = size_t(frames) * billboardViewCount;
    for (size_t i = 0; i < count; ++i)
        if (!pixels[i] || !masks[i])
            return false;
    *this = candidate;
    return true;
}

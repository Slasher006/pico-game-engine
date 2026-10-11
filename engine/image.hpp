#pragma once
#include "vector.hpp"
#include "draw.hpp"
#include <stdint.h>

class Image
{
public:
    Vector size; // Size of the image (width, height)

    Image() : size(Vector(0, 0)), data(nullptr), is_8bit(false), path("") {}
    Image(const Vector &size, bool is_8bit, const void *data = nullptr, const char *path = "")
        : size(size), data(data), is_8bit(is_8bit), path(path) {}
    ~Image() {}

    static constexpr uint8_t billboardViewCount = 8;

    bool getBillboardFrame(uint32_t frame, uint8_t view, const uint8_t *&pixels,
                          const uint8_t *&mask) const;
    uint32_t getBillboardFrameCount() const { return billboardFrames; }
    static uint8_t getBillboardView(float facingX, float facingY, float toCameraX, float toCameraY);
    bool getData(void *buffer, size_t buffer_size);  // get raw pixel data (from file or memory)
    bool isBillboard() const;
    void render(Draw *draw, const Vector &position); // render the image at a given position
    void render(Draw *draw, int16_t x, int16_t y);   // render the image at a given position

    bool setBillboard(const uint8_t *mask, uint32_t frames = 1);
    bool setBillboardFrames(const uint8_t *const *pixels, const uint8_t *const *masks,
                            uint32_t frames);

private:
    const uint8_t *const *billboardPixels = nullptr;
    const uint8_t *const *billboardMasks = nullptr;
    uint32_t billboardFrames = 0;
    const uint8_t *billboardMask = nullptr;
    const void *data; // Pointer to the image data (raw pixel data)
    bool is_8bit;     // Flag to indicate if the image uses 8-bit graphics
    const char *path; // Path to the image file
};
#include "draw.hpp"
#include ENGINE_LCD_INCLUDE
#include <math.h>

Draw::Draw() : currentFontSize(ENGINE_FONT_DEFAULT), currentTextColor(0x0000)
{
#ifdef ENGINE_LCD_INIT
    ENGINE_LCD_INIT();
#endif
}

Draw::~Draw()
{
#ifdef ENGINE_LCD_DEINIT
    ENGINE_LCD_DEINIT();
#endif
}

void Draw::circle(Vector position, uint16_t r, uint16_t color)
{
    ENGINE_LCD_CIRCLE(position.x, position.y, r, color);
}

void Draw::circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    ENGINE_LCD_CIRCLE(x, y, r, color);
}

void Draw::fillCircle(Vector position, uint16_t r, uint16_t color)
{
    ENGINE_LCD_FILL_CIRCLE(position.x, position.y, r, color);
}

void Draw::fillCircle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    ENGINE_LCD_FILL_CIRCLE(x, y, r, color);
}

void Draw::fillPolygon(uint16_t x[], uint16_t y[], int count, uint16_t color)
{
    ENGINE_LCD_FILL_POLYGON(x, y, count, color);
}

void Draw::fillPolygonAlpha(uint16_t x[], uint16_t y[], int count, uint16_t color, uint8_t alpha)
{
#ifdef ENGINE_LCD_FILL_POLYGON_ALPHA
    ENGINE_LCD_FILL_POLYGON_ALPHA(x, y, count, color, alpha);
#else
    ENGINE_LCD_FILL_POLYGON(x, y, count, color);
#endif
}

void Draw::fillRectangle(Vector position, Vector size, uint16_t color)
{
    ENGINE_LCD_FILL_RECTANGLE(position.x, position.y, size.x, size.y, color);
}

void Draw::fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    ENGINE_LCD_FILL_RECTANGLE(x, y, width, height, color);
}

Vector Draw::getDisplaySize() const noexcept
{
    return Vector(ENGINE_LCD_WIDTH, ENGINE_LCD_HEIGHT);
}

void Draw::fillScreen(uint16_t color)
{
    ENGINE_LCD_CLEAR(color);
}

void Draw::fillTriangle(Vector p1, Vector p2, Vector p3, uint16_t color)
{
    ENGINE_LCD_FILL_TRIANGLE(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color);
}

void Draw::fillTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
    ENGINE_LCD_FILL_TRIANGLE(x1, y1, x2, y2, x3, y3, color);
}

void Draw::fillTriangleAlpha(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color, uint8_t alpha)
{
#ifdef ENGINE_LCD_FILL_TRIANGLE_ALPHA
    ENGINE_LCD_FILL_TRIANGLE_ALPHA(x1, y1, x2, y2, x3, y3, color, alpha);
#else
    ENGINE_LCD_FILL_TRIANGLE(x1, y1, x2, y2, x3, y3, color);
#endif
}

void Draw::image(Vector position, const uint8_t *bitmap, Vector size)
{
    ENGINE_LCD_BLIT(position.x, position.y, size.x, size.y, bitmap);
}

void Draw::image(Vector position, const uint16_t *bitmap, Vector size)
{
    ENGINE_LCD_BLIT_16BIT(position.x, position.y, size.x, size.y, bitmap);
}

void Draw::image(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t width, uint16_t height)
{
    ENGINE_LCD_BLIT(x, y, width, height, bitmap);
}

void Draw::image(uint16_t x, uint16_t y, const uint16_t *bitmap, uint16_t width, uint16_t height)
{
    ENGINE_LCD_BLIT_16BIT(x, y, width, height, bitmap);
}

void Draw::line(Vector position, Vector size, uint16_t color)
{
    ENGINE_LCD_LINE(position.x, position.y, size.x, size.y, color);
}

void Draw::line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    ENGINE_LCD_LINE(x1, y1, x2, y2, color);
}

void Draw::pixel(Vector position, uint16_t color)
{
    ENGINE_LCD_PIXEL(position.x, position.y, color);
}

void Draw::pixel(uint16_t x, uint16_t y, uint16_t color)
{
    ENGINE_LCD_PIXEL(x, y, color);
}

void Draw::polygon(uint16_t x[], uint16_t y[], int count, uint16_t color)
{
    ENGINE_LCD_POLYGON(x, y, count, color);
}

void Draw::rectangle(Vector position, Vector size, uint16_t color)
{
    ENGINE_LCD_RECTANGLE(position.x, position.y, size.x, size.y, color);
}

void Draw::rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    ENGINE_LCD_RECTANGLE(x, y, width, height, color);
}

void Draw::setColor(uint16_t color)
{
    currentTextColor = color;
}

void Draw::setFont(ENGINE_FONT_SIZE size)
{
    currentFontSize = size;
}

void Draw::swap()
{
#ifdef ENGINE_LCD_SWAP
    ENGINE_LCD_SWAP();
#endif
}

void Draw::text(Vector position, const char *text)
{
    ENGINE_LCD_TEXT(position.x, position.y, text, currentTextColor, currentFontSize);
}

void Draw::text(uint16_t x, uint16_t y, const char *text)
{
    ENGINE_LCD_TEXT(x, y, text, currentTextColor, currentFontSize);
}

void Draw::text(Vector position, const char *text, uint16_t color, ENGINE_FONT_SIZE font)
{
    ENGINE_LCD_TEXT(position.x, position.y, text, color, font);
}

void Draw::text(uint16_t x, uint16_t y, const char *text, uint16_t color, ENGINE_FONT_SIZE font)
{
    ENGINE_LCD_TEXT(x, y, text, color, font);
}

void Draw::triangle(Vector p1, Vector p2, Vector p3, uint16_t color)
{
    ENGINE_LCD_TRIANGLE(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color);
}

void Draw::triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
    ENGINE_LCD_TRIANGLE(x1, y1, x2, y2, x3, y3, color);
}
void Draw::imageMasked(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t width, uint16_t height, const uint8_t *mask)
{
    ENGINE_LCD_BLIT_MASK_BMP(x, y, width, height, bitmap, mask);
}

static int imageEdge(double edge, int limit)
{
    if (edge <= 0.5) return 0;
    if (edge >= double(limit) + 0.5) return limit;
    const double center = edge - 0.5;
    const int pixel = int(center);
    return pixel + (center > pixel);
}

void Draw::imageMasked(float x, float y, const uint8_t *bitmap, float width, float height, const uint8_t *mask,
                       uint16_t sourceWidth, uint16_t sourceHeight)
{
    if (!bitmap || !sourceWidth || !sourceHeight || !isfinite(x) || !isfinite(y) ||
        !isfinite(width) || !isfinite(height) || width <= 0 || height <= 0)
        return;
    const Vector screen = getDisplaySize();
    const int x0 = imageEdge(x, int(screen.x)), y0 = imageEdge(y, int(screen.y));
    const int x1 = imageEdge(double(x) + width, int(screen.x)), y1 = imageEdge(double(y) + height, int(screen.y));
    if (x0 >= x1 || y0 >= y1) return;
    if (width == sourceWidth && height == sourceHeight && x >= -32768 && x <= 32767 &&
        y >= -32768 && y <= 32767 && int(x) == x && int(y) == y && mask)
    {
        imageMasked(int16_t(x), int16_t(y), bitmap, sourceWidth, sourceHeight, mask);
        return;
    }
    uint8_t pixels[64], bits[8];
    for (int tile = x0; tile < x1; tile += 64)
    {
        const int count = x1 - tile < 64 ? x1 - tile : 64;
        uint16_t columns[64];
        for (int i = 0; i < count; ++i)
        {
            const size_t sx = size_t((tile + i + 0.5 - double(x)) * sourceWidth / width);
            columns[i] = uint16_t(sx < sourceWidth ? sx : sourceWidth - 1);
        }
        size_t previous = size_t(-1);
        for (int row = y0; row < y1; ++row)
        {
            size_t sy = size_t((row + 0.5 - double(y)) * sourceHeight / height);
            if (sy >= sourceHeight) sy = sourceHeight - 1;
            if (sy != previous)
            {
                previous = sy;
                memset(bits, 0, sizeof(bits));
                for (int i = 0; i < count; ++i)
                {
                    const size_t index = sy * sourceWidth + columns[i];
                    pixels[i] = bitmap[index];
                    if (!mask || (mask[index >> 3] & (0x80u >> (index & 7))))
                        bits[i >> 3] |= 0x80u >> (i & 7);
                }
            }
            imageMasked(int16_t(tile), int16_t(row), pixels, uint16_t(count), 1, bits);
        }
    }
}

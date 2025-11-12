#ifndef EPD_H
#define EPD_H

/**
 * @file EPDController.h
 * Display controller and drawing utilities for GXUI.
 *
 * Wraps the GxEPD2 4-gray display, provides theme handling (light/dark),
 * drawing helpers (patterns, borders, scaled bitmaps), and a singleton
 * instance for convenient access across the UI.
 */

#include <GxEPD2_4G_BW.h>
#include <gdey/GxEPD2_750_GDEY075T7.h>
#include <Preferences.h>
#include <SPI.h>

#include "../../../include/fonts/fonts.h"

#define DISPLAY_THEME_KEY "display_theme"

namespace EPD {
    class Controller {
    public:
        // Public method to get the singleton instance of the class
        static Controller &getInstance() {
            static Controller instance; // Singleton instance, created once
            return instance;
        }

        // Initialize the display
        bool init(
            Preferences *prefs = nullptr,
            const bool fullInit = false
        ) {
            preferences = prefs;

            displayTheme = preferences->getUInt(
                               DISPLAY_THEME_KEY,
                               static_cast<uint32_t>(DisplayTheme::LIGHT)
                           )
                           == static_cast<uint32_t>(DisplayTheme::LIGHT)
                               ? DisplayTheme::LIGHT
                               : DisplayTheme::DARK;

            SPI.end();
            SPI.begin(46, -1, 47, -1);
            display.init(
                115200,
                fullInit
            );
            display.setRotation(
                3
            );
            display.setFont(
                &FreeMono18pt7b
            );
            return true;
        }


        [[nodiscard]] GxEPD2_4G_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT> &getDisplay() {
            return display;
        }

        // helper methods

        enum class DisplayTheme {
            LIGHT = 0,
            DARK = 1
        };

        [[nodiscard]] DisplayTheme getDisplayTheme() const {
            return displayTheme;
        }

        void setDisplayTheme(DisplayTheme mode) {
            displayTheme = mode;
            if (preferences != nullptr) {
                preferences->putUInt(DISPLAY_THEME_KEY, static_cast<uint32_t>(mode));
            }
        }

        [[nodiscard]] uint16_t getPrimaryColor(const bool inverted = false) const {
            return displayTheme ==
                   (inverted ? DisplayTheme::DARK : DisplayTheme::LIGHT)
                       ? GxEPD_BLACK
                       : GxEPD_WHITE;
        }

        void drawMultiRectBorder(
            const int16_t x,
            const int16_t y,
            const int16_t width,
            const int16_t height,
            const uint16_t color = GxEPD_BLACK,
            const int16_t loops = 3,
            const int16_t gap = 2,
            const int16_t gapMulti = 2
        ) {
            for (int i = 1; i <= loops; i++) {
                display.drawRect(
                    x + i * gap,
                    y + i * gap,
                    width - i * (gap * gapMulti),
                    height - i * (gap * gapMulti),
                    color
                );
            }
        }

        void drawMultiRoundRectBorder(
            const int16_t x,
            const int16_t y,
            const int16_t width,
            const int16_t height,
            const uint16_t color = GxEPD_BLACK,
            const int16_t loops = 3,
            const int16_t gap = 2,
            const int16_t gapMulti = 2,
            const int16_t radius = 4
        ) {
            for (int i = 1; i <= loops; i++) {
                display.drawRoundRect(
                    x + i * gap,
                    y + i * gap,
                    width - i * (gap * gapMulti),
                    height - i * (gap * gapMulti),
                    radius,
                    color
                );
            }
        }

        void drawScaledBitmap(
            const int x,
            const int y,
            const unsigned char *bitmap,
            const int srcWidth,
            const int srcHeight,
            const int targetWidth,
            const int targetHeight,
            const uint16_t color = GxEPD_BLACK
        ) {
            // Compute scaling ratios.
            float scaleX = static_cast<float>(srcWidth) / targetWidth;
            float scaleY = static_cast<float>(srcHeight) / targetHeight;
            // For each target pixel, average over corresponding source region.
            for (int ty = 0; ty < targetHeight; ty++) {
                int sy_start = static_cast<int>(ty * scaleY);
                int sy_end = static_cast<int>((ty + 1) * scaleY);
                if (sy_end > srcHeight) sy_end = srcHeight;
                for (int tx = 0; tx < targetWidth; tx++) {
                    int sx_start = static_cast<int>(tx * scaleX);
                    int sx_end = static_cast<int>((tx + 1) * scaleX);
                    if (sx_end > srcWidth) sx_end = srcWidth;
                    int regionPixels = (sy_end - sy_start) * (sx_end - sx_start);
                    int count = 0;
                    // Accumulate count for the source region.
                    for (int sy = sy_start; sy < sy_end; sy++) {
                        for (int sx = sx_start; sx < sx_end; sx++) {
                            int byteIndex = sy * ((srcWidth + 7) / 8) + (sx / 8);
                            int bitPos = 7 - (sx % 8);
                            if (bitmap[byteIndex] & (1 << bitPos)) {
                                count++;
                            }
                        }
                    }
                    // Determine if the majority of the region is "on".
                    bool pixelOn = (count > (regionPixels / 2));
                    if (pixelOn) {
                        display.drawPixel(x + tx, y + ty, color);
                    }
                }
            }
        }

        // following helper functions by Rukenshia/pomodoro :) too good not to use
        enum class Pattern {
            SOLID,
            STRIPES,
            DOTS,
            CHECKERBOARD,
            DIAGONAL_STRIPES,
            CROSS_HATCH,
            SPARSE_DOTS,
            VERY_SPARSE_DOTS
        };

        struct Bounds {
            int16_t x{};
            int16_t y{};
            uint16_t w{};
            uint16_t h{};
        };

        void drawPattern(Pattern pattern, int16_t x, int16_t y, int16_t w, int16_t h) {
            const uint8_t *patternData = patterns[(int) pattern];
            for (int16_t i = 0; i < h; i++) {
                for (int16_t j = 0; j < w; j++) {
                    if (patternData[i % 8] & (0x80 >> (j % 8))) {
                        display.drawPixel(x + j, y + i, getPrimaryColor());
                    }
                }
            }
        }

        void drawPatternInRoundedArea(
            Pattern patternNo,
            int16_t startX,
            int16_t startY,
            int16_t areaWidth,
            int16_t areaHeight,
            int16_t radius
        ) {
            const int16_t patternWidth = 8;
            const int16_t patternHeight = 8;
            const uint8_t *pattern = patterns[(int) patternNo];

            // Pre-calculate squared radius for circle tests.
            const int16_t rSq = radius * radius;

            // Define centers for the four corners.
            const int16_t centerTL_x = startX + radius;
            const int16_t centerTL_y = startY + radius;
            const int16_t centerTR_x = startX + areaWidth - radius - 1;
            const int16_t centerTR_y = startY + radius;
            const int16_t centerBL_x = startX + radius;
            const int16_t centerBL_y = startY + areaHeight - radius - 1;
            const int16_t centerBR_x = startX + areaWidth - radius - 1;
            const int16_t centerBR_y = startY + areaHeight - radius - 1;

            // Loop through every pixel in the defined area.
            for (int16_t y = startY; y < startY + areaHeight; y++) {
                for (int16_t x = startX; x < startX + areaWidth; x++) {
                    bool drawPixelFlag = true;
                    // Top-left corner
                    if (x < startX + radius && y < startY + radius) {
                        int16_t dx = centerTL_x - x;
                        int16_t dy = centerTL_y - y;
                        if ((dx * dx + dy * dy) > rSq)
                            drawPixelFlag = false;
                    }
                    // Top-right corner
                    else if (x >= startX + areaWidth - radius && y < startY + radius) {
                        int16_t dx = x - centerTR_x;
                        int16_t dy = centerTR_y - y;
                        if ((dx * dx + dy * dy) > rSq)
                            drawPixelFlag = false;
                    }
                    // Bottom-left corner
                    else if (x < startX + radius && y >= startY + areaHeight - radius) {
                        int16_t dx = centerBL_x - x;
                        int16_t dy = y - centerBL_y;
                        if ((dx * dx + dy * dy) > rSq)
                            drawPixelFlag = false;
                    }
                    // Bottom-right corner
                    else if (x >= startX + areaWidth - radius && y >= startY + areaHeight - radius) {
                        int16_t dx = x - centerBR_x;
                        int16_t dy = y - centerBR_y;
                        if ((dx * dx + dy * dy) > rSq)
                            drawPixelFlag = false;
                    }

                    if (drawPixelFlag) {
                        // Determine pattern coordinates.
                        int patternX = (x - startX) % patternWidth;
                        int patternY = (y - startY) % patternHeight;
                        // Check the bit (assumes MSB first in each byte).
                        if (pattern[patternY] & (0x80 >> patternX)) {
                            display.drawPixel(x, y, getPrimaryColor());
                        }
                    }
                }
            }
        }

        Bounds getBounds(const char *text, const GFXfont *font) {
            display.setTextSize(1);
            display.setFont(font);

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

            return {x1, y1, w, h};
        }

        Bounds drawText(
            const char *text,
            int16_t x,
            int16_t y,
            const GFXfont *font,
            uint16_t color
        ) {
            display.setTextSize(1);
            display.setFont(font);
            display.setTextColor(color);

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
            display.setCursor(x, y);
            display.print(text);

            return {x, y, w, h};
        }

        Bounds drawBottomAlignedText(
            const char *text,
            int16_t x,
            int16_t y,
            const GFXfont *font,
            uint16_t color
        ) {
            display.setTextSize(1);
            display.setFont(font);
            display.setTextColor(color);

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
            display.setCursor(x, y + h);
            display.print(text);

            return {static_cast<int16_t>(x), static_cast<int16_t>(y - h), w, h};
        }

        Bounds drawCenteredText(
            const char *text,
            int16_t x,
            int16_t y,
            const GFXfont *font,
            uint16_t color
        ) {
            display.setTextSize(1);
            display.setFont(font);
            display.setTextColor(color);

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
            // Correct the y coordinate considering y1 offset (usually negative)
            int16_t correctedY = y - h / 2 - y1;

            display.setCursor(x - w / 2, correctedY);
            display.print(text);

            return {static_cast<int16_t>(x - w / 2), correctedY, w, h};
        }

    private:
        // define 8x8 patterns
        const uint8_t pattern_solid[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        // refined stripes: wider bands (upper half and lower half)
        const uint8_t pattern_stripes[8] = {0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F};
        // refined dots: softer dot effect
        const uint8_t pattern_dots[8] = {0x88, 0x44, 0x22, 0x11, 0x11, 0x22, 0x44, 0x88};
        // new sparse dots pattern: dots further apart
        const uint8_t pattern_sparse_dots[8] = {0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00};
        // new very sparse dots pattern: dots very far apart
        const uint8_t pattern_very_sparse_dots[8] = {0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00};
        // refined checkerboard: standard alternating bits
        const uint8_t pattern_checkerboard[8] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
        // new diagonal stripes: repeated diagonal bands
        const uint8_t pattern_diagonal_stripes[8] = {0xC0, 0x30, 0x0C, 0x03, 0xC0, 0x30, 0x0C, 0x03};
        // new crosshatch: grid-like pattern with full horizontal bars on top, middle, and bottom
        const uint8_t pattern_crosshatch[8] = {0xFF, 0x92, 0x92, 0x92, 0xFF, 0x92, 0x92, 0xFF};

        // put all patterns in an array
        const std::vector<const uint8_t *> patterns = {
            pattern_solid,
            pattern_stripes,
            pattern_dots,
            pattern_checkerboard,
            pattern_diagonal_stripes,
            pattern_crosshatch,
            pattern_sparse_dots,
            pattern_very_sparse_dots
        };

        // Private constructor to restrict instantiation
        Controller()
            : display(
                GxEPD2_750_GDEY075T7(
                    45,
                    21,
                    9,
                    11
                )
            ) {
        }

        // Delete copy constructor and assignment operator to prevent copying
        Controller(
            const Controller &



        ) = delete;

        Controller &operator=(
            const Controller &



        ) = delete;

        // Member variable for the display
        GxEPD2_4G_BW<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT> display;

        // New preferences pointer for settings
        Preferences *preferences;

        // Display mode setting
        DisplayTheme displayTheme;
    };
}


#endif //EPD_H

#ifndef EPDICON_H
#define EPDICON_H
/**
 * @file EPDIcon.h
 * Icon rendering helpers for GXUI.
 *
 * Provides:
 * - EPD::IconRenderContext: extends RenderContext with a color
 * - EPD::Icon: draws a bitmap at a given size and color
 */
#include <EPDRenderable.h>

namespace EPD {
    struct IconRenderContext final : RenderContext {
        mutable uint16_t color = GxEPD_BLACK;

        explicit IconRenderContext(
            const int x = 0,
            const int y = 0,
            const int size = 0,
            const uint16_t color = GxEPD_BLACK
        ) : RenderContext(x, y, size, size), color(color) {
        }

        explicit IconRenderContext(
            const int x = 0,
            const int y = 0,
            const int width = 0,
            const int height = 0,
            const uint16_t color = GxEPD_BLACK
        ) : RenderContext(x, y, width, height), color(color) {
        }
    };

    class Icon final : public Renderable {
        std::pair<int, int> size = {100, 100};
        const unsigned char *bitmap = nullptr;

    protected:
        void renderContent(Controller &epd, const RenderContext &ctx) override {
            const auto &iconCtx = static_cast<const IconRenderContext &>(ctx);

            if (iconCtx.width == 0 && iconCtx.height == 0) {
                iconCtx.width = size.first;
                iconCtx.height = size.second;
            }
            if (iconCtx.width != 0 & iconCtx.height == 0) {
                iconCtx.height = iconCtx.width;
            } else if (iconCtx.height != 0 & iconCtx.width == 0) {
                iconCtx.width = iconCtx.height;
            }

            epd.drawScaledBitmap(
                iconCtx.x,
                iconCtx.y,
                getBitmap(),
                size.first,
                size.second,
                iconCtx.width,
                iconCtx.height,
                iconCtx.color
            );
        }

    public:
        Icon() = default;

        Icon(const std::pair<int, int> &size, const unsigned char *bitmapData): size(size), bitmap(bitmapData) {
        }

        [[nodiscard]] std::pair<int, int> getSize() const {
            return size;
        }

        [[nodiscard]] const unsigned char *getBitmap() const {
            return bitmap;
        }
    };
}
#endif //EPDICON_H

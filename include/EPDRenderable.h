#pragma once
/**
 * @file EPDRenderable.h
 * Lightweight rendering base types for the GXUI framework.
 *
 * - EPD::RenderContext captures a rectangular drawing area for a widget.
 * - EPD::Renderable is the abstract base that provides the render pipeline.
 */
namespace EPD {
    class Controller;

    /**
     * Drawing window and size passed to renderers. Coordinates and dimensions
     * are mutable to allow layout code to snap or adjust during rendering.
     */
    struct RenderContext {
        mutable int x{0};     ///< top-left x
        mutable int y{0};     ///< top-left y

        mutable int width{0}; ///< width in pixels
        mutable int height{0};///< height in pixels

        explicit RenderContext(
            const int x = 0,
            const int y = 0,
            const int width = 0,
            const int height = 0
        ) : x(x), y(y), width(width), height(height) {
        }
    };

    /**
     * Base class for all renderable elements. Subclasses implement
     * renderContent and can rely on executeRender to capture the last
     * render context for later queries.
     */
    class Renderable {
    protected:
        /** Perform the actual drawing for the given context. */
        virtual void renderContent(Controller &epd, const RenderContext &ctx) = 0;

    public:
        /** Last render window, useful for hit testing or incremental redraws. */
        mutable RenderContext lastRenderCTX = RenderContext();

        virtual ~Renderable() = default;

        /** Template method to render and store the context used. */
        virtual void executeRender(Controller &epd, const RenderContext &ctx) {
            renderContent(epd, ctx);
            lastRenderCTX = ctx;
        }

        /** Retrieve the most recent window used to draw this element. */
        void getWindow(int *x, int *y, int *width, int *height) const {
            *x = lastRenderCTX.x;
            *y = lastRenderCTX.y;
            *width = lastRenderCTX.width;
            *height = lastRenderCTX.height;
        }
    };
}

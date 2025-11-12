#ifndef EPDCOMPONENT_H
#define EPDCOMPONENT_H

/**
 * @file EPDComponent.h
 * Common UI components built on top of the Renderable base.
 *
 * Defines lightweight widgets such as progress bars that render
 * directly to the e-paper display via EPD::Controller.
 */

namespace EPD {
    class Component : public Renderable {
    };

    class ComponentProgressBar : public Component {
        String label{};
        float progress; // 0.0 to 1.0
        bool printPercentage{false};

        static constexpr int PADDING = 12;
        static constexpr int BAR_HEIGHT = 24;
        static constexpr int BORDER_RADIUS = 4;

    public:
        ComponentProgressBar(const String &label, float progress, const bool printPercentage = false)
            : label(label), progress(progress), printPercentage(printPercentage) {
            this->progress = std::clamp(progress, 0.0f, 1.0f);
        }

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            auto &display = epd.getDisplay();
            display.setFont(&FreeMonoBold12pt7b);

            int16_t x1, y1;
            uint16_t labelW, labelH;
            display.getTextBounds(label, ctx.x, ctx.y, &x1, &y1, &labelW, &labelH);

            constexpr int BAR_WIDTH = 128;
            constexpr int PERCENTAGE_MARGIN = 8;

            ctx.x = (x1 + 7) & ~7;
            ctx.y = (y1 + 7) & ~7;
            ctx.width = (BAR_WIDTH + PADDING * 2 + 7) & ~7;
            ctx.height = (labelH + BAR_HEIGHT + PERCENTAGE_MARGIN + PADDING * 2 + 7) & ~7;

            // Draw label
            display.setTextColor(epd.getPrimaryColor());
            display.setCursor(ctx.x + PADDING, ctx.y + labelH + PADDING);
            display.print(label);

            // Draw progress bar
            const int barX = ctx.x + PADDING;
            const int barY = ctx.y + labelH + PADDING * 2;
            const int fillWidth = static_cast<int>(BAR_WIDTH * progress);

            // Draw border
            display.drawRoundRect(barX, barY, BAR_WIDTH, BAR_HEIGHT, BORDER_RADIUS, epd.getPrimaryColor());

            // Draw fill
            if (fillWidth > 0) {
                //display.fillRoundRect(barX, barY, fillWidth, BAR_HEIGHT, BORDER_RADIUS, GxEPD_BLACK);
                epd.drawPatternInRoundedArea(
                    Controller::Pattern::CHECKERBOARD,
                    barX,
                    barY,
                    fillWidth,
                    BAR_HEIGHT,
                    BORDER_RADIUS
                );
            }

            if (printPercentage) {
                // Draw percentage
                char percentage[8];
                snprintf(percentage, sizeof(percentage), "%d%%", static_cast<int>(progress * 100));
                int16_t percX1, percY1;
                uint16_t percW, percH;
                display.getTextBounds(percentage, 0, 0, &percX1, &percY1, &percW, &percH);
                display.setCursor(barX + (BAR_WIDTH - percW) / 2, barY + BAR_HEIGHT + PERCENTAGE_MARGIN + percH);
                display.print(percentage);
            }
        }
    };
}
#endif //EPDCOMPONENT_H

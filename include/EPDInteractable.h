#ifndef EPDINTERACTABLE_H
#define EPDINTERACTABLE_H

/**
 * @file EPDInteractable.h
 * Core types for user-interactable UI elements on GxEPD2 displays.
 *
 * This header defines:
 *  - EPD::InteractableType: the high-level kinds of widgets/pages
 *  - EPD::InteractableActions: a mix-in providing navigation/action hooks
 *  - EPD::Interactable: a base class combining rendering + interaction
 *
 * Rendering is handled via the Renderable base; interaction hooks can be
 * overridden per widget. Color helpers consider the current display theme
 * and an inversion flag to preserve contrast on monochrome e-paper.
 */

#include <EPDController.h>
#include <EPDIcon.h>

#include "EPDRenderable.h"

namespace EPD {
    /**
     * The high-level category of an interactable element. Used for routing,
     * styling, and input handling decisions in containers/managers.
     */
    enum class InteractableType {
        PAGE,    ///< A full-screen view
        MENU,    ///< A list or group of controls
        BUTTON,  ///< Clickable action
        SELECT,  ///< Choice selector (list, dropdown, etc.)
        TEXT,    ///< Static or editable text
        SLIDER,  ///< Continuous/discrete value control
        TOGGLE   ///< On/Off switch
    };

    /**
     * Mixin with navigation/action hooks invoked by input handling.
     * Override instance methods in widgets. Static variants can be used
     * as default callbacks or for global routing if desired.
     */
    class InteractableActions {
    public:
        virtual ~InteractableActions() = default;

        static void onActionUpStatic() {
        }

        static void onActionDownStatic() {
        }

        static void onActionLeftStatic() {
        }

        static void onActionRightStatic() {
        }

        static void onActionStatic() {
        }

        //

        virtual void onActionUp() {
        }

        virtual void onActionDown() {
        }

        virtual void onActionLeft() {
        }

        virtual void onActionRight() {
        }

        virtual void onAction() {
        }
    };

    /**
     * Base class for anything that can be rendered and interacted with.
     *
     * Responsibilities:
     *  - Expose selection/active/disabled state for containers to manage
     *  - Provide default visual hints for those states
     *  - Offer color helpers that respect the current display theme
     *  - Inherit interaction hooks from InteractableActions
     */
    class Interactable : public Renderable, public InteractableActions {
        /** Whether this element accepts input at all (disabled if false). */
        mutable bool isInteractable = true;
        /** Whether this element is currently selected (focused). */
        mutable bool isSelected = false;
        /** Whether this element is currently active (pressed/toggled/engaged). */
        mutable bool isActive = false;

        /** Foreground color for LIGHT theme (text/lines). */
        const uint16_t FOREGROUND_COLOR = GxEPD_WHITE;
        /** Background color for LIGHT theme (fill/background). */
        const uint16_t BACKGROUND_COLOR = GxEPD_BLACK;
        /** Invert foreground/background for better contrast or emphasis. */
        mutable bool isInvertedColors = false;

    protected:
        /** Optional identifier for lookups, automation, or analytics. */
        String identifier{};

        /**
         * Resolve the effective foreground color considering current theme and
         * the inversion flag. In DARK theme, colors are swapped relative to LIGHT.
         */
        [[nodiscard]] uint16_t getForegroundColor(
            const Controller::DisplayTheme currentTheme = Controller::getInstance().getDisplayTheme()
        ) const {
            switch (currentTheme) {
                case Controller::DisplayTheme::LIGHT:
                    if (isInvertedColors) {
                        return BACKGROUND_COLOR;
                    }
                    return FOREGROUND_COLOR;
                case Controller::DisplayTheme::DARK:
                    return getBackgroundColor(Controller::DisplayTheme::LIGHT);
            }
            return FOREGROUND_COLOR;
        }

        [[nodiscard]] uint16_t getBackgroundColor(
            const Controller::DisplayTheme currentTheme = Controller::getInstance().getDisplayTheme()
        ) const {
            switch (currentTheme) {
                case Controller::DisplayTheme::LIGHT:
                    if (isInvertedColors) {
                        return FOREGROUND_COLOR;
                    }
                    return BACKGROUND_COLOR;
                case Controller::DisplayTheme::DARK:
                    return getForegroundColor(Controller::DisplayTheme::LIGHT);
            }
            return BACKGROUND_COLOR;
        }


        /**
         * Visual helpers for common states. Containers may call these to render
         * selection/active/disabled hints consistently across widgets.
         */
        virtual void drawOnSelection(
            Controller &epd,
            const RenderContext &ctx,
            const int radius = 6
        ) {
            if (!isSelected) return;
            epd.drawPatternInRoundedArea(
                Controller::Pattern::SPARSE_DOTS,
                ctx.x,
                ctx.y,
                ctx.width,
                ctx.height,
                radius
            );

            epd.getDisplay().drawRoundRect(ctx.x, ctx.y, ctx.width, ctx.height, radius, getBackgroundColor());

        }

        virtual void drawOnActive(
            Controller &epd,
            const RenderContext &ctx,
            const int radius = 6
        ) {
            if (!isActive) return;
            epd.drawMultiRoundRectBorder(
                ctx.x,
                ctx.y,
                ctx.width,
                ctx.height,
                getBackgroundColor(),
                3,
                1,
                2,
                radius
            );
        }

        virtual void drawOnDisabled(
            Controller &epd,
            const RenderContext &ctx,
            const int radius = 6
        ) {
            if (!isInteractable) return;
            epd.drawPatternInRoundedArea(
                Controller::Pattern::DIAGONAL_STRIPES,
                ctx.x,
                ctx.y,
                ctx.width,
                ctx.height,
                radius
            );
        }

        // overridden renderer
        void renderContent(Controller &epd, const RenderContext &ctx) override {
            if (!getIsInteractable()) {
                drawOnDisabled(epd, ctx);
            } else if (getIsActive()) {
                drawOnActive(epd, ctx);
            } else if (getIsSelected()) {
                drawOnSelection(epd, ctx);
            }
        }

    public:
        ~Interactable() override = default;

        explicit Interactable() : Renderable() {
        }

        explicit Interactable(const String &id) : identifier(id) {
        }

        [[nodiscard]] const String &getId() const { return identifier; }

        [[nodiscard]] virtual InteractableType getType() const {
            return InteractableType::PAGE;
        }

        void select() const {
            isSelected = true;
        }

        void deselect() const {
            isSelected = false;
        }

        [[nodiscard]] bool getIsSelected() const {
            return isSelected;
        }

        void activate() const {
            isActive = true;
        }

        void deactivate() const {
            isActive = false;
        }

        [[nodiscard]] bool getIsActive() const {
            return isActive;
        }

        void enableInteraction() const {
            isInteractable = true;
        }

        void disableInteraction() const {
            isInteractable = false;
        }

        [[nodiscard]] bool getIsInteractable() const {
            return isInteractable;
        }

        void setInvertColors(const bool invert = true) const {
            isInvertedColors = invert;
        }

        [[nodiscard]] bool getsColorsInverted() const {
            return isInvertedColors;
        }
    };

    class InteractableButton : public Interactable {
        String label = "Button";
        std::function<void()> action{};
        Icon *icon{nullptr};

    public:
        explicit InteractableButton(
            const String &id,
            const String &label,
            std::function<void()> action
        ): Interactable(id),
           label(label),
           action(std::move(action)) {
        }

        explicit InteractableButton(
            const String &id,
            const String &label,
            Icon *icon,
            std::function<void()> action
        ): Interactable(id),
           label(label),
           action(std::move(action)),
           icon(icon) {
        }

        [[nodiscard]] InteractableType getType() const override {
            return InteractableType::BUTTON;
        }

        void onAction() override {
            action();
            //deactivate();
        }

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            auto &display = epd.getDisplay();
            display.setFont(&FreeMonoBold12pt7b);

            int16_t x1 = ctx.x, y1 = ctx.y;
            uint16_t w = ctx.width, h = ctx.height;
            if (w == 0 || h == 0) {
                display.getTextBounds(label, ctx.x, ctx.y, &x1, &y1, &w, &h);
            }

            constexpr int PADDING = 12;
            constexpr int BORDER_RADIUS = 8;

            ctx.x = (x1 - PADDING + 7) & ~7;
            ctx.y = (y1 - PADDING + 7) & ~7;
            ctx.height = (h + PADDING * 2 + 7) & ~7;

            const int ICON_SIZE = ctx.height - PADDING;
            if (icon != nullptr) {
                w += PADDING * 2 + ICON_SIZE;
            }

            ctx.width = (w + PADDING * 2 + 7) & ~7;

            uint16_t CALCULATED_COLOR = getBackgroundColor();

            if (getIsActive()) {
                constexpr int MARGIN = PADDING / 4;
                //display.fillRoundRect(ctx.x, ctx.y, ctx.width, ctx.height, BORDER_RADIUS, getBackgroundColor());
                epd.drawPatternInRoundedArea(
                    Controller::Pattern::CHECKERBOARD,
                    ctx.x,
                    ctx.y,
                    ctx.width,
                    ctx.height,
                    BORDER_RADIUS
                );
                epd.drawMultiRoundRectBorder(
                    ctx.x + MARGIN,
                    ctx.y + MARGIN,
                    ctx.width - MARGIN * 2,
                    ctx.height - MARGIN * 2,
                    getForegroundColor(),
                    3,
                    1,
                    2,
                    BORDER_RADIUS
                );
                CALCULATED_COLOR = getForegroundColor();

                deactivate();
            } else if (getIsSelected()) {
                display.drawRoundRect(ctx.x, ctx.y, ctx.width, ctx.height, BORDER_RADIUS, getBackgroundColor());
                CALCULATED_COLOR = getBackgroundColor();
            } else if (!getIsInteractable()) {
                epd.drawPatternInRoundedArea(
                    Controller::Pattern::DIAGONAL_STRIPES,
                    ctx.x,
                    ctx.y,
                    ctx.width,
                    ctx.height,
                    BORDER_RADIUS
                );
                CALCULATED_COLOR = getForegroundColor();
            } else {
                display.fillRoundRect(ctx.x, ctx.y, ctx.width, ctx.height, BORDER_RADIUS, getBackgroundColor());
                CALCULATED_COLOR = getForegroundColor();
            }

            display.setTextColor(CALCULATED_COLOR);

            const int16_t baselineY = ctx.y + (ctx.height + h) / 2;

            if (icon != nullptr) {
                icon->executeRender(
                    epd,
                    IconRenderContext(
                        ctx.x + PADDING,
                        ctx.y + PADDING / 2,
                        ICON_SIZE,
                        CALCULATED_COLOR
                    )
                );
                display.setCursor(ctx.x + PADDING * 2 + ICON_SIZE, baselineY);
            } else {
                display.setCursor(ctx.x + PADDING, baselineY);
            }

            display.print(label);
        }
    };

    template<typename ToggleToggleEnumType = int>
    struct ToggleOption {
        String label{};
        Icon *icon = nullptr;
        ToggleToggleEnumType enumValue{};

        // Constructor without enum value
        explicit ToggleOption(const String &label, Icon *icon = nullptr)
            : label(label), icon(icon) {
        }

        explicit ToggleOption(const String &label)
            : label(label) {
        }

        explicit ToggleOption(Icon *icon = nullptr)
            : icon(icon) {
        }

        // Constructor with enum value
        ToggleOption(const String &label, Icon *icon, ToggleToggleEnumType value)
            : label(label), icon(icon), enumValue(value) {
        }

        ToggleOption(Icon *icon, ToggleToggleEnumType value)
            : icon(icon), enumValue(value) {
        }

        ToggleOption(const String &label, ToggleToggleEnumType value)
            : label(label), enumValue(value) {
        }
    };

    template<typename ToggleEnumType = int>
    class InteractableToggle : public Interactable {
        String label{};
        std::vector<ToggleOption<ToggleEnumType> > options{};
        size_t *currentIndex;
        static constexpr int PADDING = 12;
        static constexpr int TOGGLE_WIDTH = 60;
        static constexpr int TOGGLE_HEIGHT = 30;
        static constexpr int BORDER_RADIUS = 8;

    public:
        InteractableToggle(
            const String &id,
            const String &label,
            const std::vector<ToggleOption<ToggleEnumType> > &options,
            size_t *currentIndex
        ) : Interactable(id),
            label(label),
            options(options),
            currentIndex(currentIndex) {
            // Ensure the index is valid
            if (*currentIndex >= options.size()) {
                *currentIndex = 0;
            }
        }

        [[nodiscard]] InteractableType getType() const override {
            return InteractableType::TOGGLE;
        }

        [[nodiscard]] ToggleEnumType getCurrentEnumValue() const {
            return options[*currentIndex].enumValue;
        }

        void onAction() override {
            *currentIndex = (*currentIndex + 1) % options.size();
            //activate();
        }

        void onActionLeft() override {
            if (*currentIndex > 0) {
                (*currentIndex)--;
            } else {
                *currentIndex = options.size() - 1;
            }
            activate();
        }

        void onActionRight() override {
            *currentIndex = (*currentIndex + 1) % options.size();
            activate();
        }

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            auto &display = epd.getDisplay();
            display.setFont(&FreeMonoBold12pt7b);

            int16_t x1 = ctx.x, y1 = ctx.y;
            uint16_t w = 0, h = 0;


            if (!label.isEmpty()) {
                display.getTextBounds(label, ctx.x, ctx.y, &x1, &y1, &w, &h);
            } else {
                // Get height of a standard character for spacing when no label
                display.getTextBounds("M", ctx.x, ctx.y, &x1, &y1, &w, &h);
                w = 0; // Reset width since we don't have a label
            }

            // Calculate needed width based on number of options
            const int actualToggleWidth = std::max(TOGGLE_WIDTH, static_cast<int>(options.size() * (TOGGLE_WIDTH / 2)));

            ctx.x = (x1 + 7) & ~7;
            ctx.y = (y1 - PADDING + 7) & ~7;
            ctx.width = (w + actualToggleWidth + PADDING * 3 + 7) & ~7;
            ctx.height = (h + PADDING * 2 + 7) & ~7;

            // Draw label
            display.setTextColor(getBackgroundColor());
            const int16_t baselineY = ctx.y + (ctx.height + h) / 2;
            display.setCursor(ctx.x + PADDING, baselineY);
            display.print(label);

            // Draw toggle
            const int toggleX = ctx.x + ctx.width - actualToggleWidth - PADDING;
            const int toggleCenterY = baselineY - h / 2 - TOGGLE_HEIGHT / 4;

            // Draw toggle outline
            if (getIsActive() || getIsSelected()) {
                epd.drawMultiRoundRectBorder(
                    toggleX,
                    toggleCenterY,
                    actualToggleWidth,
                    TOGGLE_HEIGHT,
                    getBackgroundColor(),
                    2,
                    1,
                    2,
                    BORDER_RADIUS / 2
                );
            } else {
                display.drawRoundRect(
                    toggleX,
                    toggleCenterY,
                    actualToggleWidth,
                    TOGGLE_HEIGHT,
                    BORDER_RADIUS,
                    getBackgroundColor()
                );
            }

            // Calculate segment width
            const int segmentWidth = actualToggleWidth / options.size();

            // Draw each segment
            for (size_t i = 0; i < options.size(); i++) {
                const int segmentX = toggleX + i * segmentWidth;

                // Fill current segment
                if (i == *currentIndex) {
                    if (i == 0) {
                        // Left edge - round left corners
                        display.fillRoundRect(
                            segmentX,
                            toggleCenterY,
                            segmentWidth,
                            TOGGLE_HEIGHT,
                            BORDER_RADIUS,
                            getBackgroundColor()
                        );
                    } else if (i == options.size() - 1) {
                        // Right edge - round right corners
                        display.fillRoundRect(
                            segmentX,
                            toggleCenterY,
                            segmentWidth,
                            TOGGLE_HEIGHT,
                            BORDER_RADIUS,
                            getBackgroundColor()
                        );
                    } else {
                        // Middle segments - no rounded corners
                        display.fillRect(
                            segmentX,
                            toggleCenterY,
                            segmentWidth,
                            TOGGLE_HEIGHT,
                            getBackgroundColor()
                        );
                    }
                }

                // Use icon if available, otherwise use first letter of label
                if (Icon *icon = options[i].icon; options[i].icon != nullptr) {
                    // Use bitmap icon
                    const int iconCenterX = segmentX + segmentWidth / 2;
                    const int iconCenterY = toggleCenterY + TOGGLE_HEIGHT / 2;
                    icon->executeRender(
                        epd,
                        IconRenderContext(
                            iconCenterX - 10,
                            iconCenterY - 10,
                            20,
                            i == *currentIndex ? getForegroundColor() : getBackgroundColor()
                        )
                    );
                    continue;
                }

                String displayText;

                if (!options[i].label.isEmpty()) {
                    displayText = options[i].label.substring(0, 1);
                } else {
                    displayText = String(i);
                }

                int16_t textX, textY;
                uint16_t textW, textH;
                display.getTextBounds(displayText, 0, 0, &textX, &textY, &textW, &textH);

                display.setTextColor(i == *currentIndex ? getForegroundColor() : getBackgroundColor());
                display.setCursor(
                    segmentX + (segmentWidth - textW) / 2,
                    toggleCenterY + (TOGGLE_HEIGHT + textH) / 2 - 2
                );
                display.print(displayText);
            }

            display.setTextColor(getBackgroundColor());
        }
    };

    class InteractableSlider : public Interactable {
        String label{};
        int *value;
        int min;
        int max;
        int step;

    public:
        InteractableSlider(
            const String &id,
            const String &label,
            int *value,
            const int min,
            const int max,
            const int step = 1
        )
            : Interactable(id), label(label), value(value), min(min), max(max), step(step) {
        }

        [[nodiscard]] InteractableType getType() const override {
            return InteractableType::SLIDER;
        }

        void onActionLeft() override {
            *value = std::max(min, *value - step);
            activate();
        }

        void onActionRight() override {
            *value = std::min(max, *value + step);
            activate();
        }

        void onAction() override {
            if (getIsActive()) {
                deactivate();
            } else {
                activate();
            }
        }


        void renderContent(Controller &epd, const RenderContext &ctx) override {
            auto &display = epd.getDisplay();
            display.setFont(&FreeMonoBold12pt7b);

            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(label, ctx.x, ctx.y, &x1, &y1, &w, &h);

            constexpr int PADDING = 12;
            constexpr int SLIDER_WIDTH = 128;
            constexpr int SLIDER_HEIGHT = 24;
            constexpr int KNOB_WIDTH = 16;
            constexpr int VALUE_MARGIN = 8;
            constexpr int BORDER_RADIUS = 4;

            ctx.x = (x1 + 7) & ~7;
            ctx.y = (y1 - PADDING + 7) & ~7;
            ctx.width = (w + SLIDER_WIDTH + PADDING * 3 + 7) & ~7;
            ctx.height = (h + PADDING * 2 + VALUE_MARGIN + 24 + 7) & ~7;

            // Draw label
            display.setTextColor(getBackgroundColor());
            const int16_t baselineY = ctx.y + (ctx.height + h) / 2 - VALUE_MARGIN - 12;
            display.setCursor(ctx.x + PADDING, baselineY);
            display.print(label);

            // Draw slider track
            const int sliderX = ctx.x + ctx.width - SLIDER_WIDTH - PADDING;
            const int sliderY = ctx.y + (ctx.height - SLIDER_HEIGHT) / 2 - VALUE_MARGIN - 12;

            display.drawRoundRect(
                sliderX,
                sliderY + SLIDER_HEIGHT / 3,
                SLIDER_WIDTH,
                SLIDER_HEIGHT / 3,
                BORDER_RADIUS / 2,
                getBackgroundColor()
            );

            // Draw knob
            const float progress = static_cast<float>(*value - min) / (max - min);
            const int knobX = sliderX + (SLIDER_WIDTH - KNOB_WIDTH) * progress;

            if (getIsActive() || getIsSelected()) {
                if (getIsActive()) {
                    constexpr int OFFSET = 3;
                    display.drawRoundRect(
                        knobX - OFFSET,
                        sliderY - OFFSET,
                        KNOB_WIDTH + OFFSET * 2,
                        SLIDER_HEIGHT + OFFSET * 2,
                        BORDER_RADIUS,
                        getBackgroundColor()
                    );
                }
                display.fillRoundRect(knobX, sliderY, KNOB_WIDTH, SLIDER_HEIGHT, BORDER_RADIUS, getBackgroundColor());
            } else {
                display.fillRoundRect(knobX, sliderY, KNOB_WIDTH, SLIDER_HEIGHT, BORDER_RADIUS, getForegroundColor());
                display.drawRoundRect(knobX, sliderY, KNOB_WIDTH, SLIDER_HEIGHT, BORDER_RADIUS, getBackgroundColor());
            }

            // Draw value indicators
            display.setFont(&FreeMono12pt7b);
            char valueStr[8];

            // Min value
            snprintf(valueStr, sizeof(valueStr), "%d", min);
            display.setCursor(sliderX, sliderY + SLIDER_HEIGHT + VALUE_MARGIN + 12);
            display.print(valueStr);

            // Current value
            snprintf(valueStr, sizeof(valueStr), "%d", *value);
            int16_t currentX1, currentY1;
            uint16_t currentW, currentH;
            display.getTextBounds(valueStr, 0, 0, &currentX1, &currentY1, &currentW, &currentH);
            display.setCursor(sliderX + (SLIDER_WIDTH - currentW) / 2, sliderY + SLIDER_HEIGHT + VALUE_MARGIN + 12);
            display.print(valueStr);

            // Max value
            snprintf(valueStr, sizeof(valueStr), "%d", max);
            int16_t maxX1, maxY1;
            uint16_t maxW, maxH;
            display.getTextBounds(valueStr, 0, 0, &maxX1, &maxY1, &maxW, &maxH);
            display.setCursor(sliderX + SLIDER_WIDTH - maxW, sliderY + SLIDER_HEIGHT + VALUE_MARGIN + 12);
            display.print(valueStr);
        }
    };

    class InteractableDropdown : public Interactable {
        String label{};
        std::vector<String> options{};
        size_t *selectedIndex;
        bool isExpanded = false;
        static constexpr int MAX_VISIBLE_ITEMS = 5;

        //
        static constexpr int PADDING = 8;
        static constexpr int BORDER_RADIUS = 8;
        static constexpr int ITEM_HEIGHT = 40;
        static constexpr int ARROW_SIZE = 8;


        static constexpr int collapsedHeight = ITEM_HEIGHT + PADDING;

        int expandedHeight() const {
            return collapsedHeight +
                   std::min(MAX_VISIBLE_ITEMS, static_cast<int>(options.size())) * ITEM_HEIGHT;
        };

    public:
        InteractableDropdown(
            const String &id,
            const String &label,
            const std::vector<String> &options,
            size_t *selectedIndex
        )
            : Interactable(id), label(label), options(options), selectedIndex(selectedIndex) {
        }

        [[nodiscard]] InteractableType getType() const override {
            return InteractableType::SELECT;
        }

        void onAction() override {
            isExpanded = !isExpanded;

            if (!isExpanded) {
                deactivate();
            } else {
                activate();
            }

            lastRenderCTX.height = (isExpanded ? expandedHeight() : collapsedHeight + 7) & ~7;
        }

        void onActionUp() override {
            if (isExpanded && *selectedIndex > 0) {
                (*selectedIndex)--;
                activate();
            }
        }

        void onActionDown() override {
            if (isExpanded && *selectedIndex < options.size() - 1) {
                (*selectedIndex)++;
                activate();
            }
        }

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            auto &display = epd.getDisplay();
            display.setFont(&FreeMonoBold12pt7b);

            // Calculate dimensions
            int16_t x1, y1;
            uint16_t labelW, labelH;
            display.getTextBounds(label, ctx.x, ctx.y, &x1, &y1, &labelW, &labelH);

            uint16_t maxOptionWidth = 0;
            for (const auto &option: options) {
                uint16_t w, h;
                display.getTextBounds(option, 0, 0, &x1, &y1, &w, &h);
                maxOptionWidth = std::max(maxOptionWidth, w);
            }

            const int totalWidth = std::max(labelW, maxOptionWidth) + PADDING * 5;

            // Set the context dimensions
            ctx.width = (totalWidth + PADDING + 7) & ~7;
            ctx.height = (isExpanded ? expandedHeight() : collapsedHeight + 7) & ~7;

            // Draw main container
            const int baseX = ctx.x + PADDING;
            const int baseY = ctx.y;

            display.fillRoundRect(
                baseX,
                baseY,
                totalWidth,
                isExpanded ? expandedHeight() : collapsedHeight,
                BORDER_RADIUS,
                getForegroundColor()
            );

            display.drawRoundRect(
                baseX,
                baseY,
                totalWidth,
                isExpanded ? expandedHeight() : collapsedHeight,
                BORDER_RADIUS,
                getBackgroundColor()
            );

            // Draw label
            /*display.setTextColor(getBackgroundColor());
            display.setCursor(baseX + PADDING, baseY + ITEM_HEIGHT - PADDING / 2);
            display.print(label);*/

            // Draw selected value and arrow
            display.setTextColor(getBackgroundColor());
            const int valueY = baseY + ITEM_HEIGHT - PADDING - (PADDING / 2);
            display.setCursor(baseX + PADDING, valueY);
            display.print(options[*selectedIndex]);

            // Draw arrow
            const int arrowX = baseX + totalWidth - PADDING - ARROW_SIZE;
            const int arrowY = baseY + ITEM_HEIGHT / 2;
            if (isExpanded) {
                // Up arrow
                display.drawTriangle(
                    arrowX,
                    arrowY + ARROW_SIZE / 2,
                    arrowX + ARROW_SIZE,
                    arrowY + ARROW_SIZE / 2,
                    arrowX + ARROW_SIZE / 2,
                    arrowY - ARROW_SIZE / 2,
                    getBackgroundColor()
                );
            } else {
                // Down arrow
                display.drawTriangle(
                    arrowX,
                    arrowY - ARROW_SIZE / 2,
                    arrowX + ARROW_SIZE,
                    arrowY - ARROW_SIZE / 2,
                    arrowX + ARROW_SIZE / 2,
                    arrowY + ARROW_SIZE / 2,
                    getBackgroundColor()
                );
            }

            // Draw options when expanded
            if (isExpanded) {
                const int start = std::max(0, static_cast<int>(*selectedIndex) - MAX_VISIBLE_ITEMS / 2);
                const int end = std::min(static_cast<int>(options.size()), start + MAX_VISIBLE_ITEMS);

                for (int i = start; i < end; i++) {
                    const int itemY = baseY + ITEM_HEIGHT * (i - start + 1);
                    if (i == *selectedIndex) {
                        display.fillRoundRect(
                            baseX,
                            itemY,
                            totalWidth,
                            ITEM_HEIGHT,
                            BORDER_RADIUS,
                            getBackgroundColor()
                        );
                        display.setTextColor(getForegroundColor());
                    } else {
                        display.setTextColor(getBackgroundColor());
                    }
                    display.setCursor(baseX + PADDING, itemY + ITEM_HEIGHT - PADDING - (PADDING / 2));
                    display.print(options[i]);
                }
            }

            if (getIsSelected() && !isExpanded) {
                epd.drawMultiRoundRectBorder(
                    baseX,
                    baseY,
                    totalWidth,
                    collapsedHeight,
                    getBackgroundColor(),
                    2,
                    1,
                    2,
                    BORDER_RADIUS
                );
            }
        }
    };

    class InteractableTextInput : public Interactable {
        String label{};
        String *value{};
        static constexpr int MAX_LENGTH = 32;
        static constexpr int PADDING = 12;
        static constexpr int INPUT_HEIGHT = 40;
        static constexpr int BORDER_RADIUS = 8;
        size_t cursorPos = 0;
        static constexpr char VALID_CHARS[] =
                " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,!?-_@#$%&";
        size_t currentCharIndex = 0;
        bool isEditing = false;

    public:
        InteractableTextInput(const String &id, const String &label, String *value)
            : Interactable(id), label(label), value(value) {
            cursorPos = value->length();
            while (value->length() < MAX_LENGTH) {
                value->concat(' ');
            }
        }

        [[nodiscard]] InteractableType getType() const override {
            return InteractableType::TEXT;
        }

        void onAction() override {
            if (isEditing) {
                // Confirm character selection and exit editing mode
                value->setCharAt(cursorPos, VALID_CHARS[currentCharIndex]);
                isEditing = false;
                deactivate();
            } else if (getIsActive()) {
                // Exit active mode entirely
                deactivate();
            } else {
                // Start editing current character
                isEditing = true;
                currentCharIndex = 0;
                for (size_t i = 0; i < strlen(VALID_CHARS); i++) {
                    if (VALID_CHARS[i] == value->charAt(cursorPos)) {
                        currentCharIndex = i;
                        break;
                    }
                }
                activate();
            }
        }

        void onActionLeft() override {
            if (isEditing) {
                // Cycle through characters
                if (currentCharIndex > 0) {
                    currentCharIndex--;
                } else {
                    currentCharIndex = strlen(VALID_CHARS) - 1;
                }
            } else {
                // Move cursor left
                if (cursorPos > 0) {
                    cursorPos--;
                }
            }
            activate();
        }

        void onActionRight() override {
            if (isEditing) {
                // Cycle through characters
                if (currentCharIndex < strlen(VALID_CHARS) - 1) {
                    currentCharIndex++;
                } else {
                    currentCharIndex = 0;
                }
            } else {
                // Move cursor right
                if (cursorPos < value->length() - 1) {
                    cursorPos++;
                }
            }
            activate();
        }

        void onActionDown() override {
            if (!isEditing) {
                // Replace current character with space
                value->setCharAt(cursorPos, ' ');
                activate();
            }
        }

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            auto &display = epd.getDisplay();
            display.setFont(&FreeMonoBold12pt7b);

            int16_t x1, y1;
            uint16_t labelW, labelH;
            display.getTextBounds(label, ctx.x, ctx.y, &x1, &y1, &labelW, &labelH);

            constexpr int INPUT_WIDTH = 200;
            ctx.x = (x1 + 7) & ~7;
            ctx.y = (y1 + 7) & ~7;
            ctx.width = (INPUT_WIDTH + PADDING * 2 + 7) & ~7;
            ctx.height = (labelH + INPUT_HEIGHT + PADDING * 3 + 7) & ~7;

            // Draw label
            display.setTextColor(getBackgroundColor());
            display.setCursor(ctx.x + PADDING, ctx.y + labelH);
            display.print(label);

            // Draw input box
            const int inputX = ctx.x + PADDING;
            const int inputY = ctx.y + labelH + PADDING;

            if (getIsSelected()) {
                epd.drawMultiRoundRectBorder(
                    inputX,
                    inputY,
                    INPUT_WIDTH,
                    INPUT_HEIGHT,
                    getBackgroundColor(),
                    2,
                    1,
                    2,
                    BORDER_RADIUS
                );
            } else {
                display.drawRoundRect(inputX, inputY, INPUT_WIDTH, INPUT_HEIGHT, BORDER_RADIUS, getBackgroundColor());
            }

            // Draw text
            display.setTextColor(getBackgroundColor());
            display.setCursor(inputX + PADDING, inputY + INPUT_HEIGHT - PADDING);
            display.print(*value);

            // Draw cursor or editing highlight
            const int cursorX = inputX + PADDING + getCursorXOffset(epd);
            if (isEditing) {
                // Highlight current character being edited
                display.fillRect(cursorX - 1, inputY + PADDING, 14, INPUT_HEIGHT - PADDING * 2, getBackgroundColor());
                display.setTextColor(getForegroundColor());
                display.setCursor(cursorX, inputY + INPUT_HEIGHT - PADDING);
                display.print(VALID_CHARS[currentCharIndex]);
            } else {
                // Show cursor position
                display.drawFastVLine(cursorX, inputY + PADDING, INPUT_HEIGHT - PADDING * 2, getBackgroundColor());
            }
        }

    private:
        int getCursorXOffset(Controller &epd) const {
            if (cursorPos == 0) return 0;
            auto &display = epd.getDisplay();

            String textUpToCursor = value->substring(0, cursorPos);
            int16_t x1, y1;
            uint16_t w, h;
            display.getTextBounds(textUpToCursor, 0, 0, &x1, &y1, &w, &h);
            return w;
        }
    };

    class InteractableModal : public Interactable {
        int width;
        int height;
        std::function<void(Controller &epd, const RenderContext &ctx)> contentRenderer{};
        std::function<void()> closeCallback{};
        static constexpr int BORDER_RADIUS = 8;
        static constexpr int PADDING = 12;
        bool dismissOnAction = true;

    public:
        InteractableModal(
            const String &id,
            const int width,
            const int height,
            std::function<void(Controller &epd, const RenderContext &ctx)> contentRenderer,
            const bool dismissOnAction = true,
            std::function<void()> closeCallback = nullptr
        ) : Interactable(id),
            width(width),
            height(height),
            contentRenderer(std::move(contentRenderer)),
            closeCallback(std::move(closeCallback)),
            dismissOnAction(dismissOnAction) {
        }

        [[nodiscard]] InteractableType getType() const override {
            return InteractableType::MENU;
        }

        void onAction() override {
            if (dismissOnAction) {
                deactivate();
            } else {
                activate();
            }
        }

        void deactivate() {
            Interactable::deactivate();
            if (closeCallback) {
                closeCallback();
            }
        }

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            if (!getIsSelected() && !getIsActive()) {
                return;
            }

            auto &display = epd.getDisplay();

            // Calculate center position - align to 8-pixel boundaries
            const int modalX = (ctx.x + (ctx.width - width) / 2 + 7) & ~7;
            const int modalY = (ctx.y + (ctx.height - height) / 2 + 7) & ~7;

            // Draw modal background

            // Highlighted border for active state
            constexpr int MARGIN = PADDING / 4;
            display.fillRoundRect(modalX, modalY, width, height, BORDER_RADIUS, getForegroundColor());
            epd.drawMultiRoundRectBorder(
                modalX + MARGIN,
                modalY + MARGIN,
                width - MARGIN * 2,
                height - MARGIN * 2,
                getBackgroundColor(),
                3,
                1,
                2,
                BORDER_RADIUS
            );

            // Create a context for the content with padding
            const RenderContext contentCtx{
                modalX + PADDING,
                modalY + PADDING,
                width - (PADDING * 2),
                height - (PADDING * 2)
            };

            // Execute the content renderer function
            if (contentRenderer) {
                contentRenderer(epd, contentCtx);
            }
        }
    };
}
#endif //EPDINTERACTABLE_H

#ifndef MENUCONSTANTS_H
#define MENUCONSTANTS_H
/**
 * @file EPDMenuConstants.h
 * Layout constants and helpers used by the menu system.
 */

#include "EPDController.h"

namespace EPD {
    class MenuConstants {
    public:
        static constexpr int PADDING = 8;
        static constexpr int MARGIN_BOTTOM = 20;
        static constexpr int HEIGHT = 140 + MARGIN_BOTTOM;
        static constexpr int X_POS = PADDING;

        [[nodiscard]] static int getWidth(Controller &epd) {
            return epd.getDisplay().width() - PADDING * 2;
        }

        [[nodiscard]] static int getYPos(Controller &epd) {
            return epd.getDisplay().height() - PADDING - HEIGHT + MARGIN_BOTTOM;
        }
    };
}
#endif //MENUCONSTANTS_H

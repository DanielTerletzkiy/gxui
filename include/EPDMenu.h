#pragma once
/**
 * @file EPDMenu.h
 * Menu widgets and containers for GXUI.
 *
 * Provides:
 *  - EPD::MenuWidget: a simple item with optional Icon and text
 *  - EPD::MenuItem: an interactable menu entry with selection/active visuals
 *  - EPD::MenuRenderContext: context with menu-specific layout values
 *  - EPD::Menu: container managing a grid/list of menu items
 */

#include <EPDIcon.h>
#include "EPDRenderable.h"
#include "EPDController.h"
#include "EPDPage.h"
#include "EPDMenuConstants.h"
#include "EPDInteractable.h"
#include "EPDRenderManager.h"
#include <utility>
#include <vector>
#include <functional>
#include <memory>

namespace EPD {
    class MenuWidget : public Component {
    protected:
        String data{};
        Icon *icon = nullptr;

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            if (icon != nullptr) {
                icon->executeRender(epd, IconRenderContext(ctx.x, ctx.y, ctx.height, epd.getPrimaryColor()));
            }
            if (data.length() > 0) {
                const int PADDING = 4;
                const int OFFSET = icon != nullptr ? ctx.height + PADDING : 0;

                // Calculate text height to align vertically with icon
                int16_t x1, y1;
                uint16_t textWidth, textHeight;
                epd.getDisplay().getTextBounds(data, 0, 0, &x1, &y1, &textWidth, &textHeight);

                // Adjust y position to vertically align text with icon
                int textY = ctx.y + ((ctx.height + textHeight) / 2) - 2; // -2 is a small offset adjustment
                epd.getDisplay().setCursor(ctx.x + OFFSET, textY);
                epd.getDisplay().print(data);

                // Update context width (icon width + spacing + text width)
                ctx.width = OFFSET + PADDING + textWidth;
            } else if (icon != nullptr) {
                // If only icon, width is just the icon size
                ctx.width = ctx.height;
            }
        }

    public:
        explicit MenuWidget() = default;

        explicit MenuWidget(const String &data) : data(data) {
        }

        explicit MenuWidget(const String &data, Icon &icon) : data(data), icon(&icon) {
        }

        explicit MenuWidget(Icon &icon) : icon(&icon) {
        }

        [[nodiscard]] const Icon *getIcon() const { return icon; }
    };

    enum class MenuItemType {
        ACTION,
        SUBMENU,
        PAGE,
    };

    struct MenuRenderContext final : RenderContext {
        int menuItemSize{0};
        int iconSize{0};
        int selectedIndex{0};
    };

    class MenuItem : public Interactable {
    public:
        explicit MenuItem(String title) : title(std::move(title)), parent(nullptr) {
        }

        explicit MenuItem(String title, Icon &icon) : title(std::move(title)),
                                                      parent(nullptr), icon(&icon) {
        }

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            const auto &menuCtx = static_cast<const MenuRenderContext &>(ctx);

            if (getIcon() != nullptr) {
                const int icon_x = menuCtx.x - (MenuConstants::PADDING / 2) + (menuCtx.menuItemSize - menuCtx.iconSize)
                                   / 2;
                const int icon_y = menuCtx.y - (MenuConstants::PADDING / 2) + (menuCtx.menuItemSize - menuCtx.iconSize)
                                   / 2;

                if (icon != nullptr) {
                    icon->executeRender(
                        epd,
                        IconRenderContext(icon_x, icon_y, menuCtx.iconSize, epd.getPrimaryColor())
                    );
                }
            }

            /*if (getIsSelected()) {
                epd.drawPatternInRoundedArea(
                    menuCtx.x,
                    menuCtx.y,
                    menuCtx.menuItemSize - MenuConstants::PADDING,
                    menuCtx.menuItemSize - MenuConstants::PADDING,
                    MenuConstants::PADDING / 2,
                    Controller::Pattern::SPARSE_DOTS
                );
            }*/

            epd.drawMultiRoundRectBorder(
                menuCtx.x,
                menuCtx.y,
                menuCtx.menuItemSize - MenuConstants::PADDING,
                menuCtx.menuItemSize - MenuConstants::PADDING,
                epd.getPrimaryColor(),
                getIsSelected() ? 3 : 1,
                1,
                2,
                MenuConstants::PADDING / 2
            );

            epd.getDisplay().setCursor(
                menuCtx.x + MenuConstants::PADDING / 2,
                menuCtx.y + MenuConstants::PADDING * 2.5
            );

            epd.getDisplay().print(getMenuTypeChar(getMenuType()));
        }

        ~MenuItem() override = default;

        [[nodiscard]] String getTitle() const { return title; }

        [[nodiscard]] String getPathTitle() const {
            if (parent == nullptr) return title;
            return parent->getPathTitle() + "/" + title;
        }

        void setParent(MenuItem *p) { parent = p; }
        [[nodiscard]] MenuItem *getParent() const { return parent; }
        [[nodiscard]] Icon *getIcon() const { return icon; }

        [[nodiscard]] virtual MenuItemType getMenuType() const = 0;

        virtual void execute() = 0;

        static String getMenuTypeChar(const MenuItemType type) {
            switch (type) {
                case MenuItemType::ACTION: return "$";
                case MenuItemType::SUBMENU: return "/";
                case MenuItemType::PAGE: return ">";
            }
            return "?";
        }

    protected:
        String title = "Item";
        MenuItem *parent;
        Icon *icon = nullptr;
    };

    class SubMenu : public MenuItem {
    public:
        explicit SubMenu(const String &title) : MenuItem(title) {
        }

        explicit SubMenu(const String &title, Icon &icon) : MenuItem(title, icon) {
        }

        [[nodiscard]] MenuItemType getMenuType() const override { return MenuItemType::SUBMENU; }

        void execute() override {
        }

        void addItem(std::unique_ptr<MenuItem> item) {
            item->setParent(this);
            items.push_back(std::move(item));

            // init the first item as selected
            if (getItemsSize() == 1) {
                setSelectedIndex(0);
            }
        }

        [[nodiscard]] const std::vector<std::unique_ptr<MenuItem> > &getItems() const { return items; }
        [[nodiscard]] int getItemsSize() const { return items.size(); }

        void setSelectedIndex(const int index) {
            const auto &currentItem = *getItems()[selectedIndex];
            currentItem.deselect();

            selectedIndex = index;

            const auto &newItem = *getItems()[selectedIndex];
            newItem.select();
        }

        [[nodiscard]] int getSelectedIndex() const {
            return selectedIndex;
        }

    private:
        int selectedIndex = 0;
        std::vector<std::unique_ptr<MenuItem> > items;
    };

    class ActionMenuItem : public MenuItem {
    public:
        ActionMenuItem(const String &title, std::function<void()> action)
            : MenuItem(title), action(std::move(action)) {
        }

        ActionMenuItem(const String &title, Icon &icon, std::function<void()> action)
            : MenuItem(title, icon), action(std::move(action)) {
        }

        [[nodiscard]] MenuItemType getMenuType() const override { return MenuItemType::ACTION; }
        void execute() override { action(); }

    private:
        std::function<void()> action{};
    };

    class PageMenuItem : public MenuItem {
    public:
        explicit PageMenuItem(const String &title, std::shared_ptr<Page> page)
            : MenuItem(title), page(std::move(page)) {
        }

        PageMenuItem(const String &title, Icon &icon, std::shared_ptr<Page> page)
            : MenuItem(title, icon), page(std::move(page)) {
        }

        [[nodiscard]] MenuItemType getMenuType() const override { return MenuItemType::PAGE; }

        void execute() override {
        }

        [[nodiscard]] std::shared_ptr<Page> getPage() const { return page; }

    private:
        std::shared_ptr<Page> page{};
    };

    class MenuSystem : public Interactable {
    public:
        static void init() {
            instance().rootMenu = std::make_unique<SubMenu>("");
            instance().currentMenu = instance().rootMenu.get();
        }

        [[nodiscard]] InteractableType getType() const override {
            return InteractableType::MENU;
        }

        static bool isActive;

        static void open() {
            isActive = true;
            requestRender();
        }

        static void close() {
            isActive = false;
            requestRender(true);
        }

        static void addToRoot(std::unique_ptr<MenuItem> item) {
            instance().rootMenu->addItem(std::move(item));
        }

        static void addWidget(std::unique_ptr<MenuWidget> widget) {
            instance().widgets.push_back(std::move(widget));
        }

        void onActionUp() override {
            goBack();
        }

        void onActionDown() override {
            executeSelected();
        }

        void onActionLeft() override {
            moveSelection(true);
        }

        void onActionRight() override {
            moveSelection(false);
        }

        void onAction() override {
            executeSelected();
        }

        static void moveSelection(bool up) {
            if (auto *subMenu = static_cast<SubMenu *>(instance().currentMenu)) {
                const int itemCount = subMenu->getItems().size();
                if (up) {
                    subMenu->setSelectedIndex(
                        (subMenu->getSelectedIndex() > 0)
                            ? subMenu->getSelectedIndex() - 1
                            : itemCount - 1
                    );
                } else {
                    subMenu->setSelectedIndex(
                        (subMenu->getSelectedIndex() < itemCount - 1)
                            ? subMenu->getSelectedIndex() + 1
                            : 0
                    );
                }
            }
        }

        static void executeSelected() {
            if (auto *subMenu = static_cast<SubMenu *>(instance().currentMenu)) {
                if (const auto &items = subMenu->getItems();
                    subMenu->getSelectedIndex() >= 0 && subMenu->getSelectedIndex() < items.size()) {
                    auto &selectedItem = items[subMenu->getSelectedIndex()];
                    if (selectedItem->getMenuType() == MenuItemType::SUBMENU) {
                        instance().currentMenu = selectedItem.get();
                        //subMenu->setSelectedIndex(0);
                    } else if (selectedItem->getMenuType() == MenuItemType::PAGE) {
                        if (const auto *pageItem = static_cast<PageMenuItem *>(selectedItem.get())) {
                            RenderManager::pushPage(pageItem->getPage());
                            close();
                        }
                    }
                    selectedItem->execute();

                    if (selectedItem->getMenuType() != MenuItemType::PAGE) {
                        //requestRender();
                    }
                }
            }
        }

        static void goBack() {
            if (instance().currentMenu != instance().rootMenu.get() && instance().currentMenu->getParent()) {
                instance().currentMenu = instance().currentMenu->getParent();
                //requestRender();
            } else {
                close();
            }
        }

        void renderContent(Controller &epd, const RenderContext &ctx) override {
            if (!isActive) return;

            epd.getDisplay().fillRoundRect(
                MenuConstants::X_POS,
                MenuConstants::getYPos(epd),
                MenuConstants::getWidth(epd),
                MenuConstants::HEIGHT,
                MenuConstants::PADDING,
                epd.getPrimaryColor(true)
            );

            epd.drawMultiRoundRectBorder(
                MenuConstants::X_POS,
                MenuConstants::getYPos(epd),
                MenuConstants::getWidth(epd),
                MenuConstants::HEIGHT,
                epd.getPrimaryColor(),
                3,
                2,
                2,
                MenuConstants::PADDING
            );


            epd.getDisplay().setFont(MAIN_FONT);
            epd.getDisplay().setTextColor(
                epd.getPrimaryColor()
            );

            // Draw title
            epd.getDisplay().setCursor(
                MenuConstants::X_POS + (MenuConstants::PADDING * 2),
                MenuConstants::getYPos(epd) + (MenuConstants::PADDING * 3.5)
            );
            epd.getDisplay().print(
                currentMenu->getPathTitle()
            );

            // Draw menu items

            const int MENU_ITEM_COUNT = 5;
            const int MENU_ITEM_SIZE = (MenuConstants::getWidth(epd) - MenuConstants::PADDING * 4) / MENU_ITEM_COUNT;
            const int MENU_ITEM_ICON_PADDING = MenuConstants::PADDING * 4;
            const int MENU_ITEM_ICON_SIZE = MENU_ITEM_SIZE - MENU_ITEM_ICON_PADDING;

            int x = MenuConstants::X_POS + (MenuConstants::PADDING * 2) + MenuConstants::PADDING / 2;
            int y = MenuConstants::getYPos(epd) + MenuConstants::HEIGHT - MenuConstants::MARGIN_BOTTOM - MENU_ITEM_SIZE
                    - (
                        MenuConstants::PADDING * 1.5);

            int itemX = x;

            // Check if current menu is a submenu using getMenuType()
            if (currentMenu->getMenuType() == MenuItemType::SUBMENU) {
                int index = 0;

                const auto *subMenu = static_cast<const SubMenu *>(currentMenu);

                const auto &selectedItem = *subMenu->getItems()[subMenu->getSelectedIndex()];

                epd.getDisplay().print(MenuItem::getMenuTypeChar(selectedItem.getMenuType()));

                epd.getDisplay().setFont(&FreeMonoBold12pt7b);

                const int16_t cursorX = epd.getDisplay().getCursorX();
                const int16_t cursorY = epd.getDisplay().getCursorY();
                int16_t x1, y1;
                uint16_t w, h;
                epd.getDisplay().getTextBounds(
                    subMenu->getItems()[subMenu->getSelectedIndex()]->getTitle(),
                    cursorX,
                    cursorY,
                    &x1,
                    &y1,
                    &w,
                    &h
                );
                if (h < 16) {
                    h = 20;
                }

                epd.getDisplay().drawLine(x1, y1 + h, x1 + w, y1 + h, epd.getPrimaryColor());

                epd.getDisplay().print(selectedItem.getTitle());

                epd.getDisplay().setFont(MAIN_FONT);

                for (const auto &item: subMenu->getItems()) {
                    MenuRenderContext menuCtx;
                    menuCtx.x = itemX;
                    menuCtx.y = y;
                    menuCtx.menuItemSize = MENU_ITEM_SIZE;
                    menuCtx.iconSize = MENU_ITEM_ICON_SIZE;
                    menuCtx.selectedIndex = subMenu->getSelectedIndex();

                    item->executeRender(epd, menuCtx);

                    itemX += MENU_ITEM_SIZE;
                    index++;
                }
            }

            if (!widgets.empty()) {
                int widgetX = x;
                const int widgetY = y + MENU_ITEM_SIZE;

                for (const auto &widget: widgets) {
                    const RenderContext widgetCtx;
                    widgetCtx.x = widgetX;
                    widgetCtx.y = widgetY;
                    widgetCtx.height = 20;
                    widget->executeRender(epd, widgetCtx);
                    int wX, wY, wWidth, wHeight;
                    widget->getWindow(&wX, &wY, &wWidth, &wHeight);

                    widgetX += wWidth;
                }
            }
        }

        static MenuSystem &instance() {
            static MenuSystem inst;
            return inst;
        }

    private:
        static void requestRender(const bool fullRender = false) {
            fullRender ? RenderManager::requestFullRender() : RenderManager::requestMenuRender();
        }

        std::vector<std::unique_ptr<MenuWidget> > widgets;

        std::unique_ptr<SubMenu> rootMenu;
        MenuItem *currentMenu{nullptr};
        static const GFXfont *MAIN_FONT;
    };

    bool MenuSystem::isActive = false;
    const GFXfont *MenuSystem::MAIN_FONT = &FreeMono12pt7b;

    inline bool isMenuActive() {
        return MenuSystem::isActive;
    }

    inline Interactable &getMenuSystemInstance() {
        return MenuSystem::instance();
    }
}

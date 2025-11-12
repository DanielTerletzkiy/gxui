#pragma once
/**
 * @file EPDRenderManager.h
 * Centralized rendering coordination for GXUI.
 *
 * The RenderManager owns a queue and a render task. It determines what needs
 * to be redrawn (page, menu, current interactable) and issues drawing calls
 * to the EPD::Controller. Also provides navigation hooks through
 * InteractableActions to trigger contextual re-renders.
 */
#include <memory>
#include <stack>

#include <EPDInteractable.h>
#include <EPDRenderable.h>

#include "EPDController.h"
#include "EPDMenuConstants.h"

namespace EPD {
    class MenuSystem;

    extern bool isMenuActive();

    extern Interactable &getMenuSystemInstance();

    enum class RenderFocus {
        PAGE,
        MENU,
        INTERACTABLE,
        NONE,
    };

    class RenderManager : public InteractableActions {
    public:
        static void init(Controller &epd) {
            instance().epd = &epd;
            instance().renderQueue = xQueueCreate(QUEUE_SIZE, sizeof(RenderRequest));
            if (instance().renderQueue == nullptr) {
                Serial.println("Failed to create render queue!");
                return;
            }
            xTaskCreatePinnedToCore(renderTask, "RenderTask", 8192, nullptr, 1, &renderTaskHandle, 0);
        }

        static void pushPage(const std::shared_ptr<Page> &page) {
            page->onPageOpened();
            pageStack.push(page);
            requestFullRender();
        }

        static void popPage() {
            if (!pageStack.empty()) {
                pageStack.pop();
                requestFullRender();
            }
        }

        static void requestFullRender() {
            if (!isInitialized()) {
                Serial.println("RenderManager not initialized!");
                return;
            }
            auto queue = instance().renderQueue;
            constexpr RenderRequest req{RenderType::FULL};
            if (xQueueSend(queue, &req, 0) != pdTRUE) {
                Serial.println("Failed to send render request!");
            }
        }

        static void requestMenuRender() {
            if (!isInitialized()) {
                Serial.println("RenderManager not initialized!");
                return;
            }
            auto queue = instance().renderQueue;
            constexpr RenderRequest req{RenderType::MENU_ONLY};
            if (xQueueSend(queue, &req, 0) != pdTRUE) {
                Serial.println("Failed to send render request!");
            }
        }

        static void requestInteractableRender() {
            if (!isInitialized()) {
                Serial.println("RenderManager not initialized!");
                return;
            }
            auto queue = instance().renderQueue;
            constexpr RenderRequest req{RenderType::INTERACTABLE_ONLY};
            if (xQueueSend(queue, &req, 0) != pdTRUE) {
                Serial.println("Failed to send render request!");
            }
        }

        static std::shared_ptr<Page> getCurrentPage() {
            if (!pageStack.empty()) {
                return pageStack.top();
            }
            return nullptr;
        }

        static RenderFocus getCurrentRenderFocus() {
            if (isMenuActive()) {
                return RenderFocus::MENU;
            }
            if (!pageStack.empty()) {
                if (const auto currentInteractable = getCurrentPage()->getCurrentInteractable();
                    currentInteractable != nullptr && currentInteractable->getIsActive()) {
                    return RenderFocus::INTERACTABLE;
                }
                return RenderFocus::PAGE;
            }

            return RenderFocus::NONE;
        }

        static void requestContextualRender() {
            switch (getCurrentRenderFocus()) {
                case RenderFocus::PAGE:
                    requestFullRender();
                    break;
                case RenderFocus::MENU:
                    requestMenuRender();
                    break;
                case RenderFocus::INTERACTABLE:
                    requestInteractableRender();
                    break;
                case RenderFocus::NONE:
                default:
                    break;
            }
        }

        static Interactable *getCurrentNavigatable() {
            Serial.println("Getting current navigatable...");
            switch (getCurrentRenderFocus()) {
                case RenderFocus::PAGE:
                    Serial.println("Current render focus: PAGE");
                    return getCurrentPage().get();
                case RenderFocus::MENU:
                    Serial.println("Current render focus: MENU");
                    return &getMenuSystemInstance();
                case RenderFocus::INTERACTABLE:
                    Serial.println("Current render focus: INTERACTABLE");
                    return getCurrentPage()->getCurrentInteractable();
                case RenderFocus::NONE:
                default:
                    Serial.println("Current render focus: NONE");
                    return nullptr;
            }
        }

        static void onActionUpStatic() {
            getCurrentNavigatable()->onActionUp();
            requestContextualRender();
        }

        static void onActionDownStatic() {
            getCurrentNavigatable()->onActionDown();
            requestContextualRender();
        }

        static void onActionLeftStatic() {
            getCurrentNavigatable()->onActionLeft();
            requestContextualRender();
        }

        static void onActionRightStatic() {
            getCurrentNavigatable()->onActionRight();
            requestContextualRender();
        }

        static void onActionStatic() {
            getCurrentNavigatable()->onAction();
            requestContextualRender();
        }

    private:
        enum class RenderType {
            FULL,
            MENU_ONLY,
            INTERACTABLE_ONLY,
        };

        struct RenderRequest {
            RenderType type;
        };

        static RenderManager &instance() {
            static RenderManager inst;
            return inst;
        }

        Controller *epd{nullptr};
        static std::stack<std::shared_ptr<Page> > pageStack;
        static TaskHandle_t renderTaskHandle;
        static QueueHandle_t renderQueue;
        static constexpr size_t QUEUE_SIZE = 1;

        // this will track the amount of renders to later handle full display refresh
        static constexpr size_t MAX_RENDER_REFRESH = 20;
        size_t executedRenders = 0;

        static void renderPageCallback(const void *) {
            instance().epd->getDisplayTheme() == Controller::DisplayTheme::LIGHT
                ? instance().epd->getDisplay().fillScreen(GxEPD_WHITE)
                : instance().epd->getDisplay().fillScreen(GxEPD_BLACK);

            if (!pageStack.empty()) {
                //TODO: make this work as this prevents popover overlap, workaround is to render them later regardless of x,y pos
                if (const auto currentInteractable = getCurrentPage()->getCurrentInteractable();
                    !getCurrentPage()->shouldRenderUnfocusedContent() && currentInteractable != nullptr &&
                    currentInteractable->getIsActive()
                ) {
                    currentInteractable->executeRender(*instance().epd, currentInteractable->lastRenderCTX);
                } else {
                    getCurrentPage()->executeRender(*instance().epd, RenderContext());
                }
            }
            if (isMenuActive()) {
                getMenuSystemInstance().executeRender(*instance().epd, RenderContext());
            }
        }

        [[noreturn]] static void renderTask(void *) {
            RenderRequest req{};

            while (true) {
                if (xQueueReceive(renderQueue, &req, portMAX_DELAY) == pdTRUE) {
                    const unsigned long startTime = millis();
                    auto &display = instance().epd->getDisplay();

                    instance().executedRenders++;
                    Serial.printf(
                        "Configuring window: executed renders: %d, max renders: %d\n",
                        instance().executedRenders,
                        MAX_RENDER_REFRESH
                    );

                    if (req.type == RenderType::FULL) {
                        if (instance().executedRenders >= MAX_RENDER_REFRESH) {
                            display.setFullWindow();
                            Serial.print("Render type: FULL, ");
                            instance().executedRenders = 0;
                        } else {
                            display.setPartialWindow(
                                0,
                                0,
                                display.width(),
                                display.height()
                            );
                            Serial.print("Render type: FULL (fast partial), ");
                        }
                    } else if (req.type == RenderType::MENU_ONLY) {
                        display.setPartialWindow(
                            MenuConstants::X_POS,
                            MenuConstants::getYPos(*instance().epd),
                            MenuConstants::getWidth(*instance().epd),
                            MenuConstants::HEIGHT
                        );
                        Serial.print("Render type: MENU_ONLY, ");
                    } else if (req.type == RenderType::INTERACTABLE_ONLY) {
                        int x, y, width, height;
                        const auto interactable = getCurrentPage()->getCurrentInteractable();

                        if (interactable == nullptr) {
                            continue;
                        }

                        interactable->getWindow(
                            &x,
                            &y,
                            &width,
                            &height
                        );
                        Serial.printf("Partial window - x: %d, y: %d, width: %d, height: %d\n", x, y, width, height);

                        display.setPartialWindow(
                            x,
                            y,
                            width,
                            height
                        );
                        Serial.print("Render type: INTERACTABLE_ONLY, ");
                    }

                    display.drawPaged(renderPageCallback, nullptr);
                    const unsigned long endTime = millis();
                    Serial.printf("Time taken: %lu ms\n", endTime - startTime);
                }
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

        static bool isInitialized() {
            return instance().epd != nullptr && instance().renderQueue != nullptr;
        }
    };

    std::stack<std::shared_ptr<Page> > RenderManager::pageStack;
    TaskHandle_t RenderManager::renderTaskHandle = nullptr;
    QueueHandle_t RenderManager::renderQueue = nullptr;
}

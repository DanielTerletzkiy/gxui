#ifndef SAMPLEPAGE_H
#define SAMPLEPAGE_H

/**
 * @file SamplePage.h
 * Example page demonstrating common GXUI interactables and components.
 *
 * Shows buttons, sliders, toggles, dropdowns, text input, and a progress bar
 * with simple event handlers. Use this as a reference for building your own
 * pages. Requires icon mapping from epd/IconMapper.h.
 */

#include "EPDPage.h"
#include "EPDComponent.h"
#include <vector>
#include <epd/IconMapper.h>

using namespace EPD;

enum class LightMode {
    OFF,
    LOWEST,
    MEDIUM,
    HIGHEST
};

class SamplePage : public Page, public IconMapper {
private:
    int sliderValue = 0;
    int sliderValue2 = 70;


    // Create toggle options with enum values
    std::vector<ToggleOption<LightMode> > lightOptions = {
        ToggleOption(&dnd_dice_d4_icon(), LightMode::OFF),
        ToggleOption(&dnd_dice_d6_icon(), LightMode::LOWEST),
        ToggleOption(&dnd_dice_d8_icon(), LightMode::MEDIUM),
        ToggleOption(&dnd_dice_d10_icon(), LightMode::HIGHEST)
    };

    size_t lightModeIndex = 0;


    std::vector<String> options = {"Option 1", "Option 2", "Option 3", "Option 4", "Option 5", "Option 6", "Option 7"};
    size_t optionsIndex = 0;

    String textInput = "txt";

    float prog = 0.47f;

public:
    SamplePage(): Page() {
        addInteractable(
            std::make_unique<InteractableButton>(
                "btn1",
                "Button 1",
                []() {
                    Serial.println("Button 1 pressed");
                }
            )
        );
        addInteractable(
            std::make_unique<InteractableButton>(
                "btn2",
                "Button 2",
                []() {
                    Serial.println("Button 2 pressed");
                }
            )
        );

        addInteractable(
            std::make_unique<InteractableSlider>(
                "sli1",
                "Slider 1",
                &sliderValue,
                0,
                100,
                10
            )
        );

        addInteractable(
            std::make_unique<InteractableSlider>(
                "sli2",
                "Slider 2",
                &sliderValue2,
                50,
                100,
                1
            )
        );

        addInteractable(
            std::make_unique<InteractableToggle<LightMode>>(
                "tgl1",
                "Toggle 1",
                lightOptions,
                &lightModeIndex
            )
        );

        addInteractable(
            std::make_unique<InteractableDropdown>(
                "drp1",
                "Drop 1",
                options,
                &optionsIndex
            )
        );

        addInteractable(
            std::make_unique<InteractableTextInput>(
                "txt1",
                "Txt 1",
                &textInput
            )
        );
    }

    [[nodiscard]] String getTitle() const {
        return "Sample Dashboard";
    }

    void renderContent(Controller &epd, const RenderContext &ctx) override {
        auto &display = epd.getDisplay();
        display.setTextColor(epd.getPrimaryColor());

        // Header section
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 40);
        display.print("Sample Dashboard");

        // Draw time in top right
        display.setFont(&FreeMono12pt7b);
        display.setCursor(display.width() - 120, 40);
        display.print("12:34 PM");


        getInteractable("btn1")->executeRender(epd, RenderContext{20, 100});
        getInteractable("btn2")->executeRender(epd, RenderContext{20, 150});

        getInteractable("sli1")->executeRender(epd, RenderContext{20, 200});
        getInteractable("sli2")->executeRender(epd, RenderContext{20, 250});

        getInteractable("tgl1")->executeRender(epd, RenderContext{20, 300});
        //getInteractable("tgl1")->executeRender(epd, RenderContext{20, 420});


        getInteractable("drp1")->executeRender(epd, RenderContext{20, 350});
        getInteractable("txt1")->executeRender(epd, RenderContext{200, 350});

        std::make_unique<ComponentProgressBar>("Progress 1", prog)
                ->renderContent(epd, RenderContext{20, 400});

        // Pattern demonstrations
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(display.width() - 250, 100);
        display.print("Pattern Examples:");
        
        // Draw pattern rectangles
        epd.drawPattern(Controller::Pattern::SOLID, display.width() - 250, 110, 80, 40);
        epd.drawPattern(Controller::Pattern::STRIPES, display.width() - 150, 110, 80, 40);
        
        epd.drawPattern(Controller::Pattern::DOTS, display.width() - 250, 160, 80, 40);
        epd.drawPattern(Controller::Pattern::CHECKERBOARD, display.width() - 150, 160, 80, 40);
        
        epd.drawPattern(Controller::Pattern::DIAGONAL_STRIPES, display.width() - 250, 210, 80, 40);
        epd.drawPattern(Controller::Pattern::CROSS_HATCH, display.width() - 150, 210, 80, 40);
        
        epd.drawPattern(Controller::Pattern::SPARSE_DOTS, display.width() - 250, 260, 80, 40);
        epd.drawPattern(Controller::Pattern::VERY_SPARSE_DOTS, display.width() - 150, 260, 80, 40);
        
        // Draw a rounded rectangle with pattern
        epd.getDisplay().drawRoundRect(display.width() - 250, 310, 180, 60, 10, GxEPD_BLACK);
        epd.drawPatternInRoundedArea(Controller::Pattern::DIAGONAL_STRIPES, display.width() - 250, 310, 180, 60, 10);
    }

private:
    static void drawStatsBoxes(Controller &epd) {
        const int boxWidth = 150;
        const int boxHeight = 100;
        const int startY = 70;
        const int padding = 20;

        // Draw multiple stat boxes
        for (int i = 0; i < 4; i++) {
            int x = padding + i * (boxWidth + padding);

            epd.drawMultiRoundRectBorder(x, startY, boxWidth, boxHeight);

            auto &display = epd.getDisplay();
            display.setFont(&FreeMonoBold12pt7b);

            // Box title
            display.setCursor(x + 10, startY + 25);
            display.print("Stat " + String(i + 1));

            // Box value
            display.setFont(&FreeMono12pt7b);
            display.setCursor(x + 10, startY + 60);
            display.print(String(random(100)) + "%");
        }
    }

    static void drawGraph(Controller &epd) {
        const int graphX = 20;
        const int graphY = 200;
        const int graphWidth = epd.getDisplay().width() - 40;
        const int graphHeight = 150;

        // Draw graph border
        epd.drawMultiRoundRectBorder(graphX, graphY, graphWidth, graphHeight);

        // Generate random data points
        std::vector<int> data;
        for (int i = 0; i < 10; i++) {
            data.push_back(random(graphHeight - 40));
        }

        // Draw graph lines
        auto &display = epd.getDisplay();
        const int pointSpacing = (graphWidth - 40) / (data.size() - 1);

        for (size_t i = 0; i < data.size() - 1; i++) {
            display.drawLine(
                graphX + random(0, 21) + (i * pointSpacing),
                graphY + graphHeight - random(0, 21) - data[i],
                graphX + random(0, 21) + ((i + 1) * pointSpacing),
                graphY + graphHeight - random(0, 21) - data[i + 1],
                GxEPD_BLACK
            );
        }
    }

    static void drawInfoSection(Controller &epd) {
        auto &display = epd.getDisplay();
        const int startY = 380;

        // Draw three columns of info
        display.setFont(&FreeMono12pt7b);

        const int col1 = 20;
        const int col2 = display.width() / 3;
        const int col3 = (display.width() * 2) / 3;

        // Column 1
        display.setCursor(col1, startY);
        display.print("System Status: OK");
        display.setCursor(col1, startY + 25);
        display.print("Uptime: 23:59:59");

        // Column 2
        display.setCursor(col2, startY);
        display.print("Memory: 45%");
        display.setCursor(col2, startY + 25);
        display.print("CPU: 23%");

        // Column 3
        display.setCursor(col3, startY);
        display.print("Network: Online");
        display.setCursor(col3, startY + 25);
        display.print("Updates: 2");
    }
};

class PatternDemoPage : public Page {
public:
    PatternDemoPage() : Page() {}

    [[nodiscard]] String getTitle() const override {
        return "Pattern Functions Demo";
    }

    void renderContent(Controller &epd, const RenderContext &ctx) override {
        auto &display = epd.getDisplay();
        
        // Page title
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 40);
        display.print(getTitle());
        
        // Simple pattern demos
        drawPatternSamples(epd);
        
        // Rounded area patterns
        drawRoundedAreaPatterns(epd);
        
        // Complex pattern examples
        drawComplexExamples(epd);
    }

private:
    void drawPatternSamples(Controller &epd) {
        auto &display = epd.getDisplay();
        
        display.setFont(&FreeMono12pt7b);
        display.setCursor(20, 80);
        display.print("Pattern Types:");
        
        const int boxWidth = 80;
        const int boxHeight = 60;
        const int startX = 20;
        const int startY = 90;
        const int horizontalGap = 100;
        const int verticalGap = 80;
        
        // Draw all pattern types with labels
        const char* patternNames[] = {
            "Solid", "Stripes", "Dots", "Checkerboard", 
            "Diagonal", "CrossHatch", "SparseDots", "VerySparseDots"
        };
        
        for (int i = 0; i < 8; i++) {
            int row = i / 4;
            int col = i % 4;
            int x = startX + col * horizontalGap;
            int y = startY + row * verticalGap;
            
            // Draw border
            display.drawRect(x, y, boxWidth, boxHeight, GxEPD_BLACK);
            
            // Fill with pattern
            epd.drawPattern(
                static_cast<Controller::Pattern>(i),
                x + 1, y + 1,
                boxWidth - 2, boxHeight - 2
            );
            
            // Draw label
            display.setFont(&FreeMono9pt7b);
            display.setCursor(x, y + boxHeight + 15);
            display.print(patternNames[i]);
        }
    }
    
    void drawRoundedAreaPatterns(Controller &epd) {
        auto &display = epd.getDisplay();
        
        display.setFont(&FreeMono12pt7b);
        display.setCursor(20, 280);
        display.print("Rounded Area Patterns:");
        
        const int boxWidth = 120;
        const int boxHeight = 60;
        const int startX = 20;
        const int startY = 290;
        const int gap = 140;
        const int radius = 15;
        
        // Demonstrate patterns in rounded rectangles
        for (int i = 0; i < 4; i++) {
            int x = startX + i * gap;
            
            // Draw the pattern in a rounded area
            epd.getDisplay().drawRoundRect(x, startY, boxWidth, boxHeight, radius, GxEPD_BLACK);
            epd.drawPatternInRoundedArea(
                static_cast<Controller::Pattern>(i + 2), x + 1,
                startY + 1, boxWidth - 2,
                boxHeight - 2,
                radius
            );
        }
    }
    
    void drawComplexExamples(Controller &epd) {
        auto &display = epd.getDisplay();
        
        display.setFont(&FreeMono12pt7b);
        display.setCursor(20, 380);
        display.print("Combined Examples:");
        
        // Example 1: Multi-bordered rectangle with pattern fill
        const int ex1X = 20;
        const int ex1Y = 390;
        const int ex1Width = 180;
        const int ex1Height = 90;
        
        epd.drawMultiRectBorder(ex1X, ex1Y, ex1Width, ex1Height, GxEPD_BLACK, 2, 3, 2);
        epd.drawPattern(
            Controller::Pattern::DIAGONAL_STRIPES,
            ex1X + 7, ex1Y + 7,
            ex1Width - 14, ex1Height - 14
        );
        
        // Example 2: Multi-bordered rounded rect with pattern
        const int ex2X = 220;
        const int ex2Y = 390;
        const int ex2Width = 180;
        const int ex2Height = 90;
        const int ex2Radius = 10;
        
        epd.drawMultiRoundRectBorder(ex2X, ex2Y, ex2Width, ex2Height, GxEPD_BLACK, 2, 3, 2, ex2Radius);
        epd.drawPatternInRoundedArea(
            Controller::Pattern::CHECKERBOARD, ex2X + 7,
            ex2Y + 7, ex2Width - 14,
            ex2Height - 14,
            ex2Radius
        );
        
        // Example 3: Nested patterns
        const int ex3X = 420;
        const int ex3Y = 390;
        const int ex3Width = 180;
        const int ex3Height = 90;
        
        // Outer pattern
        epd.drawPattern(
            Controller::Pattern::STRIPES,
            ex3X, ex3Y,
            ex3Width, ex3Height
        );
        
        // Inner pattern
        epd.drawPattern(
            Controller::Pattern::CROSS_HATCH,
            ex3X + 20, ex3Y + 20,
            ex3Width - 40, ex3Height - 40
        );
        
        // Center text
        Controller::Bounds textBounds = epd.drawCenteredText(
            "Patterns!",
            ex3X + ex3Width / 2, 
            ex3Y + ex3Height / 2,
            &FreeMono9pt7b,
            GxEPD_BLACK
        );
    }
};

class DemoPage : public Page {
private:
    // System controls
    int brightnessValue = 128;
    int contrastValue = 180;
    String deviceName = "E-Paper Device";

    // Performance metrics
    float cpuLoad = 0.75f;
    float ramUsage = 0.25f;
    float diskUsage = 0.45f;
    float temperature = 0.60f;

    // Operating mode
    size_t modeIndex = 0;
    std::vector<String> modes = {"Normal", "Eco", "Performance", "Custom"};

public:
    DemoPage() : Page() {
        // System controls
        /*addInteractable(
            std::make_unique<InteractableToggle>(
                "sys_power",
                "System Power",
                &systemEnabled
            )
        );*/

        addInteractable(
            std::make_unique<InteractableDropdown>(
                "operating_mode",
                "Operating Mode",
                modes,
                &modeIndex
            )
        );

        // Display settings
        addInteractable(
            std::make_unique<InteractableSlider>(
                "brightness",
                "Brightness",
                &brightnessValue,
                0,
                255,
                15
            )
        );

        addInteractable(
            std::make_unique<InteractableSlider>(
                "contrast",
                "Contrast",
                &contrastValue,
                0,
                255,
                30
            )
        );
    }

    [[nodiscard]] String getTitle() const override {
        return "System Settings";
    }

    void renderContent(Controller &epd, const RenderContext &ctx) override {
        auto &display = epd.getDisplay();
        constexpr int leftCol = 20;
        const int rightCol = display.width() / 2 + 20;

        // Header
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(leftCol, 40);
        display.print(getTitle());

        // System controls
        //getInteractable("sys_power")->executeRender(epd, RenderContext{leftCol, 80}); // Power toggle

        // Display settings section
        display.setFont(&FreeMono12pt7b);
        display.setCursor(leftCol, 250);
        display.print("Display Settings");

        getInteractable("brightness")->executeRender(epd, RenderContext{leftCol, 270}); // Brightness
        getInteractable("contrast")->executeRender(epd, RenderContext{leftCol, 320}); // Contrast

        // Performance metrics section
        display.setFont(&FreeMono12pt7b);
        display.setCursor(leftCol, 390);
        display.print("System Status");


        std::make_unique<ComponentProgressBar>(
            "CPU Load",
            cpuLoad
        )->executeRender(epd, RenderContext{leftCol, 410});


        std::make_unique<ComponentProgressBar>(
            "RAM Usage",
            ramUsage
        )->executeRender(epd, RenderContext{rightCol, 410});


        std::make_unique<ComponentProgressBar>(
            "Disk Usage",
            diskUsage
        )->executeRender(epd, RenderContext{leftCol, 460});


        std::make_unique<ComponentProgressBar>(
            "Temperature",
            temperature
        )->executeRender(epd, RenderContext{rightCol, 460});

        getInteractable("operating_mode")->executeRender(epd, RenderContext{leftCol, 130}); // Mode dropdown
    }
};

class SettingsPage : public Page {
public:
    [[nodiscard]] String getTitle() const {
        return "Settings";
    }

    void renderContent(Controller &epd, const RenderContext &ctx) override {
        auto &display = epd.getDisplay();
        display.setTextColor(epd.getPrimaryColor());

        // Header
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 40);
        display.print(getTitle());

        // Settings menu
        drawSettingsMenu(epd);

        // System info
        drawSystemInfo(epd);

        // Footer
        drawFooter(epd);
    }

private:
    static void drawSettingsMenu(Controller &epd) {
        auto &display = epd.getDisplay();
        const int startY = 70;
        const int itemHeight = 50;
        const int width = display.width() - 40;

        const char *menuItems[] = {
            "WiFi Settings",
            "Display Options",
            "Power Management",
            "Updates",
            "Security"
        };

        for (int i = 0; i < 5; i++) {
            int y = startY + (i * (itemHeight + 10));

            epd.drawMultiRoundRectBorder(20, y, width, itemHeight);

            display.setFont(&FreeMono12pt7b);
            display.setCursor(35, y + 30);
            display.print(menuItems[i]);

            // Draw arrow
            display.fillTriangle(
                width - 20,
                y + itemHeight / 2,
                width - 30,
                y + itemHeight / 2 - 5,
                width - 30,
                y + itemHeight / 2 + 5,
                GxEPD_BLACK
            );

            // Draw icon placeholder
            display.drawRect(25, y + 10, 30, 30, GxEPD_BLACK);
        }
    }

    static void drawSystemInfo(Controller &epd) {
        auto &display = epd.getDisplay();
        const int startY = 380;
        const int boxWidth = (display.width() - 50) / 2;
        const int boxHeight = 60;

        // Draw two info boxes
        for (int i = 0; i < 2; i++) {
            int x = 20 + i * (boxWidth + 10);
            epd.drawMultiRoundRectBorder(x, startY, boxWidth, boxHeight);

            display.setFont(&FreeMono12pt7b);
            display.setCursor(x + 10, startY + 25);
            display.print(i == 0 ? "Firmware Version" : "Memory Usage");

            display.setFont(&FreeMonoBold12pt7b);
            display.setCursor(x + 10, startY + 45);
            display.print(i == 0 ? "v1.2.3" : "45%");
        }
    }

    static void drawFooter(Controller &epd) {
        auto &display = epd.getDisplay();
        display.setFont(&FreeMono12pt7b);
        display.setCursor(20, display.height() - 20);
        display.print("Last updated: 2024-01-01 12:00");
    }
};

class SensorDashboardPage : public Page {
public:
    [[nodiscard]] String getTitle() const {
        return "Sensor Data";
    }

    void renderContent(Controller &epd, const RenderContext &ctx) override {
        auto &display = epd.getDisplay();
        display.setTextColor(epd.getPrimaryColor());

        // Header
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 40);
        display.print(getTitle());

        // Draw sensor readings
        drawSensorGrid(epd);

        // Draw history chart
        drawHistoryChart(epd);

        // Draw status indicators
        drawStatusIndicators(epd);
    }

private:
    static void drawSensorGrid(Controller &epd) {
        auto &display = epd.getDisplay();
        const int startY = 70;
        const int cellWidth = display.width() / 2 - 30;
        const int cellHeight = 80;

        // Draw 4 sensor cells in a 2x2 grid
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 2; col++) {
                int x = 20 + col * (cellWidth + 20);
                int y = startY + row * (cellHeight + 10);

                epd.drawMultiRoundRectBorder(x, y, cellWidth, cellHeight);

                display.setFont(&FreeMono12pt7b);
                display.setCursor(x + 10, y + 30);

                // Sensor labels and values
                const char *labels[] = {"Temperature", "Humidity", "Pressure", "Light"};
                display.print(labels[row * 2 + col]);

                display.setFont(&FreeMonoBold12pt7b);
                display.setCursor(x + 10, y + 60);
                display.print(String(random(100)) + " " + (row * 2 + col == 0 ? "°C" : "%"));
            }
        }
    }

    static void drawHistoryChart(Controller &epd) {
        auto &display = epd.getDisplay();
        const int chartX = 20;
        const int chartY = 280;
        const int chartWidth = display.width() - 40;
        const int chartHeight = 100;

        epd.drawMultiRoundRectBorder(chartX, chartY, chartWidth, chartHeight);

        // Draw time labels
        display.setFont(&FreeMono12pt7b);
        display.setCursor(chartX + 5, chartY + chartHeight - 5);
        display.print("00:00");
        display.setCursor(chartX + chartWidth - 50, chartY + chartHeight - 5);
        display.print("23:59");

        // Draw data points
        for (int i = 0; i < 24; i++) {
            int x1 = chartX + 10 + (i * (chartWidth - 20) / 23);
            int x2 = chartX + 10 + ((i + 1) * (chartWidth - 20) / 23);
            int y1 = chartY + 10 + random(chartHeight - 40);
            int y2 = chartY + 10 + random(chartHeight - 40);

            display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
        }
    }

    static void drawStatusIndicators(Controller &epd) {
        auto &display = epd.getDisplay();
        const int startY = 400;
        const int spacing = display.width() / 3;

        display.setFont(&FreeMono12pt7b);

        // Draw status indicators with icons
        for (int i = 0; i < 3; i++) {
            int x = 20 + i * spacing;
            display.drawCircle(x + 5, startY + 5, 5, i % 2 == 0);

            display.setCursor(x + 15, startY + 10);
            const char *status[] = {"ONLINE", "ERROR", "ACTIVE"};
            display.print(status[i]);
        }
    }
};

class TextDemoPage : public Page {
public:
    TextDemoPage() : Page() {}

    [[nodiscard]] String getTitle() const override {
        return "Text Alignment Demo";
    }

    void renderContent(Controller &epd, const RenderContext &ctx) override {
        auto &display = epd.getDisplay();

        // Page title
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(20, 40);
        display.print(getTitle());

        // Show the different text alignment methods
        drawRegularTextDemo(epd);
        drawBottomAlignedTextDemo(epd);
        drawCenteredTextDemo(epd);
        drawAlignmentComparison(epd);
    }

private:
    void drawRegularTextDemo(Controller &epd) {
        auto &display = epd.getDisplay();

        const int startX = 20;
        const int startY = 80;

        // Section header
        display.setFont(&FreeMono12pt7b);
        display.setCursor(startX, startY);
        display.print("Regular Text Alignment");

        // Draw reference line
        display.drawFastHLine(startX, startY + 20, 400, GxEPD_BLACK);

        // Demo with different fonts
        Controller::Bounds bounds1 = epd.drawText(
            "Regular Text (Small)",
            startX, startY + 40,
            &FreeMono9pt7b,
            GxEPD_BLACK
        );

        Controller::Bounds bounds2 = epd.drawText(
            "Regular Text (Medium)",
            startX, startY + 70,
            &FreeMono12pt7b,
            GxEPD_BLACK
        );

        Controller::Bounds bounds3 = epd.drawText(
            "Regular Text (Large)",
            startX, startY + 110,
            &FreeMono18pt7b,
            GxEPD_BLACK
        );

        // Draw bounds rectangles
        display.drawRect(bounds1.x, bounds1.y, bounds1.w, bounds1.h, GxEPD_BLACK);
        display.drawRect(bounds2.x, bounds2.y, bounds2.w, bounds2.h, GxEPD_BLACK);
        display.drawRect(bounds3.x, bounds3.y, bounds3.w, bounds3.h, GxEPD_BLACK);
    }

    void drawBottomAlignedTextDemo(Controller &epd) {
        auto &display = epd.getDisplay();

        const int startX = 20;
        const int startY = 180;

        // Section header
        display.setFont(&FreeMono12pt7b);
        display.setCursor(startX, startY);
        display.print("Bottom-Aligned Text");

        // Draw reference line
        display.drawFastHLine(startX, startY + 40, 400, GxEPD_BLACK);

        // Demo with different fonts
        Controller::Bounds bounds1 = epd.drawBottomAlignedText(
            "Bottom Text (Small)",
            startX, startY + 40,
            &FreeMono9pt7b,
            GxEPD_BLACK
        );

        Controller::Bounds bounds2 = epd.drawBottomAlignedText(
            "Bottom Text (Medium)",
            startX + 200, startY + 40,
            &FreeMono12pt7b,
            GxEPD_BLACK
        );

        Controller::Bounds bounds3 = epd.drawBottomAlignedText(
            "Bottom Text (Large)",
            startX + 400, startY + 40,
            &FreeMono18pt7b,
            GxEPD_BLACK
        );

        // Draw bounds rectangles
        display.drawRect(bounds1.x, bounds1.y, bounds1.w, bounds1.h, GxEPD_BLACK);
        display.drawRect(bounds2.x, bounds2.y, bounds2.w, bounds2.h, GxEPD_BLACK);
        display.drawRect(bounds3.x, bounds3.y, bounds3.w, bounds3.h, GxEPD_BLACK);
    }

    void drawCenteredTextDemo(Controller &epd) {
        auto &display = epd.getDisplay();

        const int centerX = display.width() / 2;
        const int startY = 280;

        // Section header
        display.setFont(&FreeMono12pt7b);
        display.setCursor(20, startY);
        display.print("Centered Text");

        // Draw reference cross at center point
        display.drawFastHLine(centerX - 50, startY + 40, 100, GxEPD_BLACK);
        display.drawFastVLine(centerX, startY + 20, 150, GxEPD_BLACK);

        // Demo with different fonts
        Controller::Bounds bounds1 = epd.drawCenteredText(
            "Centered Text (Small)",
            centerX, startY + 40,
            &FreeMono9pt7b,
            GxEPD_BLACK
        );

        Controller::Bounds bounds2 = epd.drawCenteredText(
            "Centered Text (Medium)",
            centerX, startY + 80,
            &FreeMono12pt7b,
            GxEPD_BLACK
        );

        Controller::Bounds bounds3 = epd.drawCenteredText(
            "Centered Text (Large)",
            centerX, startY + 130,
            &FreeMono18pt7b,
            GxEPD_BLACK
        );

        // Draw bounds rectangles
        display.drawRect(bounds1.x, bounds1.y, bounds1.w, bounds1.h, GxEPD_BLACK);
        display.drawRect(bounds2.x, bounds2.y, bounds2.w, bounds2.h, GxEPD_BLACK);
        display.drawRect(bounds3.x, bounds3.y, bounds3.w, bounds3.h, GxEPD_BLACK);
    }

    void drawAlignmentComparison(Controller &epd) {
        auto &display = epd.getDisplay();

        const int startX = 50;
        const int startY = 430;
        const int centerX = display.width() / 2;

        // Section header
        display.setFont(&FreeMono12pt7b);
        display.setCursor(20, 410);
        display.print("Alignment Comparison");

        // Draw reference point
        display.fillCircle(startX, startY, 3, GxEPD_BLACK);
        display.fillCircle(centerX, startY, 3, GxEPD_BLACK);

        // Draw the three alignment types at the same coordinates
        Controller::Bounds bounds1 = epd.drawText(
            "Normal",
            startX, startY,
            &FreeMono12pt7b,
            GxEPD_BLACK
        );

        Controller::Bounds bounds2 = epd.drawBottomAlignedText(
            "Bottom",
            startX + 150, startY,
            &FreeMono12pt7b,
            GxEPD_BLACK
        );

        Controller::Bounds bounds3 = epd.drawCenteredText(
            "Centered",
            centerX, startY,
            &FreeMono12pt7b,
            GxEPD_BLACK
        );

        // Draw bounds for each text element
        display.drawRect(bounds1.x, bounds1.y, bounds1.w, bounds1.h, GxEPD_BLACK);
        display.drawRect(bounds2.x, bounds2.y, bounds2.w, bounds2.h, GxEPD_BLACK);
        display.drawRect(bounds3.x, bounds3.y, bounds3.w, bounds3.h, GxEPD_BLACK);

        // Add explanatory text
        display.setFont(&FreeMono9pt7b);
        display.setCursor(20, 480);
        display.print("The rectangles show the text bounds. Note how alignment affects positioning.");
    }
};

#endif // SAMPLEPAGE_H


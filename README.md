# GXUI > The unofficial [GxEPD2_4G](https://github.com/ZinggJM/GxEPD2_4G) UI Library

## Introduction / History
This library was originally created for an attempt at digitizing DnD5e with varying success.

After picking the project back up, I decided to open-source this library using the MIT license, for use with any GxEPD2-based display.

## Features
This librynary is still under development, but current features include:
###  Standardized Navigation System
Navigation feels like using tab on a keyboard.
Each action (left, right, up, down, select) can be mapped to a key on the keyboard, or any other input device.

**Planned:** Support for touchscreen navigation.

###  Customizable UI Elements
Each UI element can be customized to fit your needs.


#### Currently supported elements:
**Menu**
- MenuBar
- MenuPage
- MenuItem

**Interactable:**
- Button
- Toggle
- Slider
- TextInput
- Toggle
- Dropdown
- Modal

**Non-interactable:**
- Icon
- ProgressBar

#### Icon System:

This project includes a small utility script that converts SVG files into compact C++ header files containing 1-bit bitmaps suitable for e-paper displays. It also generates an index header that includes all produced icons.

- Script: `convert_icons.py`
- Default source directory: `icons`
- Default output directory: `include/icons`

Usage via CLI:

```
python3 convert_icons.py [--svg-dir <path/to/svg_root>] [--output-dir <path/to/output_headers>]
```

Parameters:
- `--svg-dir` (optional): Path to the folder that contains your SVG files. If omitted, defaults to `./icons`.
- `--output-dir` (optional): Path to the folder where generated headers will be written. If omitted, defaults to `./include/icons`.

Behavior:
- Recursively searches `--svg-dir` for all `*.svg` files.
- Preserves the directory structure under `--output-dir` and writes a matching `.h` file for each SVG.
- Generates an index header at `<output-dir>/icons.h` that `#include`s all generated icon headers.
- Each icon header defines a function named after its path, sanitized for C++ identifiers (directory separators become `_`, dashes `-` become `_`). Example: `icons/ui/arrow-left.svg` produces a function `ui_arrow_left_icon()`.

Examples:
- Use defaults (SVGs in `./icons`, headers to `./include/icons`):
  ```
  python3 convert_icons.py
  ```
- Custom locations:
  ```
  python3 convert_icons.py --svg-dir assets/svg --output-dir firmware/include/icons
  ```

Including in C++:
- After running the script, include the generated index header:
  ```cpp
  #include <icons/icons.h>
  // or, relative include if not installed as a system include
  // #include "include/icons/icons.h"
  ```
- Use an icon:
  ```cpp
  auto &icon = ui_arrow_left_icon();
  ```

Notes:
- The script requires Python with the dependencies listed in `requirements.txt`.
- If the specified SVG source directory does not exist, the script prints an error and exits without generating files.

## Getting Started
The following example shows the simplest setup with the addition of UI control via Serial
```cpp


Controller& epd = Controller::getInstance();

Preferences preferences;

void handleNavigation(const String& action)
{
    if (action == "left" || action == "a" || action == "A")
    {
        switch (RenderManager::getCurrentRenderFocus())
        {
        case RenderFocus::PAGE:
            MenuSystem::open();
            break;
        default:
            RenderManager::onActionLeftStatic();
            break;
        }
        Serial.println("Move Left");
    }
    else if (action == "right" || action == "d" || action == "D")
    {
        switch (RenderManager::getCurrentRenderFocus())
        {
        case RenderFocus::PAGE:
            epd.getDisplay().display(); // Refresh the display
            delay(100);
            epd.getDisplay().display(true);
            delay(200);
            break;
        default:
            RenderManager::onActionRightStatic();
            break;
        }
        Serial.println("Move Right");
    }
    else if (action == "down" || action == "s" || action == "S")
    {
        RenderManager::onActionDownStatic();
        Serial.println("Move Down");
    }
    else if (action == "up" || action == "w" || action == "W" || action == "b" || action == "B")
    {
        RenderManager::onActionUpStatic();
        Serial.println("Move Up");
    }
    else if (action == "enter" || action == "\r" || action == "\n")
    {
        RenderManager::onActionStatic();
        Serial.println("Enter");
    }
}


void setup()
{
    Serial.begin(115200);

    if (!preferences.begin("config", false))
    {
        Serial.println("Failed to initialize preferences");
    }
    else
    {
        Serial.println("Preferences initialized successfully");
    }
    
    epd.init(&preferences, false);

    RenderManager::init(epd);
    MenuSystem::init();
    MenuSystem::open();
}


void loop()
{
    if (Serial.available() > 0)
    {
        const char incomingByte = Serial.read();
        handleNavigation(&incomingByte);

        while (Serial.available() > 0)
            Serial.read();
    }
}
```

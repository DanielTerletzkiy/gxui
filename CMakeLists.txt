cmake_minimum_required(VERSION 3.16)
project(gxui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only interface library so CLion associates headers with a target
add_library(gxui INTERFACE)

# Public include directories for consumers and for IDE indexing
target_include_directories(gxui INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/example
)

# List headers explicitly so IDEs (CLion) show them under the target
# This does not affect build output because the target is INTERFACE
set(GXUI_HEADERS
    example/SamplePage.h
    include/EPDComponent.h
    include/EPDController.h
    include/EPDIcon.h
    include/EPDInteractable.h
    include/EPDMenu.h
    include/EPDMenuConstants.h
    include/EPDPage.h
    include/EPDRenderable.h
    include/EPDRenderManager.h
)

target_sources(gxui INTERFACE ${GXUI_HEADERS})

# Provide a no-op build target so CLion/Ninja have something to build
# This target lists all headers as sources; building it is a no-op but succeeds
add_custom_target(gxui_headers ALL SOURCES ${GXUI_HEADERS})

# Convenience alias target you can select in CLion
add_custom_target(gxui_build DEPENDS gxui_headers)

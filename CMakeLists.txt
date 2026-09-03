cmake_minimum_required(VERSION 3.31)

project(ForgeSlice VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(FORGESLICE_BUILD_TESTS "Build ForgeSlice tests" ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum supported macOS version")
endif()

add_executable(ForgeSlice
    src/main.cpp
    src/app/Application.cpp
    src/app/Application.hpp
    src/core/BuildInfo.hpp
    src/core/ForgeConfig.hpp
    src/core/PrinterProfile.cpp
    src/core/PrinterProfile.hpp
    src/core/SettingsMode.cpp
    src/core/SettingsMode.hpp
    src/slicer/Mesh.cpp
    src/slicer/Mesh.hpp
    src/slicer/Slicer.cpp
    src/slicer/Slicer.hpp
)
target_include_directories(ForgeSlice PRIVATE src)
target_compile_definitions(ForgeSlice PRIVATE FORGESLICE_VERSION=\"${PROJECT_VERSION}\")

if(APPLE)
    set_target_properties(ForgeSlice PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.forgeslice.ForgeSlice"
        MACOSX_BUNDLE_BUNDLE_NAME "ForgeSlice"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    )
endif()

if(FORGESLICE_BUILD_TESTS)
    enable_testing()
    add_executable(ForgeSliceTests
        tests/PrinterProfileTests.cpp
        src/core/PrinterProfile.cpp
        src/core/PrinterProfile.hpp
        src/slicer/Mesh.cpp
        src/slicer/Mesh.hpp
        src/slicer/Slicer.cpp
        src/slicer/Slicer.hpp
    )
    target_include_directories(ForgeSliceTests PRIVATE src)
    add_test(NAME ForgeSliceTests COMMAND ForgeSliceTests)
endif()

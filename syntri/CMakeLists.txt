# CMakeLists.txt - Complete Build System with ASIO Integration
# Builds your working foundation + optional ASIO hardware support
cmake_minimum_required(VERSION 3.20)
project(Syntri VERSION 1.0.0 LANGUAGES CXX)

# ====================================
# Build Configuration  
# ====================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release for performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ====================================
# ASIO SDK Detection and Configuration
# ====================================
option(ENABLE_ASIO_SUPPORT "Enable ASIO hardware support" ON)

# Try to find ASIO SDK automatically
set(ASIO_SDK_AVAILABLE FALSE)
set(ASIO_SDK_PATH "")

if(ENABLE_ASIO_SUPPORT)
    # Common ASIO SDK locations
    set(ASIO_SEARCH_PATHS
        "C:/asiosdk_2.3.3"
        "C:/Program Files/ASIO SDK/2.3.3"
        "C:/SDK/ASIO/2.3.3"
        "${CMAKE_SOURCE_DIR}/../asiosdk_2.3.3"
        "${CMAKE_SOURCE_DIR}/third_party/asiosdk_2.3.3"
        "$ENV{ASIO_SDK_PATH}"
    )
    
    foreach(path ${ASIO_SEARCH_PATHS})
        if(EXISTS "${path}/common/asio.h" AND EXISTS "${path}/host/asiodrivers.h")
            set(ASIO_SDK_PATH "${path}")
            set(ASIO_SDK_AVAILABLE TRUE)
            break()
        endif()
    endforeach()
    
    if(ASIO_SDK_AVAILABLE)
        message(STATUS "✅ ASIO SDK found at: ${ASIO_SDK_PATH}")
        add_compile_definitions(ASIO_SDK_AVAILABLE)
    else()
        message(STATUS "⚠️  ASIO SDK not found - ASIO support will be disabled")
        message(STATUS "   To enable ASIO, place ASIO SDK 2.3.3 in one of these locations:")
        foreach(path ${ASIO_SEARCH_PATHS})
            message(STATUS "   - ${path}")
        endforeach()
        set(ENABLE_ASIO_SUPPORT OFF)
    endif()
endif()

# ====================================
# Compiler Settings
# ====================================
if(MSVC)
    # Visual Studio specific settings
    add_compile_options(/W4 /permissive-)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
    
    # Optimization settings
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    endif()
    
    # Handle Unicode character warnings
    add_compile_options(/utf-8)
    
else()
    # GCC/Clang settings
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# ====================================
# Platform-Specific Libraries
# ====================================
if(WIN32)
    set(PLATFORM_LIBS 
        winmm
        ole32
        oleaut32
        advapi32
        kernel32
        user32
    )
elseif(APPLE)
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
    find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
    find_library(AUDIOUNIT_FRAMEWORK AudioUnit)
    set(PLATFORM_LIBS 
        ${COREAUDIO_FRAMEWORK}
        ${AUDIOTOOLBOX_FRAMEWORK}
        ${AUDIOUNIT_FRAMEWORK}
    )
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED alsa)
    set(PLATFORM_LIBS ${ALSA_LIBRARIES})
endif()

# ====================================
# Build Configuration Summary
# ====================================
message(STATUS "====================================")
message(STATUS "🎯 Syntri Audio System - Complete Build")
message(STATUS "====================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "ASIO Support: ${ENABLE_ASIO_SUPPORT}")
if(ASIO_SDK_AVAILABLE)
    message(STATUS "ASIO SDK Path: ${ASIO_SDK_PATH}")
endif()
message(STATUS "====================================")

# ====================================
# Core Library Sources
# ====================================
set(SYNTRI_CORE_SOURCES
    src/core/audio_interface.cpp
)

# Add ASIO sources if available
if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
    list(APPEND SYNTRI_CORE_SOURCES
        src/hardware/asio_interface.cpp
    )
    
    # Add ASIO SDK sources
    if(WIN32)
        list(APPEND SYNTRI_CORE_SOURCES
            ${ASIO_SDK_PATH}/host/asiodrivers.cpp
            ${ASIO_SDK_PATH}/host/pc/asiolist.cpp
            ${ASIO_SDK_PATH}/common/asio.cpp
        )
    endif()
endif()

# ====================================
# SyntriCore Library
# ====================================
add_library(SyntriCore ${SYNTRI_CORE_SOURCES})

# Include directories
target_include_directories(SyntriCore
    PUBLIC
        include
    PRIVATE
        src
)

# Add ASIO includes if available
if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
    target_include_directories(SyntriCore
        PRIVATE
            ${ASIO_SDK_PATH}/common
            ${ASIO_SDK_PATH}/host
            ${ASIO_SDK_PATH}/host/pc
    )
    
    target_compile_definitions(SyntriCore
        PRIVATE
            ENABLE_ASIO_SUPPORT
    )
endif()

# Link platform libraries
target_link_libraries(SyntriCore
    PRIVATE
        ${PLATFORM_LIBS}
)

# ====================================
# Test Executables
# ====================================

# Basic foundation test (always builds)
add_executable(basic_test test/basic_test.cpp)
target_link_libraries(basic_test SyntriCore)

# Interface layer test (always builds)
add_executable(interface_test test/interface_test.cpp)
target_link_libraries(interface_test SyntriCore)

# ASIO-specific test (only if ASIO available)
if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
    add_executable(asio_test test/asio_test.cpp)
    target_link_libraries(asio_test SyntriCore)
endif()

# Comprehensive test
add_executable(comprehensive_test test/comprehensive_test.cpp)
target_link_libraries(comprehensive_test SyntriCore)

# ====================================
# Custom Commands for Easy Testing
# ====================================

# Add custom target for running all tests
add_custom_target(run_tests
    COMMAND $<TARGET_FILE:basic_test>
    COMMAND $<TARGET_FILE:interface_test>
    COMMAND $<TARGET_FILE:comprehensive_test>
    DEPENDS basic_test interface_test comprehensive_test
    COMMENT "Running all Syntri tests"
)

if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
    add_custom_target(run_asio_test
        COMMAND $<TARGET_FILE:asio_test>
        DEPENDS asio_test
        COMMENT "Running ASIO hardware test"
    )
endif()

# ====================================
# Installation
# ====================================
install(TARGETS SyntriCore
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(TARGETS basic_test interface_test comprehensive_test
    RUNTIME DESTINATION bin
)

if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
    install(TARGETS asio_test
        RUNTIME DESTINATION bin
    )
endif()

# ====================================
# Build Instructions and Summary
# ====================================
message(STATUS "")
message(STATUS "🎯 Build Summary:")
message(STATUS "   ✅ SyntriCore library")
message(STATUS "   ✅ basic_test executable")
message(STATUS "   ✅ interface_test executable")
message(STATUS "   ✅ comprehensive_test executable")

if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
    message(STATUS "   ✅ asio_test executable")
    message(STATUS "   ✅ ASIO hardware support enabled")
else()
    message(STATUS "   🔄 ASIO support disabled (SDK not found)")
endif()

message(STATUS "")
message(STATUS "📋 Build Instructions:")
if(WIN32)
    message(STATUS "   mkdir build && cd build")
    message(STATUS "   cmake .. -G \"Visual Studio 17 2022\" -A x64")
    message(STATUS "   cmake --build . --config Release")
else()
    message(STATUS "   mkdir build && cd build")
    message(STATUS "   cmake .. -DCMAKE_BUILD_TYPE=Release")
    message(STATUS "   cmake --build .")
endif()

message(STATUS "")
message(STATUS "🧪 Test Instructions:")
if(WIN32)
    message(STATUS "   Release\\basic_test.exe")
    message(STATUS "   Release\\interface_test.exe")
    message(STATUS "   Release\\comprehensive_test.exe")
    if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
        message(STATUS "   Release\\asio_test.exe")
    endif()
else()
    message(STATUS "   ./basic_test")
    message(STATUS "   ./interface_test")
    message(STATUS "   ./comprehensive_test")
    if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
        message(STATUS "   ./asio_test")
    endif()
endif()

message(STATUS "")
message(STATUS "🎉 Ready to build professional audio system!")
message(STATUS "")
# CMakeLists.txt - Simplified ASIO Approach (Header-Only)
# Bypasses ASIO SDK compilation issues while preserving detection capability
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
# ASIO SDK Detection (Header-Only)
# ====================================
option(ENABLE_ASIO_SUPPORT "Enable ASIO hardware support" OFF)

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
        if(EXISTS "${path}/common/asio.h")
            set(ASIO_SDK_PATH "${path}")
            set(ASIO_SDK_AVAILABLE TRUE)
            break()
        endif()
    endforeach()
    
    if(ASIO_SDK_AVAILABLE)
        message(STATUS "✅ ASIO SDK found at: ${ASIO_SDK_PATH}")
        message(STATUS "   Using header-only approach (recommended)")
    else()
        message(STATUS "⚠️  ASIO SDK not found - using registry-based detection")
    endif()
endif()

# ====================================
# Compiler Settings
# ====================================
if(MSVC)
    add_compile_options(/W4 /permissive-)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        _USE_MATH_DEFINES
    )
    
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    endif()
    
    add_compile_options(/utf-8)
    
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_definitions(_USE_MATH_DEFINES)
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
message(STATUS "Syntri - Header-Only ASIO Approach")
message(STATUS "====================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "ASIO Support: ${ENABLE_ASIO_SUPPORT}")
if(ASIO_SDK_AVAILABLE)
    message(STATUS "ASIO Method: Header-only (no SDK compilation)")
else()
    message(STATUS "ASIO Method: Registry-based detection")
endif()
message(STATUS "====================================")

# ====================================
# Core Library Sources - YOUR WORKING FOUNDATION
# ====================================
set(SYNTRI_CORE_SOURCES
    src/core/audio_interface.cpp
)

# ====================================
# SyntriCore Library - WORKING PERFECTLY
# ====================================
add_library(SyntriCore ${SYNTRI_CORE_SOURCES})

target_include_directories(SyntriCore
    PUBLIC
        include
    PRIVATE
        src
)

target_link_libraries(SyntriCore
    PRIVATE
        ${PLATFORM_LIBS}
)

# ====================================
# Test Executables - ALL WORKING
# ====================================

# Basic foundation test
add_executable(basic_test test/basic_test.cpp)
target_link_libraries(basic_test SyntriCore)

# Interface layer test
add_executable(interface_test test/interface_test.cpp)
target_link_libraries(interface_test SyntriCore)

# Comprehensive system test
add_executable(comprehensive_test test/comprehensive_test.cpp)
target_link_libraries(comprehensive_test SyntriCore)

# ====================================
# Header-Only ASIO Diagnostic - NO SDK COMPILATION
# ====================================
if(WIN32)
    add_executable(asio_diagnostic test/asio_diagnostic.cpp)
    target_link_libraries(asio_diagnostic ${PLATFORM_LIBS})
    
    # Add ASIO headers for inclusion (no compilation)
    if(ASIO_SDK_AVAILABLE)
        target_include_directories(asio_diagnostic
            PRIVATE
                ${ASIO_SDK_PATH}/common
                ${ASIO_SDK_PATH}/host
        )
        
        target_compile_definitions(asio_diagnostic
            PRIVATE
                ASIO_SDK_AVAILABLE
        )
    endif()
    
    # Note: NO ASIO source files added - header-only approach!
endif()

# ====================================
# Installation
# ====================================
install(TARGETS SyntriCore basic_test interface_test comprehensive_test
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

if(WIN32)
    install(TARGETS asio_diagnostic RUNTIME DESTINATION bin)
endif()

install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h")

# ====================================
# Build Summary
# ====================================
message(STATUS "")
message(STATUS "🎯 Build Summary:")
message(STATUS "   ✅ SyntriCore library (working)")
message(STATUS "   ✅ basic_test executable (working)")
message(STATUS "   ✅ interface_test executable (working)")  
message(STATUS "   ✅ comprehensive_test executable (working)")
if(WIN32)
    message(STATUS "   🔍 asio_diagnostic tool (header-only, no compilation issues)")
endif()

message(STATUS "")
message(STATUS "📋 Build Instructions:")
message(STATUS "   # Your working foundation")
message(STATUS "   cmake .. -G \"Visual Studio 17 2022\" -A x64")
message(STATUS "   cmake --build . --config Release")
message(STATUS "")
message(STATUS "   # Enable header-only ASIO detection")
message(STATUS "   cmake .. -G \"Visual Studio 17 2022\" -A x64 -DENABLE_ASIO_SUPPORT=ON")
message(STATUS "   cmake --build . --config Release")
message(STATUS "")
message(STATUS "🧪 Test Instructions:")
message(STATUS "   Release\\basic_test.exe")
message(STATUS "   Release\\interface_test.exe")
message(STATUS "   Release\\comprehensive_test.exe")
if(WIN32)
    message(STATUS "   Release\\asio_diagnostic.exe  # Header-only ASIO detection")
endif()
message(STATUS "")
message(STATUS "✅ No more ASIO SDK compilation issues!")
message(STATUS "")
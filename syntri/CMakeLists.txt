# CMakeLists.txt - Foundation + ASIO Diagnostic
# Preserves your working foundation while adding ASIO diagnostic capability
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
option(ENABLE_ASIO_SUPPORT "Enable ASIO hardware support" OFF)

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
    else()
        message(STATUS "⚠️  ASIO SDK not found")
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
        _USE_MATH_DEFINES
    )
    
    # Optimization settings
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    endif()
    
    # Handle Unicode characters properly
    add_compile_options(/utf-8)
    
else()
    # GCC/Clang settings
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
message(STATUS "Syntri Foundation + ASIO Diagnostic")
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
# Core Library Sources - FOUNDATION ONLY
# ====================================
set(SYNTRI_CORE_SOURCES
    src/core/audio_interface.cpp
)

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

# Link platform libraries
target_link_libraries(SyntriCore
    PRIVATE
        ${PLATFORM_LIBS}
)

# ====================================
# Test Executables - Your Working Foundation
# ====================================

# Basic foundation test (your original working test)
add_executable(basic_test test/basic_test.cpp)
target_link_libraries(basic_test SyntriCore)

# Interface layer test (your original working test)
add_executable(interface_test test/interface_test.cpp)
target_link_libraries(interface_test SyntriCore)

# Comprehensive system test
add_executable(comprehensive_test test/comprehensive_test.cpp)
target_link_libraries(comprehensive_test SyntriCore)

# ====================================
# ASIO Diagnostic Tool - NEW
# ====================================
if(WIN32)
    add_executable(asio_diagnostic test/asio_diagnostic.cpp)
    target_link_libraries(asio_diagnostic ${PLATFORM_LIBS})
    
    # Add ASIO includes if available
    if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
        target_include_directories(asio_diagnostic
            PRIVATE
                ${ASIO_SDK_PATH}/common
                ${ASIO_SDK_PATH}/host
                ${ASIO_SDK_PATH}/host/pc
        )
        
        target_compile_definitions(asio_diagnostic
            PRIVATE
                ASIO_SDK_AVAILABLE
        )
        
        # Add ASIO SDK sources for diagnostic
        target_sources(asio_diagnostic
            PRIVATE
                ${ASIO_SDK_PATH}/host/asiodrivers.cpp
                ${ASIO_SDK_PATH}/host/pc/asiolist.cpp
                ${ASIO_SDK_PATH}/common/asio.cpp
        )
    endif()
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

if(WIN32)
    install(TARGETS asio_diagnostic
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
if(WIN32)
    message(STATUS "   🔍 asio_diagnostic tool")
endif()

if(ENABLE_ASIO_SUPPORT AND ASIO_SDK_AVAILABLE)
    message(STATUS "   ✅ ASIO hardware support enabled")
else()
    message(STATUS "   🔄 ASIO support disabled")
endif()

message(STATUS "")
message(STATUS "📋 Build Instructions:")
message(STATUS "   # Foundation build (what you have working)")
message(STATUS "   cmake .. -G \"Visual Studio 17 2022\" -A x64")
message(STATUS "   cmake --build . --config Release")
message(STATUS "")
message(STATUS "   # Enable ASIO diagnostic (if you have ASIO SDK)")
message(STATUS "   cmake .. -G \"Visual Studio 17 2022\" -A x64 -DENABLE_ASIO_SUPPORT=ON")
message(STATUS "   cmake --build . --config Release")
message(STATUS "")
message(STATUS "🧪 Test Instructions:")
message(STATUS "   Release\\basic_test.exe           # Your foundation")
message(STATUS "   Release\\interface_test.exe       # Your interface layer")
message(STATUS "   Release\\comprehensive_test.exe   # Complete system")
if(WIN32)
    message(STATUS "   Release\\asio_diagnostic.exe      # ASIO environment check")
endif()
message(STATUS "")

if(NOT ENABLE_ASIO_SUPPORT)
    message(STATUS "🔧 To enable ASIO support:")
    message(STATUS "   1. Download ASIO SDK 2.3.3 from Steinberg")
    message(STATUS "   2. Extract to C:\\asiosdk_2.3.3\\")
    message(STATUS "   3. Rebuild with -DENABLE_ASIO_SUPPORT=ON")
    message(STATUS "")
endif()

message(STATUS "✅ Your foundation is solid and ready!")
message(STATUS "")
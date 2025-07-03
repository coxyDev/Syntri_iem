# CMakeLists.txt - Fixed ASIO integration with proper header handling
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
# Compiler Settings
# ====================================
if(MSVC)
    # Visual Studio specific settings
    add_compile_options(/W4 /permissive-)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        _UNICODE
        UNICODE
    )
    
    # Optimization settings
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    endif()
else()
    # GCC/Clang settings
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# ====================================
# ASIO SDK Detection (Optional)
# ====================================
option(ENABLE_ASIO_SUPPORT "Enable ASIO audio driver support" ON)

if(ENABLE_ASIO_SUPPORT)
    # Look for ASIO SDK with proper path handling
    if(DEFINED ENV{ASIO_SDK_PATH})
        file(TO_CMAKE_PATH "$ENV{ASIO_SDK_PATH}" ASIO_SDK_PATH)
    else()
        # Try common locations with forward slashes (CMake prefers these)
        find_path(ASIO_SDK_PATH
            NAMES "common/asio.h"
            PATHS
                "C:/asiosdk_2.3.3"
                "C:/ASIOSDK2.3.3"
                "${CMAKE_SOURCE_DIR}/../asiosdk_2.3.3"
                "${CMAKE_SOURCE_DIR}/third_party/asiosdk"
                "/usr/local/include/asiosdk"
            DOC "Path to ASIO SDK"
        )
    endif()

    # Normalize path separators for CMake
    file(TO_CMAKE_PATH "${ASIO_SDK_PATH}" ASIO_SDK_PATH_NORMALIZED)
    
    if(ASIO_SDK_PATH_NORMALIZED AND EXISTS "${ASIO_SDK_PATH_NORMALIZED}/common/asio.h")
        message(STATUS "✅ ASIO SDK found: ${ASIO_SDK_PATH_NORMALIZED}")
        
        # ASIO source files with proper path handling
        set(ASIO_SOURCES
            "${ASIO_SDK_PATH_NORMALIZED}/common/asio.cpp"
            "${ASIO_SDK_PATH_NORMALIZED}/host/asiodrivers.cpp"
        )
        
        # Platform-specific ASIO sources
        if(WIN32)
            list(APPEND ASIO_SOURCES
                "${ASIO_SDK_PATH_NORMALIZED}/host/pc/asiolist.cpp"
            )
        endif()
        
        # Check if all ASIO files exist
        set(ASIO_FILES_EXIST TRUE)
        foreach(ASIO_FILE ${ASIO_SOURCES})
            if(NOT EXISTS "${ASIO_FILE}")
                message(WARNING "ASIO file missing: ${ASIO_FILE}")
                set(ASIO_FILES_EXIST FALSE)
            endif()
        endforeach()
        
        if(ASIO_FILES_EXIST)
            set(ASIO_INCLUDE_DIRS
                "${ASIO_SDK_PATH_NORMALIZED}/common"
                "${ASIO_SDK_PATH_NORMALIZED}/host"
                "${ASIO_SDK_PATH_NORMALIZED}/host/pc"
            )
            set(ASIO_FOUND TRUE)
            message(STATUS "✅ ASIO integration enabled")
        else()
            set(ASIO_FOUND FALSE)
            message(WARNING "❌ ASIO files missing - disabling ASIO support")
        endif()
    else()
        set(ASIO_FOUND FALSE)
        message(WARNING "❌ ASIO SDK not found - disabling ASIO support")
        message(STATUS "   Set ASIO_SDK_PATH environment variable or place SDK in:")
        message(STATUS "   - C:/asiosdk_2.3.3")
        message(STATUS "   - ${CMAKE_SOURCE_DIR}/../asiosdk_2.3.3")
    endif()
else()
    set(ASIO_FOUND FALSE)
    message(STATUS "🔕 ASIO support disabled by user")
endif()

# ====================================
# Build Configuration Summary
# ====================================
message(STATUS "====================================")
message(STATUS "Syntri Phase 1 - Fixed ASIO Integration")
message(STATUS "====================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
if(ASIO_FOUND)
    message(STATUS "ASIO Support: ✅ ENABLED")
    message(STATUS "ASIO SDK: ${ASIO_SDK_PATH_NORMALIZED}")
else()
    message(STATUS "ASIO Support: ❌ DISABLED")
endif()
message(STATUS "====================================")

# ====================================
# Core Library Sources
# ====================================
set(SYNTRI_SOURCES
    src/core/audio_interface.cpp
)

# Add ASIO implementation if available
if(ASIO_FOUND)
    list(APPEND SYNTRI_SOURCES
        src/hardware/asio_interface.cpp
        ${ASIO_SOURCES}
    )
endif()

# ====================================
# SyntriCore Library
# ====================================
add_library(SyntriCore ${SYNTRI_SOURCES})

target_include_directories(SyntriCore
    PUBLIC
        include
    PRIVATE
        src
)

# Add ASIO includes and definitions if available
if(ASIO_FOUND)
    target_include_directories(SyntriCore PRIVATE ${ASIO_INCLUDE_DIRS})
    target_compile_definitions(SyntriCore PUBLIC ENABLE_ASIO_SUPPORT)
    
    # Windows-specific ASIO setup
    if(WIN32)
        target_link_libraries(SyntriCore PRIVATE ole32 oleaut32 uuid)
        
        # Disable specific warnings for ASIO SDK and enable proper Unicode
        if(MSVC)
            target_compile_options(SyntriCore PRIVATE
                /wd4996  # Disable deprecated function warnings
                /wd4267  # Disable size_t conversion warnings
                /wd4244  # Disable type conversion warnings
                /wd4430  # Disable missing type specifier warnings (ASIO SDK issue)
                /wd2146  # Disable syntax error warnings (ASIO SDK issue)
                /wd2086  # Disable redefinition warnings (ASIO SDK issue)
            )
            
            # Add Unicode definitions for ASIO SDK
            target_compile_definitions(SyntriCore PRIVATE
                UNICODE
                _UNICODE
                STRICT=1
            )
        endif()
    endif()
endif()

# ====================================
# Test Executables
# ====================================

# Basic foundation test
add_executable(basic_test test/basic_test.cpp)
target_link_libraries(basic_test SyntriCore)

# Interface layer test
add_executable(interface_test test/interface_test.cpp)
target_link_libraries(interface_test SyntriCore)

# ASIO integration test (always built, but with fallback behavior)
add_executable(asio_test test/asio_test.cpp)
target_link_libraries(asio_test SyntriCore)

# ====================================
# Installation (Optional)
# ====================================
install(TARGETS SyntriCore basic_test interface_test asio_test
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

# ====================================
# Build Status Summary
# ====================================
message(STATUS "")
message(STATUS "🎯 Build Summary:")
message(STATUS "   ✅ SyntriCore library")
message(STATUS "   ✅ basic_test executable")
message(STATUS "   ✅ interface_test executable")
message(STATUS "   ✅ asio_test executable")
if(ASIO_FOUND)
    message(STATUS "   ✅ ASIO hardware support")
else()
    message(STATUS "   🔄 ASIO support (disabled - stub mode only)")
endif()
message(STATUS "")
message(STATUS "📋 Build Instructions:")
message(STATUS "   mkdir build && cd build")
message(STATUS "   cmake .. -G \"Visual Studio 17 2022\" -A x64")
message(STATUS "   cmake --build . --config Release")
message(STATUS "")
message(STATUS "🧪 Test Instructions:")
message(STATUS "   Release\\basic_test.exe")
message(STATUS "   Release\\interface_test.exe")
message(STATUS "   Release\\asio_test.exe")
message(STATUS "")
if(NOT ASIO_FOUND)
    message(STATUS "💡 To enable ASIO support:")
    message(STATUS "   1. Download ASIO SDK 2.3.3 from Steinberg")
    message(STATUS "   2. Extract to C:\\asiosdk_2.3.3")
    message(STATUS "   3. Reconfigure: cmake .. -DENABLE_ASIO_SUPPORT=ON")
    message(STATUS "")
endif()
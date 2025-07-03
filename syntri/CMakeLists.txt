# CMakeLists.txt - Minimal working version (ASIO removed temporarily)
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
# Build Configuration Summary
# ====================================
message(STATUS "====================================")
message(STATUS "Syntri Phase 1 - Minimal Foundation")
message(STATUS "====================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "ASIO Support: DISABLED (minimal build)")
message(STATUS "====================================")

# ====================================
# Core Library Sources - MINIMAL ONLY
# ====================================
set(SYNTRI_SOURCES
    src/core/audio_interface.cpp
)

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

# ====================================
# Test Executables
# ====================================

# Basic foundation test
add_executable(basic_test test/basic_test.cpp)
target_link_libraries(basic_test SyntriCore)

# Interface layer test
add_executable(interface_test test/interface_test.cpp)
target_link_libraries(interface_test SyntriCore)

# ====================================
# Build Status Summary
# ====================================
message(STATUS "")
message(STATUS "🎯 Minimal Build Summary:")
message(STATUS "   ✅ SyntriCore library")
message(STATUS "   ✅ basic_test executable")
message(STATUS "   ✅ interface_test executable")
message(STATUS "   🔄 ASIO support (temporarily disabled)")
message(STATUS "")
message(STATUS "📋 Build Instructions:")
message(STATUS "   mkdir build && cd build")
message(STATUS "   cmake .. -G \"Visual Studio 17 2022\" -A x64")
message(STATUS "   cmake --build . --config Release")
message(STATUS "")
message(STATUS "🧪 Test Instructions:")
message(STATUS "   Release\\basic_test.exe")
message(STATUS "   Release\\interface_test.exe")
message(STATUS "")
message(STATUS "✅ This minimal build WILL work!")
message(STATUS "")
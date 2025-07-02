cmake_minimum_required(VERSION 3.20)
project(Syntri VERSION 1.0.0 LANGUAGES CXX)

# Syntri - Ultra-Low Latency IEM System
# Phase 1: Clean ASIO Integration

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ASIO SDK Integration - Optional
option(ENABLE_ASIO "Enable ASIO support" ON)

if(ENABLE_ASIO AND WIN32)
    # Find ASIO SDK
    if(DEFINED ENV{ASIO_SDK_PATH})
        set(ASIO_SDK_PATH $ENV{ASIO_SDK_PATH})
    else()
        set(ASIO_SDK_PATH "C:/asiosdk_2.3.3")
    endif()

    # Check if ASIO SDK is available
    if(EXISTS "${ASIO_SDK_PATH}/common/asio.h")
        set(ASIO_FOUND TRUE)
        message(STATUS "✅ ASIO SDK found at: ${ASIO_SDK_PATH}")
        add_definitions(-DENABLE_ASIO_SUPPORT=1)
        include_directories(${ASIO_SDK_PATH}/common)
        include_directories(${ASIO_SDK_PATH}/host/pc)
    else()
        set(ASIO_FOUND FALSE)
        message(STATUS "⚠️  ASIO SDK not found - building without ASIO support")
        message(STATUS "   To enable ASIO: Download ASIO SDK 2.3.3 and set ASIO_SDK_PATH")
    endif()
else()
    set(ASIO_FOUND FALSE)
    message(STATUS "ASIO support disabled or not on Windows")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library source files
set(SYNTRI_SOURCES
    src/core/audio_interface.cpp
)

# Add ASIO sources only if ASIO is available
if(ASIO_FOUND)
    list(APPEND SYNTRI_SOURCES
        src/hardware/asio_interface.cpp
    )
endif()

# Core library headers
set(SYNTRI_HEADERS
    include/syntri/types.h
    include/syntri/audio_interface.h
)

if(ASIO_FOUND)
    list(APPEND SYNTRI_HEADERS
        include/syntri/asio_interface.h
    )
endif()

# Create core library
add_library(SyntriCore STATIC
    ${SYNTRI_SOURCES}
    ${SYNTRI_HEADERS}
)

# Set library properties
set_target_properties(SyntriCore PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Compiler definitions
if(ASIO_FOUND)
    target_compile_definitions(SyntriCore PRIVATE ENABLE_ASIO_SUPPORT=1)
endif()

# Test executables (your original working tests)
add_executable(basic_test
    test/basic_test.cpp
)

add_executable(interface_test
    test/interface_test.cpp
)

target_link_libraries(interface_test SyntriCore)

# ASIO test (only if ASIO available)
if(ASIO_FOUND)
    add_executable(asio_test
        test/asio_test.cpp
    )
    target_link_libraries(asio_test SyntriCore)
    target_compile_definitions(asio_test PRIVATE ENABLE_ASIO_SUPPORT=1)
endif()

# Windows-specific libraries
if(WIN32)
    target_link_libraries(basic_test winmm)
    target_link_libraries(SyntriCore winmm ole32 user32)
    target_link_libraries(interface_test winmm)
    
    if(ASIO_FOUND)
        target_link_libraries(asio_test winmm ole32 user32)
    endif()
endif()

# Build summary
message(STATUS "====================================")
message(STATUS "Syntri Phase 1 - Clean ASIO Integration")
message(STATUS "====================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
if(ASIO_FOUND)
    message(STATUS "ASIO Support: ENABLED")
    message(STATUS "ASIO SDK: ${ASIO_SDK_PATH}")
else()
    message(STATUS "ASIO Support: DISABLED (using generic interfaces)")
endif()
message(STATUS "====================================")
message(STATUS "Targets:")
message(STATUS "  basic_test - Foundation verification")
message(STATUS "  interface_test - Audio interface testing")
if(ASIO_FOUND)
    message(STATUS "  asio_test - Real ASIO hardware testing")
else()
    message(STATUS "  asio_test - NOT BUILT (no ASIO SDK)")
endif()
message(STATUS "  SyntriCore - Core library")
message(STATUS "====================================")

# Installation
set(INSTALL_TARGETS SyntriCore basic_test interface_test)
if(ASIO_FOUND)
    list(APPEND INSTALL_TARGETS asio_test)
endif()

install(TARGETS ${INSTALL_TARGETS}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/syntri
    DESTINATION include
)
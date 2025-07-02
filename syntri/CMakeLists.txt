cmake_minimum_required(VERSION 3.20)
project(Syntri VERSION 1.0.0 LANGUAGES CXX)

# Syntri - Ultra-Low Latency IEM System
# Phase 1: ASIO Integration Added

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ASIO SDK Integration
option(ENABLE_ASIO "Enable ASIO support" ON)

if(ENABLE_ASIO AND WIN32)
    # ASIO SDK Path - check environment variable first
    if(DEFINED ENV{ASIO_SDK_PATH})
        set(ASIO_SDK_PATH $ENV{ASIO_SDK_PATH})
    else()
        # Common installation paths
        set(ASIO_SDK_PATH 
            "C:/asiosdk_2.3.3"
            "C:/ASIOSDK2.3.3" 
            "C:/SDKs/asiosdk_2.3.3"
            "${CMAKE_SOURCE_DIR}/external/asio"
        )
    endif()

    # Find ASIO SDK
    find_path(ASIO_INCLUDE_DIR
        NAMES asio.h
        PATHS ${ASIO_SDK_PATH}
        PATH_SUFFIXES common
        DOC "ASIO SDK include directory"
    )

    find_path(ASIO_HOST_DIR
        NAMES asiolist.h
        PATHS ${ASIO_SDK_PATH}
        PATH_SUFFIXES host/pc
        DOC "ASIO SDK host directory"
    )

    if(ASIO_INCLUDE_DIR AND ASIO_HOST_DIR)
        set(ASIO_FOUND TRUE)
        message(STATUS "✅ ASIO SDK found at: ${ASIO_INCLUDE_DIR}/..")
        add_definitions(-DASIO_AVAILABLE=1)
    else()
        set(ASIO_FOUND FALSE)
        message(STATUS "⚠️  ASIO SDK not found - building without ASIO support")
        message(STATUS "   Set ASIO_SDK_PATH environment variable or place SDK at:")
        message(STATUS "   C:/asiosdk_2.3.3/common/asio.h")
    endif()
else()
    set(ASIO_FOUND FALSE)
    message(STATUS "ASIO support disabled")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

if(ASIO_FOUND)
    include_directories(${ASIO_INCLUDE_DIR})
    include_directories(${ASIO_HOST_DIR})
endif()

# Core library source files
set(SYNTRI_SOURCES
    src/core/audio_interface.cpp
)

# Add ASIO sources if available
if(ASIO_FOUND)
    list(APPEND SYNTRI_SOURCES
        src/hardware/asio_interface.cpp
        ${ASIO_INCLUDE_DIR}/asio.cpp
        ${ASIO_HOST_DIR}/asiolist.cpp
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
    target_compile_definitions(SyntriCore PRIVATE ASIO_AVAILABLE=1)
endif()

# Basic test executable (foundation)
add_executable(basic_test
    test/basic_test.cpp
)

# Audio interface test executable
add_executable(interface_test
    test/interface_test.cpp
)

target_link_libraries(interface_test SyntriCore)

# ASIO test executable (if ASIO available)
if(ASIO_FOUND)
    add_executable(asio_test
        test/asio_test.cpp
    )
    target_link_libraries(asio_test SyntriCore)
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
message(STATUS "Syntri Phase 1 - ASIO Integration")
message(STATUS "====================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
if(ASIO_FOUND)
    message(STATUS "ASIO Support: ENABLED")
    message(STATUS "ASIO SDK: ${ASIO_INCLUDE_DIR}/..")
else()
    message(STATUS "ASIO Support: DISABLED (fallback to stubs)")
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
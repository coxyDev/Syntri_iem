cmake_minimum_required(VERSION 3.15)
project(Syntri LANGUAGES CXX)

# ====================================
# Project Configuration
# ====================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(ENABLE_ASIO "Enable ASIO support" ON)

# ====================================
# ASIO SDK Detection and Setup
# ====================================
if(ENABLE_ASIO)
    # Find ASIO SDK
    find_path(ASIO_SDK_PATH
        NAMES common/asio.h
        PATHS
            "C:/asiosdk_2.3.3"
            "C:/ASIOSDK2.3"
            "${CMAKE_CURRENT_SOURCE_DIR}/extern/asiosdk"
        DOC "Path to ASIO SDK"
    )
    
    if(ASIO_SDK_PATH)
        message(STATUS "✅ ASIO SDK found at: ${ASIO_SDK_PATH}")
        
        # ASIO include directories
        set(ASIO_INCLUDE_DIRS
            ${ASIO_SDK_PATH}/common
            ${ASIO_SDK_PATH}/host
            ${ASIO_SDK_PATH}/host/pc
        )
        
        # ASIO source files that are known to work
        set(ASIO_SOURCES
            ${ASIO_SDK_PATH}/common/asio.cpp
            ${ASIO_SDK_PATH}/host/asiodrivers.cpp
            ${ASIO_SDK_PATH}/host/pc/asiolist.cpp
        )
        
        # Check if all required files exist
        set(ASIO_FILES_MISSING FALSE)
        foreach(file ${ASIO_SOURCES})
            if(NOT EXISTS ${file})
                message(WARNING "ASIO file missing: ${file}")
                set(ASIO_FILES_MISSING TRUE)
            endif()
        endforeach()
        
        if(NOT ASIO_FILES_MISSING)
            set(ASIO_FOUND TRUE)
            add_definitions(-DENABLE_ASIO_SUPPORT)
            message(STATUS "✅ ASIO support enabled")
        else()
            set(ASIO_FOUND FALSE)
            message(WARNING "❌ ASIO files missing - disabling ASIO support")
        endif()
    else()
        set(ASIO_FOUND FALSE)
        message(WARNING "❌ ASIO SDK not found - disabling ASIO support")
    endif()
else()
    set(ASIO_FOUND FALSE)
    message(STATUS "🔕 ASIO support disabled by user")
endif()

# ====================================
# Build Configuration
# ====================================
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
    message(STATUS "ASIO Support: DISABLED")
endif()

# ====================================
# Core Library Sources
# ====================================
set(SYNTRI_SOURCES
    src/core/audio_interface.cpp
    src/hardware/asio_interface.cpp
)

# Add ASIO sources if available
if(ASIO_FOUND)
    list(APPEND SYNTRI_SOURCES ${ASIO_SOURCES})
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

# Add ASIO includes if available
if(ASIO_FOUND)
    target_include_directories(SyntriCore PRIVATE ${ASIO_INCLUDE_DIRS})
    
    # Windows-specific ASIO setup with proper COM headers
    if(WIN32)
        target_link_libraries(SyntriCore PRIVATE ole32 oleaut32 uuid advapi32)
        target_compile_definitions(SyntriCore PRIVATE 
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            _WIN32_WINNT=0x0601
        )
    endif()
endif()

# Compiler-specific settings
if(MSVC)
    target_compile_options(SyntriCore PRIVATE /W3)
    # Disable warnings for Unicode characters in source
    target_compile_options(SyntriCore PRIVATE /wd4566)
else()
    target_compile_options(SyntriCore PRIVATE -Wall -Wextra)
endif()

# ====================================
# Test Executables
# ====================================
# Basic Test - Foundation verification
add_executable(basic_test test/basic_test.cpp)
target_link_libraries(basic_test SyntriCore)

# Interface Test - Audio interface testing
add_executable(interface_test test/interface_test.cpp)
target_link_libraries(interface_test SyntriCore)

# ASIO Test - Real hardware testing (only if ASIO available)
if(ASIO_FOUND)
    add_executable(asio_test test/asio_test.cpp)
    target_link_libraries(asio_test SyntriCore)
    message(STATUS "✅ ASIO test target enabled")
else()
    message(STATUS "🔕 ASIO test target disabled (ASIO not available)")
endif()

# ====================================
# Summary
# ====================================
message(STATUS "====================================")
message(STATUS "Targets:")
message(STATUS "  basic_test - Foundation verification")
message(STATUS "  interface_test - Audio interface testing")
if(ASIO_FOUND)
    message(STATUS "  asio_test - Real ASIO hardware testing")
endif()
message(STATUS "  SyntriCore - Core library")
message(STATUS "====================================")
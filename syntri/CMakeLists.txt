# CMakeLists.txt - Clean Build System
# Fixed version that builds successfully without ASIO SDK complications
cmake_minimum_required(VERSION 3.20)
project(Syntri VERSION 1.0.0 LANGUAGES CXX)

# =====================
# COMPILER SETTINGS
# =====================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings for clean compilation
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7+
    add_definitions(-DWINVER=0x0601)
    add_definitions(-DNOMINMAX)  # Prevent Windows.h min/max conflicts
endif()

# =====================
# DIRECTORY STRUCTURE
# =====================
set(SYNTRI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SYNTRI_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SYNTRI_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")

# =====================
# SYNTRI CORE LIBRARY
# =====================
set(SYNTRI_CORE_HEADERS
    "${SYNTRI_INCLUDE_DIR}/syntri/types.h"
    "${SYNTRI_INCLUDE_DIR}/syntri/audio_interface.h"
)

set(SYNTRI_CORE_SOURCES
    "${SYNTRI_SRC_DIR}/core/audio_interface.cpp"
)

# Create the core library
add_library(SyntriCore STATIC
    ${SYNTRI_CORE_HEADERS}
    ${SYNTRI_CORE_SOURCES}
)

target_include_directories(SyntriCore PUBLIC
    ${SYNTRI_INCLUDE_DIR}
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(SyntriCore 
        ole32      # For COM
        advapi32   # For Registry
    )
endif()

# =====================
# TEST EXECUTABLES
# =====================

# Basic Foundation Test
add_executable(basic_test "${SYNTRI_TEST_DIR}/basic_test.cpp")
target_link_libraries(basic_test SyntriCore)

# Interface Layer Test  
add_executable(interface_test "${SYNTRI_TEST_DIR}/interface_test.cpp")
target_link_libraries(interface_test SyntriCore)

# Comprehensive Test
add_executable(comprehensive_test "${SYNTRI_TEST_DIR}/comprehensive_test.cpp")
target_link_libraries(comprehensive_test SyntriCore)

# ASIO Hardware Test (Registry-based, no SDK required)
add_executable(asio_hardware_test "${SYNTRI_TEST_DIR}/asio_hardware_test.cpp")
target_link_libraries(asio_hardware_test 
    ole32      # COM system
    advapi32   # Registry access
)

# =====================
# ASIO DIAGNOSTICS
# =====================
# Keep the working diagnostic tool
if(EXISTS "${SYNTRI_TEST_DIR}/asio_diagnostic.cpp")
    add_executable(asio_diagnostic "${SYNTRI_TEST_DIR}/asio_diagnostic.cpp")
    target_link_libraries(asio_diagnostic 
        ole32 
        advapi32
    )
endif()

# =====================
# BUILD CONFIGURATION
# =====================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(SyntriCore PRIVATE DEBUG=1)
else()
    target_compile_definitions(SyntriCore PRIVATE NDEBUG=1)
endif()

# =====================
# STATUS MESSAGES
# =====================
message(STATUS "==========================================")
message(STATUS "         SYNTRI BUILD CONFIGURATION")
message(STATUS "==========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Include Directory: ${SYNTRI_INCLUDE_DIR}")
message(STATUS "Source Directory: ${SYNTRI_SRC_DIR}")
message(STATUS "Test Directory: ${SYNTRI_TEST_DIR}")
message(STATUS "")
message(STATUS "Build Targets:")
message(STATUS "  - SyntriCore (library)")
message(STATUS "  - basic_test")
message(STATUS "  - interface_test")
message(STATUS "  - comprehensive_test")
message(STATUS "  - asio_hardware_test")
if(EXISTS "${SYNTRI_TEST_DIR}/asio_diagnostic.cpp")
    message(STATUS "  - asio_diagnostic")
endif()
message(STATUS "==========================================")

# =====================
# INSTALLATION (Optional)
# =====================
install(TARGETS SyntriCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${SYNTRI_INCLUDE_DIR}/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
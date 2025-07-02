// Updated include/syntri/types.h - adds missing hardware types
#pragma once

#include <vector>
#include <string>

namespace Syntri {

    // Audio configuration constants
    constexpr int SAMPLE_RATE_96K = 96000;
    constexpr int SAMPLE_RATE_48K = 48000;
    constexpr int BUFFER_SIZE_ULTRA_LOW = 32;   // 0.33ms @ 96kHz
    constexpr int BUFFER_SIZE_LOW = 64;         // 0.67ms @ 96kHz
    constexpr int MAX_AUDIO_CHANNELS = 64;

    // Hardware types we'll support - Updated with all types
    enum class HardwareType {
        UNKNOWN,
        UAD_APOLLO_X16,
        UAD_APOLLO_X8,           // Added
        ALLEN_HEATH_AVANTIS,
        DIGICO_SD9,              // Added
        YAMAHA_CL5,              // Added
        BEHRINGER_X32,
        FOCUSRITE_SCARLETT,      // Added
        RME_BABYFACE,           // Added
        GENERIC_ASIO
    };

    // Audio sample format (32-bit float)
    using AudioSample = float;
    using AudioBuffer = std::vector<AudioSample>;
    using MultiChannelBuffer = std::vector<AudioBuffer>;

    // Simple performance metrics (no atomic complexity yet)
    struct SimpleMetrics {
        double latency_ms = 0.0;
        double cpu_usage_percent = 0.0;
        int buffer_underruns = 0;

        void reset() {
            latency_ms = 0.0;
            cpu_usage_percent = 0.0;
            buffer_underruns = 0;
        }
    };

    // Utility function - Updated with all hardware types
    inline std::string hardwareTypeToString(HardwareType type) {
        switch (type) {
        case HardwareType::UAD_APOLLO_X16: return "UAD Apollo X16";
        case HardwareType::UAD_APOLLO_X8: return "UAD Apollo X8";
        case HardwareType::ALLEN_HEATH_AVANTIS: return "Allen & Heath Avantis";
        case HardwareType::DIGICO_SD9: return "DiGiCo SD9";
        case HardwareType::YAMAHA_CL5: return "Yamaha CL5";
        case HardwareType::BEHRINGER_X32: return "Behringer X32";
        case HardwareType::FOCUSRITE_SCARLETT: return "Focusrite Scarlett";
        case HardwareType::RME_BABYFACE: return "RME Babyface Pro";
        case HardwareType::GENERIC_ASIO: return "Generic ASIO";
        default: return "Unknown";
        }
    }

} // namespace Syntri
// Syntri - Basic Audio Types
// Phase 1: Keep it simple - no complex dependencies
// Copyright (c) 2025 Syntri Technologies

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

    // Hardware types we'll support
    enum class HardwareType {
        UNKNOWN,
        UAD_APOLLO_X16,
        ALLEN_HEATH_AVANTIS,
        BEHRINGER_X32,
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

    // Utility function
    inline std::string hardwareTypeToString(HardwareType type) {
        switch (type) {
        case HardwareType::UAD_APOLLO_X16: return "UAD Apollo X16";
        case HardwareType::ALLEN_HEATH_AVANTIS: return "Allen & Heath Avantis";
        case HardwareType::BEHRINGER_X32: return "Behringer X32";
        case HardwareType::GENERIC_ASIO: return "Generic ASIO";
        default: return "Unknown";
        }
    }

} // namespace Syntri
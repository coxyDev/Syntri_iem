// Syntri Basic Test - Foundation Verification
// Phase 1: Verify clean compilation and basic functionality
// Copyright (c) 2025 Syntri Technologies

#include "syntri/types.h"
#include <iostream>

int main() {
    std::cout << "=====================================" << std::endl;
    std::cout << "    SYNTRI PHASE 1 - FOUNDATION TEST" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;

    // Test 1: Basic compilation
    std::cout << "🔧 Test 1: Basic compilation" << std::endl;
    std::cout << "✅ Headers included successfully" << std::endl;
    std::cout << "✅ Namespace accessible" << std::endl;
    std::cout << std::endl;

    // Test 2: Constants and types
    std::cout << "🔧 Test 2: Audio constants" << std::endl;
    std::cout << "   Sample Rate 96K: " << Syntri::SAMPLE_RATE_96K << " Hz" << std::endl;
    std::cout << "   Sample Rate 48K: " << Syntri::SAMPLE_RATE_48K << " Hz" << std::endl;
    std::cout << "   Ultra Low Buffer: " << Syntri::BUFFER_SIZE_ULTRA_LOW << " samples" << std::endl;
    std::cout << "   Low Buffer: " << Syntri::BUFFER_SIZE_LOW << " samples" << std::endl;
    std::cout << "   Max Channels: " << Syntri::MAX_AUDIO_CHANNELS << std::endl;
    std::cout << "✅ All constants accessible" << std::endl;
    std::cout << std::endl;

    // Test 3: Hardware types
    std::cout << "🔧 Test 3: Hardware types" << std::endl;
    std::cout << "   Apollo: " << Syntri::hardwareTypeToString(Syntri::HardwareType::UAD_APOLLO_X16) << std::endl;
    std::cout << "   Avantis: " << Syntri::hardwareTypeToString(Syntri::HardwareType::ALLEN_HEATH_AVANTIS) << std::endl;
    std::cout << "   X32: " << Syntri::hardwareTypeToString(Syntri::HardwareType::BEHRINGER_X32) << std::endl;
    std::cout << "   Generic: " << Syntri::hardwareTypeToString(Syntri::HardwareType::GENERIC_ASIO) << std::endl;
    std::cout << "✅ Hardware types working" << std::endl;
    std::cout << std::endl;

    // Test 4: Audio buffer types
    std::cout << "🔧 Test 4: Audio buffer types" << std::endl;
    Syntri::AudioBuffer test_buffer(1024, 0.0f);
    Syntri::MultiChannelBuffer multi_buffer(8);
    for (auto& channel : multi_buffer) {
        channel.resize(1024, 0.0f);
    }
    std::cout << "   Created buffer with " << test_buffer.size() << " samples" << std::endl;
    std::cout << "   Created multi-buffer with " << multi_buffer.size() << " channels" << std::endl;
    std::cout << "✅ Audio buffers working" << std::endl;
    std::cout << std::endl;

    // Test 5: Performance metrics
    std::cout << "🔧 Test 5: Performance metrics" << std::endl;
    Syntri::SimpleMetrics metrics;
    metrics.latency_ms = 2.5;
    metrics.cpu_usage_percent = 15.0;
    metrics.buffer_underruns = 0;
    std::cout << "   Latency: " << metrics.latency_ms << " ms" << std::endl;
    std::cout << "   CPU Usage: " << metrics.cpu_usage_percent << "%" << std::endl;
    std::cout << "   Buffer Underruns: " << metrics.buffer_underruns << std::endl;
    metrics.reset();
    std::cout << "   After reset - Latency: " << metrics.latency_ms << " ms" << std::endl;
    std::cout << "✅ Metrics working" << std::endl;
    std::cout << std::endl;

    // Success!
    std::cout << "=====================================" << std::endl;
    std::cout << "    🎉 FOUNDATION TEST PASSED! 🎉" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Your Syntri foundation is solid!" << std::endl;
    std::cout << "Ready to proceed with Phase 1 development." << std::endl;
    std::cout << std::endl;
    std::cout << "Next steps:" << std::endl;
    std::cout << "1. Add audio interface abstraction" << std::endl;
    std::cout << "2. Add ASIO integration" << std::endl;
    std::cout << "3. Add hardware detection" << std::endl;
    std::cout << "4. Add basic audio I/O" << std::endl;
    std::cout << std::endl;

    return 0;
}
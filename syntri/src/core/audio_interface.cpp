// Fixed src/core/audio_interface.cpp
#include "syntri/core_engine.h"
#include "syntri/audio_interface.h"
#include <iostream>
#include <algorithm>  // Add this missing include
#include <string>
#include <memory>

#ifdef ENABLE_ASIO_SUPPORT
#include "syntri/asio_interface.h"
#endif

namespace Syntri {

    std::vector<HardwareType> detectAvailableHardware() {
        std::vector<HardwareType> available_hardware;

        std::cout << "🔧 Scanning for professional audio hardware..." << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // Try to detect real ASIO hardware
        try {
            auto asio_interface = std::make_unique<ASIOInterface>();
            if (asio_interface && asio_interface->initialize()) {
                auto detected_type = asio_interface->getType();
                available_hardware.push_back(detected_type);

                std::cout << "   ✅ Found: " << hardwareTypeToString(detected_type) << std::endl;
                asio_interface->shutdown();
            }
        }
        catch (const std::exception& e) {
            std::cout << "   ⚠️  ASIO detection failed: " << e.what() << std::endl;
        }
#endif

        // Always add stub interface as fallback
        if (available_hardware.empty()) {
            available_hardware.push_back(HardwareType::GENERIC_ASIO);
            std::cout << "   ⚠️  No hardware detected, using simulation mode" << std::endl;
        }

        return available_hardware;
    }

    std::unique_ptr<AudioInterface> createAudioInterface(HardwareType type) {
        std::cout << "🔧 Creating audio interface for: " << hardwareTypeToString(type) << std::endl;

#ifdef ENABLE_ASIO_SUPPORT
        // Try real ASIO interface first
        if (type != HardwareType::GENERIC_ASIO) {
            try {
                auto asio_interface = std::make_unique<ASIOInterface>();
                if (asio_interface && asio_interface->initialize()) {
                    std::cout << "   ✅ Real ASIO interface created successfully" << std::endl;
                    // Fix: Use move() to convert unique_ptr<ASIOInterface> to unique_ptr<AudioInterface>
                    return std::unique_ptr<AudioInterface>(asio_interface.release());
                }
            }
            catch (const std::exception& e) {
                std::cout << "   ⚠️  ASIO interface creation failed: " << e.what() << std::endl;
            }
        }
#endif

        // Fallback to stub interface
        std::cout << "   ⚠️  Using stub interface (simulation mode)" << std::endl;
        return createStubInterface();
    }

    std::unique_ptr<AudioInterface> createStubInterface() {
        return std::make_unique<StubAudioInterface>();
    }

    std::string hardwareTypeToString(HardwareType type) {
        switch (type) {
        case HardwareType::UAD_APOLLO_X16: return "UAD Apollo X16";
        case HardwareType::UAD_APOLLO_X8: return "UAD Apollo X8";
        case HardwareType::ALLEN_HEATH_AVANTIS: return "Allen & Heath Avantis";
        case HardwareType::DIGICO_SD9: return "DiGiCo SD9";
        case HardwareType::YAMAHA_CL5: return "Yamaha CL5";
        case HardwareType::BEHRINGER_X32: return "Behringer X32";
        case HardwareType::FOCUSRITE_SCARLETT: return "Focusrite Scarlett";
        case HardwareType::RME_BABYFACE: return "RME Babyface Pro";
        case HardwareType::GENERIC_ASIO: return "Generic ASIO/Stub";
        default: return "Unknown Hardware";
        }
    }

    // Stub interface implementation for fallback
    class StubAudioInterface : public AudioInterface {
    private:
        bool initialized_ = false;
        bool streaming_ = false;
        HardwareType detected_type_ = HardwareType::GENERIC_ASIO;
        PerformanceMetrics metrics_;

    public:
        ~StubAudioInterface() override = default;

        bool initialize(int sample_rate, int buffer_size) override {
            std::cout << "   ✅ Stub interface initialized (simulation mode)" << std::endl;
            std::cout << "      📊 Sample Rate: " << sample_rate << " Hz" << std::endl;
            std::cout << "      📊 Buffer Size: " << buffer_size << " samples" << std::endl;
            initialized_ = true;
            return true;
        }

        void shutdown() override {
            if (streaming_) stopStreaming();
            initialized_ = false;
            std::cout << "   ✅ Stub interface shutdown" << std::endl;
        }

        bool isInitialized() const override { return initialized_; }

        HardwareType getType() const override { return detected_type_; }
        std::string getName() const override { return "Stub Audio Interface"; }
        std::string getDriverName() const override { return "Syntri Stub Driver v1.0"; }
        int getInputChannelCount() const override { return 8; }
        int getOutputChannelCount() const override { return 8; }
        std::vector<int> getSupportedSampleRates() const override { return { 44100, 48000, 96000 }; }
        std::vector<int> getSupportedBufferSizes() const override { return { 32, 64, 128, 256 }; }

        bool startStreaming(AudioProcessor* processor) override {
            if (!initialized_ || !processor) return false;

            streaming_ = true;
            std::cout << "   ✅ Stub streaming started (no real audio)" << std::endl;
            return true;
        }

        void stopStreaming() override {
            streaming_ = false;
            std::cout << "   ✅ Stub streaming stopped" << std::endl;
        }

        bool isStreaming() const override { return streaming_; }

        double getCurrentLatency() const override {
            return 2.5; // Simulated latency for testing
        }

        PerformanceMetrics getMetrics() const override { return metrics_; }

        void enableLowLatencyMode() override {
            std::cout << "   ✅ Stub low latency mode enabled" << std::endl;
        }

        bool supportsHardwareDSP() const override { return false; }
        void configureDSP() override { /* No-op for stub */ }

        bool setInputChannel(int channel, bool enabled) override { return true; }
        bool setOutputChannel(int channel, bool enabled) override { return true; }
        bool setChannelGain(int channel, float gain) override { return true; }
    };

} // namespace Syntri
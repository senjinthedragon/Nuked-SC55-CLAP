#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <vector>

#include "clap/clap.h"
#include "ext/lcd_framebuffer.h"
#include "nuked-sc55/backend/emu.h"
#include "speex/speex_resampler.h"

// Headless LCD backend: doesn't own a window, just needs to be non-null so
// LCD_Render() doesn't early-return. The actual pixel buffer is read
// directly out of lcd_t by NukedSc55::GetFramebuffer.
class NoOpLcdBackend : public LCD_Backend {
public:
    bool Start(const lcd_t&) override { return true; }
    void Stop() override {}
    void Render() override {}
};

class NukedSc55 {
public:
    enum class Model { Sc55_v1_00, Sc55_v1_10, Sc55_v1_20, Sc55_v1_21, Sc55_v2_00, Sc55mk2_v1_01 };

    // Init/shutdown
    NukedSc55(const clap_plugin_t plugin_class, const clap_host_t* host,
              const Model model);

    const clap_plugin_t* GetPluginClass();

    bool Init(const clap_plugin* plugin_instance);
    void Shutdown();

    bool Activate(const double sample_rate, const uint32_t min_frame_count,
                  const uint32_t max_frame_count);

    // Processing
    clap_process_status Process(const clap_process_t* process);

    void Flush(const clap_input_events_t* in, const clap_output_events_t* out);

    void PublishFrame(const float left, const float right);

    // State handling
    bool LoadState(const clap_istream_t* stream);
    bool SaveState(const clap_ostream_t* stream);

    // LCD framebuffer extension (see ext/lcd_framebuffer.h). Must only be
    // called from a single consistent thread, never the audio thread.
    static bool GetFramebuffer(const clap_plugin_t* plugin, uint32_t* out_width,
                                uint32_t* out_height, uint32_t* out_row_stride_pixels,
                                const uint32_t** out_pixels);

private:
    std::filesystem::path path = {};

    Model model = {};

    clap_plugin_t plugin_class         = {};
    const clap_host_t* host            = nullptr;
    const clap_plugin* plugin_instance = nullptr;

    std::unique_ptr<Emulator> emu = nullptr;
    std::unique_ptr<LCD_Backend> lcd_backend = nullptr;

    double render_sample_rate_hz = 0.0;
    double output_sample_rate_hz = 0.0;

    std::array<std::vector<float>, 2> render_buf = {};

    SpeexResamplerState* resampler = nullptr;
    bool do_resample               = false;
    double resample_ratio          = 0.0f;

    // Methods
    std::vector<std::filesystem::path> GetRomEnvDirs();
    std::vector<std::filesystem::path> GetRomBasePaths();

    void ProcessEvent(const clap_event_header_t* event);

    void RenderAudio(const uint32_t num_frames);

    void ResampleAndPublishFrames(const uint32_t num_out_frames,
                                  float* out_left, float* out_right);
};

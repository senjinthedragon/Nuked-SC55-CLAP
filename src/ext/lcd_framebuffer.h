#pragma once

#include <cstdint>

#include "clap/clap.h"

// Custom CLAP extension exposing the emulated Roland SC-55 LCD status panel
// as a raw pixel buffer, for hosts that want to render it themselves (e.g.
// as an overlay composited into their own video output) instead of relying
// on a plugin-owned window, which this headless CLAP plugin does not have.
#define NUKED_SC55_EXT_LCD_FRAMEBUFFER "net.johnnovak.nuked_sc55_clap.lcd_framebuffer/1"

// Pixel format: 0xAABBGGRR (R in the low byte -- matches SDL_PIXELFORMAT_BGR888, which the reference standalone app uses to display this same buffer), one uint32_t per pixel.
//
// Row stride is always returned via out_row_stride_pixels and must be used
// instead of out_width when computing row offsets into out_pixels -- the
// underlying buffer is allocated at a fixed maximum size and only the top-
// left out_width x out_height sub-rect is meaningful.
//
// get_framebuffer must be called from a single, consistent thread only
// (typically the host's video/render thread), never concurrently, and never
// from the audio thread -- it is not synchronized against the plugin's
// audio processing.
typedef struct nuked_sc55_lcd_framebuffer {
    bool(CLAP_ABI* get_framebuffer)(const clap_plugin_t* plugin,
                                     uint32_t* out_width,
                                     uint32_t* out_height,
                                     uint32_t* out_row_stride_pixels,
                                     const uint32_t** out_pixels);
} nuked_sc55_lcd_framebuffer_t;

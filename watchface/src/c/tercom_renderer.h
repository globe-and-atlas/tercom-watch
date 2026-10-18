#pragma once

#include <pebble.h>
#include "config.h"

typedef struct {
  int hours;
  int minutes;
  bool is_24h;
  int battery_percent;
  int heading_deg;          // -1 if invalid/unavailable
  int32_t step_count;
  int32_t altitude_ft;
  int16_t pitch_offset_px;  // -20 to +20
  int32_t roll_angle;       // TRIG_MAX_ANGLE scaled
  int16_t scroll_phase;     // 0..255 for animation
} TercomTelemetry;

// Initialize renderer resources (fonts, etc.)
void tercom_renderer_init(void);

// Deinitialize renderer resources
void tercom_renderer_deinit(void);

// Main drawing routine called by the display layer
void tercom_renderer_draw(GContext *ctx, GRect bounds, const TercomTelemetry *telemetry);

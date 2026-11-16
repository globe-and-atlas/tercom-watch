#pragma once

#include <pebble.h>
#include "config.h"

#include "terrain_model.h"

typedef struct {
  int palette;
  int flight_mode;
  int sensitivity;
  int switch_tracks;
  int top_field;
  int date_format;
  int show_steps;
  int show_course;
  int compass_mode;         // 0 off, 1 on
  int open_flight;          // 0 off, 1 fly when the face opens
  int year, month, day;
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
  TerrainType terrain_type; // 0..4
} TercomTelemetry;

// Return display string for track (e.g. "TRK-01 CRAGS")
const char* tercom_terrain_track_name(TerrainType type);

// Initialize renderer resources (fonts, etc.)
void tercom_renderer_init(void);

// Deinitialize renderer resources
void tercom_renderer_deinit(void);

// Main drawing routine called by the display layer
void tercom_renderer_draw(GContext *ctx, GRect bounds, const TercomTelemetry *telemetry);

#pragma once

#include <pebble.h>
#include "config.h"

typedef enum {
  TERRAIN_CRAGS = 0,   // The Original v1.0.1 dual peaks & pass
  TERRAIN_CANYON = 1,  // Sheer gorge / trench corridor
  TERRAIN_DUNES = 2,   // Sinuous transverse sand dunes & ripples
  TERRAIN_CALDERA = 3, // Volcanic crater caldera & central cone
  TERRAIN_ALPINE = 4,  // Sawtooth multi-summit alpine massif
  TERRAIN_COUNT = 5,
} TerrainType;

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

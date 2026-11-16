#pragma once
#include <stdint.h>
typedef enum {
  TERRAIN_CRAGS = 0,   // The Original v1.0.1 dual peaks & pass
  TERRAIN_CANYON = 1,  // Sheer gorge / trench corridor
  TERRAIN_DUNES = 2,   // Sinuous transverse sand dunes & ripples
  TERRAIN_CALDERA = 3, // Volcanic crater caldera & central cone
  TERRAIN_ALPINE = 4,  // Sawtooth multi-summit alpine massif
  TERRAIN_COUNT = 5,
} TerrainType;

int16_t tercom_elevation(TerrainType type, int c, int r, int phase_step, int m_weight);
int tercom_course_column(int row, int phase);
int tercom_flight_gain(int elapsed, int total);
int tercom_flight_frames(int mode, int fps);

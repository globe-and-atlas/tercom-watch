#include "terrain_model.h"

int tercom_course_column(int row, int phase) {
  // Period matches the 8-step flight phase so the course never jumps back at the wrap.
  static const int8_t bend[8] = {0, 1, 2, 1, 0, -1, -2, -1};
  return 10 + bend[((row + phase) % 8 + 8) % 8];
}

int tercom_flight_gain(int elapsed, int total) {
  if (total <= 0 || elapsed <= 0 || elapsed >= total) return 0;
  int edge = total / 5;
  if (edge < 1) edge = 1;
  int distance = elapsed < total - elapsed ? elapsed : total - elapsed;
  if (distance >= edge) return 1024;
  int x = distance * 1024 / edge;
  return x * x / 1024 * (3072 - 2 * x) / 1024;
}

int16_t tercom_elevation(TerrainType type, int c, int r, int phase_step, int m_weight) {
  switch (type) {
    case TERRAIN_CANYON: {
      int distance = c - tercom_course_column(r, phase_step);
      if (distance < 0) distance = -distance;
      int height = distance <= 2 ? 1 : distance == 3 ? 3 :
                   distance == 4 ? 12 : distance == 5 ? 30 : 34 + (r % 3);
      return height * m_weight / 30;
    }

    case TERRAIN_DUNES: {
      // Sinuous transverse sand dunes & ripples shifting with forward motion
      int dune_phase = (c * 3 + r * 5 + phase_step * 2) % 16;
      static const int8_t s_dune_wave[16] = {
        0, 3, 7, 12, 18, 24, 28, 24, 18, 12, 6, 2, 0, 0, 1, 1
      };
      int16_t h_dune = s_dune_wave[dune_phase];
      int cross_phase = ((c * 2 - r * 3) % 8 + 8) % 8;
      static const int8_t s_cross[8] = { 0, 2, 5, 7, 6, 3, 1, 0 };
      h_dune += s_cross[cross_phase];
      return (h_dune * (m_weight + 12)) / 36;
    }

    case TERRAIN_CALDERA: {
      int dc = c - 10;
      int dr = (r - 4) * 2;
      int radius = dc * dc + dr * dr;
      int ridge = radius - 36;
      if (ridge < 0) ridge = -ridge;
      int height = ridge < 26 ? 32 - ridge : 2;
      if (radius < 9) height = 10 - radius;
      return height * (m_weight + 8) / 32;
    }

    case TERRAIN_ALPINE: {
      // Sawtooth multi-summit alpine massif with jagged cols
      int16_t h_alpine = 0;
      static const int s_peaks[4] = { 3, 8, 13, 18 };
      static const int16_t s_pheights[4] = { 32, 42, 38, 30 };
      for (int p = 0; p < 4; p++) {
        int offset = ((r + p * 2) % 5) - 2;
        int dp = c - (s_peaks[p] + offset);
        if (dp < 0) dp = -dp;
        if (dp <= 3) {
          int16_t ph = s_pheights[p] - dp * 10;
          if (ph > h_alpine) h_alpine = ph;
        }
      }
      return (h_alpine * m_weight) / 33;
    }

    case TERRAIN_CRAGS:
    default: {
      // The Original: dual craggy peaks (col 5 & 16) + foothills
      int d_left = c - 5;
      if (d_left < 0) d_left = -d_left;
      int16_t h_left = 0;
      if (d_left <= 5) {
        h_left = ((5 - d_left) * (5 - d_left) * 36) / 25;
      }
      int d_flank = c - 2;
      if (d_flank < 0) d_flank = -d_flank;
      if (d_flank <= 3) {
        h_left += (3 - d_flank) * 3;
      }

      int d_right = c - 16;
      if (d_right < 0) d_right = -d_right;
      int16_t h_right = 0;
      if (d_right <= 5) {
        h_right = ((5 - d_right) * (5 - d_right) * 40) / 25;
      }
      int d_sh = c - 19;
      if (d_sh < 0) d_sh = -d_sh;
      if (d_sh <= 3) {
        h_right += (3 - d_sh) * 4;
      }

      static const int8_t s_roll[8] = { 2, 4, 6, 5, 2, 0, 1, 3 };
      int wave_idx = (c + phase_step + r) % 8;
      int16_t h_mid = s_roll[wave_idx];

      return ((h_left + h_right) * m_weight) / 36 + h_mid;
    }
  }
}


int tercom_flight_frames(int mode, int fps) {
  return mode == 0 ? 0 : mode == 1 ? 2 * fps : 4500 * fps / 1000;
}

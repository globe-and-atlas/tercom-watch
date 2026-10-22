#include "tercom_renderer.h"
#include <stdio.h>

static GFont s_font_time;
static GFont s_font_telemetry;
static GFont s_font_small;

const char* tercom_terrain_track_name(TerrainType type) {
  switch (type) {
    case TERRAIN_CRAGS:   return "TRK-01:CRAGS";
    case TERRAIN_CANYON:  return "TRK-02:CANYON";
    case TERRAIN_DUNES:   return "TRK-03:DUNES";
    case TERRAIN_CALDERA: return "TRK-04:CALDERA";
    case TERRAIN_ALPINE:  return "TRK-05:ALPINE";
    default:              return "TRK-01:CRAGS";
  }
}

void tercom_renderer_init(void) {
  s_font_time = fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS);
  s_font_telemetry = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_font_small = fonts_get_system_font(FONT_KEY_GOTHIC_14);
}

void tercom_renderer_deinit(void) {
  // System fonts managed by OS
}

static GPoint rotate_around_center(GPoint pt, GPoint center, int32_t angle) {
  if (angle == 0) {
    return pt;
  }
  int32_t dx = pt.x - center.x;
  int32_t dy = pt.y - center.y;
  int32_t s = sin_lookup(angle);
  int32_t c = cos_lookup(angle);
  int32_t rx = (dx * c - dy * s) / TRIG_MAX_RATIO;
  int32_t ry = (dx * s + dy * c) / TRIG_MAX_RATIO;
  return GPoint(center.x + rx, center.y + ry);
}

/*
 * 6-line Altitude / Pitch Ladder with Time Reference:
 * - 6 horizontal lines spaced across the upper HUD viewport.
 * - Left labels: Hours reference (e.g. 12, 11, 10, 09, 08, 07).
 * - Right labels: Minutes in 5-minute increments (25..00 or 55..30).
 */
static void draw_altitude_time_gauge(GContext *ctx, GPoint center, int16_t pitch_offset, int32_t roll_angle, const TercomTelemetry *telemetry) {
  graphics_context_set_stroke_color(ctx, COLOR_HORIZON);
  graphics_context_set_text_color(ctx, COLOR_TELEMETRY);

  // Top indicator chevron ▼ above the ladder
  GPoint top_pip = rotate_around_center(GPoint(center.x, 19 + pitch_offset), center, roll_angle);
  GPoint top_pip_l = rotate_around_center(GPoint(center.x - 4, 15 + pitch_offset), center, roll_angle);
  GPoint top_pip_r = rotate_around_center(GPoint(center.x + 4, 15 + pitch_offset), center, roll_angle);
  graphics_draw_line(ctx, top_pip_l, top_pip);
  graphics_draw_line(ctx, top_pip, top_pip_r);

  // Current hour calculation
  int cur_hour = telemetry->hours;
  if (!telemetry->is_24h) {
    cur_hour = cur_hour % 12;
    if (cur_hour == 0) cur_hour = 12;
  }

  // Right side 5-minute increments:
  // First half of hour (0-29 min): 25, 20, 15, 10, 05, 00
  // Second half of hour (30-59 min): 55, 50, 45, 40, 35, 30
  int min_base = (telemetry->minutes >= 30) ? 30 : 0;
  static const int s_min_offsets[6] = { 25, 20, 15, 10, 5, 0 };

  // 6 horizontal ladder lines (spaced by 14 pixels from y=26 to y=96)
  static const int16_t s_line_y[6] = { 26, 40, 54, 68, 82, 96 };

  for (int i = 0; i < 6; i++) {
    int16_t y = s_line_y[i] + pitch_offset;
    if (y < 20 || y > 114) {
      continue;
    }

    // Left hour label: Line 2 corresponds to current hour
    int line_hour = cur_hour + (2 - i);
    if (!telemetry->is_24h) {
      while (line_hour <= 0) line_hour += 12;
      while (line_hour > 12) line_hour -= 12;
    } else {
      while (line_hour < 0) line_hour += 24;
      while (line_hour >= 24) line_hour -= 24;
    }

    char hour_str[8];
    snprintf(hour_str, sizeof(hour_str), "%02d", line_hour);

    // Right minute label (5-minute increment)
    int line_min = min_base + s_min_offsets[i];
    char min_str[8];
    snprintf(min_str, sizeof(min_str), "%02d", line_min);

    // Left rung: outer tick to inner segment
    GPoint l_outer = rotate_around_center(GPoint(center.x - 46, y), center, roll_angle);
    GPoint l_inner = rotate_around_center(GPoint(center.x - 22, y), center, roll_angle);
    GPoint l_tick  = rotate_around_center(GPoint(center.x - 46, y - 3), center, roll_angle);
    graphics_draw_line(ctx, l_outer, l_inner);
    graphics_draw_line(ctx, l_outer, l_tick);

    // Right rung: inner segment to outer tick
    GPoint r_inner = rotate_around_center(GPoint(center.x + 22, y), center, roll_angle);
    GPoint r_outer = rotate_around_center(GPoint(center.x + 46, y), center, roll_angle);
    GPoint r_tick  = rotate_around_center(GPoint(center.x + 46, y - 3), center, roll_angle);
    graphics_draw_line(ctx, r_inner, r_outer);
    graphics_draw_line(ctx, r_outer, r_tick);

    // Draw Left Hour Label
    GPoint l_txt_pos = rotate_around_center(GPoint(center.x - 70, y - 7), center, roll_angle);
    graphics_draw_text(ctx, hour_str, s_font_small,
                       GRect(l_txt_pos.x, l_txt_pos.y, 22, 14),
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);

    // Draw Right Minute Label (5-min increments)
    GPoint r_txt_pos = rotate_around_center(GPoint(center.x + 49, y - 7), center, roll_angle);
    graphics_draw_text(ctx, min_str, s_font_small,
                       GRect(r_txt_pos.x, r_txt_pos.y, 22, 14),
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  }

  // Main Horizon Baseline at y = 138
  int16_t horiz_y = 138 + pitch_offset;
  GPoint h1 = rotate_around_center(GPoint(8, horiz_y), center, roll_angle);
  GPoint h2 = rotate_around_center(GPoint(SCREEN_WIDTH - 8, horiz_y), center, roll_angle);
  graphics_draw_line(ctx, h1, h2);

  // Center chevron indicator on horizon: ^
  GPoint pip_c = rotate_around_center(GPoint(center.x, horiz_y - 4), center, roll_angle);
  GPoint pip_l = rotate_around_center(GPoint(center.x - 4, horiz_y), center, roll_angle);
  GPoint pip_r = rotate_around_center(GPoint(center.x + 4, horiz_y), center, roll_angle);
  graphics_draw_line(ctx, pip_l, pip_c);
  graphics_draw_line(ctx, pip_c, pip_r);

  // Pitch tick marks along horizon
  for (int tx = center.x - 60; tx <= center.x + 60; tx += 20) {
    if (tx == center.x) continue;
    GPoint p_top = rotate_around_center(GPoint(tx, horiz_y - 3), center, roll_angle);
    GPoint p_bot = rotate_around_center(GPoint(tx, horiz_y + 3), center, roll_angle);
    graphics_draw_line(ctx, p_top, p_bot);
  }
}

/*
 * Flight Path Reticle & Digital Time Readout
 */
static void draw_reticle_and_time(GContext *ctx, GPoint center, const TercomTelemetry *telemetry) {
  const int radius = 33;

  // Mask interior of reticle with solid black to isolate CRT digits
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_circle(ctx, center, radius - 1);

  // Reticle circle in yellow/amber
  graphics_context_set_stroke_color(ctx, COLOR_RETICLE);
  graphics_draw_circle(ctx, center, radius);

  // Outer crosshair brackets
  graphics_draw_line(ctx, GPoint(center.x, center.y - radius), GPoint(center.x, center.y - radius - 7));
  graphics_draw_line(ctx, GPoint(center.x, center.y + radius), GPoint(center.x, center.y + radius + 7));
  graphics_draw_line(ctx, GPoint(center.x - radius, center.y), GPoint(center.x - radius - 7, center.y));
  graphics_draw_line(ctx, GPoint(center.x + radius, center.y), GPoint(center.x + radius + 7, center.y));

  // Inner angle tick marks
  graphics_draw_line(ctx, GPoint(center.x - 14, center.y + 19), GPoint(center.x - 8, center.y + 25));
  graphics_draw_line(ctx, GPoint(center.x + 14, center.y + 19), GPoint(center.x + 8, center.y + 25));

  // Time text inside reticle
  char time_str[8];
  int display_hour = telemetry->hours;
  if (!telemetry->is_24h) {
    display_hour = display_hour % 12;
    if (display_hour == 0) {
      display_hour = 12;
    }
  }
  snprintf(time_str, sizeof(time_str), "%d:%02d", display_hour, telemetry->minutes);

  graphics_context_set_text_color(ctx, COLOR_TIME_TEXT);
  GRect text_box = GRect(center.x - 48, center.y - 17, 96, 32);
  graphics_draw_text(ctx, time_str, s_font_time, text_box,
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

/*
 * Compute elevation for each of the 5 selectable terrain bundle models:
 * 0: CRAGS   - Original v1.0.1 dual mountain peaks & pass
 * 1: CANYON  - Sheer gorge corridor / trench
 * 2: DUNES   - Sinuous transverse sand dunes & ripples
 * 3: CALDERA - Volcanic crater rim & central cone
 * 4: ALPINE  - Sawtooth multi-summit alpine massif
 */
static int16_t compute_elevation(TerrainType type, int c, int r, int phase_step, int m_weight) {
  switch (type) {
    case TERRAIN_CANYON: {
      // Sheer gorge: high plateaus on flanks, steep drop to flat trench floor
      int16_t h_wall = 0;
      if (c <= 4) {
        h_wall = 34 - (c * 2);
      } else if (c == 5) {
        h_wall = 16;
      } else if (c == 15) {
        h_wall = 16;
      } else if (c >= 16) {
        h_wall = 26 + ((c - 16) * 2);
      } else {
        int d_center = c - 10;
        if (d_center < 0) d_center = -d_center;
        h_wall = (d_center <= 2) ? 1 : 4;
      }
      return (h_wall * m_weight) / 30;
    }

    case TERRAIN_DUNES: {
      // Sinuous transverse sand dunes & ripples shifting with forward motion
      int dune_phase = (c * 3 + r * 5 + phase_step * 2) % 16;
      static const int8_t s_dune_wave[16] = {
        0, 3, 7, 12, 18, 24, 28, 24, 18, 12, 6, 2, 0, 0, 1, 1
      };
      int16_t h_dune = s_dune_wave[dune_phase];
      int cross_phase = (c * 2 - r * 3 + 16) % 8;
      static const int8_t s_cross[8] = { 0, 2, 5, 7, 6, 3, 1, 0 };
      h_dune += s_cross[cross_phase];
      return (h_dune * (m_weight + 12)) / 36;
    }

    case TERRAIN_CALDERA: {
      // Sunken volcanic crater basin & central cone
      int dc = c - 10;
      int dr = (r - 3) * 2;
      int dist_sq = dc * dc + dr * dr;
      int16_t h_crater = 0;
      if (dist_sq <= 3) {
        h_crater = 20 - dist_sq * 4;
      } else if (dist_sq >= 16 && dist_sq <= 54) {
        int d_rim = dist_sq - 32;
        if (d_rim < 0) d_rim = -d_rim;
        h_crater = (18 - d_rim) * 2;
        if (h_crater < 0) h_crater = 0;
      } else {
        h_crater = 3;
      }
      return (h_crater * m_weight) / 28;
    }

    case TERRAIN_ALPINE: {
      // Sawtooth multi-summit alpine massif with jagged cols
      int16_t h_alpine = 0;
      static const int s_peaks[4] = { 3, 8, 13, 18 };
      static const int16_t s_pheights[4] = { 32, 42, 38, 30 };
      for (int p = 0; p < 4; p++) {
        int dp = c - s_peaks[p];
        if (dp < 0) dp = -dp;
        if (dp <= 3) {
          int16_t ph = s_pheights[p] - dp * 10;
          if (ph > h_alpine) h_alpine = ph;
        }
      }
      return (h_alpine * m_weight) / 36;
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

/*
 * 3D Topographic Wireframe Terrain Mesh Renderer:
 * Computes edge-to-edge perspective grid using the active TerrainType profile.
 */
static void draw_terrain_mesh(GContext *ctx, const TercomTelemetry *telemetry) {
  GPoint pts[NUM_DEPTH_ROWS][NUM_GRID_COLS];
  int center_x = SCREEN_WIDTH / 2;
  int phase_step = telemetry->scroll_phase / 32;
  int16_t lean_x = (telemetry->roll_angle * 8) / (TRIG_MAX_ANGLE / 16);
  int16_t lean_y = telemetry->pitch_offset_px / 2;

  // Mountain peak height weights across depth rows:
  static const int16_t s_mountain_weight[NUM_DEPTH_ROWS] = {
    16, 26, 36, 30, 22, 14, 8, 3, 1, 0, 0, 0
  };

  // Base Y positions for depth rows (perspective compression)
  static const int16_t s_row_base_y[NUM_DEPTH_ROWS] = {
    138, 140, 143, 148, 154, 162, 171, 181, 191, 200, 208, 215
  };

  // Compute 3D grid vertices
  for (int r = 0; r < NUM_DEPTH_ROWS; r++) {
    int16_t base_y = s_row_base_y[r];
    int32_t depth_angle = (r * TRIG_MAX_ANGLE) / NUM_DEPTH_ROWS;
    int32_t flight_angle = ((int32_t)telemetry->scroll_phase * TRIG_MAX_ANGLE / 256 + depth_angle) % TRIG_MAX_ANGLE;
    int32_t wave = sin_lookup(flight_angle) - sin_lookup(depth_angle);
    base_y += (wave * (r + 1) * 4) / (TRIG_MAX_RATIO * NUM_DEPTH_ROWS);
    // Edge-to-edge lateral spread: spans the full width of display
    int16_t half_w = 90 + ((r * 22) / (NUM_DEPTH_ROWS - 1));
    int16_t m_weight = s_mountain_weight[r];

    for (int c = 0; c < NUM_GRID_COLS; c++) {
      int16_t x = center_x - half_w + ((2 * half_w * c) / (NUM_GRID_COLS - 1));
      int16_t elev = compute_elevation(telemetry->terrain_type, c, r, phase_step, m_weight);
      int16_t y = base_y - elev;
      x += (lean_x * (r + 1)) / NUM_DEPTH_ROWS;
      y += (lean_y * (r + 1)) / NUM_DEPTH_ROWS;

      pts[r][c] = GPoint(x, y);
    }
  }

  // Crags and Alpine use a finer secondary mesh to preserve layered ridges
  // at Emery resolution. Primary contours are drawn afterwards for contrast.
  if (telemetry->terrain_type == TERRAIN_CRAGS || telemetry->terrain_type == TERRAIN_ALPINE) {
    graphics_context_set_stroke_color(ctx, COLOR_TERRAIN_SECONDARY);
    for (int r = 0; r < NUM_DEPTH_ROWS - 1; r++) {
      GPoint previous = GPoint((pts[r][0].x + pts[r + 1][0].x) / 2,
                              (pts[r][0].y + pts[r + 1][0].y) / 2);
      for (int c = 1; c < NUM_GRID_COLS; c++) {
        GPoint current = GPoint((pts[r][c].x + pts[r + 1][c].x) / 2,
                                (pts[r][c].y + pts[r + 1][c].y) / 2 + ((c * 3 + r + phase_step) % 3) - 1);
        graphics_draw_line(ctx, previous, current);
        previous = current;
      }
    }
    for (int c = 0; c < NUM_GRID_COLS - 1; c++) {
      GPoint previous = GPoint((pts[0][c].x + pts[0][c + 1].x) / 2,
                              (pts[0][c].y + pts[0][c + 1].y) / 2);
      for (int r = 1; r < NUM_DEPTH_ROWS; r++) {
        GPoint current = GPoint((pts[r][c].x + pts[r][c + 1].x) / 2,
                                (pts[r][c].y + pts[r][c + 1].y) / 2 + ((c + r * 2 + phase_step) % 3) - 1);
        graphics_draw_line(ctx, previous, current);
        previous = current;
      }
    }
  }

  // Draw lateral contour lines (horizontal along terrain ridges)
  for (int r = 0; r < NUM_DEPTH_ROWS; r++) {
    GColor line_color = (r % 2 == 0) ? COLOR_TERRAIN_PRIMARY : COLOR_TERRAIN_SECONDARY;
    graphics_context_set_stroke_color(ctx, line_color);

    for (int c = 0; c < NUM_GRID_COLS - 1; c++) {
      graphics_draw_line(ctx, pts[r][c], pts[r][c + 1]);
    }
  }

  // Draw longitudinal perspective grid lines (running towards vanishing point)
  for (int c = 0; c < NUM_GRID_COLS; c++) {
    GColor line_color = (c == 5 || c == 16 || c == 10 || c == 0 || c == NUM_GRID_COLS - 1)
                        ? COLOR_TERRAIN_PRIMARY
                        : COLOR_TERRAIN_SECONDARY;
    graphics_context_set_stroke_color(ctx, line_color);

    for (int r = 0; r < NUM_DEPTH_ROWS - 1; r++) {
      graphics_draw_line(ctx, pts[r][c], pts[r + 1][c]);
    }
  }
}

static void draw_telemetry_bars(GContext *ctx, GRect bounds, const TercomTelemetry *telemetry) {
  graphics_context_set_text_color(ctx, COLOR_TELEMETRY);

  // Top header: TRK-01:CRAGS (Left) | ALT: 1,280FT (Right)
  char top_l_str[24];
  char top_r_str[24];
  snprintf(top_l_str, sizeof(top_l_str), "%s", tercom_terrain_track_name(telemetry->terrain_type));
  snprintf(top_r_str, sizeof(top_r_str), "ALT: %ldFT", (long)telemetry->altitude_ft);

  graphics_draw_text(ctx, top_l_str, s_font_telemetry,
                     GRect(4, 2, 115, 18),
                     GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, top_r_str, s_font_telemetry,
                     GRect(SCREEN_WIDTH - 90, 2, 86, 18),
                     GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);

  // Bottom telemetry bar: HDG | STEPS / DIST | BATT
  char bot_hdg_str[24];
  if (telemetry->heading_deg >= 0) {
    snprintf(bot_hdg_str, sizeof(bot_hdg_str), "HDG:%03d°", telemetry->heading_deg);
  } else {
    snprintf(bot_hdg_str, sizeof(bot_hdg_str), "HDG:---");
  }

  char bot_steps_str[20];
  snprintf(bot_steps_str, sizeof(bot_steps_str), "STP:%ld", (long)telemetry->step_count);

  char bot_batt_str[16];
  snprintf(bot_batt_str, sizeof(bot_batt_str), "BAT:%d%%", telemetry->battery_percent);

  graphics_draw_text(ctx, bot_hdg_str, s_font_telemetry,
                     GRect(4, SCREEN_HEIGHT - 16, 62, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, bot_steps_str, s_font_telemetry,
                     GRect(68, SCREEN_HEIGHT - 16, 68, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, bot_batt_str, s_font_telemetry,
                     GRect(SCREEN_WIDTH - 66, SCREEN_HEIGHT - 16, 62, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
}

void tercom_renderer_draw(GContext *ctx, GRect bounds, const TercomTelemetry *telemetry) {
  // Clear display background to phosphor CRT black
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  GPoint center = GPoint(SCREEN_WIDTH / 2, 62);

  // 1. Draw 6-line altitude time ladder & artificial horizon
  draw_altitude_time_gauge(ctx, center, telemetry->pitch_offset_px, telemetry->roll_angle, telemetry);

  // 2. Draw 3D topographic wireframe terrain mesh with active terrain model
  draw_terrain_mesh(ctx, telemetry);

  // 3. Draw central flight-path reticle and digital time
  draw_reticle_and_time(ctx, center, telemetry);

  // 4. Draw telemetry readouts (headers and complications)
  draw_telemetry_bars(ctx, bounds, telemetry);
}

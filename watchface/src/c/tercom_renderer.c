#include "tercom_renderer.h"
#include <stdio.h>

static GFont s_font_time;
static GFont s_font_telemetry;
static GFont s_font_small;

// Spacing constants for terrain mesh
static const int16_t s_depth_y[NUM_DEPTH_ROWS] = {
  116, 122, 130, 140, 154, 172, 194, 218, 240
};

// Lateral spread half-width at each depth row (from horizon to screen bottom)
static const int16_t s_spread_w[NUM_DEPTH_ROWS] = {
  34, 42, 54, 68, 84, 102, 124, 146, 170
};

// Topographic mountain profile per column (c: 0..10, center at 5)
// Creates jagged peaks on flanks and a rolling valley in center
static const int16_t s_col_profile[NUM_GRID_COLS] = {
  22, 34, 38, 24, 12, 6, 10, 22, 36, 32, 18
};

// Harmonic wave modulators for continuous forward terrain motion
static const int16_t s_wave_phase[8] = {
  4, 8, 14, 18, 15, 9, 3, 1
};

void tercom_renderer_init(void) {
  s_font_time = fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS);
  s_font_telemetry = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_font_small = fonts_get_system_font(FONT_KEY_GOTHIC_14);
}

void tercom_renderer_deinit(void) {
  // System fonts do not need to be destroyed
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

static void draw_pitch_ladder(GContext *ctx, GPoint center, int16_t pitch_offset, int32_t roll_angle) {
  graphics_context_set_stroke_color(ctx, COLOR_HORIZON);

  // Pitch angles to display: +20, +10, 0, -10
  static const int16_t s_pitch_deg[] = { 20, 10, 0, -10 };
  const int count = sizeof(s_pitch_deg) / sizeof(s_pitch_deg[0]);

  for (int i = 0; i < count; i++) {
    int16_t deg = s_pitch_deg[i];
    // Scale: 1.5 pixels per degree
    int16_t y = (center.y + 40) - (deg * 15 / 10) + pitch_offset;
    if (y < 26 || y > 116) {
      continue;
    }

    if (deg == 0) {
      // Horizon baseline
      GPoint p1 = rotate_around_center(GPoint(16, y), center, roll_angle);
      GPoint p2 = rotate_around_center(GPoint(SCREEN_WIDTH - 16, y), center, roll_angle);
      graphics_draw_line(ctx, p1, p2);

      // Center chevron indicator ^
      GPoint pip = rotate_around_center(GPoint(center.x, y - 4), center, roll_angle);
      GPoint pip_l = rotate_around_center(GPoint(center.x - 4, y), center, roll_angle);
      GPoint pip_r = rotate_around_center(GPoint(center.x + 4, y), center, roll_angle);
      graphics_draw_line(ctx, pip_l, pip);
      graphics_draw_line(ctx, pip, pip_r);
    } else {
      // Pitch ladder rungs: left rung and right rung with tick
      int16_t half_w = (deg > 0) ? 22 : 18;
      int16_t tick = (deg > 0) ? 4 : -4;

      // Left rung
      GPoint l1 = rotate_around_center(GPoint(center.x - 36, y), center, roll_angle);
      GPoint l2 = rotate_around_center(GPoint(center.x - 36 + half_w, y), center, roll_angle);
      GPoint l_tick = rotate_around_center(GPoint(center.x - 36, y + tick), center, roll_angle);
      graphics_draw_line(ctx, l1, l2);
      graphics_draw_line(ctx, l1, l_tick);

      // Right rung
      GPoint r1 = rotate_around_center(GPoint(center.x + 36 - half_w, y), center, roll_angle);
      GPoint r2 = rotate_around_center(GPoint(center.x + 36, y), center, roll_angle);
      GPoint r_tick = rotate_around_center(GPoint(center.x + 36, y + tick), center, roll_angle);
      graphics_draw_line(ctx, r1, r2);
      graphics_draw_line(ctx, r2, r_tick);
    }
  }
}

static void draw_reticle_and_time(GContext *ctx, GPoint center, const TercomTelemetry *telemetry) {
  const int radius = 33;

  // Mask interior of reticle with solid black to isolate CRT digits from pitch ladder
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

static void draw_terrain_mesh(GContext *ctx, int16_t scroll_phase) {
  GPoint pts[NUM_DEPTH_ROWS][NUM_GRID_COLS];
  int center_x = SCREEN_WIDTH / 2;

  // Compute 3D grid vertices
  for (int r = 0; r < NUM_DEPTH_ROWS; r++) {
    int16_t base_y = s_depth_y[r];
    int16_t half_w = s_spread_w[r];
    int wave_idx = (r + (scroll_phase / 32)) % 8;
    int16_t wave_val = s_wave_phase[wave_idx];

    for (int c = 0; c < NUM_GRID_COLS; c++) {
      // Lateral X coordinate spread
      int16_t x = center_x - half_w + ((2 * half_w * c) / (NUM_GRID_COLS - 1));

      // Elevation computation: column profile modulated by wave and row depth
      int16_t elev = (s_col_profile[c] * wave_val * (r + 2)) / 48;
      int16_t y = base_y - elev;

      pts[r][c] = GPoint(x, y);
    }
  }

  // Draw lateral ridge contour lines (horizontal along ridges)
  for (int r = 0; r < NUM_DEPTH_ROWS; r++) {
    GColor line_color = (r % 2 == 0) ? COLOR_TERRAIN_PRIMARY : COLOR_TERRAIN_SECONDARY;
    graphics_context_set_stroke_color(ctx, line_color);

    for (int c = 0; c < NUM_GRID_COLS - 1; c++) {
      graphics_draw_line(ctx, pts[r][c], pts[r][c + 1]);
    }
  }

  // Draw longitudinal perspective grid lines (vertical running to vanishing point)
  for (int c = 0; c < NUM_GRID_COLS; c++) {
    GColor line_color = (c == 0 || c == NUM_GRID_COLS - 1 || c == 5)
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

  // Top header: TERCOM: TRK-04 (Left) | ALT: 1,280FT (Right)
  char top_l_str[24];
  char top_r_str[24];
  snprintf(top_l_str, sizeof(top_l_str), "TERCOM: TRK-04");
  snprintf(top_r_str, sizeof(top_r_str), "ALT: %ldFT", (long)telemetry->altitude_ft);

  graphics_draw_text(ctx, top_l_str, s_font_telemetry,
                     GRect(4, 3, 110, 18),
                     GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, top_r_str, s_font_telemetry,
                     GRect(SCREEN_WIDTH - 90, 3, 86, 18),
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
                     GRect(4, SCREEN_HEIGHT - 17, 62, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, bot_steps_str, s_font_telemetry,
                     GRect(68, SCREEN_HEIGHT - 17, 68, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, bot_batt_str, s_font_telemetry,
                     GRect(SCREEN_WIDTH - 66, SCREEN_HEIGHT - 17, 62, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
}

void tercom_renderer_draw(GContext *ctx, GRect bounds, const TercomTelemetry *telemetry) {
  // Clear display background to phosphor CRT black
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  GPoint center = GPoint(SCREEN_WIDTH / 2, 64);

  // 1. Draw pitch ladder & artificial horizon
  draw_pitch_ladder(ctx, center, telemetry->pitch_offset_px, telemetry->roll_angle);

  // 2. Draw 3D topographic wireframe terrain mesh
  draw_terrain_mesh(ctx, telemetry->scroll_phase);

  // 3. Draw central flight-path reticle and digital time
  draw_reticle_and_time(ctx, center, telemetry);

  // 4. Draw telemetry readouts (headers and complications)
  draw_telemetry_bars(ctx, bounds, telemetry);
}

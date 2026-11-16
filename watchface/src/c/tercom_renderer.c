#include "tercom_renderer.h"
#include <stdio.h>


typedef struct { GColor primary, secondary, ladder, reticle, time, telemetry; } Palette;
static Palette s_colors;
static void select_palette(int index) {
#ifdef PBL_BW
  (void)index;
  s_colors = (Palette){GColorWhite, GColorWhite, GColorWhite, GColorWhite, GColorWhite, GColorWhite};
#else
  switch (index) {
    case 1: s_colors = (Palette){GColorFromRGB(255,170,0), GColorFromRGB(85,85,0), GColorFromRGB(170,85,0), GColorFromRGB(255,255,85), GColorFromRGB(255,255,170), GColorFromRGB(255,170,85)}; break;
    case 2: s_colors = (Palette){GColorFromRGB(0,170,255), GColorFromRGB(0,85,170), GColorFromRGB(0,85,85), GColorFromRGB(170,255,255), GColorFromRGB(170,255,255), GColorFromRGB(85,170,255)}; break;
    case 3: s_colors = (Palette){GColorLightGray, GColorDarkGray, GColorDarkGray, GColorWhite, GColorWhite, GColorLightGray}; break;
    default: s_colors = (Palette){GColorMalachite, GColorIslamicGreen, GColorDarkGreen, GColorChromeYellow, GColorFromRGB(170,255,170), GColorMintGreen}; break;
  }
#endif
}

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
  s_font_time = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  s_font_telemetry = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_font_small = fonts_get_system_font(FONT_KEY_GOTHIC_14);
}

void tercom_renderer_deinit(void) {
  // System fonts managed by OS
}

static int hud_x(int x) {
  return x * SCREEN_WIDTH / 200;
}

static int hud_y(int y) {
#ifdef PBL_ROUND
  return 20 + y * 105 / 100;
#elif defined(PBL_BW)
  return y * 168 / 228;
#else
  return y;
#endif
}

static bool label_clear(GPoint position) {
  int16_t center = SCREEN_WIDTH / 2;
  int16_t text_right = position.x + 22;
#ifdef PBL_ROUND
  if (position.y < 55 && (position.x < 40 || text_right > 220)) return false;
#endif
  return position.x >= 0 && text_right <= SCREEN_WIDTH &&
         position.y >= hud_y(18) && position.y + 14 <= hud_y(112) &&
         (text_right <= center - hud_x(56) || position.x >= center + hud_x(56) ||
          position.y + 14 <= hud_y(62) - 17 || position.y >= hud_y(62) + 20);
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
  graphics_context_set_stroke_color(ctx, s_colors.ladder);
  graphics_context_set_text_color(ctx, s_colors.ladder);

  // Top indicator chevron ▼ above the ladder
  GPoint top_pip = rotate_around_center(GPoint(center.x, hud_y(19) + pitch_offset), center, roll_angle);
  GPoint top_pip_l = rotate_around_center(GPoint(center.x - 4, hud_y(15) + pitch_offset), center, roll_angle);
  GPoint top_pip_r = rotate_around_center(GPoint(center.x + 4, hud_y(15) + pitch_offset), center, roll_angle);
  graphics_draw_line(ctx, top_pip_l, top_pip);
  graphics_draw_line(ctx, top_pip, top_pip_r);

  // Current hour calculation
  int cur_hour = telemetry->hours;
  if (!telemetry->is_24h) {
    cur_hour = cur_hour % 12;
    if (cur_hour == 0) cur_hour = 12;
  }

  // Moving tapes: each scale scrolls so the current time rests on a fixed
  // pointer level with the clock. Later values sit higher, like an altitude tape.
  // Minutes move 2.8 px per minute; hours creep up through the hour.
  const int now_y = 62, rung = 14;
  int min_slide = (telemetry->minutes % 5) * rung / 5;
  int hour_slide = telemetry->minutes * rung / 60;
  int min_base = telemetry->minutes - telemetry->minutes % 5;

  for (int k = -3; k <= 3; k++) {
    for (int side = -1; side <= 1; side += 2) {
      int design_y = now_y - k * rung + (side < 0 ? hour_slide : min_slide);
      if (design_y < 20 || design_y > 114) continue;
      int16_t y = hud_y(design_y) + pitch_offset;

      GPoint outer = rotate_around_center(GPoint(center.x + side * hud_x(46), y), center, roll_angle);
      GPoint inner = rotate_around_center(GPoint(center.x + side * hud_x(22), y), center, roll_angle);
      GPoint tick  = rotate_around_center(GPoint(center.x + side * hud_x(46), y - 3), center, roll_angle);
      graphics_draw_line(ctx, outer, inner);
      graphics_draw_line(ctx, outer, tick);

#ifdef PBL_BW
      continue;  // Pebble 2's clock fills the width: tape lines and pointers only
#endif
      char label[8];
      if (side < 0) {
        int line_hour = cur_hour + k;
        if (!telemetry->is_24h) {
          while (line_hour <= 0) line_hour += 12;
          while (line_hour > 12) line_hour -= 12;
        } else {
          while (line_hour < 0) line_hour += 24;
          while (line_hour >= 24) line_hour -= 24;
        }
        snprintf(label, sizeof(label), "%02d", line_hour);
      } else {
        snprintf(label, sizeof(label), "%02d", ((min_base + 5 * k) % 60 + 60) % 60);
      }
      int label_x = side < 0 ? -hud_x(82) : hud_x(60);
      GPoint pos = rotate_around_center(GPoint(center.x + label_x, y - 7), center, roll_angle);
      if (label_clear(pos)) graphics_draw_text(ctx, label, s_font_small, GRect(pos.x, pos.y, 22, 14),
          GTextOverflowModeWordWrap, side < 0 ? GTextAlignmentRight : GTextAlignmentLeft, NULL);
    }
  }

  // "Now" pointers on the outer edge of each tape, in the clock's colour.
  graphics_context_set_stroke_color(ctx, s_colors.reticle);
  int16_t now_px = hud_y(now_y) + pitch_offset;
  for (int side = -1; side <= 1; side += 2) {
    GPoint tip  = rotate_around_center(GPoint(center.x + side * hud_x(89), now_px), center, roll_angle);
    GPoint back_top = rotate_around_center(GPoint(center.x + side * hud_x(95), now_px - 4), center, roll_angle);
    GPoint back_bot = rotate_around_center(GPoint(center.x + side * hud_x(95), now_px + 4), center, roll_angle);
    graphics_draw_line(ctx, back_top, tip);
    graphics_draw_line(ctx, tip, back_bot);
  }
  graphics_context_set_stroke_color(ctx, s_colors.ladder);

  // Main Horizon Baseline at y = 138
  int16_t horiz_y = hud_y(138) + pitch_offset;
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
  for (int tx = center.x - hud_x(60); tx <= center.x + hud_x(60); tx += 20) {
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
  const int radius = hud_x(36);
  const int clock_width = SCREEN_WIDTH < 200 ? 90 : 112;

  // Mask interior of reticle with solid black to isolate CRT digits
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_circle(ctx, center, radius - 1);

  // Reticle circle in yellow/amber
  graphics_context_set_stroke_color(ctx, s_colors.reticle);
  graphics_draw_circle(ctx, center, radius);

  // Outer crosshair brackets
  graphics_draw_line(ctx, GPoint(center.x, center.y - radius), GPoint(center.x, center.y - radius - 7));
  graphics_draw_line(ctx, GPoint(center.x, center.y + radius), GPoint(center.x, center.y + radius + 7));
  graphics_draw_line(ctx, GPoint(center.x - radius, center.y), GPoint(center.x - radius - 7, center.y));
  graphics_draw_line(ctx, GPoint(center.x + radius, center.y), GPoint(center.x + radius + 7, center.y));

  // Inner angle tick marks
  graphics_draw_line(ctx, GPoint(center.x - 14, center.y + 19), GPoint(center.x - 8, center.y + 25));
  graphics_draw_line(ctx, GPoint(center.x + 14, center.y + 19), GPoint(center.x + 8, center.y + 25));

  // A quiet horizontal clock window breaks the reticle sides rather than
  // letting scales and ring strokes collide with wide 24-hour times.
  graphics_context_set_fill_color(ctx, COLOR_BG);
#ifdef PBL_PLATFORM_EMERY
  graphics_fill_rect(ctx, GRect(center.x - 56, center.y - 17, 112, 37), 0, GCornerNone);
#else
  graphics_fill_rect(ctx, GRect(center.x - clock_width / 2, center.y - 16, clock_width, 34), 0, GCornerNone);
#endif

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

  graphics_context_set_text_color(ctx, s_colors.time);
  GRect text_box = GRect(center.x - clock_width / 2, center.y - 21, clock_width, 42);
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
    base_y += (wave * (r + 1) * 10) / (TRIG_MAX_RATIO * NUM_DEPTH_ROWS);
    // Edge-to-edge lateral spread: spans the full width of display
    int16_t half_w = 90 + ((r * 22) / (NUM_DEPTH_ROWS - 1));
    int16_t m_weight = s_mountain_weight[r];

    for (int c = 0; c < NUM_GRID_COLS; c++) {
      int16_t x = center_x - half_w + ((2 * half_w * c) / (NUM_GRID_COLS - 1));
      int blend = telemetry->scroll_phase % 32;
      int a = tercom_elevation(telemetry->terrain_type, c, r, phase_step, m_weight);
      int b = tercom_elevation(telemetry->terrain_type, c, r, (phase_step + 1) % 8, m_weight);
      int16_t elev = (a * (32 - blend) + b * blend) / 32;
      int16_t y = base_y - elev;
      x += (lean_x * (r + 1)) / NUM_DEPTH_ROWS;
      y += (lean_y * (r + 1)) / NUM_DEPTH_ROWS;

      pts[r][c] = GPoint(center_x + hud_x(x - center_x), hud_y(y));
    }
  }

  // Crags and Alpine use a finer secondary mesh to preserve layered ridges
  // at Emery resolution. Primary contours are drawn afterwards for contrast.
#ifndef PBL_BW
  if (telemetry->terrain_type == TERRAIN_CRAGS || telemetry->terrain_type == TERRAIN_ALPINE) {
    graphics_context_set_stroke_color(ctx, s_colors.secondary);
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

#endif

  // Draw lateral contour lines (horizontal along terrain ridges)
  for (int r = 0; r < NUM_DEPTH_ROWS; r++) {
#ifdef PBL_BW
    if (r < 8 && r % 2 != 0) continue;
#endif
    GColor line_color = (r % 2 == 0) ? s_colors.primary : s_colors.secondary;
    graphics_context_set_stroke_color(ctx, line_color);

    for (int c = 0; c < NUM_GRID_COLS - 1; c++) {
      graphics_draw_line(ctx, pts[r][c], pts[r][c + 1]);
    }
  }

  // Draw longitudinal perspective grid lines (running towards vanishing point)
  for (int c = 0; c < NUM_GRID_COLS; c++) {
#ifdef PBL_BW
    if (c % 2 != 0) continue;
#endif
    GColor line_color = (c == 5 || c == 16 || c == 10 || c == 0 || c == NUM_GRID_COLS - 1)
                        ? s_colors.primary
                        : s_colors.secondary;
    graphics_context_set_stroke_color(ctx, line_color);

    for (int r = 0; r < NUM_DEPTH_ROWS - 1; r++) {
      graphics_draw_line(ctx, pts[r][c], pts[r + 1][c]);
    }
  }
  if (telemetry->show_course) {
    graphics_context_set_stroke_color(ctx, s_colors.reticle);
    for (int r = 3; r < NUM_DEPTH_ROWS - 1; r += 2) {
      int c = telemetry->terrain_type == TERRAIN_CANYON ? tercom_course_column(r, phase_step) : 10;
      int next = telemetry->terrain_type == TERRAIN_CANYON ? tercom_course_column(r + 1, phase_step) : 10;
      graphics_draw_line(ctx, pts[r][c], pts[r + 1][next]);
    }
  }

}

static void draw_telemetry_bars(GContext *ctx, GRect bounds, const TercomTelemetry *telemetry) {
  graphics_context_set_text_color(ctx, s_colors.telemetry);

  // Top header: TRK-01:CRAGS (Left) | ALT: 1,280FT (Right)
  char top_l_str[24];
  char top_r_str[24];
  const char *track = tercom_terrain_track_name(telemetry->terrain_type);
#ifdef PBL_BW
  snprintf(top_l_str, sizeof(top_l_str), "%.2s %s", track + 4, track + 7);
#else
  snprintf(top_l_str, sizeof(top_l_str), "%s", track);
#endif
  switch (telemetry->top_field) {
    case 1: snprintf(top_r_str, sizeof(top_r_str), "STP:%ld", (long)telemetry->step_count); break;
    case 2:
      if (telemetry->heading_deg >= 0) snprintf(top_r_str, sizeof(top_r_str), "HDG:%03d", telemetry->heading_deg);
      else snprintf(top_r_str, sizeof(top_r_str), "HDG:---");
      break;
    case 3: snprintf(top_r_str, sizeof(top_r_str), "BAT:%d%%", telemetry->battery_percent); break;
    case 4: top_r_str[0] = '\0'; break;
    default: snprintf(top_r_str, sizeof(top_r_str), "SIM:%ld", (long)telemetry->altitude_ft); break;
  }

#ifdef PBL_PLATFORM_EMERY
  graphics_draw_text(ctx, top_l_str, s_font_telemetry,
                     GRect(4, 2, 108, 18),
                     GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, top_r_str, s_font_telemetry,
                     GRect(112, 2, 84, 18),
                     GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
#else
#ifdef PBL_ROUND
  const GRect top_left = GRect(50, 24, 92, 16);
  const GRect top_right = GRect(143, 24, 66, 16);
#else
  const GRect top_left = GRect(3, 0, 82, 16);
  const GRect top_right = GRect(86, 0, 55, 16);
#endif
  graphics_draw_text(ctx, top_l_str, s_font_telemetry, top_left,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, top_r_str, s_font_telemetry, top_right,
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

#endif

  // Bottom telemetry bar: HDG | STEPS / DIST | BATT
  char bot_hdg_str[24];
#ifdef PBL_COMPASS
  bool compass_shown = telemetry->compass_mode != 0;
#else
  bool compass_shown = false;  // Pebble 2 has no magnetometer; leave the slot blank.
#endif
  if (!compass_shown) bot_hdg_str[0] = '\0';
  else if (telemetry->heading_deg >= 0)
    snprintf(bot_hdg_str, sizeof(bot_hdg_str), "HDG:%03d°", telemetry->heading_deg);
  else snprintf(bot_hdg_str, sizeof(bot_hdg_str), "HDG:---");

  char bot_steps_str[20];
  if (telemetry->date_format == 1)
    snprintf(bot_steps_str, sizeof(bot_steps_str), "%04d-%02d-%02d", telemetry->year, telemetry->month, telemetry->day);
  else if (telemetry->date_format == 2)
    snprintf(bot_steps_str, sizeof(bot_steps_str), "%02d/%02d/%04d", telemetry->month, telemetry->day, telemetry->year);
  else if (telemetry->date_format == 3)
    snprintf(bot_steps_str, sizeof(bot_steps_str), "%02d/%02d/%04d", telemetry->day, telemetry->month, telemetry->year);
  else if (telemetry->show_steps)
    snprintf(bot_steps_str, sizeof(bot_steps_str), "STP:%ld", (long)telemetry->step_count);
  else bot_steps_str[0] = '\0';

  char bot_batt_str[16];
#ifdef PBL_BW
  snprintf(bot_batt_str, sizeof(bot_batt_str), "%d%%", telemetry->battery_percent);
#else
  snprintf(bot_batt_str, sizeof(bot_batt_str), "BAT:%d%%", telemetry->battery_percent);
#endif

#ifdef PBL_PLATFORM_EMERY
  graphics_draw_text(ctx, bot_hdg_str, s_font_telemetry,
                     GRect(4, SCREEN_HEIGHT - 16, 62, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, bot_steps_str, s_font_small,
                     GRect(59, SCREEN_HEIGHT - 16, 82, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, bot_batt_str, s_font_telemetry,
                     GRect(SCREEN_WIDTH - 66, SCREEN_HEIGHT - 16, 62, 16),
                     GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
#else
#ifdef PBL_ROUND
  const GRect footer = GRect(42, 218, 176, 16);
#else
  const GRect footer = GRect(0, 151, SCREEN_WIDTH, 17);
#endif
  int edge_width = (footer.size.w - 68) / 2;
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, footer, 2, GCornersAll);
  graphics_draw_text(ctx, bot_hdg_str, s_font_small,
                     GRect(footer.origin.x + 2, footer.origin.y, edge_width, 16),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, bot_steps_str, s_font_small,
                     GRect(SCREEN_WIDTH / 2 - 31, footer.origin.y, 62, 16),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  graphics_draw_text(ctx, bot_batt_str, s_font_small,
                     GRect(SCREEN_WIDTH - footer.origin.x - edge_width - 2, footer.origin.y, edge_width, 16),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
#endif
}

void tercom_renderer_draw(GContext *ctx, GRect bounds, const TercomTelemetry *telemetry) {
  select_palette(telemetry->palette);
  // Clear display background to phosphor CRT black
  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  GPoint center = GPoint(SCREEN_WIDTH / 2, hud_y(62));

  // 1. Draw 6-line altitude time ladder & artificial horizon
  draw_altitude_time_gauge(ctx, center, telemetry->pitch_offset_px, telemetry->roll_angle, telemetry);

  // 2. Draw 3D topographic wireframe terrain mesh with active terrain model
  draw_terrain_mesh(ctx, telemetry);

  // 3. Draw central flight-path reticle and digital time
  draw_reticle_and_time(ctx, center, telemetry);

  // 4. Draw telemetry readouts (headers and complications)
  draw_telemetry_bars(ctx, bounds, telemetry);
}

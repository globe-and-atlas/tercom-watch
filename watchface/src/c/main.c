#include <pebble.h>
#include "config.h"
#include "tercom_renderer.h"

static Window *s_window;
static Layer *s_canvas_layer;
static TercomTelemetry s_telemetry = {.heading_deg = -1};

static AppTimer *s_anim_timer = NULL;
static int s_anim_elapsed = 0;
static int s_anim_total = 0;
static bool s_is_active_anim = false;
static int16_t s_rest_scroll_phase = 0;
static uint32_t s_last_tap_ms = 0;
static bool s_tap_pending = false;

static void update_telemetry(void) {
  // 1. Time
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

#if TERCOM_TEST_MINUTE >= 0
  s_telemetry.hours = TERCOM_TEST_MINUTE / 60;
  s_telemetry.minutes = TERCOM_TEST_MINUTE % 60;
#else
  s_telemetry.hours = t->tm_hour;
  s_telemetry.minutes = t->tm_min;
#endif
  s_telemetry.is_24h = clock_is_24h_style();
  s_telemetry.year = t->tm_year + 1900;
  s_telemetry.month = t->tm_mon + 1;
  s_telemetry.day = t->tm_mday;

  // 2. Battery
#if TERCOM_TEST_BATTERY >= 0
  s_telemetry.battery_percent = TERCOM_TEST_BATTERY;
#else
  BatteryChargeState charge_state = battery_state_service_peek();
  s_telemetry.battery_percent = charge_state.charge_percent;
#endif

  // 3. Heading / Compass
#if TERCOM_TEST_HEADING >= 0
  s_telemetry.heading_deg = TERCOM_TEST_HEADING;
#endif

  // 4. Health / Steps
#if TERCOM_TEST_STEPS >= 0
  s_telemetry.step_count = TERCOM_TEST_STEPS;
#else
  HealthMetric metric = HealthMetricStepCount;
  time_t start = time_start_of_today();
  time_t end = time(NULL);
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    s_telemetry.step_count = (int32_t)health_service_sum_today(metric);
  } else {
    s_telemetry.step_count = 0;
  }
#endif

  // 5. Altitude (estimated or test)
#if TERCOM_TEST_ALT >= 0
  s_telemetry.altitude_ft = TERCOM_TEST_ALT;
#else
  // Default flight cruise altitude based on steps progress
  s_telemetry.altitude_ft = 1200 + (s_telemetry.step_count % 800);
#endif

  // 6. Terrain Model
#if TERCOM_TEST_TERRAIN >= 0
  s_telemetry.terrain_type = (TerrainType)TERCOM_TEST_TERRAIN;
#endif
}

static void stop_flight(void) {
  if (s_anim_timer) { app_timer_cancel(s_anim_timer); s_anim_timer = NULL; }
  if (s_is_active_anim) s_telemetry.scroll_phase = s_rest_scroll_phase;
  s_is_active_anim = false;
  s_telemetry.roll_angle = 0;
  s_telemetry.pitch_offset_px = 0;
  if (s_canvas_layer) layer_mark_dirty(s_canvas_layer);
}

static void anim_timer_callback(void *data) {
  s_anim_timer = NULL;
  if (++s_anim_elapsed >= s_anim_total) { stop_flight(); return; }
  int gain = tercom_flight_gain(s_anim_elapsed, s_anim_total);
  s_telemetry.scroll_phase = (s_telemetry.scroll_phase + 1 + gain * 3 / 1024) % 256;
  AccelData accel = {0};
  int32_t target_roll = 0;
  int target_pitch = 0;
  if (s_telemetry.sensitivity && accel_service_peek(&accel) == 0) {
    int sensitivity = s_telemetry.sensitivity; // 1 gentle, 2 normal, 3 responsive
    target_roll = (int32_t)accel.x * (TRIG_MAX_ANGLE / 16) / 1000;
    target_roll = target_roll * sensitivity / 2 * gain / 1024;
    target_pitch = accel.y * 15 / 1000 * sensitivity / 2 * gain / 1024;
  }
  s_telemetry.roll_angle = (s_telemetry.roll_angle * 3 + target_roll) / 4;
  s_telemetry.pitch_offset_px = (s_telemetry.pitch_offset_px * 3 + target_pitch) / 4;
  layer_mark_dirty(s_canvas_layer);
  s_anim_timer = app_timer_register(1000 / ACTIVE_ANIM_FPS, anim_timer_callback, NULL);
}

static void trigger_active_flight(void) {
  if (!s_telemetry.flight_mode) return;
  if (!s_is_active_anim) s_rest_scroll_phase = s_telemetry.scroll_phase;
  s_is_active_anim = true;
  s_anim_elapsed = 0;
  s_anim_total = tercom_flight_frames(s_telemetry.flight_mode, ACTIVE_ANIM_FPS);
  if (!s_anim_timer)
    s_anim_timer = app_timer_register(1000 / ACTIVE_ANIM_FPS, anim_timer_callback, NULL);
}

static void handle_accel_tap(AccelAxisType axis, int32_t direction) {
  time_t seconds; uint16_t millis;
  time_ms(&seconds, &millis);
  uint32_t now = (uint32_t)seconds * 1000u + millis;
  if (s_tap_pending && now - s_last_tap_ms <= 650) {
    if (s_telemetry.switch_tracks) {
      s_telemetry.terrain_type = (s_telemetry.terrain_type + 1) % TERRAIN_COUNT;
      persist_write_int(STORAGE_KEY_TERRAIN, s_telemetry.terrain_type);
      vibes_short_pulse();
      layer_mark_dirty(s_canvas_layer);
    }
    s_tap_pending = false;
  } else s_tap_pending = true;
  s_last_tap_ms = now;
  trigger_active_flight();
}

static int load_setting(int key, int maximum, int fallback) {
  int value = persist_exists(key) ? persist_read_int(key) : fallback;
  return value >= 0 && value <= maximum ? value : fallback;
}

static void inbox_received(DictionaryIterator *iterator, void *context) {
  stop_flight();
  int *values[] = {&s_telemetry.palette, &s_telemetry.flight_mode,
    &s_telemetry.sensitivity, &s_telemetry.switch_tracks, &s_telemetry.top_field,
    &s_telemetry.date_format, &s_telemetry.show_steps, &s_telemetry.show_course};
  const int limits[] = {3, 2, 3, 1, 4, 3, 1, 1};
  const uint32_t keys[] = {MESSAGE_KEY_PALETTE, MESSAGE_KEY_FLIGHT, MESSAGE_KEY_SENSITIVITY,
    MESSAGE_KEY_SWITCH_TRACKS, MESSAGE_KEY_TOP_FIELD, MESSAGE_KEY_DATE,
    MESSAGE_KEY_SHOW_STEPS, MESSAGE_KEY_COURSE};
  for (int i = 0; i < 8; i++) {
    Tuple *tuple = dict_find(iterator, keys[i]);
    if (tuple && tuple->value->int32 >= 0 && tuple->value->int32 <= limits[i]) {
      *values[i] = tuple->value->int32;
      persist_write_int(2 + i, *values[i]);
    }
  }
  Tuple *terrain = dict_find(iterator, MESSAGE_KEY_TERRAIN);
  if (terrain && terrain->value->int32 >= 0 && terrain->value->int32 < TERRAIN_COUNT) {
    s_telemetry.terrain_type = terrain->value->int32;
    persist_write_int(STORAGE_KEY_TERRAIN, s_telemetry.terrain_type);
  }
  s_tap_pending = false;
  update_telemetry();
  layer_mark_dirty(s_canvas_layer);
}

#ifdef PBL_COMPASS
static void handle_compass_heading(CompassHeadingData data) {
#if TERCOM_TEST_HEADING < 0
  // Per _PEBBLE instructions: emulator has no compass, status below Calibrating is unavailable
  if (data.compass_status < CompassStatusCalibrating) {
    s_telemetry.heading_deg = -1;
  } else {
    s_telemetry.heading_deg = (int)(data.true_heading * 360 / TRIG_MAX_ANGLE);
  }
#endif
  if (!s_is_active_anim) {
    layer_mark_dirty(s_canvas_layer);
  }
}

#endif

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_telemetry();
  if (!s_is_active_anim) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  tercom_renderer_draw(ctx, bounds, &s_telemetry);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  tercom_renderer_init();

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  s_telemetry.palette = load_setting(2, 3, 0);
  s_telemetry.flight_mode = load_setting(3, 2, 2);
  s_telemetry.sensitivity = load_setting(4, 3, 2);
  s_telemetry.switch_tracks = load_setting(5, 1, 1);
  s_telemetry.top_field = load_setting(6, 4, 0);
  s_telemetry.date_format = load_setting(7, 3, 0);
  s_telemetry.show_steps = load_setting(8, 1, 1);
  s_telemetry.show_course = load_setting(9, 1, 0);
  // Load saved terrain preference (defaults to original TERRAIN_CRAGS)
  if (persist_exists(STORAGE_KEY_TERRAIN)) {
    s_telemetry.terrain_type = (TerrainType)persist_read_int(STORAGE_KEY_TERRAIN);
    if (s_telemetry.terrain_type >= TERRAIN_COUNT) {
      s_telemetry.terrain_type = TERRAIN_CRAGS;
    }
  } else {
    s_telemetry.terrain_type = TERRAIN_CRAGS;
  }

  update_telemetry();

  // Trigger brief introductory flight sequence on startup
  trigger_active_flight();
}

static void window_unload(Window *window) {
  if (s_anim_timer) {
    app_timer_cancel(s_anim_timer);
    s_anim_timer = NULL;
  }
  layer_destroy(s_canvas_layer);
  tercom_renderer_deinit();
}

static void init(void) {
  s_window = window_create();
  window_set_background_color(s_window, COLOR_BG);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(s_window, true);

  app_message_register_inbox_received(inbox_received);
  app_message_open(256, 64);

  // Subscriptions
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  accel_tap_service_subscribe(handle_accel_tap);
#ifdef PBL_COMPASS
  compass_service_subscribe(handle_compass_heading);
  compass_service_set_heading_filter(TRIG_MAX_ANGLE / 72); // 5 degree updates
#endif
}

static void deinit(void) {
  app_message_deregister_callbacks();
#ifdef PBL_COMPASS
  compass_service_unsubscribe();
#endif
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

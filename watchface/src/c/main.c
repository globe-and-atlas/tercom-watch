#include <pebble.h>
#include "config.h"
#include "tercom_renderer.h"

static Window *s_window;
static Layer *s_canvas_layer;
static TercomTelemetry s_telemetry;

static AppTimer *s_anim_timer = NULL;
static int s_anim_frames_remaining = 0;
static bool s_is_active_anim = false;
static int16_t s_rest_scroll_phase = 0;
static time_t s_last_tap_time = 0;

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

static void anim_timer_callback(void *data) {
  if (s_anim_frames_remaining > 0) {
    s_anim_frames_remaining--;

    // Advance terrain forward
    s_telemetry.scroll_phase = (s_telemetry.scroll_phase + 4) % 256;

    // Peek accelerometer for live tilt during active mode
    AccelData accel = { 0 };
    if (accel_service_peek(&accel) == 0) {
      // accel.x: -1000..+1000 (mG), accel.y: -1000..+1000
      s_telemetry.roll_angle = (int32_t)accel.x * (TRIG_MAX_ANGLE / 16) / 1000;
      s_telemetry.pitch_offset_px = (int16_t)(accel.y * 15 / 1000);
    }

    layer_mark_dirty(s_canvas_layer);
    s_anim_timer = app_timer_register(1000 / ACTIVE_ANIM_FPS, anim_timer_callback, NULL);
  } else {
    s_is_active_anim = false;
    s_anim_timer = NULL;
    s_telemetry.scroll_phase = s_rest_scroll_phase;
    s_telemetry.roll_angle = 0;
    s_telemetry.pitch_offset_px = 0;
    layer_mark_dirty(s_canvas_layer);
  }
}

static void trigger_active_flight(void) {
  if (!s_is_active_anim) {
    s_rest_scroll_phase = s_telemetry.scroll_phase;
  }
  s_is_active_anim = true;
  s_anim_frames_remaining = (ACTIVE_ANIM_DURATION_MS * ACTIVE_ANIM_FPS) / 1000;
  if (!s_anim_timer) {
    s_anim_timer = app_timer_register(1000 / ACTIVE_ANIM_FPS, anim_timer_callback, NULL);
  }
}

static void handle_accel_tap(AccelAxisType axis, int32_t direction) {
  time_t now = time(NULL);

  // Quick double-tap (within 1 second) cycles to next terrain in the bundle!
  if (now - s_last_tap_time <= 1) {
    s_telemetry.terrain_type = (TerrainType)((s_telemetry.terrain_type + 1) % TERRAIN_COUNT);
    persist_write_int(STORAGE_KEY_TERRAIN, s_telemetry.terrain_type);
    vibes_short_pulse();
  }
  s_last_tap_time = now;

  trigger_active_flight();
}

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

  // Subscriptions
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  accel_tap_service_subscribe(handle_accel_tap);
  compass_service_subscribe(handle_compass_heading);
  compass_service_set_heading_filter(TRIG_MAX_ANGLE / 72); // 5 degree updates
}

static void deinit(void) {
  compass_service_unsubscribe();
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

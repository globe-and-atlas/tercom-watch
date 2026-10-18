#pragma once

#include <pebble.h>

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 200
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 228
#endif

#ifndef HORIZON_Y
#define HORIZON_Y 122
#endif

// Visual Palette (Emery 64-color)
#define COLOR_BG                GColorBlack
#define COLOR_TERRAIN_PRIMARY   GColorMalachite
#define COLOR_TERRAIN_SECONDARY GColorIslamicGreen
#define COLOR_HORIZON           GColorMintGreen
#define COLOR_RETICLE           GColorChromeYellow
#define COLOR_TIME_TEXT         GColorMintGreen
#define COLOR_TELEMETRY         GColorMintGreen
#define COLOR_ALERT             GColorSunsetOrange

// Animation & Performance
#define ACTIVE_ANIM_FPS         25
#define ACTIVE_ANIM_DURATION_MS 4500
#define NUM_DEPTH_ROWS          12
#define NUM_GRID_COLS           21

// Test Fixtures (Can be overridden via -D or env)
#ifndef TERCOM_TEST_MINUTE
#define TERCOM_TEST_MINUTE -1
#endif

#ifndef TERCOM_TEST_HEADING
#define TERCOM_TEST_HEADING -1
#endif

#ifndef TERCOM_TEST_STEPS
#define TERCOM_TEST_STEPS -1
#endif

#ifndef TERCOM_TEST_BATTERY
#define TERCOM_TEST_BATTERY -1
#endif

#ifndef TERCOM_TEST_ALT
#define TERCOM_TEST_ALT -1
#endif

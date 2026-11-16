from pathlib import Path
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ModelTests(unittest.TestCase):
    def test_terrain_safety_and_flight_envelope(self):
        out = ROOT / '.tmp'
        out.mkdir(exist_ok=True)
        source = out / 'terrain_test.c'
        source.write_text(r'''
#include <assert.h>
#include "terrain_model.h"
int main(void) {
  long signatures[5] = {0};
  for (int t = 0; t < 5; t++)
    for (int c = 0; c < 21; c++)
      for (int r = 0; r < 12; r++)
        for (int phase = 0; phase < 8; phase++) {
          int h = tercom_elevation(t, c, r, phase, 36);
          assert(h >= 0 && h <= 100);
          signatures[t] += h * (1 + c + r * 21 + phase * 252);
          int course = tercom_course_column(r, phase);
          assert(course >= 8 && course <= 12);
        }
  // Each flight step advances the canyon course one row, including the 7 -> 0 wrap.
  for (int r = 0; r < 11; r++)
    for (int phase = 0; phase < 8; phase++)
      assert(tercom_course_column(r, (phase + 1) % 8) == tercom_course_column(r + 1, phase));
  for (int a = 0; a < 5; a++)
    for (int b = a + 1; b < 5; b++) assert(signatures[a] != signatures[b]);
  assert(tercom_flight_frames(0, 25) == 0);
  assert(tercom_flight_frames(1, 25) == 50);
  assert(tercom_flight_frames(2, 25) == 112);
  for (int frame = 0; frame <= 112; frame++) {
    int gain = tercom_flight_gain(frame, 112);
    assert(gain >= 0 && gain <= 1024);
    assert(gain == tercom_flight_gain(112 - frame, 112));
  }
  assert(tercom_flight_gain(0, 112) == 0);
  assert(tercom_flight_gain(56, 112) == 1024);
  assert(tercom_flight_gain(112, 112) == 0);
  return 0;
}
''')
        binary = out / 'terrain_test'
        subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                        '-fsanitize=address,undefined', '-I', str(ROOT / 'watchface/src/c'),
                        str(source), str(ROOT / 'watchface/src/c/terrain_model.c'),
                        '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)

    def test_phone_setting_validation(self):
        subprocess.run(['node', '-e', r'''
const fs = require('fs'), vm = require('vm'), assert = require('assert');
const events = {};
const context = {module: {exports: {}}, require: () => function () { this.generateUrl = () => "data:clay"; },
  Pebble: {addEventListener: (name, fn) => events[name] = fn}, console};
vm.runInNewContext(fs.readFileSync('watchface/src/pkjs/index.js', 'utf8'), context);
const normalize = context.module.exports.normalized;
assert.equal(normalize({}).PALETTE, 0);
assert.equal(normalize({FLIGHT: '0'}).FLIGHT, 0);
assert.equal(normalize({DATE: '3'}).DATE, 3);
assert.equal(normalize({PALETTE: 99}).PALETTE, 0);
assert.equal(normalize({SENSITIVITY: -1}).SENSITIVITY, 2);
assert.equal(normalize({SWITCH_TRACKS: 0}).SWITCH_TRACKS, 0);
assert.equal(normalize({}).TERRAIN, 5);           // keep watch track by default
assert.equal(normalize({TERRAIN: '3'}).TERRAIN, 3);
assert.equal(normalize({TERRAIN: 6}).TERRAIN, 5);
assert.equal(normalize({}).COMPASS, 1);
assert.equal(normalize({COMPASS: '0'}).COMPASS, 0);
assert.equal(normalize({COMPASS: 2}).COMPASS, 1);
const terrain = context.module.exports.configuration.find(item => item.messageKey === 'TERRAIN');
assert.equal(terrain.defaultValue, '5');
assert.equal(terrain.options[0].value, '5');
// Opening settings resets a stale track choice so a save keeps the watch track.
const store = {'clay-settings': JSON.stringify({TERRAIN: '2', PALETTE: '1'})};
context.localStorage = {getItem: k => store[k] ?? null, setItem: (k, v) => store[k] = String(v)};
let opened = null;
context.Pebble.openURL = url => opened = url;
events.showConfiguration();
assert.equal(JSON.parse(store['clay-settings']).TERRAIN, '5');
assert.equal(JSON.parse(store['clay-settings']).PALETTE, '1');
assert.equal(events.ready, undefined); // startup cannot overwrite wrist-selected track
'''], cwd=ROOT, check=True)

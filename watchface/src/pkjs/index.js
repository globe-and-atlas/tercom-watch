var Clay = require('@rebble/clay');

function select(key, label, value, labels) {
  return {type: 'select', messageKey: key, label: label, defaultValue: String(value),
    options: labels.map(function (text, index) { return {label: text, value: String(index)}; })};
}

// Track is a one-shot choice: the form always reopens on "Keep current track" so
// saving other settings never undoes a track picked by double tap on the watch.
var KEEP_TRACK = '5';

var configuration = [
  {type: 'heading', defaultValue: 'TERCOM'},
  select('PALETTE', 'Instrument palette', 0, ['Green phosphor', 'Amber terminal', 'Ice blue', 'Monochrome']),
  {type: 'select', messageKey: 'TERRAIN', label: 'Track', defaultValue: KEEP_TRACK,
    options: [{label: 'Keep current track', value: KEEP_TRACK}].concat(
      ['Crags', 'Canyon', 'Dunes', 'Caldera', 'Alpine'].map(function (text, index) {
        return {label: text, value: String(index)};
      }))},
  select('FLIGHT', 'Tap flight', 2, ['Off', 'Short — 2 seconds', 'Standard — 4.5 seconds']),
  select('OPEN_FLIGHT', 'Opening flight when the face appears', 1, ['Off', 'On']),
  select('SENSITIVITY', 'Tilt response during flight', 2, ['Off', 'Gentle', 'Normal', 'Responsive']),
  select('SWITCH_TRACKS', 'Double tap changes track', 1, ['Off', 'On']),
  select('COMPASS', 'Compass', 1, ['Off', 'On']),
  select('TOP_FIELD', 'Upper right readout', 0, ['SIM — simulated altitude', 'Daily steps', 'Compass heading', 'Battery', 'Off']),
  {type: 'text', defaultValue: 'SIM is fictional cruise altitude derived from steps, not measured elevation. Heading shows --- when unavailable. Compass Off saves battery.'},
  select('DATE', 'Date in lower center', 0, ['Off', 'YYYY-MM-DD', 'MM/DD/YYYY', 'DD/MM/YYYY']),
  select('SHOW_STEPS', 'Lower center when date is off', 1, ['Empty', 'Daily steps']),
  select('COURSE', 'Dashed course toward terrain', 0, ['Off', 'On']),
  {type: 'submit', defaultValue: 'Save to watch'}
];

var clay = new Clay(configuration, null, {autoHandleEvents: false});

function normalized(saved) {
  var maxima = [3, 5, 2, 3, 1, 4, 3, 1, 1, 1, 1];
  var defaults = [0, 5, 2, 2, 1, 0, 0, 1, 0, 1, 1];
  var keys = ['PALETTE', 'TERRAIN', 'FLIGHT', 'SENSITIVITY', 'SWITCH_TRACKS', 'TOP_FIELD', 'DATE', 'SHOW_STEPS', 'COURSE', 'COMPASS', 'OPEN_FLIGHT'];
  var out = {};
  keys.forEach(function (key, i) {
    var value = Number(saved[key]);
    out[key] = isFinite(value) && Math.floor(value) === value && value >= 0 && value <= maxima[i] ? value : defaults[i];
  });
  return out;
}

function resetTrackChoice() {
  try {
    var stored = JSON.parse(localStorage.getItem('clay-settings') || '{}');
    stored.TERRAIN = KEEP_TRACK;
    localStorage.setItem('clay-settings', JSON.stringify(stored));
  } catch (error) { /* Unreadable storage: Clay falls back to the form defaults. */ }
}

Pebble.addEventListener('showConfiguration', function () {
  resetTrackChoice();
  Pebble.openURL(clay.generateUrl());
});
Pebble.addEventListener('webviewclosed', function (event) {
  if (!event || !event.response || event.response === 'CANCELLED') return;
  try {
    clay.getSettings(event.response);
    Pebble.sendAppMessage(normalized(JSON.parse(localStorage.getItem('clay-settings') || '{}')));
    resetTrackChoice();
  } catch (error) { /* Leave saved watch settings intact when the response is invalid. */ }
});

// Watch persistence is authoritative at startup, including tracks selected by wrist tap.
if (typeof module !== 'undefined') module.exports = {configuration: configuration, normalized: normalized, KEEP_TRACK: KEEP_TRACK};

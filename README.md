# TERCOM

A native watchface for Pebble Time 2, Pebble Round 2, and Pebble 2 with five procedural wireframe landscapes, an instrument clock, and brief wrist-controlled flight.

## Features

- Four palettes: Green phosphor (default), Amber terminal, Ice blue, Monochrome on color watches. Pebble 2 uses a simplified monochrome mesh.
- Bolder clock flanked by moving hour and minute tapes: yellow pointers level with the clock mark the current time, and the tapes slide every minute. Pebble 2 shows tape lines and pointers without numbers.
- Crags, winding Canyon, Dunes, Caldera, and layered Alpine terrains.
- Flight Off, Short (2 seconds), or Standard (about 4.5 seconds); four tilt responses including Off.
- Optional double-tap track switching; manual track selection in phone settings.
- Upper-right field: explicitly simulated altitude, steps, heading, battery, or empty.
- Compass: On (default) or Off.
- Opening flight: On (default) or Off for the short flight when the face appears. Pebble 2 has no compass and leaves the heading slot blank.
- Optional lower-center date (ISO, US, European), steps, or empty.
- Optional illustrative dashed course line. Landscapes and course are fictional, not navigation data.

Use the settings gear in the Pebble phone app. A single wrist tap triggers the selected flight duration; a rapid pair (150–650 ms apart) changes the track if enabled. Taps during a flight do not extend it, and taps within 3 seconds of a completed flight do not start another. After flight the exact starting terrain position returns. Watch persistence retains settings and the wrist-selected track; the phone does not overwrite them at startup. The settings track field always opens on "Keep current track"; pick a track there only to change it.

## Build and verify

```bash
python3 -m unittest discover -s tests -v
python3 execution/build.py
python3 execution/capture_release.py release/TERCOM-1.2.2.pbw .tmp/release-1.2.2
python3 execution/check_open_flight.py release/TERCOM-1.2.2.pbw .tmp/open-flight
```

Build output: `release/TERCOM-1.2.2.pbw`. The build wrapper cleans and removes fixture environment values. Native flight GIFs and screenshots are in `store/listing-1.2.2/`.

Version 1.2.2 is published at https://apps.repebble.com/tercom_c6f081e1607448bf92df7de4. Physical-watch readability, wrist feel, phone settings flow, and battery cost remain open device checks.
